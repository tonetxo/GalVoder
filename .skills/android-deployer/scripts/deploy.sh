#!/bin/bash

# Script de despliegue automático local para Aethereum
SDK_PATH="/home/tonetxo/Android/Sdk"
APP_PATH="/mnt/datos/Antigravity/Aethereum/android"
PACKAGE_NAME="com.antigravity.vocodergal"

export ANDROID_HOME=$SDK_PATH
export PATH=$PATH:$SDK_PATH/platform-tools

echo "🚀 Compilando e instalando Aethereum..."

cd $APP_PATH
./gradlew assembleDebug && \
adb install -r app/build/outputs/apk/debug/app-debug.apk && \
adb shell am start -n $PACKAGE_NAME/.MainActivity

if [ $? -eq 0 ]; then
    echo "✨ App instalada e lanzada con éxito."
else
    echo "❌ Fallou o proceso. Revisa a conexión ADB."
    exit 1
fi
