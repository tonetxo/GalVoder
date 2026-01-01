package com.antigravity.vocodergal.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.antigravity.vocodergal.ui.theme.*

/**
 * Selector de forma de onda.
 */
@Composable
fun WaveformSelector(
    selected: Int,
    onSelect: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val waveforms = listOf("SAW", "SQR", "TRI", "SIN")
    
    Row(
        modifier = modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceEvenly
    ) {
        waveforms.forEachIndexed { index, name ->
            WaveformButton(
                name = name,
                isSelected = index == selected,
                onClick = { onSelect(index) }
            )
        }
    }
}

@Composable
private fun WaveformButton(
    name: String,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    Box(
        modifier = Modifier
            .size(60.dp)
            .clip(RoundedCornerShape(8.dp))
            .background(
                if (isSelected) {
                    Brush.verticalGradient(listOf(BronzeGold, DarkBronze))
                } else {
                    Brush.verticalGradient(listOf(SteamGray, DarkWood))
                }
            )
            .border(2.dp, if (isSelected) GlowAmber else BronzeGold, RoundedCornerShape(8.dp))
            .clickable { onClick() },
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = name,
            color = if (isSelected) DarkWood else OldPaper,
            fontSize = 12.sp,
            fontWeight = FontWeight.Bold
        )
    }
}

/**
 * Selector de parámetro para los ejes XY.
 */
@Composable
fun ParamSelector(
    label: String,
    options: List<String>,
    selected: String,
    onSelect: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val currentIndex = options.indexOf(selected)
    
    Column(
        modifier = modifier,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            text = label,
            color = BronzeGold,
            fontSize = 12.sp,
            fontWeight = FontWeight.Medium
        )
        Spacer(modifier = Modifier.height(4.dp))
        Box(
            modifier = Modifier
                .width(100.dp) // Ancho fijo para estabilizar el layout
                .clip(RoundedCornerShape(4.dp))
                .background(Brush.verticalGradient(listOf(DarkWood, SteamGray.copy(alpha = 0.5f))))
                .border(1.dp, BronzeGold.copy(alpha = 0.6f), RoundedCornerShape(4.dp))
                .clickable {
                    val nextIndex = (currentIndex + 1) % options.size
                    onSelect(options[nextIndex])
                }
                .padding(vertical = 6.dp),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = selected.uppercase(),
                color = GlowAmber,
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold,
                letterSpacing = 1.sp
            )
        }
    }
}
