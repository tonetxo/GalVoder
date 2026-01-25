#pragma once

#include "DSPComponents.h"
#include "VocoderProcessor.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <oboe/Oboe.h>
#include <vector>

/**
 * Motor de audio principal basado en Oboe.
 * Gestiona el ciclo de vida de los streams y el callback de procesamiento.
 */
class VocoderEngine : public oboe::AudioStreamDataCallback {
public:
  VocoderEngine();
  ~VocoderEngine();

  bool start();
  void stop();

  // Implementación de oboe::AudioStreamDataCallback
  oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream,
                                        void *audioData,
                                        int32_t numFrames) override;

  // Parámetros
  void setPitch(float pitch);
  void setIntensity(float intensity);
  void setWaveform(int type);
  void setVibrato(float amount);
  void setEcho(float amount);
  void setTremolo(float amount);
  void setNoiseThreshold(float threshold);

  // Soporte de archivo / Modulador Interno
  void setMicActive(bool active);
  void setModulatorBuffer(const float *data, int32_t numSamples);
  void setSource(int source); // 0 = Mic, 1 = File
  void setFilePlaying(bool playing);
  void resetFileIndex();
  void setCarrierBuffer(const float *data, int32_t numSamples);

  // Grabación Interna
  void startRecording();
  void stopRecording();
  bool isRecording() const { return mIsRecording; }

  // Getters
  float getVULevel() const;
  std::vector<float> getWaveformData() const;

private:
  void createStreams();
  void closeStreams();

  std::shared_ptr<oboe::AudioStream> mInputStream;
  std::shared_ptr<oboe::AudioStream> mOutputStream;
  std::unique_ptr<VocoderProcessor> mProcessor;

  std::vector<float> mInputBuffer;
  std::vector<float> mOutputBuffer;
  std::vector<float> mWaveformBuffer;
  std::vector<float> mCarrierTempBuffer; // Pre-alloc

  // Buffer para archivo / Modulador grabado
  std::vector<float> mModulatorFileBuffer;
  std::atomic<int32_t> mFileReadIndex{0};

  // Buffer para Carrier externo
  std::vector<float> mCarrierFileBuffer;
  std::atomic<int32_t> mCarrierReadIndex{0};

  std::atomic<int> mSource{0}; // 0 = Mic, 1 = File
  std::atomic<int> mWaveformType{0};
  std::atomic<bool> mIsFilePlaying{false};
  std::atomic<bool> mIsMicActive{false};

  // Estado de grabación interna
  std::atomic<bool> mIsRecording{false};
  std::vector<float> mRecordedData;
  std::atomic<int32_t> mRecordIndex{0};
  std::mutex mRecordingMutex;

  std::atomic<float> mVULevel{0.0f};
  bool mIsRunning = false;

  static constexpr int kSampleRate = 48000;
  static constexpr int kChannelCount = 1;
  static constexpr int kFramesPerBuffer = 256;
  static constexpr int kMaxRecordSeconds = 10;
  static constexpr int kMaxRecordSamples = kSampleRate * kMaxRecordSeconds;
};
