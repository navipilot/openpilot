package io.zoompilot.jetlink.ui.status

import io.zoompilot.jetlink.device.DeviceHealth
import io.zoompilot.jetlink.server.Engine
import io.zoompilot.jetlink.server.HistorySample
import io.zoompilot.jetlink.server.Medium
import io.zoompilot.jetlink.server.ModelRow
import io.zoompilot.jetlink.server.RunState
import io.zoompilot.jetlink.server.Snapshot
import io.zoompilot.jetlink.server.Stats
import io.zoompilot.jetlink.ui.Format
import io.zoompilot.jetlink.ui.Thermal
import io.zoompilot.jetlink.ui.Tone
import io.zoompilot.jetlink.usb.UsbState

/**
 * Everything the Status tab draws, as plain values, so the views are a
 * function of it and previews need no server.
 */
data class StatusState(
    val runState: RunState = RunState.Serving,
    val snapshot: Snapshot = Snapshot(),
    val health: DeviceHealth = DeviceHealth(),
    val usb: UsbState = UsbState.None,
) {
    val engine: Engine get() = snapshot.engine
    val connected: Boolean get() = snapshot.connected

    /** The engine's model by name. */
    val modelName: String? get() = snapshot.modelName(engine.sha256)

    /** The last ten seconds of frames. */
    val recent: Stats? get() = snapshot.recent
    val history: List<HistorySample> get() = snapshot.history

    /** What the connected comma's link is carried over; the server sends it only while one is. */
    val medium: Medium? get() = snapshot.medium

    /** The model a comma nobody changed asks for, for the empty state. */
    val defaultModel: ModelRow? get() = snapshot.models.firstOrNull { it.isDefault }
    val hasPreparedModel: Boolean get() = snapshot.models.any { it.isPrepared }

    /** The model list could not be fetched, and there is none cached. */
    val catalogUnavailable: Boolean
        get() {
            val catalog = snapshot.catalog ?: return false
            return !catalog.error.isNullOrBlank() && catalog.count == 0
        }

    val isServingFrames: Boolean get() = connected && engine.state == "ready" && recent != null

    /** What the main card shows. */
    sealed interface Hero {
        /** The comma is connected to a loaded model; null until its first frames land. */
        data class Budget(val stats: Stats?) : Hero
        data class Progress(val engine: Engine) : Hero
        data object Waiting : Hero
        data object NoModel : Hero
        /** The server failed (`model` false) or the model did. */
        data class Failed(val model: Boolean) : Hero
        /** Stopped from the notification or Settings. */
        data object Stopped : Hero
    }

    val hero: Hero
        get() {
            when (runState) {
                is RunState.Failed -> return Hero.Failed(model = false)
                RunState.Stopped -> return Hero.Stopped
                else -> {}
            }
            return when (engine.state) {
                "building", "loading" -> Hero.Progress(engine)
                "failed" -> Hero.Failed(model = true)
                "ready" -> if (connected) Hero.Budget(recent) else Hero.Waiting
                else -> if (hasPreparedModel) Hero.Waiting else Hero.NoModel
            }
        }

    /** One word or two for where things stand, and what it means. */
    enum class Summary(val title: String, val tone: Tone) {
        Failed("Stopped", Tone.Bad),
        Starting("Starting", Tone.Info),
        Stopped("Stopped", Tone.Neutral),
        Preparing("Preparing Model", Tone.Info),
        Loading("Loading Model", Tone.Info),
        ModelFailed("Model Failed", Tone.Bad),
        AllowUsb("Allow USB", Tone.Warning),
        Connected("Connected", Tone.Good),
        ConnectedSlow("Connected", Tone.Warning),
        NoModel("No Model", Tone.Warning),
        Waiting("Waiting for Comma", Tone.Neutral),
        Disconnected("Disconnected", Tone.Warning),
    }

    val summary: Summary
        get() {
            when (runState) {
                is RunState.Failed -> return Summary.Failed
                RunState.Starting -> return Summary.Starting
                RunState.Stopped -> return Summary.Stopped
                RunState.Serving -> {}
            }
            when (engine.state) {
                "building" -> return Summary.Preparing
                "loading" -> return Summary.Loading
                "failed" -> return Summary.ModelFailed
            }
            if (usb == UsbState.NeedsPermission && !connected) return Summary.AllowUsb
            return when (snapshot.link.state) {
                // orange over USB 2: the title is the one line both orientations show
                "connected" -> when {
                    engine.state != "ready" -> Summary.NoModel
                    medium?.slow == true -> Summary.ConnectedSlow
                    else -> Summary.Connected
                }
                "disconnected" -> Summary.Disconnected
                else -> Summary.Waiting
            }
        }

    /** The summary with the link it is over: "Connected over USB 3". */
    val headline: String
        get() {
            val summary = summary
            val medium = medium
            return if ((summary == Summary.Connected || summary == Summary.ConnectedSlow) && medium != null) {
                "Connected over ${medium.title}"
            } else {
                summary.title
            }
        }

    /** The line under the title: the summary and the model. */
    val subtitle: String get() = listOfNotNull(headline, modelName).joinToString(" · ")

    /** Beside the summary away from Status: the headroom while serving, else the model. */
    val accessoryDetail: String
        get() {
            val p99 = recent?.servedMs?.p99
            return if (isServingFrames && p99 != null) Format.headroomText(p99) else modelName.orEmpty()
        }

    /** Under the Link tile: why there is none, or that a slow one costs frames. */
    val linkNote: String
        get() {
            val medium = medium ?: return if (usb is UsbState.Attached) "Connecting" else "Waiting"
            return if (medium.slow) "Slow, use USB 3" else "Connected"
        }

    /** Under Waiting for Comma. */
    val waitingDescription: String
        get() = if (usb is UsbState.Attached) "Connecting over USB." else "Plug in the comma."
}

/** The Phone tiles' words, from DeviceHealth. */
object DeviceText {
    fun thermal(health: DeviceHealth): Thermal = Thermal.of(health.thermal)

    /** "Throttling" when the heat costs frames, else the battery's temperature. */
    fun temperatureNote(health: DeviceHealth): String? =
        thermal(health).note ?: health.batteryTemp?.let { "Battery ${Math.round(it)} °C" }

    /** "82", the unit set beside it; "--" when unknown. */
    fun batteryValue(health: DeviceHealth): String = health.batteryLevel?.toString() ?: "--"

    fun powerText(health: DeviceHealth): String = when {
        health.charging -> if ((health.batteryLevel ?: 0) >= 100) "Charged" else "Charging"
        health.batteryLevel == null -> "Unknown"
        health.powerSave -> "Battery Saver"
        else -> "Not Charging"
    }

    /** A phone running a model twenty times a second belongs on power. */
    fun batteryTone(health: DeviceHealth): Tone = when {
        health.charging || health.batteryLevel == null -> Tone.Neutral
        health.batteryLevel < 20 -> Tone.Bad
        else -> Tone.Warning
    }

    /** "2.4", in GB, beside the unit; "--" when unknown. */
    fun memoryValue(health: DeviceHealth): String =
        if (health.availableMemory > 0) Format.gigabytes(health.availableMemory) else "--"

    fun memoryNote(health: DeviceHealth): String? = when {
        health.availableMemory <= 0 -> null
        health.memoryTight -> "Low"
        else -> "Free"
    }

    fun memoryTone(health: DeviceHealth): Tone = if (health.memoryTight) Tone.Warning else Tone.Neutral
}
