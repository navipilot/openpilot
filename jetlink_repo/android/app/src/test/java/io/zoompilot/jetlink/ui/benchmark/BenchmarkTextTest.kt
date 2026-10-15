package io.zoompilot.jetlink.ui.benchmark

import io.zoompilot.jetlink.server.Engine
import io.zoompilot.jetlink.server.RunState
import io.zoompilot.jetlink.settings.Chip
import io.zoompilot.jetlink.ui.PreviewData
import io.zoompilot.jetlink.ui.Tone
import io.zoompilot.jetlink.ui.Verdict
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Before
import org.junit.Test
import java.util.Locale

/** The Benchmark tab's blocker, verdict and notes. */
class BenchmarkTextTest {
    private lateinit var locale: Locale

    @Before
    fun englishNumbers() {
        locale = Locale.getDefault()
        Locale.setDefault(Locale.US)
    }

    @After
    fun restore() {
        Locale.setDefault(locale)
    }

    @Test
    fun whatStopsARun() {
        assertEquals("The server is not running.", BenchmarkText.blocker(RunState.Stopped, PreviewData.waiting))
        assertEquals("Load a model first.", BenchmarkText.blocker(RunState.Serving, PreviewData.waiting.copy(engine = Engine())))
        assertEquals("Disconnect the comma first.", BenchmarkText.blocker(RunState.Serving, PreviewData.serving))
        assertNull(BenchmarkText.blocker(RunState.Serving, PreviewData.waiting))
        // a model still preparing is not loaded
        assertEquals("Load a model first.", BenchmarkText.blocker(RunState.Serving, PreviewData.preparing))
    }

    @Test
    fun aRunWithNoFrames() {
        val none = PreviewData.report.copy(frames = 0, seconds = 178.0)
        assertEquals(Verdict.Slow, Verdict.of(none))
        assertEquals("Not one frame finished in 2:58.", BenchmarkText.verdictDetail(none, Verdict.of(none)))
        val stopped = none.copy(cancelled = true)
        assertEquals(Verdict.None, Verdict.of(stopped))
        assertEquals("Stopped before a frame was measured.", BenchmarkText.verdictDetail(stopped, Verdict.of(stopped)))
        assertEquals(Verdict.Good, Verdict.of(PreviewData.report))
    }

    @Test
    fun totalsNotes() {
        val report = PreviewData.report
        assertEquals("1:00 at 20 Hz", BenchmarkText.framesNote(report))
        assertEquals("None over 35 ms", BenchmarkText.overNote(report))
        assertEquals("3 over 35 ms", BenchmarkText.overNote(report.copy(over35 = 3)))
        assertEquals("From normal", BenchmarkText.thermalNote(report))
        assertEquals("Throughout", BenchmarkText.thermalNote(report.copy(thermalAtStart = "fair")))
    }

    @Test
    fun theChip() {
        assertEquals("Snapdragon 8 Gen 3 · NPU v75", BenchmarkText.chipLine("Snapdragon 8 Gen 3", 75))
        assertEquals("SM7550", BenchmarkText.chipLine("SM7550", null))
        assertEquals("Should keep up" to Tone.Good, BenchmarkText.expectation(Chip.Expectation.Recommended))
        assertEquals("Not measured yet" to Tone.Warning, BenchmarkText.expectation(Chip.Expectation.Unmeasured))
    }
}
