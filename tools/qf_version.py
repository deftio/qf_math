#!/usr/bin/env python3
"""Read, print, update, and verify qf_math release version metadata."""

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class Version:
    major: int
    minor: int
    patch: int

    @classmethod
    def parse(cls, text: str) -> "Version":
        match = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", text.strip())
        if not match:
            raise SystemExit(f"invalid semantic version: {text!r}")
        return cls(*(int(part) for part in match.groups()))

    @property
    def string(self) -> str:
        return f"{self.major}.{self.minor}.{self.patch}"

    @property
    def tag(self) -> str:
        return f"v{self.string}"

    @property
    def hex_int(self) -> int:
        if max(self.major, self.minor, self.patch) > 255:
            raise SystemExit("version components must fit in one byte for QF_MATH_VERSION_HEX")
        return (self.major << 16) | (self.minor << 8) | self.patch

    @property
    def hex_c(self) -> str:
        return f"0x{self.hex_int:06x}"

    @property
    def hex_report(self) -> str:
        return f"0x{self.hex_int:x}"

    @property
    def badge(self) -> str:
        return self.string.replace("-", "--")


def read_text(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def write_text(rel: str, text: str) -> None:
    (ROOT / rel).write_text(text, encoding="utf-8")


def current_version() -> Version:
    text = read_text("src/qf_math.h")
    match = re.search(r'^#define QF_MATH_VERSION\s+"([^"]+)"', text, re.MULTILINE)
    if not match:
        raise SystemExit("could not find QF_MATH_VERSION in src/qf_math.h")
    return Version.parse(match.group(1))


def replace_one(text: str, pattern: str, replacement: str, path: str) -> str:
    new_text, count = re.subn(pattern, replacement, text, count=1, flags=re.MULTILINE)
    if count != 1:
        raise SystemExit(f"expected one replacement in {path}: {pattern}")
    return new_text


def update_text_files(version: Version) -> None:
    replacements = {
        "src/qf_math.h": [
            (r'^#define QF_MATH_VERSION\s+"[^"]+"', f'#define QF_MATH_VERSION     "{version.string}"'),
            (r"^#define QF_MATH_VERSION_HEX\s+0x[0-9A-Fa-f]+", f"#define QF_MATH_VERSION_HEX  {version.hex_c}"),
        ],
        "README.md": [
            (r"version-[0-9]+\.[0-9]+\.[0-9]+-blue", f"version-{version.badge}-blue"),
        ],
        "docs/API.md": [
            (r'#define QF_MATH_VERSION\s+"[^"]+"', f'#define QF_MATH_VERSION      "{version.string}"'),
            (r"#define QF_MATH_VERSION_HEX\s+0x[0-9A-Fa-f]+", f"#define QF_MATH_VERSION_HEX   {version.hex_c}"),
        ],
        "pages/api.html": [
            (r'#define QF_MATH_VERSION\s+"[0-9]+\.[0-9]+\.[0-9]+"', f'#define QF_MATH_VERSION      "{version.string}"'),
            (r"#define QF_MATH_VERSION_HEX\s+0x[0-9A-Fa-f]+", f"#define QF_MATH_VERSION_HEX   {version.hex_c}"),
        ],
        "library.properties": [
            (r"^version=.*", f"version={version.string}"),
        ],
        "idf_component.yml": [
            (r'^version:\s*"?[0-9]+\.[0-9]+\.[0-9]+"?', f'version: "{version.string}"'),
        ],
        "docs/INTEGRATION.md": [
            (r'version:\s*">=[0-9]+\.[0-9]+\.[0-9]+"', f'version: ">={version.string}"'),
        ],
        "pages/integration.html": [
            (r'version:\s+"(?:&gt;=|>=)[0-9]+\.[0-9]+\.[0-9]+"', f'version: "&gt;={version.string}"'),
        ],
        "llms.txt": [
            (
                r"Current revision: `[0-9]+\.[0-9]+\.[0-9]+` \(`QF_MATH_VERSION_HEX` = `0x[0-9A-Fa-f]+`\)",
                f"Current revision: `{version.string}` (`QF_MATH_VERSION_HEX` = `{version.hex_c}`)",
            ),
        ],
        "agents.md": [
            (
                r"Version is currently `[0-9]+\.[0-9]+\.[0-9]+` \(`QF_MATH_VERSION_HEX` = `0x[0-9A-Fa-f]+`\)",
                f"Version is currently `{version.string}` (`QF_MATH_VERSION_HEX` = `{version.hex_c}`)",
            ),
        ],
    }

    for rel, rules in replacements.items():
        text = read_text(rel)
        for pattern, replacement in rules:
            text = replace_one(text, pattern, replacement, rel)
        write_text(rel, text)

    version_json = ROOT / "pages" / "version.json"
    if version_json.exists():
        version_json.write_text(json.dumps({"version": version.string}) + "\n", encoding="utf-8")


def update_library_json(version: Version) -> None:
    path = ROOT / "library.json"
    data = json.loads(path.read_text(encoding="utf-8"))
    data["version"] = version.string
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def version_payload(version: Version) -> dict[str, str | int]:
    return {
        "version": version.string,
        "tag": version.tag,
        "hex_c": version.hex_c,
        "hex_int": version.hex_int,
        "hex_report": version.hex_report,
        "readme_badge": f"https://img.shields.io/badge/version-{version.badge}-blue.svg",
        "platformio": version.string,
        "esp_idf_component": version.string,
    }


def print_version(version: Version, fmt: str) -> None:
    payload = version_payload(version)
    if fmt == "json":
        print(json.dumps(payload, indent=2, sort_keys=True))
    elif fmt == "shell":
        for key, value in payload.items():
            print(f"QF_{key.upper()}={json.dumps(str(value))}")
    elif fmt == "markdown":
        print(f"- Version: `{version.string}`")
        print(f"- Tag: `{version.tag}`")
        print(f"- C hex: `{version.hex_c}`")
        print(f"- PlatformIO: `{version.string}`")
        print(f"- ESP-IDF component: `{version.string}`")
    else:
        raise SystemExit(f"unknown format: {fmt}")


def verify(version: Version) -> None:
    expected = {
        "src/qf_math.h": [f'"{version.string}"', version.hex_c],
        "README.md": [f"version-{version.badge}-blue"],
        "docs/API.md": [f'"{version.string}"', version.hex_c],
        "library.properties": [f"version={version.string}"],
        "library.json": [f'"version": "{version.string}"'],
        "idf_component.yml": [f'version: "{version.string}"'],
        "llms.txt": [f"Current revision: `{version.string}`", f"`{version.hex_c}`"],
        "agents.md": [f"Version is currently `{version.string}`", f"`{version.hex_c}`"],
    }
    optional_expected = {
        "pages/api.html": [f'QF_MATH_VERSION      "{version.string}"', version.hex_c],
        "pages/version.json": [f'"version": "{version.string}"'],
    }

    problems: list[str] = []
    for rel, needles in {**expected, **optional_expected}.items():
        path = ROOT / rel
        if not path.exists():
            if rel in expected:
                problems.append(f"missing {rel}")
            continue
        text = path.read_text(encoding="utf-8")
        for needle in needles:
            if needle not in text:
                problems.append(f"{rel} missing {needle!r}")

    text = read_text("src/qf_math.h")
    hex_match = re.search(r"^#define QF_MATH_VERSION_HEX\s+(0x[0-9A-Fa-f]+)", text, re.MULTILINE)
    if not hex_match or int(hex_match.group(1), 16) != version.hex_int:
        problems.append("src/qf_math.h hex macro does not match version")

    if problems:
        raise SystemExit("version verification failed:\n- " + "\n- ".join(problems))
    print(f"Version metadata verified for {version.string} ({version.hex_c})")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="cmd", required=True)

    show = sub.add_parser("show", help="print current version metadata")
    show.add_argument("--format", choices=("json", "shell", "markdown"), default="json")

    update = sub.add_parser("update", help="update all versioned source/docs/package files")
    update.add_argument("version")

    verify_cmd = sub.add_parser("verify", help="verify all versioned files match")
    verify_cmd.add_argument("version", nargs="?")

    args = parser.parse_args()
    if args.cmd == "show":
        print_version(current_version(), args.format)
    elif args.cmd == "update":
        version = Version.parse(args.version)
        update_text_files(version)
        update_library_json(version)
        verify(version)
    elif args.cmd == "verify":
        version = Version.parse(args.version) if args.version else current_version()
        verify(version)


if __name__ == "__main__":
    main()
