package com.antigravity.vocodergal.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.antigravity.vocodergal.ui.components.*
import com.antigravity.vocodergal.ui.theme.*
import com.antigravity.vocodergal.viewmodel.VocoderViewModel

/**
 * Pantalla principal del vocoder, similar a la UI de Aethereum.
 */
@Composable
fun VocoderScreen(
    viewModel: VocoderViewModel,
    modifier: Modifier = Modifier
) {
    val isRunning by viewModel.isRunning.collectAsState()
    val vuLevel by viewModel.vuLevel.collectAsState()
    val waveformData by viewModel.waveformData.collectAsState()
    val waveform by viewModel.waveform.collectAsState()
    val selectedXParam by viewModel.selectedXParam.collectAsState()
    val selectedYParam by viewModel.selectedYParam.collectAsState()
    
    Column(
        modifier = modifier
            .fillMaxSize()
            .background(
                Brush.verticalGradient(
                    colors = listOf(DarkWood, DarkWood.copy(alpha = 0.8f))
                )
            )
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        // Título
        Text(
            text = "⚙ VOCODER GAL ⚙",
            color = BronzeGold,
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold
        )
        
        // Botón de encendido
        PowerButton(
            isOn = isRunning,
            onClick = { viewModel.togglePower() }
        )
        
        // VU Meter
        VUMeter(
            level = vuLevel,
            modifier = Modifier.fillMaxWidth()
        )
        
        // XY Pad con selectores
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            ParamSelector(
                label = "X:",
                options = listOf("pitch", "vibrato", "echo"),
                selected = selectedXParam,
                onSelect = { viewModel.setXParam(it) }
            )
            ParamSelector(
                label = "Y:",
                options = listOf("intensity", "tremolo"),
                selected = selectedYParam,
                onSelect = { viewModel.setYParam(it) }
            )
        }
        
        XYPad(
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f),
            onMove = { x, y -> viewModel.updatePad(x, y) }
        )
        
        // Selector de forma de onda
        WaveformSelector(
            selected = waveform,
            onSelect = { viewModel.setWaveform(it) }
        )
        
        // Osciloscopio
        Oscilloscope(
            waveformData = waveformData,
            modifier = Modifier.fillMaxWidth()
        )
    }
}

@Composable
private fun PowerButton(
    isOn: Boolean,
    onClick: () -> Unit
) {
    Box(
        modifier = Modifier
            .size(80.dp)
            .clip(CircleShape)
            .background(
                if (isOn) {
                    Brush.radialGradient(listOf(GlowAmber, BronzeGold))
                } else {
                    Brush.radialGradient(listOf(SteamGray, DarkWood))
                }
            )
            .border(4.dp, BronzeGold, CircleShape)
            .clickable { onClick() },
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = if (isOn) "ON" else "OFF",
            color = if (isOn) DarkWood else OldPaper,
            fontSize = 16.sp,
            fontWeight = FontWeight.Bold
        )
    }
}
