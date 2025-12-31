package com.antigravity.vocodergal.audio

/**
 * Bridge JNI para comunicarse con el motor de audio C++.
 */
class VocoderBridge {
    
    private var nativeHandle: Long = 0
    
    init {
        System.loadLibrary("vocoder")
    }
    
    fun create() {
        if (nativeHandle == 0L) {
            nativeHandle = nativeCreate()
        }
    }
    
    fun destroy() {
        if (nativeHandle != 0L) {
            nativeDestroy(nativeHandle)
            nativeHandle = 0
        }
    }
    
    fun start(): Boolean {
        return if (nativeHandle != 0L) nativeStart(nativeHandle) else false
    }
    
    fun stop() {
        if (nativeHandle != 0L) nativeStop(nativeHandle)
    }
    
    fun setPitch(pitch: Float) {
        if (nativeHandle != 0L) nativeSetPitch(nativeHandle, pitch)
    }
    
    fun setIntensity(intensity: Float) {
        if (nativeHandle != 0L) nativeSetIntensity(nativeHandle, intensity)
    }
    
    fun setWaveform(type: Int) {
        if (nativeHandle != 0L) nativeSetWaveform(nativeHandle, type)
    }
    
    fun setVibrato(amount: Float) {
        if (nativeHandle != 0L) nativeSetVibrato(nativeHandle, amount)
    }
    
    fun setEcho(amount: Float) {
        if (nativeHandle != 0L) nativeSetEcho(nativeHandle, amount)
    }
    
    fun getVULevel(): Float {
        return if (nativeHandle != 0L) nativeGetVULevel(nativeHandle) else 0f
    }
    
    fun getWaveformData(): FloatArray {
        return if (nativeHandle != 0L) nativeGetWaveformData(nativeHandle) else FloatArray(0)
    }
    
    // Métodos nativos externos
    private external fun nativeCreate(): Long
    private external fun nativeDestroy(handle: Long)
    private external fun nativeStart(handle: Long): Boolean
    private external fun nativeStop(handle: Long)
    private external fun nativeSetPitch(handle: Long, pitch: Float)
    private external fun nativeSetIntensity(handle: Long, intensity: Float)
    private external fun nativeSetWaveform(handle: Long, type: Int)
    private external fun nativeSetVibrato(handle: Long, amount: Float)
    private external fun nativeSetEcho(handle: Long, amount: Float)
    private external fun nativeGetVULevel(handle: Long): Float
    private external fun nativeGetWaveformData(handle: Long): FloatArray
}
