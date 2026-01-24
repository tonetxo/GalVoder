package com.antigravity.vocodergal.ui

import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.animation.core.RepeatMode
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.FolderOpen
import androidx.compose.material.icons.filled.Mic
import androidx.compose.material.icons.filled.MicOff
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.PowerSettingsNew
import androidx.compose.material.icons.filled.RadioButtonChecked
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.graphics.Brush
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.clickable
import androidx.compose.ui.draw.clip
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.drawscope.Stroke
import com.antigravity.vocodergal.ui.components.*
import com.antigravity.vocodergal.ui.theme.*
import com.antigravity.vocodergal.viewmodel.VocoderViewModel

/**
 * Pantalla principal del vocoder (GalVoder).
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
    val isMicActive by viewModel.isMicActive.collectAsState()
    val isFilePlaying by viewModel.isFilePlaying.collectAsState()
    val hasFileLoaded by viewModel.hasFileLoaded.collectAsState()
    val isDecoding by viewModel.isDecoding.collectAsState()
    val isRecording by viewModel.isRecording.collectAsState()
    var resetTrigger by remember { mutableStateOf(0) }

    // Animación de parpadeo para el botón REC
    val infiniteTransition = rememberInfiniteTransition(label = "blink")
    val recAlpha by infiniteTransition.animateFloat(
        initialValue = 1f,
        targetValue = 0.3f,
        animationSpec = infiniteRepeatable(
            animation = tween(500),
            repeatMode = RepeatMode.Reverse
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
            text = "GALVODER",
            style = MaterialTheme.typography.headlineLarge,
            color = MaterialTheme.colorScheme.primary,
            modifier = Modifier.padding(bottom = 20.dp) // Aumentado para separar do título
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
                    contentDescription = "Enerxía",
                    tint = if (isRunning) Color.Black else MaterialTheme.colorScheme.onSurface
                )
            }

            // VU Meter Analógico
            VUMeter(
                level = vuLevel,
                modifier = Modifier
                    .weight(1f)
                    .height(70.dp) // Reducido para compactar
                    .padding(horizontal = 12.dp)
            )

            // 4 Botones de control: MIC, REC, LOAD, PLAY
            Row(
                horizontalArrangement = Arrangement.spacedBy(4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                // MIC toggle
                IconButton(
                    onClick = { viewModel.toggleMicActive() },
                    modifier = Modifier
                        .size(36.dp)
                        .background(
                            if (isMicActive) MaterialTheme.colorScheme.primary
                            else Color.Transparent,
                            shape = MaterialTheme.shapes.small
                        )
                ) {
                    Icon(
                        imageVector = if (isMicActive) Icons.Default.Mic else Icons.Default.MicOff,
                        contentDescription = "Micrófono",
                        tint = if (isMicActive) Color.Black else MaterialTheme.colorScheme.onSurface
                    )
                }

                // REC toggle
                IconButton(
                    onClick = { viewModel.toggleRecording() },
                    modifier = Modifier
                        .size(36.dp)
                        .graphicsLayer(alpha = if (isRecording) recAlpha else 1f)
                        .background(
                            if (isRecording) Color.Red.copy(alpha = 0.3f)
                            else Color.Transparent,
                            shape = MaterialTheme.shapes.small
                        )
                ) {
                    Icon(
                        imageVector = if (isRecording) Icons.Default.Stop else Icons.Default.RadioButtonChecked,
                        contentDescription = "Gravar",
                        tint = if (isRecording) Color.Red else MaterialTheme.colorScheme.onSurface
                    )
                }

                // LOAD file
                IconButton(
                    onClick = { onLoadFile() },
                    modifier = Modifier.size(36.dp)
                ) {
                    if (isDecoding) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(20.dp),
                            strokeWidth = 2.dp
                        )
                    } else {
                        Icon(
                            imageVector = Icons.Default.FolderOpen,
                            contentDescription = "Cargar",
                            tint = if (hasFileLoaded) MaterialTheme.colorScheme.tertiary 
                                   else MaterialTheme.colorScheme.onSurface
                        )
                    }
                }

                // PLAY/STOP toggle
                IconButton(
                    onClick = { viewModel.toggleFilePlayback() },
                    modifier = Modifier
                        .size(36.dp)
                        .background(
                            if (isFilePlaying) MaterialTheme.colorScheme.tertiary
                            else Color.Transparent,
                            shape = MaterialTheme.shapes.small
                        ),
                    enabled = hasFileLoaded
                ) {
                    Icon(
                        imageVector = if (isFilePlaying) Icons.Default.Stop else Icons.Default.PlayArrow,
                        contentDescription = "Reproducir",
                        tint = if (isFilePlaying) Color.Black 
                               else if (hasFileLoaded) MaterialTheme.colorScheme.onSurface
                               else MaterialTheme.colorScheme.onSurface.copy(alpha = 0.3f)
                    )
                }
            }
        }

        Spacer(modifier = Modifier.height(8.dp)) // Espacio reducido para baixar o Pad

        // XY Pad
        XYPad(
            resetKey = resetTrigger,
            onMove = { x, y -> viewModel.updatePad(x, y) },
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f)
                .padding(horizontal = 4.dp)
        )

        Spacer(modifier = Modifier.height(16.dp))

        // Estrutura de control balanceada (Etiquetas e Botóns aliñados horizontalmente)
        val allParams = listOf("ton", "intensidade", "vibrato", "eco", "trémolo")
        
        Column(modifier = Modifier.fillMaxWidth()) {
            // Fila de etiquetas sincronizada co peso dos botóns
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(modifier = Modifier.weight(1f), contentAlignment = Alignment.Center) {
                    Text(text = "X", color = BronzeGold, fontSize = 12.sp, fontWeight = FontWeight.Medium)
                }
                Spacer(modifier = Modifier.width(44.dp)) // Oco para o círculo de Reset
                Box(modifier = Modifier.weight(1f), contentAlignment = Alignment.Center) {
                    Text(text = "Y", color = BronzeGold, fontSize = 12.sp, fontWeight = FontWeight.Medium)
                }
            }
            
            Spacer(modifier = Modifier.height(4.dp))

            // Fila de interactores (Botóns + Reset)
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(modifier = Modifier.weight(1f), contentAlignment = Alignment.Center) {
                    ParamButton(
                        options = allParams,
                        selected = selectedX,
                        onSelect = { viewModel.setXParam(it) }
                    )
                }

                // Botón RESET Circular (44dp)
                Box(
                    modifier = Modifier
                        .size(44.dp)
                        .clip(CircleShape)
                        .background(Brush.verticalGradient(listOf(DarkWood, SteamGray.copy(alpha = 0.5f))))
                        .border(1.dp, BronzeGold.copy(alpha = 0.6f), CircleShape)
                        .clickable { 
                            viewModel.resetParams()
                            resetTrigger++
                        },
                    contentAlignment = Alignment.Center
                ) {
                    Canvas(modifier = Modifier.size(20.dp)) {
                        val color = GlowAmber
                        val strokeWidth = 2.dp.toPx()
                        
                        drawCircle(
                            color = color,
                            radius = 4.dp.toPx(),
                            style = Stroke(width = strokeWidth)
                        )
                        
                        drawLine(
                            color = color,
                            start = Offset(size.width / 2, 0f),
                            end = Offset(size.width / 2, size.height),
                            strokeWidth = strokeWidth
                        )
                        drawLine(
                            color = color,
                            start = Offset(0f, size.height / 2),
                            end = Offset(size.width, size.height / 2),
                            strokeWidth = strokeWidth
                        )
                    }
                }

                Box(modifier = Modifier.weight(1f), contentAlignment = Alignment.Center) {
                    ParamButton(
                        options = allParams,
                        selected = selectedY,
                        onSelect = { viewModel.setYParam(it) }
                    )
                }
            }
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
                .height(80.dp) // Reducido un pouco
        )
    }
}
