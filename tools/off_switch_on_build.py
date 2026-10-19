"""Build the tree with OFF constexpr switches flipped ON, then restore them.

An OFF switch's ON arm is never compiled until someone flips it, so a flip can
fail on warnings-as-errors the OFF build never sees (C4702 after an early
return in an `if constexpr`, C4189, ...). This tool flips a set of switches ON
in the working tree, runs scripts/build.ps1, restores every touched file and
prints the compiler and linker diagnostics by file and line.

Usage (from the tree root):
    python tools/off_switch_on_build.py              # cc13 switches, build, restore
    python tools/off_switch_on_build.py --list       # show the set, touch nothing
    python tools/off_switch_on_build.py --pattern Goaway
    python tools/off_switch_on_build.py --names kRocketTaskBound kLuaSpawnBound
    python tools/off_switch_on_build.py --restore    # after a killed run

Selection: every `constexpr bool kName = false` definition under include/ and
src/ is a candidate. The default set is the names containing Cc13 plus every
switch named in docs/CC13_*.md; --pattern (regex on the name) and --names
select from all candidates instead. Parents are added from static_asserts of
the form `static_assert(!kChild || kParent, ...)` until the set is closed.

Safety: the original bytes of each touched file are copied to
local/off_switch_on_build/ before any edit and written back in a finally block,
so a failed build or Ctrl+C still restores the tree. If a backup is present at
start, the previous run was killed hard: the tool refuses to run until
--restore puts the backed-up files back. Exit status: 0 when the build passes,
1 when it fails, 2 on a usage or restore problem.
"""
from __future__ import annotations

import argparse
import json
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
BACKUP = ROOT / "local" / "off_switch_on_build"
MANIFEST = BACKUP / "manifest.json"
SOURCE_GLOBS = ("include/**/*.hpp", "include/**/*.h", "src/**/*.cpp", "src/**/*.hpp", "src/**/*.h")
DEFINITION = re.compile(r"constexpr bool (k\w+)\s*=\s*false\b")
PARENT = re.compile(r"static_assert\(\s*!\s*(k\w+)\s*\|\|\s*(k\w+)")
DOC_NAME = re.compile(r"\bk[A-Z]\w*(?:Bound|Enabled)\b")
DIAGNOSTIC = re.compile(
    r"^\s*(?P<file>[^\s(][^(]*?)\((?P<line>\d+)(?:,\d+)?\)\s*:\s*"
    r"(?P<kind>fatal error|error|warning) (?P<code>[A-Z]+\d+)\s*:\s*(?P<text>.*?)\s*(?:\[[^\]]*\])?$")
LINKER = re.compile(r"^\s*(?P<file>\S+)\s*:\s*(?P<kind>fatal error|error|warning) (?P<code>LNK\d+)\s*:\s*(?P<text>.*?)\s*(?:\[[^\]]*\])?$")


def source_files() -> list[pathlib.Path]:
    seen: dict[pathlib.Path, None] = {}
    for pattern in SOURCE_GLOBS:
        for path in ROOT.glob(pattern):
            seen[path] = None
    return sorted(seen)


def read(path: pathlib.Path) -> str:
    return path.read_bytes().decode("utf-8", errors="surrogateescape")


def scan(files: list[pathlib.Path]) -> tuple[dict[str, list[pathlib.Path]], dict[str, set[str]]]:
    """OFF definitions by name, and static_assert parents by child."""
    definitions: dict[str, list[pathlib.Path]] = {}
    parents: dict[str, set[str]] = {}
    for path in files:
        text = read(path)
        for m in DEFINITION.finditer(text):
            definitions.setdefault(m.group(1), [])
            if path not in definitions[m.group(1)]:
                definitions[m.group(1)].append(path)
        for m in PARENT.finditer(text):
            parents.setdefault(m.group(1), set()).add(m.group(2))
    return definitions, parents


def doc_names() -> set[str]:
    names: set[str] = set()
    for doc in (ROOT / "docs").glob("CC13_*.md"):
        names.update(DOC_NAME.findall(read(doc)))
    return names


def select(args, definitions: dict[str, list[pathlib.Path]], parents: dict[str, set[str]]) -> list[str]:
    if args.names:
        missing = [n for n in args.names if n not in definitions]
        if missing:
            print("not an OFF switch definition: " + ", ".join(missing), file=sys.stderr)
            sys.exit(2)
        chosen = set(args.names)
    elif args.pattern:
        chosen = {n for n in definitions if re.search(args.pattern, n)}
    else:
        named = doc_names()
        chosen = {n for n in definitions if "Cc13" in n or n in named}
    # Close over the parents an ON child requires.
    pending = list(chosen)
    while pending:
        for parent in parents.get(pending.pop(), ()):
            if parent in definitions and parent not in chosen:
                chosen.add(parent)
                pending.append(parent)
    return sorted(chosen)


def restore(originals: dict[pathlib.Path, bytes]) -> None:
    for path, data in originals.items():
        path.write_bytes(data)
    bad = [p for p, data in originals.items() if p.read_bytes() != data]
    if bad:
        raise SystemExit("restore FAILED, copies kept in %s: %s" % (BACKUP, ", ".join(map(str, bad))))
    shutil.rmtree(BACKUP, ignore_errors=True)


def restore_from_backup() -> int:
    if not MANIFEST.exists():
        print("no backup at %s; nothing to restore" % BACKUP)
        return 0
    entries = json.loads(MANIFEST.read_text(encoding="utf-8"))
    originals = {ROOT / rel: (BACKUP / "files" / rel).read_bytes() for rel in entries}
    restore(originals)
    print("restored %d files from %s" % (len(originals), BACKUP))
    return 0


def flip(names: list[str], definitions: dict[str, list[pathlib.Path]],
         originals: dict[pathlib.Path, bytes]) -> int:
    """Fills `originals` before the first edit, so the caller can always restore."""
    files = sorted({p for n in names for p in definitions[n]})
    originals.update({p: p.read_bytes() for p in files})
    # Back up to disk before the first edit, so a hard kill can be undone.
    (BACKUP / "files").mkdir(parents=True, exist_ok=True)
    for path, data in originals.items():
        rel = path.relative_to(ROOT)
        target = BACKUP / "files" / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    MANIFEST.write_text(json.dumps([str(p.relative_to(ROOT)) for p in files], indent=1), encoding="utf-8")
    wanted = set(names)
    sites = 0
    for path in files:
        text = read(path)

        def on(m: re.Match) -> str:
            nonlocal sites
            if m.group(1) not in wanted:
                return m.group(0)
            sites += 1
            return m.group(0)[: -len("false")] + "true"

        path.write_bytes(DEFINITION.sub(on, text).encode("utf-8", errors="surrogateescape"))
    return sites


def build(extra: list[str]) -> tuple[int, str]:
    shell = shutil.which("pwsh") or shutil.which("powershell")
    if shell is None:
        raise SystemExit("no PowerShell on PATH")
    command = [shell, "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
               str(ROOT / "scripts" / "build.ps1"), *extra]
    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, errors="replace")
    return run.returncode, run.stdout + run.stderr


def report(log: str) -> int:
    found: dict[tuple, None] = {}
    for line in log.splitlines():
        m = DIAGNOSTIC.match(line) or LINKER.match(line)
        if m:
            d = m.groupdict()
            found[(d["file"].strip(), int(d.get("line") or 0), d["kind"], d["code"], d["text"])] = None
    by_file: dict[str, list[tuple]] = {}
    for key in found:
        by_file.setdefault(key[0], []).append(key)
    for file in sorted(by_file):
        print(file)
        for _, line, kind, code, text in sorted(by_file[file], key=lambda k: k[1]):
            where = "(%d)" % line if line else ""
            print("  %s %s %s: %s" % (where, kind, code, text[:200]))
    return len(found)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--pattern", help="regex on switch names, over every OFF definition")
    parser.add_argument("--names", nargs="+", help="explicit switch names")
    parser.add_argument("--list", action="store_true", help="print the set and exit")
    parser.add_argument("--restore", action="store_true", help="put back a killed run's backup")
    parser.add_argument("--log", default="local/off_switch_on_build.log", help="full build log path")
    parser.add_argument("build_args", nargs="*", help="passed to build.ps1 after --")
    args = parser.parse_args()

    if args.restore:
        return restore_from_backup()
    if MANIFEST.exists():
        print("a previous run left a backup in %s; run with --restore first" % BACKUP, file=sys.stderr)
        return 2

    definitions, parents = scan(source_files())
    names = select(args, definitions, parents)
    if not names:
        print("no OFF switches selected")
        return 2
    print("%d switches:" % len(names))
    for n in names:
        print("  %s  %s" % (n, ", ".join(str(p.relative_to(ROOT)) for p in definitions[n])))
    if args.list:
        return 0

    originals: dict[pathlib.Path, bytes] = {}
    try:
        sites = flip(names, definitions, originals)
        print("flipped %d sites in %d files; building" % (sites, len(originals)), flush=True)
        code, log = build(args.build_args)
    finally:
        if originals:
            restore(originals)
            print("restored %d files" % len(originals))
    log_path = ROOT / args.log
    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.write_text(log, encoding="utf-8")
    count = report(log)
    print("build.ps1 exit %d; %d distinct diagnostics; log %s" % (code, count, args.log))
    return 0 if code == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
