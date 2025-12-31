#include "VocoderProcessor.h"
#include <algorithm>
#include <cmath>

constexpr std::array<float, VocoderProcessor::kNumBands>
    VocoderProcessor::kBandFrequencies;

VocoderProcessor::VocoderProcessor(float sampleRate)
    : mSampleRate(sampleRate), mCarrier(sampleRate), mVibratoLFO(sampleRate) {
  mNoiseThreshold = 0.002f; // Aún más bajo
  mIntensity = 5.0f;        // Mucho más intensidad de salida
  mEchoBuffer.resize(kEchoSamples, 0.0f);
  mVibratoLFO.setFrequency(5.0f);
  mVibratoLFO.setWaveform(Oscillator::Waveform::Sine);
  initBands();
}

void VocoderProcessor::initBands() {
  constexpr float Q = 8.0f; // Un poco más ancho para dejar pasar más energía

  for (int i = 0; i < kNumBands; i++) {
    mBands[i].frequency = kBandFrequencies[i];
    mBands[i].modFilter.setCoefficients(kBandFrequencies[i], Q, mSampleRate);
    mBands[i].carFilter.setCoefficients(kBandFrequencies[i], Q, mSampleRate);
    mBands[i].envelope = EnvelopeFollower(mSampleRate);
  }
}

void VocoderProcessor::process(const float *input, float *output,
                               int numFrames) {
  for (int frame = 0; frame < numFrames; frame++) {
    // Aplicar vibrato
    float vibratoMod = mVibratoLFO.process() * mVibratoAmount * 20.0f;
    mCarrier.setFrequency(mBasePitch + vibratoMod);

    // Generar carrier
    float carrierSample = mCarrier.process();

    // Señal de entrada (modulador) - Aplicar pre-amplificación fuerte
    float modSample = input[frame] * 20.0f;

    // Calcular nivel global del modulador para noise gate
    float modLevel = std::abs(modSample);

    // Procesar cada banda
    float outputSample = 0.0f;

    if (modLevel > mNoiseThreshold) {
      for (auto &band : mBands) {
        // Filtrar modulador y extraer envolvente
        float filteredMod = band.modFilter.process(modSample);
        float envelope = band.envelope.process(filteredMod);

        // Filtrar carrier
        float filteredCar = band.carFilter.process(carrierSample);

        // Aplicar envolvente al carrier
        outputSample += filteredCar * envelope * mIntensity;
      }
    }

    // Normalizar por número de bandas
    outputSample /= static_cast<float>(kNumBands) * 0.5f;

    // Aplicar eco
    if (mEchoAmount > 0.001f) {
      float delayed = mEchoBuffer[mEchoIndex];
      outputSample += delayed * mEchoAmount;
      mEchoBuffer[mEchoIndex] = outputSample;
      mEchoIndex = (mEchoIndex + 1) % kEchoSamples;
    }

    // Limitar salida
    output[frame] = std::clamp(outputSample, -1.0f, 1.0f);
  }
}

void VocoderProcessor::setPitch(float pitch) {
  mBasePitch = std::clamp(pitch, 50.0f, 400.0f);
}

void VocoderProcessor::setIntensity(float intensity) {
  mIntensity = std::clamp(intensity, 0.2f, 2.0f);
}

void VocoderProcessor::setWaveform(int type) {
  mCarrier.setWaveform(static_cast<Oscillator::Waveform>(type));
}

void VocoderProcessor::setVibrato(float amount) {
  mVibratoAmount = std::clamp(amount, 0.0f, 1.0f);
}

void VocoderProcessor::setEcho(float amount) {
  mEchoAmount = std::clamp(amount, 0.0f, 0.7f);
}

void VocoderProcessor::setNoiseThreshold(float threshold) {
  mNoiseThreshold = std::clamp(threshold, 0.01f, 0.2f);
}
