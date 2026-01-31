---
name: android-deployer
description: Skill local para compilar e instalar Aethereum/VocoGal. Usa Gradle e ADB para enviar a app ao móbil.
---

# Android Deployer (Local)

Este skill automatiza o ciclo de vida de **Aethereum** dende a compilación ata o dispositivo físico.

## Configuración Local

- **SDK**: `/home/tonetxo/Android/Sdk`
- **Proxecto**: `/mnt/datos/Antigravity/Aethereum/android`

## Comandos

### 1. Despregamento Completo (Build + Install + Run)

```bash
bash .skills/android-deployer/scripts/deploy.sh
```

### 2. Xerar Bundle (Play Store)

```bash
cd android && ./gradlew bundleRelease
```

### 3. Ver Logs

```bash
adb logcat *:S VocoderEngine:V VocoderProcessor:V
```

### 4. Solución de Problemas (Erro de Sinatura)

Se falla a instalación por conflito de sinaturas:

```bash
adb uninstall com.tonetxo.vocodergal
```
