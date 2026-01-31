#include "VocoderProcessor.h"
#include <algorithm>
#include <cmath>

// Constantes de procesamiento con nombres descriptivos
static constexpr float kModulatorPreamp =
    10.0f; // Restaurado a 10.0 para equilibrio cuerpo/definición
static constexpr float kThresholdHysteresis =
    0.4f; // Ajustado (era 0.6) para reducir salto de volumen (clic)
static constexpr float kOutputNormalization = 0.55f; // Normalización standard
static constexpr float kVibratoDepthHz = 20.0f; // Profundidad del vibrato en Hz

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
  sNoiseThreshold.setTarget(0.003f); // Umbral bajo para evitar cortes
  sIntensity.setTarget(0.8f);        // Ganancia standard
  sBasePitch.setTarget(140.0f);

  mEchoBuffer.resize(kEchoSamples, 0.0f);

  // Vibrato LFO: 5Hz Sine
  mVibratoLFO.setFrequency(5.0f);
  mVibratoLFO.setWaveform(Oscillator::Waveform::Sine);

  // Tremolo LFO: 6Hz Sine
  mTremoloLFO.setFrequency(6.0f);
  mTremoloLFO.setWaveform(Oscillator::Waveform::Sine);

  // Filtro anti-acople (200Hz HPF - equilibrado)
  mModHPF.setCoefficients(200.0f, 0.707f, sampleRate);

  initBands();
}

void VocoderProcessor::initBands() {
  constexpr float Q =
      12.0f; // Restaurado a 12.0 para buena separación y definición
  for (int i = 0; i < kNumBands; i++) {
    mBands[i].frequency = kBandFrequencies[i];
    mBands[i].modFilter.setCoefficients(kBandFrequencies[i], Q, mSampleRate);
    mBands[i].carFilter.setCoefficients(kBandFrequencies[i], Q, mSampleRate);
    mBands[i].envelope = EnvelopeFollower(mSampleRate);
  }
}

void VocoderProcessor::process(const float *input, const float *extCarrier,
                               float *output, int numFrames) {
  for (int frame = 0; frame < numFrames; frame++) {
    // Obtener valores suavizados por cada frame
    float currentPitch = sBasePitch.process();
    float currentVibrato = sVibratoAmount.process();
    float currentIntensity = sIntensity.process();
    float currentEcho = sEchoAmount.process();
    float currentTremolo = sTremoloAmount.process();
    float currentThreshold = sNoiseThreshold.process();

    // Aplicar vibrato al pitch
    float vibratoMod = mVibratoLFO.process() * currentVibrato * kVibratoDepthHz;
    mCarrier.setFrequency(currentPitch + vibratoMod);

    // Generar carrier (usar externo se existe, senón usar oscilador)
    float carrierSample =
        (extCarrier != nullptr) ? extCarrier[frame] : mCarrier.process();

    // Modulador: Preamplificación
    float modSample = input[frame] * kModulatorPreamp;

    // Aplicar HPF para quitar retumbo de graves que causa acople
    modSample = mModHPF.process(modSample);

    float outputSample = 0.0f;
    // Procesar cada banda
    for (int i = 0; i < kNumBands; i++) {
      auto &band = mBands[i];

      // Modulador: Filtro e seguidor de envolvente
      float modFiltered = band.modFilter.process(modSample);
      float envelope = band.envelope.process(modFiltered);

      // Noise Gate: Solo procesar se supera o umbral
      // Uso de histéresis para evitar flutuacións rápidas
      if (envelope > currentThreshold) {
        float filteredCar = band.carFilter.process(carrierSample);
        // Boost de envolvente con histéresis suave pro-rata
        float boost = (envelope - currentThreshold * kThresholdHysteresis);
        outputSample += filteredCar * boost * currentIntensity;
      }
    }

    // Normalización base de salida
    outputSample *= kOutputNormalization;

    // Aplicar tremolo (modulación de amplitud post-vocoder)
    if (currentTremolo > 0.001f) {
      float tremoloMod =
          1.0f - (mTremoloLFO.process() * 0.5f + 0.5f) * currentTremolo;
      outputSample *= tremoloMod;
    }

    // Aplicar eco con fade-out gradual do buffer para evitar artefactos
    float delayed = mEchoBuffer[mEchoIndex];
    if (currentEcho > 0.001f) {
      // Eco activo: comportamento normal
      outputSample += delayed * currentEcho;
      mEchoBuffer[mEchoIndex] = outputSample;
    } else {
      // Eco inactivo: fade-out gradual do buffer (decay factor 0.95)
      // Isto evita que datos antigos causen clics ao reactivar o eco
      mEchoBuffer[mEchoIndex] = delayed * 0.95f;
    }
    mEchoIndex = (mEchoIndex + 1) % kEchoSamples;

    // Soft-clipper con tanh para saturación musical (evita distorsión dura)
    outputSample = std::tanh(outputSample);

    output[frame] = outputSample;
  }
}

void VocoderProcessor::setPitch(float pitch) {
  sBasePitch.setTarget(std::clamp(pitch, 50.0f, 400.0f));
}

void VocoderProcessor::setIntensity(float intensity) {
  sIntensity.setTarget(
      std::clamp(intensity, 0.2f, 4.0f)); // Aumentado o teito de 3.0 a 4.0
}

void VocoderProcessor::setWaveform(int type) {
  if (type >= 0 && type <= 3) {
    mCarrier.setWaveform(static_cast<Oscillator::Waveform>(type));
  }
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
