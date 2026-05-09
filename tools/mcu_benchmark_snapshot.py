#!/usr/bin/env python3
"""
Flash examples/lilygo_t_display_s3_bench and rewrite compare/MCU_BENCHMARK_SNAPSHOT.md
from UART output between DOC_TABLE markers.

Requires: PlatformIO (pio), pyserial, compare deps (make compare-deps).
Optional env: MCU_SERIAL_PORT=/dev/cu.usbmodemXXXX
"""

from __future__ import annotations

import argparse
import glob
import os
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

try:
    import serial
except ImportError:
    print("error: install pyserial (e.g. pip install pyserial)", file=sys.stderr)
    sys.exit(2)

START = ":::: DOC_TABLE_START ::::"
END = ":::: DOC_TABLE_END ::::"


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def guess_serial_port() -> str | None:
    env = os.environ.get("MCU_SERIAL_PORT", "").strip()
    if env:
        return env
    patterns = (
        "/dev/cu.usbmodem*",
        "/dev/cu.usbserial*",
        "/dev/tty.usbmodem*",
        "/dev/ttyUSB*",
        "/dev/ttyACM*",
    )
    for pat in patterns:
        found = sorted(glob.glob(pat))
        if found:
            # Prefer cu.* on macOS (callout device for reading)
            return found[0]
    return None


def wait_for_serial_port(preferred: str | None, timeout_sec: float = 30.0) -> str | None:
    deadline = time.monotonic() + timeout_sec
    while time.monotonic() < deadline:
        if preferred and Path(preferred).exists():
            return preferred
        guessed = guess_serial_port()
        if guessed:
            return guessed
        time.sleep(0.25)
    return preferred if preferred and Path(preferred).exists() else None


def run_upload(project: Path, port: str) -> None:
    cmd = [
        "pio",
        "run",
        "-d",
        str(project),
        "-t",
        "upload",
        "--upload-port",
        port,
    ]
    print("+", " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=repo_root(), check=True)


def capture_markdown(port: str, timeout_sec: float) -> str:
    print(f"+ serial {port} @ 115200 (timeout {timeout_sec:.0f}s)", flush=True)
    ser = serial.Serial(port, 115200, timeout=0.25)
    try:
        ser.reset_input_buffer()
    except Exception:
        pass

    buf = ""
    t_end = time.monotonic() + timeout_sec
    while time.monotonic() < t_end:
        chunk = ser.read(8192)
        if chunk:
            buf += chunk.decode("utf-8", errors="replace")
            if START in buf and END in buf:
                break
        time.sleep(0.02)
    ser.close()

    if START not in buf or END not in buf:
        raise RuntimeError(
            f"markers not found after {timeout_sec:.0f}s — got {len(buf)} chars (check port / baud / firmware)"
        )

    a = buf.index(START) + len(START)
    b = buf.index(END, a)
    body = buf[a:b].strip()
    body = body.strip("\r\n")
    bstrip = body.lstrip()
    if bstrip.startswith("<!--"):
        endcom = bstrip.find("-->")
        if endcom != -1:
            body = bstrip[endcom + 3 :].lstrip()
    return body


DOC_HEADER = """# MCU benchmark snapshot

_Generated:_ **{timestamp}** (`tools/mcu_benchmark_snapshot.py` — LilyGO / ESP32-S3 Arduino bench).

Tables match [`BENCHMARK_REPORT.md`](BENCHMARK_REPORT.md) sections (accuracy %, wall-clock microseconds, speed vs libm). Loop shape and grids match host **`make compare`** (`benchmark_core.c`). MCU timing uses **`esp_timer_get_time()`**.

Firmware: [`examples/lilygo_t_display_s3_bench/`](../examples/lilygo_t_display_s3_bench/README.md).

Regenerate:

```bash
make compare-deps
make mcu-benchmark-snapshot
# or: MCU_SERIAL_PORT=/dev/cu.usbmodem2101 python3 tools/mcu_benchmark_snapshot.py
```

---
"""


def write_snapshot(out_path: Path, body_md: str) -> None:
    ts = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%SZ")
    text = DOC_HEADER.format(timestamp=ts) + "\n" + body_md.rstrip() + "\n"
    out_path.write_text(text, encoding="utf-8")
    print(f"wrote {out_path}", flush=True)


def main() -> int:
    ap = argparse.ArgumentParser(description="Capture MCU benchmark Markdown via UART")
    ap.add_argument(
        "--project",
        default="examples/lilygo_t_display_s3_bench",
        help="PlatformIO project directory",
    )
    ap.add_argument("--port", default=None, help="Serial device (default: guess or MCU_SERIAL_PORT)")
    ap.add_argument(
        "--out",
        default="compare/MCU_BENCHMARK_SNAPSHOT.md",
        help="Output Markdown path (repo-relative)",
    )
    ap.add_argument(
        "--skip-upload",
        action="store_true",
        help="Only read serial (device must already be running the bench firmware)",
    )
    ap.add_argument(
        "--timeout",
        type=float,
        default=2700.0,
        help="Seconds to wait for DOC_TABLE markers after upload",
    )
    args = ap.parse_args()

    root = repo_root()
    proj = (root / args.project).resolve()
    out = (root / args.out).resolve()

    third = root / "build/compare/third_party/libfixmath/libfixmath/fix16.c"
    if not third.is_file():
        print("error: missing compare deps — run: make compare-deps", file=sys.stderr)
        return 1

    port = args.port or guess_serial_port()
    if not port:
        print(
            "error: no serial port — set MCU_SERIAL_PORT or plug board (USB JTAG/serial)",
            file=sys.stderr,
        )
        return 1

    if not Path(port).exists():
        print(f"error: port does not exist: {port}", file=sys.stderr)
        return 1

    try:
        if not args.skip_upload:
            run_upload(proj, port)
            port_after_reset = wait_for_serial_port(port, 30.0)
            if not port_after_reset:
                raise RuntimeError("serial port did not reappear after upload/reset")
            port = port_after_reset
            time.sleep(2.0)

        body = capture_markdown(port, args.timeout)
        # Sanity: expect markdown tables
        if "### MCU benchmark snapshot" not in body and "### Accuracy" not in body:
            print("warning: unexpected capture payload (missing expected headings)", file=sys.stderr)

        write_snapshot(out, body)
    except subprocess.CalledProcessError as e:
        print(f"upload failed (exit {e.returncode})", file=sys.stderr)
        return e.returncode or 1
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
