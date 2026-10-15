package io.zoompilot.jetlink.ui.status

import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Build
import androidx.compose.material.icons.filled.Cable
import androidx.compose.material.icons.filled.Speed
import androidx.compose.material.icons.filled.StopCircle
import androidx.compose.material.icons.filled.Usb
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material.icons.outlined.Inventory2
import androidx.compose.material.icons.outlined.ReportProblem
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import io.zoompilot.jetlink.server.Engine
import io.zoompilot.jetlink.server.Stats
import io.zoompilot.jetlink.ui.Format
import io.zoompilot.jetlink.ui.JetlinkTheme
import io.zoompilot.jetlink.ui.Room
import io.zoompilot.jetlink.ui.components.EmptyState
import io.zoompilot.jetlink.ui.components.FigureRow
import io.zoompilot.jetlink.ui.components.HeadroomRing
import io.zoompilot.jetlink.ui.components.SummaryCard
import io.zoompilot.jetlink.ui.components.cardBackground
import io.zoompilot.jetlink.usb.UsbState

/** What the Status tab's buttons do, handed down from the screen. */
class StatusActions(
    val useDefault: () -> Unit = {},
    val openModels: () -> Unit = {},
    val retry: () -> Unit = {},
    val start: () -> Unit = {},
    val refreshCatalog: () -> Unit = {},
    val askUsb: () -> Unit = {},
)

/**
 * The main card: the headroom while the comma is served, and otherwise
 * whatever stands between the comma and the big model.
 */
@Composable
fun HeroCard(state: StatusState, modifier: Modifier = Modifier, compact: Boolean = false, actions: StatusActions = StatusActions()) {
    when (val hero = state.hero) {
        is StatusState.Hero.Budget -> Headroom(hero.stats, modifier, compact)
        is StatusState.Hero.Progress -> Progress(hero.engine, state.modelName, modifier)
        StatusState.Hero.Waiting ->
            if (state.usb == UsbState.NeedsPermission) {
                AllowUsb(modifier, compact, actions.askUsb)
            } else {
                Waiting(state.waitingDescription, modifier, compact)
            }
        StatusState.Hero.NoModel -> NoModel(state, modifier, compact, actions)
        is StatusState.Hero.Failed -> Failed(hero.model, modifier, compact, actions.retry)
        StatusState.Hero.Stopped ->
            EmptyState(Icons.Filled.StopCircle, "Jetlink Stopped", modifier, compact = compact) {
                Button(onClick = actions.start) { Text("Start") }
            }
    }
}

@Composable
private fun Headroom(stats: Stats?, modifier: Modifier, compact: Boolean) {
    val colors = JetlinkTheme.colors
    val p99 = stats?.servedMs?.p99
    val tint = p99?.let { colors.tone(Room.forP99(it).tone) } ?: colors.secondaryText
    SummaryCard("Headroom", Icons.Filled.Speed, tint, modifier, trailing = "Last 10 s") {
        Column(
            Modifier.fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(if (compact) 12.dp else 20.dp),
        ) {
            HeadroomRing(
                p99,
                Modifier.widthIn(max = if (compact) 180.dp else 290.dp).fillMaxWidth(),
                lineWidth = if (compact) 14.dp else 22.dp,
                compact = compact,
            )
            FigureRow(
                listOf(
                    Triple("P99", p99, Color.Unspecified),
                    Triple("Max", stats?.servedMs?.max, Color.Unspecified),
                ),
            )
        }
    }
}

@Composable
private fun Progress(engine: Engine, modelName: String?, modifier: Modifier) {
    val colors = JetlinkTheme.colors
    val title = if (engine.state == "loading") "Loading" else "Preparing"
    SummaryCard(title, Icons.Filled.Build, colors.info, modifier, trailing = Format.progressAmount(engine.frac, engine.msg)) {
        Text(modelName ?: "Model", style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Bold)
        // a step with no estimate moves rather than sit at 0%
        if (engine.frac > 0) {
            LinearProgressIndicator(
                progress = { engine.frac.coerceIn(0.0, 1.0).toFloat() },
                modifier = Modifier.fillMaxWidth(),
                color = colors.info,
                trackColor = colors.info.copy(alpha = 0.2f),
            )
        } else {
            LinearProgressIndicator(Modifier.fillMaxWidth(), color = colors.info, trackColor = colors.info.copy(alpha = 0.2f))
        }
        Text(Format.stageName(engine.stage), style = MaterialTheme.typography.bodyMedium, color = colors.secondaryText)
    }
}

@Composable
private fun Waiting(description: String, modifier: Modifier, compact: Boolean) {
    val pulse = rememberInfiniteTransition(label = "waiting").animateFloat(
        initialValue = 1f,
        targetValue = 0.35f,
        animationSpec = infiniteRepeatable(tween(1000, easing = LinearEasing), RepeatMode.Reverse),
        label = "pulse",
    )
    EmptyState(
        Icons.Filled.Cable,
        "Waiting for Comma",
        modifier,
        description = description,
        compact = compact,
        iconAlpha = { pulse.value },
    )
}

/** Android has not let Jetlink use the comma yet. */
@Composable
fun AllowUsb(modifier: Modifier = Modifier, compact: Boolean = false, askAgain: () -> Unit) {
    EmptyState(
        Icons.Filled.Usb,
        "Allow USB",
        modifier,
        description = "Tap OK when Android asks, and tick Always open.",
        compact = compact,
    ) {
        Button(onClick = askAgain) { Text("Ask Again") }
    }
}

@Composable
private fun NoModel(state: StatusState, modifier: Modifier, compact: Boolean, actions: StatusActions) {
    val unavailable = state.catalogUnavailable
    EmptyState(
        Icons.Outlined.Inventory2,
        "No Model",
        modifier,
        description = if (unavailable) "Couldn't load models. Check your connection." else "Get a model before you drive.",
        compact = compact,
    ) {
        if (unavailable) {
            OutlinedButton(onClick = actions.refreshCatalog) { Text("Try Again") }
        } else {
            val row = state.defaultModel
            if (row != null && row.canUse) {
                Button(onClick = actions.useDefault) { Text("Get ${row.displayName}") }
            }
            TextButton(onClick = actions.openModels) { Text("Browse Models") }
        }
    }
}

/** What went wrong, in a line; the error itself is in Logs. */
@Composable
private fun Failed(model: Boolean, modifier: Modifier, compact: Boolean, retry: () -> Unit) {
    EmptyState(
        Icons.Outlined.ReportProblem,
        if (model) "Model Failed" else "Jetlink Stopped",
        modifier,
        description = if (model) "Couldn't prepare this model. See Logs for details." else "See Logs for details.",
        compact = compact,
    ) {
        OutlinedButton(onClick = retry) { Text("Try Again") }
    }
}

/** A line of trouble above the cards: Android's reason the comma could not be opened. */
@Composable
fun WarningBanner(text: String, modifier: Modifier = Modifier) {
    val colors = JetlinkTheme.colors
    Row(
        modifier.fillMaxWidth().cardBackground().padding(14.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Icon(Icons.Filled.Warning, contentDescription = null, tint = colors.warning, modifier = Modifier.size(20.dp))
        Spacer(Modifier.width(10.dp))
        Text(text, style = MaterialTheme.typography.bodyMedium, modifier = Modifier.weight(1f))
    }
}
