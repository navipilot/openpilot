import { html } from "/assets/vendor/arrow-core.js";

const HOME_STATE = {
  status: "loading", // loading | ready | error
  data: null,
  unit: "miles",
  error: "",
  initialized: false,
};

function withTimeout(promise, timeoutMs, label) {
  return new Promise((resolve, reject) => {
    const timerId = setTimeout(() => reject(new Error(`${label} timed out`)), timeoutMs);
    promise.then((value) => {
      clearTimeout(timerId);
      resolve(value);
    }).catch((err) => {
      clearTimeout(timerId);
      reject(err);
    });
  });
}

function formatInt(value) {
  const n = Number(value || 0);
  return Number.isFinite(n) ? n.toLocaleString("en-US", { maximumFractionDigits: 0 }) : "0";
}

function renderDiskUsageSection(state) {
  const { data } = state;
  const shell = document.getElementById("home_shell");
  if (!shell) return;

  if (state.status === "error") {
    shell.innerHTML = `<p class="error">Failed to load data: ${state.error}</p>`;
    return;
  }

  if (state.status !== "ready" || !data) {
    shell.innerHTML = "<p>Loading...</p>";
    return;
  }

  const driveStats = data.driveStats || {};
  const softwareInfo = data.softwareInfo || {};
  const diskUsage = Array.isArray(data.diskUsage) ? data.diskUsage : [];
  const diskError = Array.isArray(data.diskError) ? data.diskError : [];

  const statBlock = (title, stats = {}) => `
    <div class="drivingStat">
      <h2>${title}</h2>
      <div><p>${formatInt(stats.drives)}</p><p>drives</p></div>
      <div><p>${formatInt(stats.distance)}</p><p>${stats.unit || state.unit}</p></div>
      <div><p>${formatInt(stats.hours)}</p><p>hours</p></div>
    </div>
  `;

  const diskBlock = (disk = {}) => {
    const usedPct = Number.parseFloat(disk.usedPercentage) || 0;
    const rightRadius = usedPct >= 100 ? "0" : "var(--border-radius-md)";
    return `
      <div class="disk">
        <p>${disk.used || "0 GB"} used of ${disk.size || "0 GB"}</p>
        <div class="progress">
          <div
            class="bar"
            style="
              border-bottom-right-radius: ${rightRadius};
              border-top-right-radius: ${rightRadius};
              width: ${Math.max(0, 100 - usedPct)}%;
            "
          ></div>
        </div>
      </div>
    `;
  };

  const softwareFields = [
    ["Branch Name", softwareInfo.branchName],
    ["Build", softwareInfo.buildEnvironment],
    ["Commit Hash", softwareInfo.commitHash],
    ["Fork Maintainer", softwareInfo.forkMaintainer],
    ["Update Available", softwareInfo.updateAvailable],
    ["Version Date", softwareInfo.versionDate],
  ];

  const softwareMarkup = softwareFields
    .map(([label, value]) => `<p><strong>${label}:</strong> ${value ?? "Unknown"}</p>`)
    .join("");

  const diskMarkup = diskError.length
    ? `<p>${diskError.join("<br>")}</p>`
    : (diskUsage.length ? diskUsage.map(diskBlock).join("") : diskBlock({}));

  shell.innerHTML = `
    <div>
      <h1>Galaxy</h1>

      <div class="drivingStats">
        ${statBlock("All Time", driveStats.all)}
        ${statBlock("Past Week", driveStats.week)}
        ${statBlock("StarPilot", driveStats.frogpilot)}
      </div>

      <h2>Disk Usage</h2>
      <div class="diskUsage">
        ${diskMarkup}
      </div>

      <h2>Software Info</h2>
      <div class="softwareInfo">
        <div class="softwareGrid">${softwareMarkup}</div>
      </div>
    </div>
  `;
}

async function initializeHome() {
  try {
    const [statsResponse, unitResponse] = await Promise.all([
      withTimeout(fetch("/api/stats"), 5000, "stats request"),
      withTimeout(fetch("/api/params?key=IsMetric"), 5000, "metric request"),
    ]);

    if (!statsResponse.ok) throw new Error(`stats API error: ${statsResponse.status}`);
    if (!unitResponse.ok) throw new Error(`params API error: ${unitResponse.status}`);

    const statsJson = await withTimeout(statsResponse.json(), 5000, "stats JSON parse");
    const isMetricText = (await withTimeout(unitResponse.text(), 5000, "metric read")).trim();

    HOME_STATE.data = statsJson;
    HOME_STATE.unit = isMetricText === "1" ? "kilometers" : "miles";
    HOME_STATE.status = "ready";
  } catch (err) {
    HOME_STATE.status = "error";
    HOME_STATE.error = err?.message || String(err);
  }

  renderDiskUsageSection(HOME_STATE);
}

export function Home() {
  setTimeout(() => {
    renderDiskUsageSection(HOME_STATE);
    if (!HOME_STATE.initialized) {
      HOME_STATE.initialized = true;
      initializeHome();
    }
  }, 0);

  return html`<div id="home_shell"><p>Loading...</p></div>`;
}
