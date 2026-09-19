# GalVoder 🎙️⚙️

<p align="center">
  <strong>Vintage Aesthetic Real-Time Voice Vocoder & Synthesizer</strong><br>
  <em>Sintetizador de voz tipo vocoder vintage con procesamento en tempo real para Android e Web</em>
</p>

<p align="center">
  <a href="https://play.google.com/store/apps/details?id=com.tonetxo.vocodergal">
    <img src="https://play.google.com/intl/en_us/badges/static/images/badges/en_badge_web_generic.png" alt="Get it on Google Play" height="80">
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-Android%20%7C%20Web-green" alt="Platform">
  <img src="https://img.shields.io/badge/DSP%20Engine-C%2B%2B%20Native-blue" alt="C++ Native DSP">
  <img src="https://img.shields.io/badge/Android-Kotlin-orange" alt="Kotlin">
  <img src="https://img.shields.io/badge/Web%20Audio-Web%20Audio%20API-9cf" alt="Web Audio API">
  <img src="https://img.shields.io/badge/Styling-Tailwind%20CSS-38bdf8" alt="Tailwind CSS">
  <img src="https://img.shields.io/badge/License-MIT-yellow" alt="License">
</p>

---

## 🌟 English Overview

**GalVoder** is a retro-futuristic voice vocoder and spectral synthesizer designed for experimental sound designers, musicians, and vocal performers. Blending a meticulously crafted vintage analog machinery aesthetic with high-performance digital signal processing (DSP), GalVoder transforms human voice and live audio inputs into robotic, harmonic, and otherworldly acoustic textures.

### Key Features
- **Ultra-Low Latency DSP**: High-performance native **C++** vocoder filterbank algorithm optimized for Android devices and modern web browsers.
- **Vintage MK-XII Aesthetic**: Analog-inspired control deck featuring physical VU needles, warm vacuum-tube style indicators, rotary knobs, and tactile switches.
- **Dual-Axis XY Touch Pad**: Expressive multidimensional modulation controlling frequency, formant carrier frequencies, resonance, vibrato, and timbre simultaneously.
- **Audio Cylinders & Mic Input**: Load external audio samples or plug directly into the microphone for live stage performance.
- **Ether Waveform Oscilloscope**: Real-time visual phase and spectral analysis monitoring output timbre.
- **Cross-Platform**: Available natively on [Google Play Store](https://play.google.com/store/apps/details?id=com.tonetxo.vocodergal) for Android, and executable on any modern web browser.

---

## 🌐 Galego

**GalVoder** é un sintetizador de voz tipo vocoder cunha estética **vintage** coidada e capacidades avanzadas de deseño sonoro. Deseñado para transformar a voz ou calquera fonte de audio en texturas robóticas, harmónicas e experimentais.

### 🚀 Características principais

- **Procesamento en Tempo Real**: Algoritmo de vocoder de alta fidelidade e latencia ultrabaixa.
- **Estética Vintage MK-XII**: Interface inspirada en maquinaria analóxica clásica, con vúmetros de agulla, luces indicadoras e controis rotativos.
- **Pad Control XY**: Control dinámico e simultáneo de dous parámetros (frecuencia, intensidade, vibrato, etc.) para unha manipulación expresiva do son.
- **Cilindros de Audio**: Carga arquivos de son externos ou utiliza a entrada de micrófono en directo.
- **Monitor de Éter**: Visualización en tempo real da forma de onda a través dun osciloscopio integrado.
- **Consola do Oráculo AI ✨** (*En desenvolvemento*): Sección para a xeración de frases místicas e calibración automática do sistema mediante IA.

---

## 📲 Descarga para Android

Podes instalar a aplicación directamente dende Google Play Store:
👉 **[Instalar GalVoder en Google Play](https://play.google.com/store/apps/details?id=com.tonetxo.vocodergal)**

---

## 🛠️ Arquitectura e Tecnoloxías

Este proxecto combina tecnoloxías web modernas con procesamento nativo de alto rendemento:

- **Mobile (Android)**: Motor de procesamento de sinal dixital (DSP) escrito en **C++** e integrado en **Kotlin** para garantir o rendemento en tempo real sen latencia nin cortes de son.
- **Audio Web**: **Web Audio API** para o motor de síntese e bancos de filtros no navegador.
- **Frontend**: HTML5, Vanilla JavaScript, e **Tailwind CSS** para unha interface reactiva, fluída e adaptable a calquera pantalla.

---

## 📥 Instalación e Uso

### Web
Simplemente abre o arquivo `index.html` nun navegador moderno ou serve o cartafol mediante un servidor web local:
```bash
# Exemplo con Python
python3 -m http.server 8080
```
E abre `http://localhost:8080` no teu navegador.

### Android
O proxecto inclúe o código fonte nativo no cartafol `/android`. Podes abrir o proxecto directamente en **Android Studio** para compilar e depurar en dispositivos físicos ou emuladores.

---

## 🤝 Contribucións

As contribucións son benvidas! Se tes ideas para novos filtros vocoder, formas de onda portadoras ou melloras no motor DSP en C++, por favor abre un issue ou envía un pull request.

---

## 📄 Licenza

Este proxecto está dispoñible baixo a licenza MIT.
