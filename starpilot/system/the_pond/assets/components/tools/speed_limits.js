import { html, reactive } from "/assets/vendor/arrow-core.js"

const state = reactive({
  canProcessNow: false,
  fetched: false,
  loading: true,
  processing: false,
  reason: "",
  status: "Checking...",
  submitting: false,
})

let pollTimer = null

async function fetchStatus() {
  try {
    const response = await fetch("/api/speed_limits/status")
    const result = await response.json()

    state.canProcessNow = Boolean(result.canProcessNow)
    state.processing = Boolean(result.processing)
    state.reason = result.reason || ""
    state.status = result.status || "Idle"
  } catch (error) {
    state.canProcessNow = false
    state.processing = false
    state.reason = "Failed to load processor status."
    state.status = "Unavailable"
  }

  state.loading = false
}

async function handleProcessNow() {
  if (state.submitting || state.processing || !state.canProcessNow) {
    return
  }

  state.submitting = true
  try {
    const response = await fetch("/api/speed_limits/process", { method: "POST" })
    const result = await response.json()

    if (response.ok) {
      showSnackbar(result.message || "Speed limit processing started.")
    } else {
      showSnackbar(result.error || "Failed to start speed limit processing.", "error")
    }
  } catch (error) {
    showSnackbar("Failed to start speed limit processing.", "error")
  }

  state.submitting = false
  fetchStatus()
}

export function SpeedLimits() {
  function handleDownload() {
    const link = document.createElement("a")
    link.href = "/api/speed_limits"
    link.download = "speed_limits.json"
    link.click()

    showSnackbar("Download started...")
  }

  if (!state.fetched) {
    state.fetched = true
    fetchStatus()

    if (pollTimer === null) {
      pollTimer = setInterval(fetchStatus, 3000)
    }
  }

  return html`
    <div class="download-speed-limits-wrapper">
      <section class="download-speed-limits-widget">
        <div class="download-speed-limits-title">Download Speed Limits</div>
        <p class="download-speed-limits-text">
          Enable "Speed Limit Filler" on the device, drive to collect data, then process and download it here when parked.
        </p>
        <p class="download-speed-limits-status">
          ${() => state.loading ? "Checking processor status..." : `Processor Status: ${state.status}`}
        </p>
        ${() => !state.loading && state.reason && state.reason !== state.status ? html`
          <p class="download-speed-limits-note">${state.reason}</p>
        ` : ""}
        <div class="download-speed-limits-button-wrapper">
          <button class="download-speed-limits-button" @click="${handleDownload}">Download</button>
          <button
            class="download-speed-limits-button"
            @click="${handleProcessNow}"
            disabled="${() => state.submitting || state.processing || !state.canProcessNow}"
          >
            ${() => state.processing || state.submitting ? "Processing..." : "Process Now"}
          </button>
          <a class="download-speed-limits-link" href="https://nerf.077769.xyz/" target="_blank" rel="noopener noreferrer">
            Submit speed limits here
          </a>
        </div>
      </section>
    </div>
  `
}
