# USB and comma resource benchmarks

Run these with the comma parked and offroad. The recording bench uses private
Params and messaging, leaves the gadget owner running, and never starts controls
or pandad. A real ignition change stops it. Camera processes must not already be
running. Use a checkout of the candidate package; the bench imports that package
even when the installed openpilot checkout contains another Jetlink revision.

## Recording workload

From the comma's openpilot checkout:

```sh
OUTPUT=/data/tmp/jetlink-recording-test \
  jetlink/scripts/comma/jetlink_live_bench.sh 180 --record --resources
```

Set `OPENPILOT=/data/openpilot` when staging the Jetlink checkout elsewhere.
The output directory must not exist. The bench stops before free disk space
falls below 2 GiB; recordings remain in the output directory for inspection.

The bench runs the modeld manager would start for the active driving model:
modeld_v2 for a sunnypilot (tinygrad) bundle, stock modeld for openpilot's own;
`--modeld stock` or `--modeld tinygrad` chooses. It publishes deviceState and
the calibration at the car's rates (2 and 4 Hz): modeld recomputes both warp
matrices on every calibration, 0.55 ms of its frame.

`--record` runs camerad, modeld, driver monitoring, encoderd, loggerd and
logmessaged. Python logs and video files stay under the output directory.
A run of at least 150 seconds must produce two complete recording segments,
each with an rlog and all three camera streams. Inspect `summary.json`,
`frames.csv` and the per-process logs; the summary includes startup and separate
small/large-model statistics, not just the last few frames.

`--resources` samples the child processes once a second into `resources.jsonl`:
CPU ticks, faults, per-thread scheduling and context switches, VM counters,
memory totals and free page orders. PSS is sampled every ten seconds from
`smaps` (AGNOS's 4.9 kernel has no `smaps_rollup`). The first line records the
sysctls the link tunes, so a run shows whether they were applied. `sampler_ms`
records collection cost. Convert CPU tick deltas using the recorded clock frequency and elapsed
monotonic time; 100% means one whole core. Compare matched steady-state windows.

`--write-chunk 8192` or `16384` overrides the candidate's AIO request size. An unchanged older checkout can override this internally, so check its
code and actual send diagnostics before calling it a small-write baseline.
`--small` measures the small model. Keep model, host, recording and power settings
matched when comparing results.

## Byte integrity under signals

The separate diagnostic replaces inference with a SHA-256 reply. It checks every
payload byte, sequence and padding boundary. Frame and chunk positions are
stamped into random data, so replaying a previous chunk changes the digest.
The comma's main thread also receives frequent SIGUSR2 signals. Synchronous
FunctionFS writes replayed bytes when a signal interrupted one; the AIO writes
never wait inside a transfer, and this is how that is checked.

The host needs Python's `libusb1` package and native libusb. Stop the normal
server so it cannot claim the interface, and arrange to restart it even if the
test fails. On a Linux host with the installed service, for example:

```sh
bash -c 'trap "sudo systemctl start jetlink-server.service" EXIT
  sudo systemctl stop jetlink-server.service
  sudo timeout 1800 python3 scripts/comma/jetlink_usb_integrity.py --host --frames 100000'
```

Within 45 seconds, run the same checkout's diagnostic on the comma:

```sh
PYTHONPATH=/data/openpilot /usr/local/venv/bin/python3 \
  jetlink/scripts/comma/jetlink_usb_integrity.py --frames 100000
```

The comma borrows the gadget from its owner and releases it on completion or
exception. No modeld may run alongside it. Match `--frames` and `--payload-bytes`
on both ends; the default payload is 393,216 bytes. This test is intentionally
separate from timing runs: hashing, signal generation and synchronous Python
host reads change the workload. It does not establish inference-output parity.

## What the fix changes and what still needs qualification

FunctionFS in the AGNOS 4.9 kernel allocates a contiguous buffer for each
request. Sending a complete inference as one request needs an order-7
allocation, which recording rollover leaves none of. A synchronous write also
holds the endpoint's only request until the host has every byte, so smaller
writes leave the bus idle between them (+1.4 ms p50 at 32 KiB).

Each message now goes out in one `io_submit` of 8 KiB requests. The kernel
copies the bytes at submit and dwc3 streams the queued requests back to back.
8 KiB is the largest size SLUB serves from a slab cache, so no request ever
waits on compaction: 32 KiB and 16 KiB AIO requests stalled up to 94 ms and
23.6 ms when the comma had few free order-3 and order-2 blocks. No write waits
inside a transfer, so there is no write watchdog and no signal mask. A host
that stops taking data fills a 1 MiB budget: a frame is then held, not sent,
and other messages wait under their deadline before the gadget is dropped.

`jetlinkSend` diagnostics are logged once a second and the failed send is logged
on error. `last_send` describes one message: its bytes, requests, time in
`io_submit` and waiting for room, and what the host had not yet taken when it
came (`backlog_kb`, `backlog_ms`). `totals` retains counts and maxima for the
whole transport session.

Smaller receive reserves save 256 KiB per transport and lower the queue ceiling
from 8 MiB to 512 KiB. This is not a claim of a 7.5 MiB steady-state PSS saving.
Frames go out from the warp's output, which lives in IO-coherent GPU memory the
CPU reads through its cache, on the USB and the phone cable paths alike.

Before promoting a fork pin, qualify the reporter's device and host, USB 2/3,
uploads and model geometries; run recording/memory-pressure soaks and repeated
reconnects; test host stalls and cable loss; compare complete-run latency, CPU
and PSS. A clean short recording run or a digest-only test cannot replace those
checks.

AIO with 8 KiB requests, 120 s against a synchronous 32 KiB control run back to
back on the same bench: p50 23.95 against 24.85 ms, p99.9 26.61 against 30.25 ms,
maximum 33.21 against 35.98 ms, no held frames, and modeld at 21.9% against 21.4%
of one core.

With the coherent warp output and the replayed warp (2026-10-07), a settled
180 s recording run: p50 21.07 ms, p99 21.51 ms, p99.9 23.11 ms, no held frames,
modeld at 16.3% of one core. `vm.extra_free_kbytes` at 32 MiB took a 180 s
recording run from 49 direct-reclaim stalls (the longest 148 ms) to none.
FunctionFS's debug log turned off took the frame's `io_submit` from 1.20 to
0.88 ms and frames from 21.14 to 20.86 ms at p50. The byte-integrity run passed
100,000 frames with 187,842 signals and no mismatched byte.
