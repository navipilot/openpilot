package io.zoompilot.jetlink.ui.settings

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.consumeWindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import io.zoompilot.jetlink.ui.JetlinkTheme
import io.zoompilot.jetlink.ui.components.ActionRow
import io.zoompilot.jetlink.ui.components.CardSpacing
import io.zoompilot.jetlink.ui.components.FormFooter
import io.zoompilot.jetlink.ui.components.FormSection
import io.zoompilot.jetlink.ui.components.PushedScreen
import io.zoompilot.jetlink.ui.components.RowDivider

private const val GUIDE = "https://github.com/zoompilot/jetlink/blob/main/docs/android-app.md#connect-the-comma"

private val steps = listOf(
    "On the comma, set Jetlink to USB, offroad.",
    "Plug a USB 3 hub with power pass-through into the phone.",
    "Connect the hub to the comma with a USB-A to USB-C cable.",
    "Plug a charger into the hub.",
    "When Android asks, tap OK and tick Always open.",
    "Wait for Connected.",
)

private val notes = listOf(
    "OnePlus, OPPO and realme: turn on OTG connection in Settings.",
    "Jetlink keeps running with the screen off while its notification shows.",
)

/** How the comma and the phone meet, in a few steps for a reader standing at the car. */
@Composable
fun ConnectHelpScreen(back: () -> Unit) {
    val uriHandler = LocalUriHandler.current
    PushedScreen("Connecting the Comma", back) { padding ->
        Column(
            Modifier
                .padding(padding)
                .consumeWindowInsets(padding)
                .fillMaxSize()
                .verticalScroll(rememberScrollState())
                .padding(horizontal = CardSpacing)
                .padding(top = 8.dp, bottom = 24.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(20.dp),
        ) {
            FormSection(
                null,
                footer = { FormFooter("Or use a USB-C OTG adapter instead of the hub. The phone then does not charge.") },
            ) {
                steps.forEachIndexed { index, step ->
                    if (index > 0) RowDivider()
                    Step(index + 1, step)
                }
            }
            FormSection(
                "Good to Know",
                footer = { FormFooter("Every hop must be USB 3: the phone, the hub and the cable. USB 2 leaves less time for each frame.") },
            ) {
                notes.forEachIndexed { index, note ->
                    if (index > 0) RowDivider()
                    Text(note, style = MaterialTheme.typography.bodyLarge, modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 12.dp))
                }
            }
            FormSection(null) {
                ActionRow("Learn More", { runCatching { uriHandler.openUri(GUIDE) } }, color = MaterialTheme.colorScheme.primary, chevron = false)
            }
        }
    }
}

@Composable
private fun Step(number: Int, text: String) {
    Row(
        Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 12.dp).semantics(mergeDescendants = true) {},
        verticalAlignment = Alignment.Top,
    ) {
        Text(
            number.toString(),
            style = MaterialTheme.typography.bodyLarge,
            fontWeight = FontWeight.SemiBold,
            color = JetlinkTheme.colors.secondaryText,
            textAlign = TextAlign.End,
            modifier = Modifier.width(18.dp),
        )
        Text(text, style = MaterialTheme.typography.bodyLarge, modifier = Modifier.padding(start = 12.dp))
    }
}

@Preview(showBackground = true, heightDp = 800)
@Composable
private fun ConnectHelpPreview() {
    JetlinkTheme { ConnectHelpScreen(back = {}) }
}
