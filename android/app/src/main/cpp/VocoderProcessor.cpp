#include "VocoderProcessor.h"
#include <algorithm>
#include <cmath>

constexpr std::array<float, VocoderProcessor::kNumBands>
    VocoderProcessor::kBandFrequencies;

VocoderProcessor::VocoderProcessor(float sampleRate)
    : mSampleRate(sampleRate), mCarrier(sampleRate), mVibratoLFO(sampleRate),
      mTremoloLFO(sampleRate) {

  // Configurar constantes de tiempo para los suavizadores (~30ms)
  float tc = 30.0f;
  sIntensity.setTimeConstant(tc, sampleRate);
  sNoiseThreshold.setTimeConstant(tc, sampleRate);
  sEchoAmount.setTimeConstant(tc, sampleRate);
  sVibratoAmount.setTimeConstant(tc, sampleRate);
  sTremoloAmount.setTimeConstant(tc, sampleRate);
  sBasePitch.setTimeConstant(tc, sampleRate);

  // Valores iniciales
  sNoiseThreshold.setTarget(0.012f);
  sIntensity.setTarget(1.2f);
  sBasePitch.setTarget(140.0f);

  mEchoBuffer.resize(kEchoSamples, 0.0f);

  // Vibrato LFO: 5Hz Sine
  mVibratoLFO.setFrequency(5.0f);
  mVibratoLFO.setWaveform(Oscillator::Waveform::Sine);

  // Tremolo LFO: 6Hz Sine
  mTremoloLFO.setFrequency(6.0f);
  mTremoloLFO.setWaveform(Oscillator::Waveform::Sine);

  initBands();
}

void VocoderProcessor::initBands() {
  constexpr float Q = 25.0f;
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
    // Obtener valores suavizados por cada frame
    float currentPitch = sBasePitch.process();
    float currentVibrato = sVibratoAmount.process();
    float currentIntensity = sIntensity.process();
    float currentEcho = sEchoAmount.process();
    float currentTremolo = sTremoloAmount.process();
    float currentThreshold = sNoiseThreshold.process();

    // Aplicar vibrato al pitch
    float vibratoMod = mVibratoLFO.process() * currentVibrato * 20.0f;
    mCarrier.setFrequency(currentPitch + vibratoMod);

    // Generar carrier
    float carrierSample = mCarrier.process();

    // Modulador: Preamplificación
    float modSample = input[frame] * 12.0f;

    // Vocoding: Bandas
    float outputSample = 0.0f;
    for (auto &band : mBands) {
      float filteredMod = band.modFilter.process(modSample);
      float envelope = band.envelope.process(filteredMod);

      // Puerta suave (Soft Gate) para evitar clics al entrar/salir de la banda
      if (envelope > currentThreshold) {
        float filteredCar = band.carFilter.process(carrierSample);
        // La resta (envelope - threshold) actúa como un fade-in natural
        outputSample +=
            filteredCar * (envelope - currentThreshold) * currentIntensity;
      }
    }

    // Normalización base
    outputSample *= 0.5f;

    // Aplicar tremolo (modulación de amplitud post-vocoder)
    if (currentTremolo > 0.001f) {
      float tremoloMod =
          1.0f - (mTremoloLFO.process() * 0.5f + 0.5f) * currentTremolo;
      outputSample *= tremoloMod;
    }

    // Aplicar eco
    if (currentEcho > 0.001f) {
      float delayed = mEchoBuffer[mEchoIndex];
      outputSample += delayed * currentEcho;
      mEchoBuffer[mEchoIndex] = outputSample;
      mEchoIndex = (mEchoIndex + 1) % kEchoSamples;
    }

    // Hard limiter
    output[frame] = std::clamp(outputSample, -1.0f, 1.0f);
  }
}

void VocoderProcessor::setPitch(float pitch) {
  sBasePitch.setTarget(std::clamp(pitch, 50.0f, 400.0f));
}

void VocoderProcessor::setIntensity(float intensity) {
  sIntensity.setTarget(
      std::clamp(intensity, 0.2f, 3.0f)); // Aumentado ligeramente el rango
}

void VocoderProcessor::setWaveform(int type) {
  mCarrier.setWaveform(static_cast<Oscillator::Waveform>(type));
}

void VocoderProcessor::setVibrato(float amount) {
  sVibratoAmount.setTarget(std::clamp(amount, 0.0f, 1.0f));
}

void VocoderProcessor::setEcho(float amount) {
  sEchoAmount.setTarget(std::clamp(amount, 0.0f, 0.7f));
}

void VocoderProcessor::setTremolo(float amount) {
  sTremoloAmount.setTarget(std::clamp(amount, 0.0f, 1.0f));
}

void VocoderProcessor::setNoiseThreshold(float threshold) {
  sNoiseThreshold.setTarget(std::clamp(threshold, 0.005f, 0.2f));
}
