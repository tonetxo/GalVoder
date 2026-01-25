# Estado do Proxecto: GalVoder (Aethereum)

**Data**: 25 de Xaneiro, 2026
**Versión Actual**: 1.2.0-beta (Internal Demo)

## 📝 Descrición Xeral

GalVoder é un sintetizador vocoder de 20 bandas de alto rendemento para Android, deseñado cunha estética "Retro-Futurista / Steampunk" (Brass, Wood, Steam). Permite a modulación da voz en tempo real ou a través de ficheiros, utilizando ondas sintéticas ou audio externo como portadora.

## 🚀 Funcionalidades Principais

- **Core DSP**: Vocoder de 20 bandas implementado en C++ (Oboe) para baixa latencia.
- **Entradas**:
  - Micrófono en directo (optimizado para evitar feedback/acoples).
  - Carga de ficheiros de audio como modulador (`audio/*`).
- **Portadoras (Carriers)**:
  - 4 Formas de onda sintéticas (Saw, Square, Triangle, Sine).
  - 📂 **Novo**: Carga de audio externo como carrier (permite usar ritmos ou pads propios).
- **Controis**:
  - **XY Pad**: Control dinámico de dous parámetros simultáneos con botón de Reset central.
  - **Parámetros integrados**: Ton (Pitch), Intensidade, Vibrato, Eco e Trémolo.
- **Utilidades**:
  - Gravación interna da saída procesada.
  - Vúmetro analóxico de alta precisión.
  - Osciloscopio en tempo real.

## 🛠️ Tecnoloxías Utilizadas

- **UI**: Jetpack Compose (Kotlin) con deseño personalizado (Glassmorphism + Neumorphism).
- **Audio Engine**: C++17, Oboe (Google Low Latency Audio), JNI.
- **Automatización**: Skill personalizada `android-deployer` para compilación e instalación rápida vía ADB.

## 📅 Fitos Recentes

- [x] Implementación do Pad XY e selector de parámetros.
- [x] Refactorización estética (BrassGold, SteamGray, DarkWood).
- [x] **Derradeira mellora**: Engadida a portadora externa 📂 e centrado o selector horizontalmente.
- [x] Sistema de despregamento automático configurado en `.skills/`.

## 🔜 Próximos Pasos

1. Publicar a Landing Page 3D (xa deseñada en `/3d_landing`).
2. Implementar a descarga da demo `.AppImage` (pendente de URL de GitHub Release).
3. Optimización final do ruído de fondo en determinadas condicións de ganancia.

---
*Proxecto xestionado por **Antigravity** en colaboración co usuario.*
