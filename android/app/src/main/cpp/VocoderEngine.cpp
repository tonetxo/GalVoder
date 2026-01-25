#include "VocoderEngine.h"
#include <algorithm>
#include <android/log.h>

#define LOG_TAG "VocoderEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

VocoderEngine::VocoderEngine() {
  mProcessor = std::make_unique<VocoderProcessor>(kSampleRate);
  mInputBuffer.resize(kFramesPerBuffer, 0.0f);
  mOutputBuffer.resize(kFramesPerBuffer, 0.0f);
  mWaveformBuffer.resize(256, 0.0f);
  // Pre-alocar buffers de traballo para evitar asignacións no callback
  mCarrierWorkBuffer.resize(kFramesPerBuffer, 0.0f);
  mMicWorkBuffer.resize(kFramesPerBuffer, 0.0f);
  mRecordedData.reserve(kSampleRate * 10); // Reservar para 10 segundos
  LOGI("VocoderEngine created");
}

VocoderEngine::~VocoderEngine() {
  stop();
  LOGI("VocoderEngine destroyed");
}

bool VocoderEngine::start() {
  if (mIsRunning)
    return true;

  createStreams();

  if (mInputStream && mOutputStream) {
    auto res1 = mInputStream->requestStart();
    auto res2 = mOutputStream->requestStart();

    if (res1 == oboe::Result::OK && res2 == oboe::Result::OK) {
      mIsRunning = true;
      LOGI("VocoderEngine started successfully");
      return true;
    } else {
      LOGE("Failed to requestStart: input=%d, output=%d", (int)res1, (int)res2);
    }
  }

  LOGE("Failed to start VocoderEngine - streams null or open failed");
  return false;
}

void VocoderEngine::stop() {
  if (!mIsRunning)
    return;

  mIsRunning = false;
  closeStreams();
  LOGI("VocoderEngine stopped");
}

void VocoderEngine::createStreams() {
  // Crear stream de entrada (micrófono)
  oboe::AudioStreamBuilder inputBuilder;
  inputBuilder.setDirection(oboe::Direction::Input)
      ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
      ->setSharingMode(oboe::SharingMode::Exclusive)
      // Activar procesamento de audio do sistema para anti-acople
      // VoiceCommunication activa AEC, NS e AGC automaticamente
      ->setInputPreset(oboe::InputPreset::VoiceCommunication)
      ->setSampleRate(kSampleRate)
      ->setChannelCount(kChannelCount)
      ->setFormat(oboe::AudioFormat::Float)
      ->setFramesPerDataCallback(kFramesPerBuffer);

  auto inputResult = inputBuilder.openStream(mInputStream);
  if (inputResult != oboe::Result::OK) {
    LOGE("Failed to open input stream: %s", oboe::convertToText(inputResult));
  }

  // Crear stream de salida
  oboe::AudioStreamBuilder outputBuilder;
  outputBuilder.setDirection(oboe::Direction::Output)
      ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
      ->setSharingMode(oboe::SharingMode::Exclusive)
      ->setSampleRate(kSampleRate)
      ->setChannelCount(kChannelCount)
      ->setFormat(oboe::AudioFormat::Float)
      ->setFramesPerDataCallback(kFramesPerBuffer)
      ->setDataCallback(this);

  auto outputResult = outputBuilder.openStream(mOutputStream);
  if (outputResult != oboe::Result::OK) {
    LOGE("Failed to open output stream: %s", oboe::convertToText(outputResult));
  }
}

void VocoderEngine::closeStreams() {
  if (mInputStream) {
    mInputStream->stop();
    mInputStream->close();
    mInputStream.reset();
  }
  if (mOutputStream) {
    mOutputStream->stop();
    mOutputStream->close();
    mOutputStream.reset();
  }
}

oboe::DataCallbackResult VocoderEngine::onAudioReady(oboe::AudioStream *stream,
                                                     void *audioData,
                                                     int32_t numFrames) {

  auto *outputData = static_cast<float *>(audioData);

  // Asegurar que os buffers teñan tamaño suficiente (só cambia en inicio)
  if (mInputBuffer.size() < static_cast<size_t>(numFrames)) {
    mInputBuffer.resize(numFrames, 0.0f);
    mCarrierWorkBuffer.resize(numFrames, 0.0f);
    mMicWorkBuffer.resize(numFrames, 0.0f);
  }

  // Usar buffers pre-alocados en lugar de crear novos vectores
  std::fill(mCarrierWorkBuffer.begin(), mCarrierWorkBuffer.begin() + numFrames,
            0.0f);
  std::fill(mMicWorkBuffer.begin(), mMicWorkBuffer.begin() + numFrames, 0.0f);
  bool hasExtCarrier = false;

  bool gotInput = false;

  // Siempre intentamos leer el micro si el stream está vivo, incluso si mSource
  // es File, para permitir la grabación de fondo o el VU meter global.
  auto inputState =
      mInputStream ? mInputStream->getState() : oboe::StreamState::Unknown;

  if (mInputStream && (inputState == oboe::StreamState::Started ||
                       inputState == oboe::StreamState::Starting)) {
    auto result = mInputStream->read(mMicWorkBuffer.data(), numFrames, 0);
    if (result.value() > 0) {
      // Si estamos grabando, guardar la señal del micro
      if (mIsRecording) {
        std::lock_guard<std::mutex> lock(mRecordingMutex);
        mRecordedData.insert(mRecordedData.end(), mMicWorkBuffer.begin(),
                             mMicWorkBuffer.begin() + result.value());
      }

      if (mSource.load() == 0 && mIsMicActive.load()) { // SOURCE_MIC y activo
        std::copy(mMicWorkBuffer.begin(), mMicWorkBuffer.end(),
                  mInputBuffer.begin());
        gotInput = true;
      }
    }
  }

  if (mSource.load() == 1) { // SOURCE_FILE
    if (!mModulatorFileBuffer.empty() && mIsFilePlaying.load()) {
      int32_t readIdx = mFileReadIndex.load();
      for (int i = 0; i < numFrames; i++) {
        mInputBuffer[i] = mModulatorFileBuffer[readIdx];
        readIdx = (readIdx + 1) % (int32_t)mModulatorFileBuffer.size();
      }
      mFileReadIndex.store(readIdx);
      gotInput = true;
    }
  }

  // Comprobar se temos carrier externo (tipo 4)
  if (mWaveformType.load() == 4 && !mCarrierFileBuffer.empty()) {
    int32_t readIdx = mCarrierReadIndex.load();
    for (int i = 0; i < numFrames; i++) {
      mCarrierWorkBuffer[i] = mCarrierFileBuffer[readIdx];
      readIdx = (readIdx + 1) % (int32_t)mCarrierFileBuffer.size();
    }
    mCarrierReadIndex.store(readIdx);
    hasExtCarrier = true;
  }

  if (!gotInput) {
    std::fill(mInputBuffer.begin(), mInputBuffer.begin() + numFrames, 0.0f);
  }

  // Calcular VU Level (RMS con balística y escalado)
  // Usar mInputBuffer si hay entrada activa, o currentMicData si estamos
  // grabando
  float sumSq = 0.0f;
  const float *vuSource =
      gotInput ? mInputBuffer.data() : mMicWorkBuffer.data();
  for (int i = 0; i < numFrames; i++) {
    float val = vuSource[i];
    sumSq += val * val;
  }
  float rms = std::sqrt(sumSq / (float)numFrames);

  // Escalado no lineal y boost para que la voz sea más visible (similar a curva
  // de VU real)
  float targetVU = std::pow(rms * 1.8f, 0.6f);

  // Ballistics: Ataque rápido, liberación más lenta
  float currentVU = mVULevel.load();
  float factor = (targetVU > currentVU) ? 0.25f : 0.08f;
  float nextVU = currentVU + (targetVU - currentVU) * factor;

  mVULevel = std::clamp(nextVU, 0.0f, 1.2f);

  // Procesar vocoder
  mProcessor->process(mInputBuffer.data(),
                      hasExtCarrier ? mCarrierWorkBuffer.data() : nullptr,
                      outputData, numFrames);

  // Copiar datos para visualización
  int displaySamples = std::min(numFrames, 256);
  std::copy(outputData, outputData + displaySamples, mWaveformBuffer.begin());

  return oboe::DataCallbackResult::Continue;
}

void VocoderEngine::startRecording() {
  std::lock_guard<std::mutex> lock(mRecordingMutex);
  mRecordedData.clear();
  mIsRecording = true;
  LOGI("Internal recording started");
}

void VocoderEngine::stopRecording() {
  mIsRecording = false;
  std::lock_guard<std::mutex> lock(mRecordingMutex);

  if (!mRecordedData.empty()) {
    // Normalización automática de la grabación antes de cargarla
    float maxAbs = 0.0f;
    for (float v : mRecordedData) {
      maxAbs = std::max(maxAbs, std::abs(v));
    }

    if (maxAbs > 0.0f) {
      float factor = 0.9f / maxAbs;
      for (float &v : mRecordedData)
        v *= factor;
    }

    mModulatorFileBuffer = mRecordedData;
    mFileReadIndex = 0;
    LOGI("Internal recording stopped. Captured %d samples",
         (int)mModulatorFileBuffer.size());
  }
}

void VocoderEngine::setModulatorBuffer(const float *data, int32_t numSamples) {
  mModulatorFileBuffer.assign(data, data + numSamples);
  mFileReadIndex.store(0);
  LOGI("Loaded %d samples into modulator buffer", numSamples);
}

void VocoderEngine::setSource(int source) {
  mSource.store(source);
  LOGI("Switching source to: %s", source == 0 ? "Microphone" : "File");
}

void VocoderEngine::setFilePlaying(bool playing) {
  mIsFilePlaying.store(playing);
}

void VocoderEngine::setMicActive(bool active) {
  mIsMicActive.store(active);
  LOGI("Mic active: %s", active ? "true" : "false");
}

void VocoderEngine::resetFileIndex() { mFileReadIndex.store(0); }

// Setters
void VocoderEngine::setPitch(float pitch) { mProcessor->setPitch(pitch); }

void VocoderEngine::setIntensity(float intensity) {
  mProcessor->setIntensity(intensity);
}

void VocoderEngine::setWaveform(int type) {
  mWaveformType.store(type);
  if (type < 4) {
    mProcessor->setWaveform(type);
  }
}

void VocoderEngine::setCarrierBuffer(const float *data, int32_t numSamples) {
  mCarrierFileBuffer.assign(data, data + numSamples);
  mCarrierReadIndex.store(0);
  LOGI("External carrier loaded: %d samples", numSamples);
}

void VocoderEngine::setVibrato(float amount) { mProcessor->setVibrato(amount); }

void VocoderEngine::setEcho(float amount) { mProcessor->setEcho(amount); }

void VocoderEngine::setTremolo(float amount) { mProcessor->setTremolo(amount); }

void VocoderEngine::setNoiseThreshold(float threshold) {
  mProcessor->setNoiseThreshold(threshold);
}

// Getters
float VocoderEngine::getVULevel() const { return mVULevel.load(); }

std::vector<float> VocoderEngine::getWaveformData() const {
  return mWaveformBuffer;
}
