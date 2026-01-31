#pragma once

#include "DSPComponents.h"
#include <array>
#include <vector>

/**
 * Procesador de vocoder con 20 bandas.
 * Analiza la señal moduladora y aplica su envolvente al carrier.
 */
class VocoderProcessor {
public:
  static constexpr int kNumBands = 20;

  VocoderProcessor(float sampleRate);

  void process(const float *input, const float *extCarrier, float *output,
               int numFrames);

  // Parámetros
  void setPitch(float pitch);
  void setIntensity(float intensity);
  void setWaveform(int type);
  void setVibrato(float amount);
  void setEcho(float amount);
  void setTremolo(float amount);
  void setNoiseThreshold(float threshold);

private:
  float mSampleRate;

  // Suavizadores de parámetros para evitar clics
  ParameterSmoother sIntensity;
  ParameterSmoother sNoiseThreshold;
  ParameterSmoother sEchoAmount;
  ParameterSmoother sVibratoAmount;
  ParameterSmoother sTremoloAmount;
  ParameterSmoother sBasePitch;

  // Oscilador carrier
  Oscillator mCarrier;

  // LFO para vibrato
  Oscillator mVibratoLFO;

  // LFO para tremolo
  Oscillator mTremoloLFO;

  // Filtro pasa-altos para el modulador (anti-rumble/acople)
  HighPassFilter mModHPF;

  // Bandas del vocoder
  struct Band {
    BandpassFilter modFilter;
    BandpassFilter carFilter;
    EnvelopeFollower envelope;
    float frequency;
  };
  std::array<Band, kNumBands> mBands;

  // Buffer de eco
  std::vector<float> mEchoBuffer;
  int mEchoIndex = 0;
  static constexpr int kEchoSamples = 14400; // 300ms @ 48kHz

  // Frecuencias de las bandas (mismas que Aethereum)
  // Frecuencias de las bandas OPTIMIZADAS para voz (mayor densidad en 1k-5k)
  static constexpr std::array<float, kNumBands> kBandFrequencies = {
      100,  160,  240,  350,  480,  640,  840,  1100, 1400,  1750,
      2150, 2600, 3100, 3700, 4400, 5300, 6500, 8000, 10500, 14000};

  void initBands();
};
