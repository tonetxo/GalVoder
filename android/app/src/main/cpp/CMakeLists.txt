cmake_minimum_required(VERSION 3.22.1)
project("vocoder")

# Configuración de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fuentes del proyecto
add_library(vocoder SHARED
    vocoder_jni.cpp
    VocoderEngine.cpp
    VocoderProcessor.cpp
)

# Incluir Oboe (baja latencia)
find_package(oboe REQUIRED CONFIG)

# Librerías del sistema Android
find_library(log-lib log)
find_library(android-lib android)

# Enlazar librerías
target_link_libraries(vocoder
    oboe::oboe
    ${log-lib}
    ${android-lib}
)

# Align native libraries to 16KB for Android 15 support
target_link_options(vocoder PUBLIC 
    "-Wl,-z,common-page-size=16384" 
    "-Wl,-z,max-page-size=16384"
)

# Optimizaciones
target_compile_options(vocoder PRIVATE
    -O3
    -ffast-math
    -funroll-loops
)
