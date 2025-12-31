package com.antigravity.vocodergal.viewmodel

import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.antigravity.vocodergal.audio.VocoderBridge
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

/**
 * ViewModel que gestiona el estado del vocoder y la comunicación con el motor C++.
 */
class VocoderViewModel : ViewModel() {
    
    companion object {
        private const val TAG = "VocoderViewModel"
    }
    
    private val bridge = VocoderBridge()
    private var hasPermission = false
    
    // Estado de la UI
    private val _isRunning = MutableStateFlow(false)
    val isRunning: StateFlow<Boolean> = _isRunning.asStateFlow()
    
    private val _vuLevel = MutableStateFlow(0f)
    val vuLevel: StateFlow<Float> = _vuLevel.asStateFlow()
    
    private val _waveformData = MutableStateFlow(FloatArray(256))
    val waveformData: StateFlow<FloatArray> = _waveformData.asStateFlow()
    
    // Parámetros
    private val _pitch = MutableStateFlow(140f)
    val pitch: StateFlow<Float> = _pitch.asStateFlow()
    
    private val _intensity = MutableStateFlow(0.6f)
    val intensity: StateFlow<Float> = _intensity.asStateFlow()
    
    private val _waveform = MutableStateFlow(0) // 0=Saw, 1=Square, 2=Tri, 3=Sine
    val waveform: StateFlow<Int> = _waveform.asStateFlow()
    
    private val _vibrato = MutableStateFlow(0f)
    val vibrato: StateFlow<Float> = _vibrato.asStateFlow()
    
    private val _echo = MutableStateFlow(0f)
    val echo: StateFlow<Float> = _echo.asStateFlow()
    
    // Selección del pad XY
    private val _selectedXParam = MutableStateFlow("pitch")
    val selectedXParam: StateFlow<String> = _selectedXParam.asStateFlow()
    
    private val _selectedYParam = MutableStateFlow("intensity")
    val selectedYParam: StateFlow<String> = _selectedYParam.asStateFlow()
    
    init {
        Log.d(TAG, "ViewModel init - creating bridge")
        bridge.create()
        startUIUpdates()
    }
    
    fun onPermissionGranted() {
        Log.d(TAG, "Permission granted")
        hasPermission = true
    }
    
    private fun startUIUpdates() {
        viewModelScope.launch {
            while (true) {
                if (_isRunning.value) {
                    _vuLevel.value = bridge.getVULevel()
                    _waveformData.value = bridge.getWaveformData()
                }
                delay(16) // ~60 FPS
            }
        }
    }
    
    fun togglePower() {
        Log.d(TAG, "togglePower called, isRunning=${_isRunning.value}, hasPermission=$hasPermission")
        
        if (_isRunning.value) {
            Log.d(TAG, "Stopping engine")
            bridge.stop()
            _isRunning.value = false
        } else {
            if (!hasPermission) {
                Log.e(TAG, "No microphone permission!")
                return
            }
            
            Log.d(TAG, "Starting engine")
            if (bridge.start()) {
                Log.d(TAG, "Engine started successfully")
                _isRunning.value = true
            } else {
                Log.e(TAG, "Failed to start engine")
            }
        }
    }
    
    fun updatePad(x: Float, y: Float) {
        updateParam(_selectedXParam.value, x)
        updateParam(_selectedYParam.value, 1f - y)
    }
    
    private fun updateParam(param: String, value: Float) {
        when (param) {
            "pitch" -> {
                val pitch = 50f + value * 350f // 50-400 Hz
                _pitch.value = pitch
                bridge.setPitch(pitch)
            }
            "intensity" -> {
                val intensity = 0.2f + value * 1f // 0.2-1.2
                _intensity.value = intensity
                bridge.setIntensity(intensity)
            }
            "vibrato" -> {
                _vibrato.value = value
                bridge.setVibrato(value)
            }
            "tremolo" -> {
                // Tremolo se implementará después
            }
            "echo" -> {
                val echo = value * 0.7f
                _echo.value = echo
                bridge.setEcho(echo)
            }
        }
    }
    
    fun setWaveform(type: Int) {
        Log.d(TAG, "setWaveform: $type")
        _waveform.value = type
        bridge.setWaveform(type)
    }
    
    fun setXParam(param: String) {
        _selectedXParam.value = param
    }
    
    fun setYParam(param: String) {
        _selectedYParam.value = param
    }
    
    override fun onCleared() {
        super.onCleared()
        Log.d(TAG, "onCleared - destroying bridge")
        bridge.stop()
        bridge.destroy()
    }
}
