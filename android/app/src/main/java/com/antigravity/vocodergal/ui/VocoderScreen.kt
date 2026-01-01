package com.antigravity.vocodergal.ui

import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.animation.core.RepeatMode
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Mic
import androidx.compose.material.icons.filled.Pause
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.PowerSettingsNew
import androidx.compose.material.icons.filled.RadioButtonChecked
import androidx.compose.material.icons.filled.RadioButtonUnchecked
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.UploadFile
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.unit.dp
import com.antigravity.vocodergal.ui.components.*
import com.antigravity.vocodergal.viewmodel.VocoderViewModel

/**
 * Pantalla principal del vocoder.
 */
@Composable
fun VocoderScreen(
    viewModel: VocoderViewModel,
    onLoadFile: () -> Unit
) {
    val isRunning by viewModel.isRunning.collectAsState()
    val vuLevel by viewModel.vuLevel.collectAsState()
    val waveformData by viewModel.waveformData.collectAsState()
    val currentWaveform by viewModel.waveform.collectAsState()
    val selectedX by viewModel.selectedXParam.collectAsState()
    val selectedY by viewModel.selectedYParam.collectAsState()
    val isMicSource by viewModel.isMicSource.collectAsState()
    val isFilePlaying by viewModel.isFilePlaying.collectAsState()
    val isDecoding by viewModel.isDecoding.collectAsState()
    val isRecording by viewModel.isRecording.collectAsState()

    // Animación de parpadeo para el botón REC
    val infiniteTransition = androidx.compose.animation.core.rememberInfiniteTransition(label = "blink")
    val recAlpha by infiniteTransition.animateFloat(
        initialValue = 1f,
        targetValue = 0.3f,
        animationSpec = androidx.compose.animation.core.infiniteRepeatable(
            animation = androidx.compose.animation.core.tween(500),
            repeatMode = androidx.compose.animation.core.RepeatMode.Reverse
        ),
        label = "alpha"
    )

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        // Título
        Text(
            text = "VOCODER GAL",
            style = MaterialTheme.typography.headlineLarge,
            color = MaterialTheme.colorScheme.primary,
            modifier = Modifier.padding(bottom = 16.dp)
        )

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Botón de encendido
            IconButton(
                onClick = { viewModel.togglePower() },
                modifier = Modifier
                    .size(64.dp)
                    .background(
                        if (isRunning) MaterialTheme.colorScheme.primary 
                        else MaterialTheme.colorScheme.surfaceVariant,
                        shape = MaterialTheme.shapes.medium
                    )
            ) {
                Icon(
                    imageVector = Icons.Default.PowerSettingsNew,
                    contentDescription = "Power",
                    tint = if (isRunning) Color.Black else MaterialTheme.colorScheme.onSurface
                )
            }

            // VU Meter Analógico
            VUMeter(
                level = vuLevel,
                modifier = Modifier
                    .weight(1f)
                    .height(80.dp)
                    .padding(horizontal = 16.dp)
            )

            // Controles de Fuente y Archivo
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    // Mic Botón
                    IconButton(
                        onClick = { if (!isMicSource) viewModel.toggleSource() },
                        modifier = Modifier
                            .size(40.dp)
                            .background(
                                if (isMicSource) MaterialTheme.colorScheme.secondary
                                else Color.Transparent,
                                shape = MaterialTheme.shapes.small
                            )
                    ) {
                        Icon(
                            imageVector = Icons.Default.Mic,
                            contentDescription = "Mic",
                            tint = if (isMicSource) Color.Black else MaterialTheme.colorScheme.onSurface
                        )
                    }

                    Spacer(modifier = Modifier.width(4.dp))

                    // File Botón
                    IconButton(
                        onClick = { if (isMicSource) viewModel.toggleSource() },
                        modifier = Modifier
                            .size(40.dp)
                            .background(
                                if (!isMicSource) MaterialTheme.colorScheme.tertiary
                                else Color.Transparent,
                                shape = MaterialTheme.shapes.small
                            )
                    ) {
                        Icon(
                            imageVector = Icons.Default.PlayArrow,
                            contentDescription = "File",
                            tint = if (!isMicSource) Color.Black else MaterialTheme.colorScheme.onSurface
                        )
                    }
                }

                Spacer(modifier = Modifier.height(8.dp))

                // Controles adicionales si es modo archivo
                Row(verticalAlignment = Alignment.CenterVertically) {
                    // Botón REC
                    IconButton(
                        onClick = { viewModel.toggleRecording() },
                        modifier = Modifier
                            .size(32.dp)
                            .graphicsLayer(alpha = if (isRecording) recAlpha else 1f)
                    ) {
                        Icon(
                            imageVector = if (isRecording) Icons.Default.RadioButtonChecked else Icons.Default.RadioButtonUnchecked,
                            contentDescription = "Record",
                            tint = if (isRecording) Color.Red else MaterialTheme.colorScheme.onSurface
                        )
                    }

                    Spacer(modifier = Modifier.width(4.dp))

                    if (!isMicSource) {
                        IconButton(
                            onClick = { viewModel.toggleFilePlayback() },
                            modifier = Modifier.size(32.dp)
                        ) {
                            Icon(
                                imageVector = if (isFilePlaying) Icons.Default.Pause else Icons.Default.PlayArrow,
                                contentDescription = "Play/Pause",
                                tint = MaterialTheme.colorScheme.primary
                            )
                        }
                        IconButton(
                            onClick = { viewModel.resetFilePlayback() },
                            modifier = Modifier.size(32.dp)
                        ) {
                            Icon(
                                imageVector = Icons.Default.Refresh,
                                contentDescription = "Reset",
                                tint = MaterialTheme.colorScheme.primary
                            )
                        }
                    }
                    
                    IconButton(
                        onClick = { onLoadFile() },
                        modifier = Modifier.size(32.dp)
                    ) {
                        if (isDecoding) {
                            CircularProgressIndicator(modifier = Modifier.size(16.dp))
                        } else {
                            Icon(
                                imageVector = Icons.Default.UploadFile,
                                contentDescription = "Load File",
                                tint = MaterialTheme.colorScheme.primary
                            )
                        }
                    }
                }
            }
        }

        Spacer(modifier = Modifier.height(24.dp))

        // XY Pad
        XYPad(
            onMove = { x, y -> viewModel.updatePad(x, y) },
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f)
        )

        Spacer(modifier = Modifier.height(16.dp))

        // Selectores
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly
        ) {
            ParamSelector(
                label = "X",
                options = listOf("pitch", "vibrato", "echo"),
                selected = selectedX,
                onSelect = { viewModel.setXParam(it) }
            )
            ParamSelector(
                label = "Y",
                options = listOf("intensity", "tremolo"),
                selected = selectedY,
                onSelect = { viewModel.setYParam(it) }
            )
        }

        Spacer(modifier = Modifier.height(16.dp))

        WaveformSelector(
            selected = currentWaveform,
            onSelect = { viewModel.setWaveform(it) }
        )

        Spacer(modifier = Modifier.height(16.dp))

        Oscilloscope(
            waveformData = waveformData,
            modifier = Modifier
                .fillMaxWidth()
                .height(100.dp)
        )
    }
}
