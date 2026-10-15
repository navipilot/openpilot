package io.zoompilot.jetlink.ui.benchmark

import io.zoompilot.jetlink.server.BenchReport
import io.zoompilot.jetlink.server.RunState
import io.zoompilot.jetlink.server.Snapshot
import io.zoompilot.jetlink.settings.Chip
import io.zoompilot.jetlink.ui.Format
import io.zoompilot.jetlink.ui.Thermal
import io.zoompilot.jetlink.ui.Tone
import io.zoompilot.jetlink.ui.Verdict

/** The Benchmark tab's words, as plain functions of the state. */
object BenchmarkText {
    /** Why a run cannot start now, in a sentence; null when it can. The server refuses the same things. */
    fun blocker(serving: Boolean, modelLoaded: Boolean, commaConnected: Boolean): String? = when {
        !serving -> "The server is not running."
        !modelLoaded -> "Load a model first."
        commaConnected -> "Disconnect the comma first."
        else -> null
    }

    fun blocker(runState: RunState, snapshot: Snapshot): String? =
        blocker(runState == RunState.Serving, loadedSha(snapshot) != null, snapshot.connected)

    /** The model a benchmark would run: the engine's, once it is ready. */
    fun loadedSha(snapshot: Snapshot): String? = snapshot.engine.sha256?.takeIf { snapshot.engine.state == "ready" }

    /** A benchmark that has not finished. */
    fun running(snapshot: Snapshot): Boolean = snapshot.benchmark?.state == "running"

    /** The line under the verdict: what it means, or that no frame finished. */
    fun verdictDetail(report: BenchReport, verdict: Verdict): String =
        if (report.frames == 0 && verdict == Verdict.Slow) "Not one frame finished in ${Format.clock(report.seconds)}." else verdict.detail

    /** "1:00 at 20 Hz", under the frame count. */
    fun framesNote(report: BenchReport): String = "${Format.clock(report.seconds)} at 20 Hz"

    fun overNote(report: BenchReport): String =
        if (report.over35 > 0) "${Format.integer(report.over35)} over 35 ms" else "None over 35 ms"

    /** "Throughout", or where the temperature started: "From normal". */
    fun thermalNote(report: BenchReport): String =
        if (report.thermalAtStart == report.thermalAtEnd) "Throughout" else "From ${Thermal.of(report.thermalAtStart).title.lowercase()}"

    /** "Snapdragon 8 Gen 3 · NPU v75". */
    fun chipLine(name: String, hexagon: Int?): String = listOfNotNull(name, hexagon?.let { "NPU v$it" }).joinToString(" · ")

    /** How well this phone should do, before a benchmark says for sure. */
    fun expectation(expectation: Chip.Expectation): Pair<String, Tone> = when (expectation) {
        Chip.Expectation.Recommended -> "Should keep up" to Tone.Good
        Chip.Expectation.Possible -> "Might keep up" to Tone.Warning
        Chip.Expectation.TooOld -> "NPU too old" to Tone.Bad
        Chip.Expectation.Unmeasured -> "Not measured yet" to Tone.Warning
    }
}
