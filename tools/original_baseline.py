#!/usr/bin/env python3
"""Record provenance and user-data preservation for an original-game baseline.

This tool never launches the game or changes the source installation. Evidence
is written under this worktree's ignored local/ directory. A separate guarded
launcher and motion sampler perform the live capture.
"""
from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import stat
import struct
import subprocess
import time
from datetime import datetime, timezone


ROOT = Path(__file__).resolve().parents[1]


def evidence_path(value: str) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = ROOT / path
    path = path.resolve()
    if not path.is_relative_to((ROOT / "local").resolve()):
        raise ValueError("baseline evidence must stay inside this worktree's local/")
    path.parent.mkdir(parents=True, exist_ok=True)
    return path


def write_json(path: Path, value: dict) -> None:
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def stamp() -> str:
    return datetime.now(timezone.utc).isoformat()


def file_record(path: Path) -> dict:
    before = path.stat()
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    after = path.stat()
    if (before.st_size, before.st_mtime_ns) != (after.st_size, after.st_mtime_ns):
        raise RuntimeError(f"file changed during inventory: {path}")
    return {"size": after.st_size, "mtime_ns": after.st_mtime_ns,
            "sha256": digest.hexdigest()}


def documents_folder() -> Path:
    if os.name != "nt":
        raise RuntimeError("Windows known-folder lookup is required")
    folder = ctypes.create_unicode_buffer(32768)
    hr = ctypes.windll.shell32.SHGetFolderPathW(None, 5, None, 0, folder)
    if hr != 0:
        raise OSError(f"SHGetFolderPathW(CSIDL_PERSONAL) failed: {hr:#x}")
    return Path(folder.value).resolve()


def protected_files(roots: list[Path]) -> dict:
    result = {}
    for root in roots:
        key = str(root)
        try:
            root_info = root.lstat()
        except FileNotFoundError:
            result[key] = {"exists": False, "files": {}}
            continue
        if root_info.st_file_attributes & stat.FILE_ATTRIBUTE_REPARSE_POINT:
            raise ValueError(f"unexpected protected-root reparse point: {root}")
        paths = []
        if root.is_file():
            paths = [root]
        else:
            def refuse_unreadable(error):
                raise error
            for folder, directories, files in os.walk(root, followlinks=False,
                                                      onerror=refuse_unreadable):
                for name in directories + files:
                    entry = Path(folder) / name
                    if entry.lstat().st_file_attributes & stat.FILE_ATTRIBUTE_REPARSE_POINT:
                        raise ValueError(f"unexpected reparse point in protected data: {entry}")
                paths.extend(Path(folder) / name for name in files)
        result[key] = {"exists": True, "files": {
            str(path.relative_to(root)) if root.is_dir() else path.name: file_record(path)
            for path in sorted(paths)}}
    return result


def inventory(args: argparse.Namespace) -> int:
    install = Path(args.game_root).resolve(strict=True)
    exe = install / "battlestationspacific.exe"
    with exe.open("rb") as stream:
        dos = stream.read(64)
        if dos[:2] != b"MZ":
            raise ValueError("original executable has no DOS header")
        stream.seek(struct.unpack_from("<I", dos, 60)[0])
        pe = stream.read(56)
    if pe[:4] != b"PE\0\0" or struct.unpack_from("<H", pe, 4)[0] != 0x14C:
        raise ValueError("expected the original x86 PE executable")
    runtime = [p for p in install.iterdir() if p.is_file() and
               p.suffix.lower() in {".dll", ".cfg"}]
    runtime += [exe, install / "gamemode.lua",
                install / "scripts/datatables/shipglobals.lua"]
    roots = [documents_folder() / "Battlestations-Pacific", install / "profiles"]
    roots.extend(Path(p).resolve() for p in args.protect)
    output = evidence_path(args.output)
    value = {"schema": 1, "recorded_at": stamp(), "game_root": str(install),
             "git_commit": subprocess.check_output(
                 ["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
             "executable": str(exe),
             "pe": {"machine": "x86", "image_base": hex(struct.unpack_from("<I", pe, 52)[0]),
                    "entry_rva": hex(struct.unpack_from("<I", pe, 40)[0])},
             "runtime": {str(p.relative_to(install)): file_record(p)
                         for p in sorted(set(runtime)) if p.is_file()},
             "protected": protected_files(roots),
             "limits": ["Installed runtime may contain gameplay mods and replacement DLLs.",
                        "Hashes establish provenance, not original-game fidelity."]}
    write_json(output, value)
    print(json.dumps({"output": str(output), "exe_sha256": value["runtime"][exe.name]["sha256"],
                      "runtime_files": len(value["runtime"]),
                      "protected_files": sum(len(v["files"]) for v in value["protected"].values())}))
    return 0


def verify(args: argparse.Namespace) -> int:
    before = json.loads(Path(args.before).read_text(encoding="utf-8"))
    after = protected_files([Path(p) for p in before["protected"]])
    changes = []
    for root, old in before["protected"].items():
        new = after[root]
        if old["exists"] != new["exists"]:
            changes.append({"root": root, "change": "existence"})
        for name in sorted(old["files"].keys() | new["files"].keys()):
            if old["files"].get(name) != new["files"].get(name):
                changes.append({"root": root, "file": name, "change": "file identity or timestamp"})
    output = evidence_path(args.output)
    write_json(output, {"schema": 1, "recorded_at": stamp(), "before": str(Path(args.before).resolve()),
                        "unchanged": not changes, "changes": changes, "protected": after})
    print(json.dumps({"output": str(output), "unchanged": not changes, "changes": len(changes)}))
    return int(bool(changes))


def event(args: argparse.Namespace) -> int:
    output = evidence_path(args.output)
    with output.open("a", encoding="utf-8") as stream:
        stream.write(json.dumps({"utc": stamp(), "monotonic": time.monotonic(),
                                 "mission": args.mission, "event": args.event,
                                 "details": args.details}) + "\n")
    print(json.dumps({"output": str(output), "event": args.event}))
    return 0


def native_options(args: argparse.Namespace) -> int:
    """Normalize only the private copy for the original text-mode reader."""
    stage = evidence_path(args.stage).resolve(strict=True)
    preparation = json.loads((stage / "preparation.json").read_text(encoding="utf-8-sig"))
    if Path(preparation["stage_root"]).resolve() != stage:
        raise ValueError("preparation stage does not match")
    expected = stage / "Documents" / "Battlestations-Pacific" / "options.txt"
    target = expected.resolve(strict=True)
    if target != expected or target.stat().st_nlink != 1:
        raise ValueError("private options must be an independent ordinary copy")
    matches = [r for r in preparation["profile_files_sha256"] if r["path"].lower() == "options.txt"]
    if len(matches) != 1:
        raise ValueError("preparation must contain one original options identity")
    baseline = matches[0]
    source = Path(preparation["source_profile"]) / "options.txt"
    original = source.read_bytes()
    if hashlib.sha256(original).hexdigest() != baseline["sha256"] or len(original) != baseline["length"]:
        raise ValueError("original options identity changed; take a fresh inventory")
    receipt_path = stage / "native-options.json"
    if receipt_path.exists():
        receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
        if (receipt["source_record"] != baseline or
                file_record(target)["sha256"] != receipt["private_record"]["sha256"]):
            raise ValueError("existing native-options receipt does not match")
        print(json.dumps({"output": str(receipt_path), "status": "already_normalized"}))
        return 0
    if target.read_bytes() != original:
        raise ValueError("private options differ from the recorded source before normalization")
    lines, omitted = [], []
    for line in original.decode("ascii").splitlines():
        key = line.split(maxsplit=1)[0] if line.split() else ""
        if key in {"SoundEnabled", "HardwareReported"}:
            omitted.append(line)
        else:
            lines.append(line)
    normalized = ("\n".join(lines) + "\n").encode("ascii")
    if b"\0" in normalized or b"\r" in normalized:
        raise ValueError("native options must be LF-only with no NUL")
    backup = stage / "options-before-native-normalization.txt"
    with backup.open("xb") as stream:
        stream.write(original)
    target.write_bytes(normalized)
    receipt = {"schema": "bsp.original-baseline.native-options.v1", "recorded_at": stamp(),
               "stage_root": str(stage), "source_path": str(source), "private_path": str(target),
               "source_record": baseline,
               "private_record": {"path": "options.txt", "length": len(normalized),
                                  "sha256": hashlib.sha256(normalized).hexdigest()},
               "omitted_lines": omitted, "encoding": "ASCII", "line_endings": "LF", "nul_terminated": False,
               "evidence": "008D8190 uses physical ftell size with text fread and ignores its count; reader ignores the two native-writer fields",
               "original_unchanged": source.read_bytes() == original}
    write_json(receipt_path, receipt)
    if not receipt["original_unchanged"]:
        raise RuntimeError("original options changed during private normalization")
    print(json.dumps({"output": str(receipt_path), "private_bytes": len(normalized),
                      "source_bytes": len(original), "original_unchanged": True}))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    command = commands.add_parser("inventory")
    command.add_argument("--game-root", required=True)
    command.add_argument("--protect", action="append", default=[])
    command.add_argument("--output", default="local/original-baseline/before.json")
    command.set_defaults(run=inventory)
    command = commands.add_parser("verify")
    command.add_argument("--before", required=True)
    command.add_argument("--output", default="local/original-baseline/after.json")
    command.set_defaults(run=verify)
    command = commands.add_parser("event")
    command.add_argument("--mission", default="USN04")
    command.add_argument("--event", required=True)
    command.add_argument("--details", default="")
    command.add_argument("--output", default="local/original-baseline/events.jsonl")
    command.set_defaults(run=event)
    command = commands.add_parser("native-options")
    command.add_argument("--stage", required=True)
    command.set_defaults(run=native_options)
    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    raise SystemExit(main())
