#!/usr/bin/env python3
"""Export a commit, flip compile-time switches in the export only, and build it.

Usage:
    python tools/pair_export.py --commit <sha> --flip kSwitch=<true|false|0|1> [--flip ...] \
        --out local\\<name> [--mission-log-hint <mission>] [--no-build]

Steps:
  1. `git archive <commit>` of this repository, unpacked into <out>. On a re-export only the
     files whose bytes differ are rewritten, and files the commit no longer has are removed
     (`build/` is never touched), so the CMake build in <out> stays incremental.
  2. Each flip rewrites exactly one `[static|inline] constexpr bool <name> = true|false;` line in
     the exported bytes before they are compared, so reverting or changing a flip on a later
     export rewrites that file too. A name that is absent, or present more than once in the
     tree, fails before anything is written.
  3. `<out>/scripts/build.ps1` (the export's own copy, so it builds <out>/build/win32).
  4. Prints the SHA-256 prefix of <out>/build/win32/Release/bsp_game.exe and the
     `./tools/run_game.ps1` line that runs it. It never runs the game.

The switch's worktree file is never edited. Reference rows are never taken from an export
with a flip (docs/TOOLING.md).
"""
from __future__ import annotations

import argparse
import hashlib
import io
import json
import re
import subprocess
import sys
import tarfile
import time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
MANIFEST = ".pair_export.json"
SOURCE_SUFFIXES = {".cpp", ".hpp", ".h", ".inl", ".cc", ".cxx", ".hh"}
TRUE_WORDS = {"true": "true", "1": "true", "false": "false", "0": "false"}


def switch_re(name: str) -> re.Pattern:
    return re.compile(
        rb"(?P<head>(?:\b(?:static|inline)\s+)*\bconstexpr\s+bool\s+" + re.escape(name.encode())
        + rb"\s*=\s*)(?P<value>true|false)(?P<tail>\s*;)"
    )


def parse_flips(items):
    flips = {}
    for item in items:
        if "=" not in item:
            raise SystemExit(f"pair_export: --flip needs NAME=VALUE, got {item!r}")
        name, value = item.split("=", 1)
        name, value = name.strip(), value.strip().lower()
        if not re.fullmatch(r"k\w+", name):
            raise SystemExit(f"pair_export: {name!r} does not look like a kSwitch name")
        if value not in TRUE_WORDS:
            raise SystemExit(f"pair_export: {item!r}: value must be true, false, 1 or 0")
        flips[name] = TRUE_WORDS[value]
    return flips


def archive(commit: str) -> dict:
    sha = subprocess.run(["git", "-C", str(REPO), "rev-parse", "--verify", commit + "^{commit}"],
                         capture_output=True, text=True)
    if sha.returncode:
        raise SystemExit(f"pair_export: unknown commit {commit!r}: {sha.stderr.strip()}")
    full = sha.stdout.strip()
    data = subprocess.run(["git", "-C", str(REPO), "archive", "--format=tar", full],
                          capture_output=True, check=True).stdout
    files = {}
    with tarfile.open(fileobj=io.BytesIO(data)) as tar:
        for member in tar.getmembers():
            if member.isfile():
                files[member.name] = tar.extractfile(member).read()
    return {"sha": full, "files": files}


def apply_flips(files: dict, flips: dict) -> list:
    applied = []
    for name, value in flips.items():
        pat = switch_re(name)
        hits = []
        for path, data in files.items():
            if Path(path).suffix.lower() in SOURCE_SUFFIXES:
                for m in pat.finditer(data):
                    line = data.count(b"\n", 0, m.start()) + 1
                    hits.append((path, line, m))
        if len(hits) != 1:
            where = ", ".join(f"{p}:{ln}" for p, ln, _ in hits) or "nowhere"
            raise SystemExit(f"pair_export: `constexpr bool {name} = ...;` must appear exactly once; "
                             f"found {len(hits)} ({where})")
        path, line, m = hits[0]
        old = m.group("value").decode()
        data = files[path]
        files[path] = data[:m.start("value")] + value.encode() + data[m.end("value"):]
        applied.append({"name": name, "file": path, "line": line, "was": old, "now": value})
    return applied


def sync_tree(out: Path, files: dict) -> dict:
    written, same = [], 0
    for rel, data in files.items():
        dest = out / rel
        if dest.is_file() and dest.read_bytes() == data:
            same += 1
            continue
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(data)
        written.append(rel)
    removed = []
    old_manifest = out / MANIFEST
    if old_manifest.is_file():
        previous = json.loads(old_manifest.read_text(encoding="utf-8")).get("files", [])
        for rel in previous:
            if rel not in files and not rel.startswith("build/"):
                target = out / rel
                if target.is_file():
                    target.unlink()
                    removed.append(rel)
    return {"written": written, "unchanged": same, "removed": removed}


def sha256_prefix(path: Path, n: int = 12) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()[:n].upper()


def build(out: Path) -> int:
    script = out / "scripts" / "build.ps1"
    cmd = ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script)]
    print(f"build: {' '.join(cmd)}", flush=True)
    proc = subprocess.run(cmd, cwd=str(out), capture_output=True, text=True, errors="replace")
    log = out / "pair_export_build.log"
    log.write_text(proc.stdout + proc.stderr, encoding="utf-8")
    if proc.returncode:
        tail = (proc.stdout + proc.stderr).strip().splitlines()[-15:]
        print("\n".join("  " + t for t in tail))
        print(f"build: FAILED (exit {proc.returncode}); whole output in {log}")
    else:
        print(f"build: ok; output in {log}")
    return proc.returncode


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="export + flip + incremental build of a commit")
    ap.add_argument("--commit", required=True)
    ap.add_argument("--flip", action="append", default=[], metavar="kName=VALUE")
    ap.add_argument("--out", required=True, help=r"export directory, e.g. local\cs_on")
    ap.add_argument("--mission", default="<mission>", help="mission tag for the printed log name")
    ap.add_argument("--no-build", action="store_true", help="export and flip only")
    args = ap.parse_args(argv)

    flips = parse_flips(args.flip)
    out = Path(args.out)
    if not out.is_absolute():
        out = (REPO / out).resolve()
    try:
        out.relative_to(REPO / "local")
    except ValueError:
        print(f"warning: {out} is outside this worktree's local\\; run_game.ps1 needs a relative -Exe",
              file=sys.stderr)

    exported = archive(args.commit)
    applied = apply_flips(exported["files"], flips)   # fails before anything is written
    out.mkdir(parents=True, exist_ok=True)
    stats = sync_tree(out, exported["files"])
    manifest = {
        "commit": exported["sha"],
        "flips": applied,
        "exported_at": time.strftime("%Y-%m-%dT%H:%M:%S"),
        "files": sorted(exported["files"]),
    }
    (out / MANIFEST).write_text(json.dumps(manifest, indent=1), encoding="utf-8")

    print(f"export: {exported['sha'][:9]} -> {out}")
    print(f"  {len(stats['written'])} written, {stats['unchanged']} unchanged, {len(stats['removed'])} removed")
    for rel in stats["written"][:10]:
        print(f"    wrote {rel}")
    if len(stats["written"]) > 10:
        print(f"    ... {len(stats['written']) - 10} more")
    for f in applied:
        print(f"  flip {f['name']}: {f['was']} -> {f['now']} at {f['file']}:{f['line']}")
    if not applied:
        print("  no flips (a control export)")

    if args.no_build:
        return 0
    started = time.time()
    rc = build(out)
    exe = out / "build" / "win32" / "Release" / "bsp_game.exe"
    if not exe.is_file():
        print("bsp_game.exe: not built")
        return rc or 1
    if rc and exe.stat().st_mtime < started and stats["written"]:
        print("bsp_game.exe: STALE (from an earlier export; this build failed). Do not run it.")
        return rc
    try:
        rel_exe = exe.relative_to(REPO)
    except ValueError:
        rel_exe = exe
    name = out.name
    print(f"bsp_game.exe: {rel_exe}  SHA-256 {sha256_prefix(exe)}  mtime "
          + time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(exe.stat().st_mtime)))
    print("run (from the worktree root, one at a time):")
    print(f"  ./tools/run_game.ps1 -Exe {rel_exe} -Log local\\{name}_{args.mission}.log -- "
          f"--frames <F> --press-start-frame 30 --menu-select <MISSION> --mission-frames <M> "
          f"--mission-frame-seconds 0.05")
    if rc:
        print("note: build.ps1 failed after the executable existed (ctest?); read the build log "
              "before using this binary")
    return rc


if __name__ == "__main__":
    sys.exit(main())
