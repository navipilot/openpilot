package io.zoompilot.jetlink.ui.benchmark

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.consumeWindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.BarChart
import androidx.compose.material.icons.filled.Dangerous
import androidx.compose.material.icons.filled.LocalFireDepartment
import androidx.compose.material.icons.filled.Memory
import androidx.compose.material.icons.filled.Movie
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Share
import androidx.compose.material.icons.filled.SlowMotionVideo
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material.icons.filled.Timer
import androidx.compose.material.icons.filled.Verified
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import io.zoompilot.jetlink.AppGraph
import io.zoompilot.jetlink.server.BenchReport
import io.zoompilot.jetlink.server.BenchWindow
import io.zoompilot.jetlink.server.Benchmark
import io.zoompilot.jetlink.server.RunState
import io.zoompilot.jetlink.server.Snapshot
import io.zoompilot.jetlink.settings.Chip
import io.zoompilot.jetlink.settings.Processor
import io.zoompilot.jetlink.ui.Format
import io.zoompilot.jetlink.ui.JetlinkTheme
import io.zoompilot.jetlink.ui.PreviewData
import io.zoompilot.jetlink.ui.Thermal
import io.zoompilot.jetlink.ui.Tone
import io.zoompilot.jetlink.ui.Verdict
import io.zoompilot.jetlink.ui.components.CardSpacing
import io.zoompilot.jetlink.ui.components.FigureRow
import io.zoompilot.jetlink.ui.components.MetricRow
import io.zoompilot.jetlink.ui.components.MetricTile
import io.zoompilot.jetlink.ui.components.ReadableWidth
import io.zoompilot.jetlink.ui.components.SummaryCard
import io.zoompilot.jetlink.ui.components.share
import io.zoompilot.jetlink.ui.components.tile
import io.zoompilot.jetlink.ui.icon
import kotlinx.coroutines.launch

/** The phone's chip, as the run card names it. */
data class ChipInfo(val line: String, val expectation: String, val tone: Tone) {
    companion object {
        fun current(processor: Processor): ChipInfo {
            val (text, tone) = BenchmarkText.expectation(Chip.expectation(processor))
            return ChipInfo(BenchmarkText.chipLine(Chip.name, Chip.hexagon), text, tone)
        }
    }
}

/** What the Benchmark tab's buttons do. */
class BenchmarkActions(
    val start: (Int) -> Unit = {},
    val cancel: () -> Unit = {},
)

/**
 * Is this phone fast enough, and does it stay fast enough? The loaded model
 * at the comma's pace on the phone alone.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BenchmarkScreen(graph: AppGraph) {
    val context = LocalContext.current
    val snapshot by graph.server.snapshot.collectAsStateWithLifecycle()
    val runState by graph.server.runState.collectAsStateWithLifecycle()
    val settings by graph.settings.values.collectAsStateWithLifecycle()
    var refusal by remember { mutableStateOf<String?>(null) }
    var starting by remember { mutableStateOf(false) }
    val processor = settings.processor.resolved()
    val chip = remember(processor) { ChipInfo.current(processor) }
    val actions = BenchmarkActions(
        start = { seconds ->
            refusal = null
            starting = true
            graph.scope.launch {
                val reply = graph.server.benchmark(seconds)
                if (!reply.ok) {
                    refusal = listOfNotNull("Couldn't start the benchmark.", reply.error?.let(Format::sentence)).joinToString(" ")
                }
                starting = false
            }
        },
        cancel = { graph.scope.launch { graph.server.cancelBenchmark() } },
    )
    val report = snapshot.benchmark?.report
    val reportText = snapshot.benchmark?.reportText
    Scaffold(
        containerColor = JetlinkTheme.colors.grouped,
        topBar = {
            TopAppBar(
                title = { Text("Benchmark", fontWeight = FontWeight.Bold) },
                actions = {
                    if (report != null && reportText != null) {
                        IconButton(onClick = { share(context, reportText, "Jetlink Benchmark") }) {
                            Icon(Icons.Filled.Share, contentDescription = "Share")
                        }
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = JetlinkTheme.colors.grouped,
                    scrolledContainerColor = JetlinkTheme.colors.grouped,
                ),
            )
        },
    ) { padding ->
        BenchmarkContent(
            snapshot = snapshot,
            runState = runState,
            // what the loaded engine runs on, which a Tensor NPU that took no model leaves to the GPU
            processor = Processor.hardwareOf(snapshot.engine.accelerator) ?: processor.title,
            chip = chip,
            refusal = refusal,
            starting = starting,
            actions = actions,
            modifier = Modifier.padding(padding).consumeWindowInsets(padding),
        )
    }
}

/** The run and its results, without the server, for previews. */
@Composable
fun BenchmarkContent(
    snapshot: Snapshot,
    runState: RunState,
    processor: String,
    chip: ChipInfo,
    refusal: String?,
    starting: Boolean,
    actions: BenchmarkActions,
    modifier: Modifier = Modifier,
) {
    val report = snapshot.benchmark?.report
    val spacing = Arrangement.spacedBy(CardSpacing)
    BoxWithConstraints(modifier.fillMaxSize()) {
        val wide = maxWidth >= 840.dp
        Column(
            Modifier.fillMaxSize().verticalScroll(rememberScrollState()).padding(horizontal = CardSpacing).padding(top = 8.dp, bottom = 24.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            if (wide && report != null && report.frames > 0) {
                // A tablet or an open fold has room for the run and its verdict beside the numbers behind it.
                Row(horizontalArrangement = spacing) {
                    Column(Modifier.weight(1f), verticalArrangement = spacing) {
                        RunCard(snapshot, runState, processor, chip, refusal, starting, actions)
                        VerdictCard(report)
                    }
                    Column(Modifier.weight(1f), verticalArrangement = spacing) { Results(report) }
                }
            } else {
                Column(Modifier.widthIn(max = ReadableWidth).fillMaxWidth(), verticalArrangement = spacing) {
                    RunCard(snapshot, runState, processor, chip, refusal, starting, actions)
                    if (report != null) {
                        VerdictCard(report)
                        if (report.frames > 0) Results(report)
                    }
                }
            }
        }
    }
}

/** The numbers behind the verdict: the totals, then the run ten seconds at a time. */
@Composable
private fun ColumnScope.Results(report: BenchReport) {
    Totals(report)
    if (report.windows.size > 1) WindowsCard(report.windows)
}

@Composable
private fun RunCard(
    snapshot: Snapshot,
    runState: RunState,
    processor: String,
    chip: ChipInfo,
    refusal: String?,
    starting: Boolean,
    actions: BenchmarkActions,
) {
    val colors = JetlinkTheme.colors
    val sha = BenchmarkText.loadedSha(snapshot)
    val event = snapshot.benchmark
    val running = BenchmarkText.running(snapshot)
    val blocker = BenchmarkText.blocker(runState, snapshot)
    SummaryCard("Benchmark", Icons.Filled.Timer, colors.info, trailing = processor) {
        Column(verticalArrangement = Arrangement.spacedBy(2.dp)) {
            Text(
                sha?.let { snapshot.modelName(it) ?: "Model" } ?: "No Model",
                style = MaterialTheme.typography.headlineSmall,
                fontWeight = FontWeight.Bold,
            )
            Text(
                buildAnnotatedString {
                    withStyle(SpanStyle(color = colors.secondaryText)) { append(chip.line + " · ") }
                    withStyle(SpanStyle(color = colors.tone(chip.tone))) { append(chip.expectation) }
                },
                style = MaterialTheme.typography.bodyMedium,
            )
        }
        if (event != null && running) {
            Progress(event, actions.cancel)
        } else {
            Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                val enabled = blocker == null && !starting
                Button(onClick = { actions.start(60) }, enabled = enabled, modifier = Modifier.weight(1f)) {
                    Icon(Icons.Filled.PlayArrow, contentDescription = null, modifier = Modifier.size(18.dp))
                    Spacer(Modifier.width(6.dp))
                    Text("1 Minute")
                }
                FilledTonalButton(onClick = { actions.start(600) }, enabled = enabled, modifier = Modifier.weight(1f)) {
                    Icon(Icons.Filled.LocalFireDepartment, contentDescription = null, modifier = Modifier.size(18.dp))
                    Spacer(Modifier.width(6.dp))
                    Text("10 Minutes")
                }
            }
        }
        val failure = refusal ?: if (event?.state == "failed") "The benchmark failed. See Logs for details." else null
        if (failure != null) {
            Text(failure, style = MaterialTheme.typography.bodySmall, color = colors.bad)
        } else if (!running && blocker != null) {
            Text(blocker, style = MaterialTheme.typography.bodySmall, color = colors.warning)
        }
        Text("Run it with the phone charging and in its mount.", style = MaterialTheme.typography.bodySmall, color = colors.secondaryText)
    }
}

@Composable
private fun Progress(event: Benchmark, cancel: () -> Unit) {
    val colors = JetlinkTheme.colors
    Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
        LinearProgressIndicator(
            progress = { (event.elapsed / maxOf(event.total, 1.0)).coerceIn(0.0, 1.0).toFloat() },
            modifier = Modifier.fillMaxWidth(),
            color = colors.info,
            trackColor = colors.info.copy(alpha = 0.2f),
        )
        Row {
            Text(
                "${Format.clock(event.elapsed)} of ${Format.clock(event.total)}",
                style = MaterialTheme.typography.bodyMedium,
                color = colors.secondaryText,
                modifier = Modifier.weight(1f),
            )
            Text("${Format.integer(event.frames)} frames", style = MaterialTheme.typography.bodyMedium, color = colors.secondaryText)
        }
        FigureRow(listOf(Triple("P50", event.frame?.p50, Color.Unspecified), Triple("P99", event.frame?.p99, Color.Unspecified)))
        OutlinedButton(
            onClick = cancel,
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(contentColor = colors.bad),
        ) {
            Icon(Icons.Filled.Stop, contentDescription = null, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(6.dp))
            Text("Cancel")
        }
    }
}

private fun verdictIcon(verdict: Verdict): ImageVector = when (verdict) {
    Verdict.Good -> Icons.Filled.Verified
    Verdict.Tight -> Icons.Filled.Warning
    Verdict.Slow -> Icons.Filled.Dangerous
    Verdict.None -> Icons.Filled.Timer
}

/** Fast enough, tight, or too slow, and the numbers that say so. */
@Composable
private fun VerdictCard(report: BenchReport) {
    val colors = JetlinkTheme.colors
    val verdict = Verdict.of(report)
    val tone = colors.tone(verdict.tone)
    SummaryCard("Verdict", verdictIcon(verdict), tone, trailing = if (report.cancelled) "Stopped Early" else null) {
        Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
            Text(verdict.title, style = MaterialTheme.typography.headlineMedium, fontWeight = FontWeight.Bold, color = tone)
            Text(BenchmarkText.verdictDetail(report, verdict), style = MaterialTheme.typography.bodyMedium, color = colors.secondaryText)
        }
        // no frames, no numbers: zeros would read as fast
        if (report.frames > 0) {
            FigureRow(
                listOf(
                    Triple("P99", report.frame.p99, tone),
                    Triple("Max", report.frame.max, Color.Unspecified),
                    Triple("Mean", report.frame.mean, Color.Unspecified),
                ),
            )
        }
    }
}

@Composable
private fun Totals(report: BenchReport) {
    val colors = JetlinkTheme.colors
    val thermal = Thermal.of(report.thermalAtEnd)
    Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
        MetricRow {
            MetricTile(
                "Frames", Icons.Filled.Movie, colors.teal, Format.integer(report.frames), tile(),
                note = BenchmarkText.framesNote(report),
            )
            MetricTile(
                "Over Budget", Icons.Filled.SlowMotionVideo, colors.pink, Format.integer(report.over50), tile(),
                note = BenchmarkText.overNote(report),
                noteColor = if (report.over50 > 0) colors.bad else null,
            )
        }
        MetricRow {
            MetricTile(
                "Temperature", thermal.icon, colors.orange, thermal.title, tile(),
                note = BenchmarkText.thermalNote(report),
                noteColor = colors.tone(thermal.tone),
            )
            MetricTile(
                "Model", Icons.Filled.Memory, colors.purple, Format.decimal(report.accelerator.mean), tile(),
                unit = "ms",
                note = "Mean, model alone",
            )
        }
    }
}

/** The run ten seconds at a time, with the phone's temperature as each closed. */
@Composable
private fun WindowsCard(windows: List<BenchWindow>) {
    val colors = JetlinkTheme.colors
    SummaryCard("Over Time", Icons.Filled.BarChart, colors.indigo, trailing = "10 s each") {
        Column {
            windows.forEachIndexed { index, window ->
                if (index > 0) HorizontalDivider()
                val thermal = Thermal.of(window.thermal)
                val verdict = Verdict.of(window.frame.p99)
                Row(Modifier.fillMaxWidth().padding(vertical = 6.dp), verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        Format.clock(window.startSecond.toDouble()),
                        style = MaterialTheme.typography.bodyMedium,
                        color = colors.secondaryText,
                        modifier = Modifier.width(48.dp),
                    )
                    Text(
                        "P99 ${Format.ms(window.frame.p99)}",
                        style = MaterialTheme.typography.bodyMedium,
                        color = if (verdict == Verdict.Good) Color.Unspecified else colors.tone(verdict.tone),
                        modifier = Modifier.weight(1f),
                    )
                    Icon(thermal.icon, contentDescription = null, tint = colors.tone(thermal.tone), modifier = Modifier.size(16.dp))
                    Spacer(Modifier.width(4.dp))
                    Text(thermal.title, style = MaterialTheme.typography.bodyMedium, color = colors.tone(thermal.tone))
                }
            }
        }
    }
}

@Preview(showBackground = true, heightDp = 1400)
@Composable
private fun BenchmarkPreview() {
    JetlinkTheme {
        Column(Modifier.background(JetlinkTheme.colors.grouped)) {
            BenchmarkContent(
                snapshot = PreviewData.serving.copy(link = PreviewData.waiting.link, medium = null),
                runState = RunState.Serving,
                processor = "NPU + GPU",
                chip = ChipInfo("Snapdragon 8 Gen 3 · NPU v75", "Should keep up", Tone.Good),
                refusal = null,
                starting = false,
                actions = BenchmarkActions(),
            )
        }
    }
}

@Preview(showBackground = true, heightDp = 700, uiMode = android.content.res.Configuration.UI_MODE_NIGHT_YES)
@Composable
private fun BenchmarkRunningPreview() {
    JetlinkTheme(dark = true) {
        Column(Modifier.background(JetlinkTheme.colors.grouped)) {
            BenchmarkContent(
                snapshot = PreviewData.waiting.copy(
                    benchmark = Benchmark(state = "running", elapsed = 12.0, total = 60.0, frames = 240, frame = PreviewData.report.frame),
                ),
                runState = RunState.Serving,
                processor = "NPU + GPU",
                chip = ChipInfo("Snapdragon 8 Gen 2 · NPU v73", "Might keep up", Tone.Warning),
                refusal = null,
                starting = false,
                actions = BenchmarkActions(),
            )
        }
    }
}
