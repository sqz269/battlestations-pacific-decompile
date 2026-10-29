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
        if not root.exists():
            result[key] = {"exists": False, "files": {}}
            continue
        paths = []
        if root.is_file():
            paths = [root]
        else:
            for folder, directories, files in os.walk(root, followlinks=False):
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
    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    raise SystemExit(main())
