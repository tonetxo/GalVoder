#pragma once

#include <oboe/Oboe.h>
#include <memory>
#include <vector>
#include <atomic>
#include "VocoderProcessor.h"

/**
 * Motor principal del vocoder que gestiona el audio con Oboe.
 * Implementa AudioStreamDataCallback para procesamiento en tiempo real.
 */
class VocoderEngine : public oboe::AudioStreamDataCallback {
public:
    VocoderEngine();
    ~VocoderEngine();

    // Control del motor
    bool start();
    void stop();
    
    // Parámetros del vocoder
    void setPitch(float pitch);
    void setIntensity(float intensity);
    void setWaveform(int type);
    void setVibrato(float amount);
    void setEcho(float amount);
    void setNoiseThreshold(float threshold);
    
    // Datos para UI
    float getVULevel() const;
    std::vector<float> getWaveformData() const;
    
    // Callback de Oboe
    oboe::DataCallbackResult onAudioReady(
        oboe::AudioStream* stream,
        void* audioData,
        int32_t numFrames) override;

private:
    // Streams de audio
    std::shared_ptr<oboe::AudioStream> mInputStream;
    std::shared_ptr<oboe::AudioStream> mOutputStream;
    
    // Procesador del vocoder
    std::unique_ptr<VocoderProcessor> mProcessor;
    
    // Estado
    std::atomic<bool> mIsRunning{false};
    std::atomic<float> mVULevel{0.0f};
    
    // Buffers
    std::vector<float> mInputBuffer;
    std::vector<float> mOutputBuffer;
    std::vector<float> mWaveformBuffer;
    
    // Configuración
    static constexpr int32_t kSampleRate = 48000;
    static constexpr int32_t kChannelCount = 1;
    static constexpr int32_t kFramesPerBuffer = 256;
    
    void createStreams();
    void closeStreams();
};
