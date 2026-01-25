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
  mCarrierTempBuffer.resize(kFramesPerBuffer, 0.0f);
  mRecordedData.resize(kMaxRecordSamples, 0.0f);
  LOGI("VocoderEngine created with pre-allocated buffers");
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

  // Asegurar que el buffer de entrada tenga tamaño suficiente (solo si cambia
  // drásticamente)
  if (mInputBuffer.size() < static_cast<size_t>(numFrames)) {
    mInputBuffer.resize(numFrames, 0.0f);
    mCarrierTempBuffer.resize(numFrames, 0.0f);
  }

  bool hasExtCarrier = false;
  bool gotInput = false;

  // Siempre intentamos leer el micro si el stream está vivo
  auto inputState =
      mInputStream ? mInputStream->getState() : oboe::StreamState::Unknown;

  if (mInputStream && (inputState == oboe::StreamState::Started ||
                       inputState == oboe::StreamState::Starting)) {
    // Leemos directamente al buffer de entrada para ahorrar una copia si es la
    // fuente. Usamos mOutputBuffer temporalmente como buffer de lectura del
    // micro si no es la fuente activa para evitar alocación.
    float *micTarget =
        (mSource.load() == 0) ? mInputBuffer.data() : mOutputBuffer.data();
    auto result = mInputStream->read(micTarget, numFrames, 0);

    if (result == oboe::Result::OK && result.value() > 0) {
      int32_t readFrames = result.value();

      // Si estamos grabando, guardar la señal del micro de forma segura
      if (mIsRecording.load()) {
        int32_t idx = mRecordIndex.load();
        int32_t remaining = kMaxRecordSamples - idx;
        int32_t toCopy = std::min(readFrames, remaining);
        if (toCopy > 0) {
          std::copy(micTarget, micTarget + toCopy, mRecordedData.begin() + idx);
          mRecordIndex.store(idx + toCopy);
        } else {
          mIsRecording.store(false); // Buffer lleno
        }
      }

      if (mSource.load() == 0 && mIsMicActive.load()) {
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
      mCarrierTempBuffer[i] = mCarrierFileBuffer[readIdx];
      readIdx = (readIdx + 1) % (int32_t)mCarrierFileBuffer.size();
    }
    mCarrierReadIndex.store(readIdx);
    hasExtCarrier = true;
  }

  // Calcular VU Level (RMS con balística y escalado)
  float sumSq = 0.0f;
  // Usamos el buffer que contenga la señal de entrada real (mic o archivo)
  // Si estamos grabando o en modo mic, mostramos el nivel del micro
  const float *vuSource =
      (mSource.load() == 0 || mIsRecording.load())
          ? ((mSource.load() == 0) ? mInputBuffer.data() : mOutputBuffer.data())
          : mInputBuffer.data();

  for (int i = 0; i < numFrames; i++) {
    float val = vuSource[i];
    sumSq += val * val;
  }
  float rms = std::sqrt(sumSq / (float)numFrames);

  if (!gotInput) {
    std::fill(mInputBuffer.begin(), mInputBuffer.begin() + numFrames, 0.0f);
  }

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
                      hasExtCarrier ? mCarrierTempBuffer.data() : nullptr,
                      outputData, numFrames);

  // Copiar datos para visualización
  int displaySamples = std::min(numFrames, 256);
  std::copy(outputData, outputData + displaySamples, mWaveformBuffer.begin());

  return oboe::DataCallbackResult::Continue;
}

void VocoderEngine::startRecording() {
  mRecordIndex.store(0);
  mIsRecording.store(true);
  LOGI("Internal recording started (max %d s)", kMaxRecordSeconds);
}

void VocoderEngine::stopRecording() {
  mIsRecording.store(false);
  int32_t totalSamples = mRecordIndex.load();

  if (totalSamples > 0) {
    // Normalización automática de la grabación antes de cargarla
    float maxAbs = 0.0f;
    for (int i = 0; i < totalSamples; i++) {
      maxAbs = std::max(maxAbs, std::abs(mRecordedData[i]));
    }

    if (maxAbs > 0.0f) {
      float factor = 0.9f / maxAbs;
      for (int i = 0; i < totalSamples; i++)
        mRecordedData[i] *= factor;
    }

    // Copiar la parte grabada al buffer del modulador
    mModulatorFileBuffer.assign(mRecordedData.begin(),
                                mRecordedData.begin() + totalSamples);
    mFileReadIndex.store(0);
    LOGI("Internal recording stopped. Captured %d samples", totalSamples);
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
