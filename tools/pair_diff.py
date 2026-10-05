#!/usr/bin/env python3
"""Compare two bsp_game run logs of a same-tree pair (OFF against ON).

Usage:
    python tools/pair_diff.py <off.log> <on.log> [--json out.json] [--limit N]

What it compares, in the order the report prints it:
  1. the run parameters (the milestone header line and the frame-jitter line);
  2. the mission-clock offset between the runs, from the first common `gunnery step`
     line (its `t=` at the same step number), with the first-hit delta beside it;
  3. the gameplay rows: deaths, hit records (hull), damage, shots, first hit, torpedo-task
     and dive-bomb-task releases, torpedo drops, plane water contacts, controlled unit
     moved, units, mission end / failure time;
  4. the per-entity tables row by row: `death row`, `plane death mode`, and the end-of-run
     unit table (`unit side guns ... sunk_at killed_by`), with the clock offset
     subtracted from every time column before comparing;
  5. the whole native call table (the rows after `host methods N concrete, M unimplemented`)
     both ways: rows added, removed, status changed, calls moved;
  6. every `summary` line, keyed by its leading words and occurrence;
  7. every other line, compared as a multiset of masked lines (order-insensitive).

Noise is masked or ignored before any comparison; see NOISE below and docs/TOOLING.md.

Exit code: 0 = identical apart from noise; 1 = something differs but every gameplay row and
per-entity table is identical (the switch's own native rows and summary line, typically);
3 = a gameplay row or a per-entity row moved; 2 = usage or parse error. Differing run
parameters or a non-zero clock offset never give 0.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, OrderedDict
from pathlib import Path

# ---------------------------------------------------------------------------
# Noise. Each entry: id, where it comes from, and how it is neutralised.
# "mask" rewrites the matching part of the line before any comparison;
# "native-calls" ignores the calls count of one native table row.
# ---------------------------------------------------------------------------

NOISE = [
    {
        "id": "heap-pointers",
        "why": "every 8-digit hex value is masked: heap objects land inside the image's address range too (0x005C2F28 in one run), so no range test separates them; native table addresses are parsed before masking",
        "kind": "mask-hex",
    },
    {
        "id": "log-path",
        "why": "the milestone header carries log=<path>; each run has its own log name",
        "kind": "mask",
        "pattern": re.compile(r"(\blog=)\S+"),
    },
    {
        "id": "module-directory",
        "why": "`module directory <exe dir>`: an OFF and an ON build sit in different directories",
        "kind": "mask",
        "pattern": re.compile(r"^(module directory ).*$"),
    },
    {
        "id": "harness-slot",
        "why": "tools/run_game.ps1 passes --instance-tag slotN; the slot a run takes varies",
        "kind": "mask",
        "pattern": re.compile(r'^(harness: single-instance mutex name suffixed with instance tag ).*$'),
    },
    {
        "id": "harness-affinity",
        "why": "tools/run_game.ps1 passes --affinity-core per slot (2, 4, 6)",
        "kind": "mask",
        "pattern": re.compile(r"^(harness: main thread pinned to processor )\d+"),
    },
    {
        "id": "thread-ids",
        "why": "`native renderer constructed ... worker=<tid>` and `device startup ... render_thread=<tid>` are OS thread ids",
        "kind": "mask",
        "pattern": re.compile(r"\b((?:worker|render_thread)=)\d+"),
    },
    {
        "id": "press-start-blink",
        "why": "the title screen's press-start text alpha blinks on the wall clock before the mission (docs/GAME_EXECUTABLE.md, --frame-jitter measured table)",
        "kind": "mask",
        "pattern": re.compile(r"^(\s*text press_start_Text\b.*\bcolor=)\([^)]*\)"),
    },
    {
        "id": "prewindow-fmod-calls",
        "why": "`sound startup before window: ... fmod_calls=N` counts FMOD polls before the window, a wall-clock count",
        "kind": "mask",
        "pattern": re.compile(r"^(sound startup before window:.*\bfmod_calls=)\d+"),
    },
    {
        "id": "present-interval-header",
        "why": "`present interval <x> (harness override)` appears only when --present-interval is in force (docs/TOOLING.md section 7); the line is dropped",
        "kind": "drop",
        "pattern": re.compile(r"^present interval \w+ \(harness override\)$"),
    },
    {
        "id": "present-interval-device",
        "why": "`device created by full native startup ... interval=` shows the D3D present interval, which the harness override changes; lockstep frames make it a wall-time setting only",
        "kind": "mask",
        "pattern": re.compile(r"^(device created by full native startup .*\binterval=)\S+"),
    },
    {
        "id": "avoidance-refills",
        "why": "`ship avoidance search: ... refills=N` differs on identical runs (257 vs 265 on one binary, 2026-09-22)",
        "kind": "mask",
        "pattern": re.compile(r"^(\s*ship avoidance search:.*\brefills=)\d+"),
    },
    {
        "id": "ring-scan-clear-37c",
        "why": "the ring-scan table's clear_37c column reads the 9999.0 sentinel or FLT_MAX by heap state (the uninitialised searcher bounds; docs/SHIP_AI_OPEN_ITEMS.md section 140); only those two values are masked",
        "kind": "mask",
        "pattern": re.compile(
            r"^(\s+\S.*\s-?\d+\.\d{4}\s+-?\d+\.\d{4})\s+"
            r"(?:9999\.0|340282346638528859811704183484516925440\.0)(?=\s+\d+\s+[0-9a-f]+$)"),
    },
    {
        "id": "sector-scan-clip-arc-zones",
        "why": "ShipAiSectorScan::clip_arc_zones_00415970 varies between identical JM08 runs of one binary (12000, then absent; cc9-init2 LSH_OFF/LSH_OFF2_JM08); its count and its presence are ignored",
        "kind": "native-calls",
        "name": "ShipAiSectorScan::clip_arc_zones",
    },
    {
        "id": "sector-scan-zone-segment-crossing",
        "why": "ShipAiSectorScan::zone_segment_crossing_004158e0 varies with clip_arc_zones (6000, then absent, on the same JM08 pair); count and presence ignored",
        "kind": "native-calls",
        "name": "ShipAiSectorScan::zone_segment_crossing",
    },
    {
        "id": "clearance-static-zone-blocks",
        "why": "ShipAiClearance::static_zone_blocks_009d57e0 (3 calls) is present or absent across seven JM08 runs of identical binaries, independent of any switch (docs/SENTITY_INIT_ATTACH_ORDER.md 20.4); count and presence ignored",
        "kind": "native-calls",
        "name": "ShipAiClearance::static_zone_blocks",
    },
    {
        "id": "clearance-static-zone-clearance",
        "why": "ShipAiClearance::static_zone_clearance_00415d70 (3 calls) comes and goes with static_zone_blocks on the same JM08 runs; count and presence ignored",
        "kind": "native-calls",
        "name": "ShipAiClearance::static_zone_clearance",
    },
    {
        "id": "clearance-category-enabled",
        "why": "ShipAiClearance::category_enabled_009ec770 moved 7026 -> 6999 between two USN13 runs of one binary with gameplay identical (cc9-terrain2 VS_ON2/VS_ON3_USN13; docs/SCENE_CONTENTS_HOSTS.md 15); count and presence ignored",
        "kind": "native-calls",
        "name": "ShipAiClearance::category_enabled",
    },
    {
        "id": "clearance-avoidance-enabled",
        "why": "ShipAiClearance::avoidance_enabled_0080e160 moved 1109 -> 1108 on the same USN13 pair; count and presence ignored",
        "kind": "native-calls",
        "name": "ShipAiClearance::avoidance_enabled",
    },
    {
        "id": "pretranslate-count",
        "why": "PlatformLoopCallbacks::pretranslate counts window messages (focus, paint), which depend on the desktop",
        "kind": "native-calls",
        "name": "PlatformLoopCallbacks::pretranslate",
    },
]

HEX_RE = re.compile(r"\b(0x)?([0-9A-Fa-f]{8})\b")


def _mask_hex(match: re.Match) -> str:
    return (match.group(1) or "") + "#"


def mask(line: str) -> str:
    for rule in NOISE:
        if rule["kind"] == "mask":
            line = rule["pattern"].sub(lambda m: m.group(1) + "~", line)
    return HEX_RE.sub(_mask_hex, line)


NATIVE_CALL_NOISE = tuple(r["name"] for r in NOISE if r["kind"] == "native-calls")


def native_noise(name: str) -> bool:
    """Native rows whose count (and, since a zero-call row is not printed, presence) is noise;
    matched by prefix because a row name may carry an address suffix."""
    return name.startswith(NATIVE_CALL_NOISE)
DROP_PATTERNS = [r["pattern"] for r in NOISE if r["kind"] == "drop"]

# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

NATIVE_HEADER_RE = re.compile(r"^host methods (\d+) concrete, (\d+) unimplemented")
NATIVE_ROW_RE = re.compile(r"^  (\S.*?)\s+(\S+)\s+(concrete|UNIMPLEMENTED)\s+calls=(\d+)\s*$")
HOST_FIRST_CALL_RE = re.compile(r"^host \S.* \[[^\]]+\] (concrete|UNIMPLEMENTED)")
UNIT_HEADER_RE = re.compile(r"^  unit\s+side\s+guns\s+cats\b.*\bsunk_at\s+killed_by")
UNIT_ROW_RE = re.compile(
    r"^  (?P<unit>\S.*?)\s+(?P<side>-?\d+)\s+(?P<guns>\d+)\s+(?P<cats>(?:\d+:\d+\s+)*)"
    r"(?P<range>-?\d+)\s+(?P<nearest>-?\d+)\s+(?P<shots>-?\d+)\s+(?P<hits>-?\d+)\s+"
    r"(?P<dealt>-?[\d.]+)\s+(?P<taken>-?[\d.]+)\s+(?P<health>-?[\d.]+)\s+(?P<sunk_at>-?[\d.]+)\s+"
    r"(?P<killed_by>.*?)\s*$"
)
GUNNERY_STEP_RE = re.compile(r"^\s+gunnery step (\d+) t=([\d.]+) .*\bshots=(\d+)")
KEY_RE = re.compile(r"(?:(?<=\s)|^)([A-Za-z_][\w./-]*)=")
TIME_FIELDS = {"t", "first_damage", "sunk_at"}


def parse_kv(text: str) -> "OrderedDict[str, str]":
    """k=v pairs where values may contain spaces; a trailing bare word prefixes later keys
    (`hits c0=1 c1=0 dmg c0=...` gives hits.c0, dmg.c0)."""
    out: "OrderedDict[str, str]" = OrderedDict()
    matches = list(KEY_RE.finditer(text))
    prefix = ""
    for i, m in enumerate(matches):
        end = matches[i + 1].start() if i + 1 < len(matches) else len(text)
        value = text[m.end():end].strip()
        key = (prefix + "." if prefix else "") + m.group(1)
        parts = value.split(" ")
        next_prefix = prefix
        if len(parts) > 1 and re.fullmatch(r"[a-z]{3,}", parts[-1]) and i + 1 < len(matches):
            next_prefix = parts[-1]
            value = " ".join(parts[:-1])
        n = 2
        base = key
        while key in out:
            key = f"{base}#{n}"
            n += 1
        out[key] = value
        prefix = next_prefix
    return out


def summary_key(body: str) -> str:
    words = []
    for tok in body.split(" "):
        if "=" in tok:
            break
        words.append(tok)
        if tok.endswith(":") or len(words) >= 4:
            break
    return " ".join(words)


class Run:
    def __init__(self, path: Path):
        self.path = path
        self.milestone = None
        self.jitter = "frame jitter off"  # logs from before the option print no line
        self.native_header = None
        self.native = OrderedDict()   # (name, addr) -> (status, calls)
        self.deaths = OrderedDict()   # victim -> kv
        self.death_modes = OrderedDict()
        self.units = OrderedDict()    # unit -> dict
        self.summaries = OrderedDict()  # key#n -> masked text
        self.summary_raw = OrderedDict()
        self.steps = {}               # gunnery step number -> (t, shots)
        self.last_shots = None
        self.other = []               # masked lines not covered above
        self._parse()

    @staticmethod
    def _unique(d, key):
        k, n = key, 2
        while k in d:
            k = f"{key}#{n}"
            n += 1
        return k

    def _parse(self):
        in_native = False
        in_units = False
        pending_summaries = []
        with open(self.path, encoding="utf-8", errors="replace") as fh:
            for raw in fh:
                line = raw.rstrip("\r\n")
                if any(pat.match(line) for pat in DROP_PATTERNS):
                    continue
                if in_native:
                    m = NATIVE_ROW_RE.match(line)
                    if m:
                        key = self._unique(self.native, (m.group(1), m.group(2)))
                        self.native[key] = (m.group(3), int(m.group(4)))
                        continue
                    in_native = False
                if in_units:
                    m = UNIT_ROW_RE.match(line)
                    if m:
                        d = m.groupdict()
                        d["cats"] = d["cats"].strip()
                        self.units[self._unique(self.units, d["unit"].strip())] = d
                        continue
                    in_units = False
                if line.startswith("bsp_game milestone"):
                    self.milestone = mask(line)
                    continue
                if line.startswith("frame jitter"):
                    self.jitter = line
                    continue
                m = NATIVE_HEADER_RE.match(line)
                if m:
                    self.native_header = (int(m.group(1)), int(m.group(2)))
                    in_native = True
                    continue
                if UNIT_HEADER_RE.match(line):
                    in_units = True
                    continue
                if line.startswith("death row: "):
                    kv = parse_kv(line[len("death row: "):])
                    self.deaths[self._unique(self.deaths, kv.get("victim", "?"))] = kv
                    continue
                if line.startswith("plane death mode: "):
                    kv = parse_kv(line[len("plane death mode: "):])
                    self.death_modes[self._unique(self.death_modes, kv.get("unit", "?"))] = kv
                    continue
                if line.startswith("summary "):
                    pending_summaries.append(line[len("summary "):])
                    continue
                m = GUNNERY_STEP_RE.match(line)
                if m:
                    self.steps[int(m.group(1))] = float(m.group(2))
                    self.last_shots = int(m.group(3))
                if HOST_FIRST_CALL_RE.match(line):
                    continue  # the native table carries the same status with counts
                self.other.append(mask(line))
        # Repeated summary keys (30 `mission gunnery barrel device=N` lines) are keyed by their
        # first field as well, so one line more or less does not shift every later key.
        def first_field(body, key):
            return next((t for t in body[len(key):].split(" ") if "=" in t), "")

        base = [summary_key(b) for b in pending_summaries]
        base_counts = Counter(base)
        # level 1: the first field's name (`mission gunnery units=`); level 2, only when that
        # still repeats, the whole first field (`mission gunnery barrel device=202`)
        named = [
            f"{k} {first_field(b, k).split('=')[0]}=" if base_counts[k] > 1 else k
            for k, b in zip(base, pending_summaries)
        ]
        named_counts = Counter(named)
        for k0, k1, body in zip(base, named, pending_summaries):
            key = k1
            if named_counts[k1] > 1 and k1 != k0:
                key = f"{k0} {first_field(body, k0)}"
            key = self._unique(self.summaries, key)
            self.summaries[key] = mask(body)
            self.summary_raw[key] = body

    # -- headline values -------------------------------------------------------
    def summary_kv(self, prefix):
        for body in self.summary_raw.values():
            if body.startswith(prefix):
                return parse_kv(body)
        return {}

    def summary_text(self, prefix):
        for body in self.summary_raw.values():
            if body.startswith(prefix):
                return body
        return None

    def native_calls(self, name_prefix):
        total, found = 0, False
        for (name, _addr), (_status, calls) in self.native.items():
            if name.startswith(name_prefix):
                total += calls
                found = True
        return total if found else None

    def headline(self):
        dmg = self.summary_kv("mission gunnery damage")
        world = self.summary_kv("mission world units")
        torp = self.summary_kv("mission torpedo task:")
        dive = self.summary_kv("mission dive-bomb task:")
        drop = self.summary_kv("mission gunnery torpedo_drop")
        end = self.summary_text("mission end:")
        rows = OrderedDict()
        rows["deaths"] = dmg.get("deaths")
        rows["hit records"] = dmg.get("hit_records")
        rows["hull hits"] = dmg.get("hull")
        rows["damage"] = dmg.get("total_damage")
        rows["shots"] = None if self.last_shots is None else str(self.last_shots)
        rows["first hit"] = dmg.get("first_hit")
        rows["torpedo-task releases"] = _of(torp.get("releases"), torp.get("aircraft"))
        rows["dive-bomb-task releases"] = _of(dive.get("releases"), dive.get("aircraft"))
        rows["torpedo drops"] = drop.get("drops")
        wc = self.native_calls("Plane::water_contact")
        rows["plane water contacts"] = None if wc is None else str(wc)
        rows["controlled moved"] = (
            f"{world.get('controlled')} {world.get('moved')}" if world.get("moved") else None
        )
        rows["units"] = world.get("units") or world.get("mission world units")
        rows["mission end"] = None if end is None else end[len("mission end:"):].strip()
        if self.native_header:
            # counted from the rows, without the noise rows, which may be absent in one run
            kept = [st for (name, _addr), (st, _c) in self.native.items() if not native_noise(name)]
            rows["host methods concrete/unimplemented"] = "%d / %d" % (
                kept.count("concrete"), kept.count("UNIMPLEMENTED"))
        return rows


def _of(a, b):
    if a is None:
        return None
    return f"{a} of {b}" if b is not None else a


# ---------------------------------------------------------------------------
# Comparison
# ---------------------------------------------------------------------------

def clock_offset(off: Run, on: Run):
    common = sorted(set(off.steps) & set(on.steps))
    step_off = None
    if common:
        s = common[0]
        step_off = round(on.steps[s] - off.steps[s], 4)
    fh_off = off.summary_kv("mission gunnery damage").get("first_hit", "").split(" ")[0]
    fh_on = on.summary_kv("mission gunnery damage").get("first_hit", "").split(" ")[0]
    fh_delta = None
    try:
        fh_delta = round(float(fh_on) - float(fh_off), 4)
    except ValueError:
        pass
    return {
        "anchor": f"gunnery step {common[0]}" if common else None,
        "offset": step_off if step_off is not None else 0.0,
        "first_hit_delta": fh_delta,
    }


def _num(v):
    try:
        return float(v.split(" ")[0])
    except (ValueError, AttributeError):
        return None


def compare_kv(a: dict, b: dict, offset: float):
    changes = []
    for k in list(a.keys()) + [k for k in b if k not in a]:
        va, vb = a.get(k), b.get(k)
        if k.split("#")[0] in TIME_FIELDS and va is not None and vb is not None:
            na, nb = _num(va), _num(vb)
            if na is not None and nb is not None and na >= 0 and nb >= 0:
                if abs((nb - offset) - na) < 0.005:
                    continue
                changes.append((k, va, vb))
                continue
        if va != vb:
            changes.append((k, va, vb))
    return changes


def compare_tables(off: dict, on: dict, offset: float):
    added = [k for k in on if k not in off]
    removed = [k for k in off if k not in on]
    changed = []
    for k in off:
        if k in on:
            ch = compare_kv(off[k], on[k], offset)
            if ch:
                changed.append((k, ch))
    return {"added": added, "removed": removed, "changed": changed}


def compare_native(off: Run, on: Run):
    res = {"added": [], "removed": [], "status": [], "calls": [], "noise": []}
    for key, (st, calls) in on.native.items():
        if key not in off.native:
            if native_noise(key[0]):
                res["noise"].append((key, "absent", calls))
            else:
                res["added"].append((key, st, calls))
    for key, (st, calls) in off.native.items():
        if key not in on.native:
            if native_noise(key[0]):
                res["noise"].append((key, calls, "absent"))
            else:
                res["removed"].append((key, st, calls))
            continue
        st2, calls2 = on.native[key]
        if st != st2:
            res["status"].append((key, st, st2, calls, calls2))
        elif calls != calls2:
            bucket = "noise" if native_noise(key[0]) else "calls"
            res[bucket].append((key, calls, calls2))
    return res


def compare_summaries(off: Run, on: Run):
    res = {"added": [], "removed": [], "changed": []}
    for k, v in on.summaries.items():
        if k not in off.summaries:
            res["added"].append((k, on.summary_raw[k]))
    for k, v in off.summaries.items():
        if k not in on.summaries:
            res["removed"].append((k, off.summary_raw[k]))
        elif v != on.summaries[k]:
            fa, fb = parse_kv(v), parse_kv(on.summaries[k])
            fields = [(x, fa.get(x), fb.get(x)) for x in list(fa) + [y for y in fb if y not in fa]
                      if fa.get(x) != fb.get(x)]
            res["changed"].append((k, fields, off.summary_raw[k], on.summary_raw[k]))
    return res


def compare_other(off: Run, on: Run):
    ca, cb = Counter(off.other), Counter(on.other)
    only_off = ca - cb
    only_on = cb - ca
    seen_a, seen_b = Counter(), Counter()
    first_off, first_on = [], []
    for line in off.other:
        if seen_a[line] < only_off[line]:
            seen_a[line] += 1
            first_off.append(line)
    for line in on.other:
        if seen_b[line] < only_on[line]:
            seen_b[line] += 1
            first_on.append(line)
    return {"only_off": first_off, "only_on": first_on}


def diff_runs(off: Run, on: Run):
    offset = clock_offset(off, on)
    off_h, on_h = off.headline(), on.headline()
    headline = [(k, off_h.get(k), on_h.get(k)) for k in off_h]
    return {
        "params": {"off": off.milestone, "on": on.milestone,
                   "jitter_off": off.jitter, "jitter_on": on.jitter},
        "clock": offset,
        "headline": headline,
        "deaths": compare_tables(off.deaths, on.deaths, offset["offset"]),
        "death_modes": compare_tables(off.death_modes, on.death_modes, offset["offset"]),
        "units": compare_tables(off.units, on.units, offset["offset"]),
        "native": compare_native(off, on),
        "summaries": compare_summaries(off, on),
        "other": compare_other(off, on),
        "counts": {
            "native_rows": [len(off.native), len(on.native)],
            "death_rows": [len(off.deaths), len(on.deaths)],
            "plane_death_modes": [len(off.death_modes), len(on.death_modes)],
            "unit_rows": [len(off.units), len(on.units)],
            "summary_lines": [len(off.summaries), len(on.summaries)],
            "other_lines": [len(off.other), len(on.other)],
        },
    }


GAMEPLAY_ROWS_EXCLUDED = {"host methods concrete/unimplemented"}


def gameplay_moved(d) -> bool:
    if any(a != b for k, a, b in d["headline"] if k not in GAMEPLAY_ROWS_EXCLUDED):
        return True
    return any(d[t][x] for t in ("deaths", "death_modes", "units")
               for x in ("added", "removed", "changed"))


def is_identical(d) -> bool:
    p = d["params"]
    if p["off"] != p["on"] or p["jitter_off"] != p["jitter_on"]:
        return False
    if d["clock"]["offset"]:
        return False
    if any(a != b for _k, a, b in d["headline"]):
        return False
    if gameplay_moved(d):
        return False
    n = d["native"]
    if n["added"] or n["removed"] or n["status"] or n["calls"]:
        return False
    s = d["summaries"]
    if s["added"] or s["removed"] or s["changed"]:
        return False
    o = d["other"]
    return not (o["only_off"] or o["only_on"])


# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------

def _clip(text, width=160):
    text = "" if text is None else str(text)
    return text if len(text) <= width else text[: width - 3] + "..."


def report(d, limit: int) -> str:
    out = []
    w = out.append
    p = d["params"]
    if p["off"] != p["on"] or p["jitter_off"] != p["jitter_on"]:
        w("RUN PARAMETERS DIFFER (not a same-parameter pair):")
        w("  off: " + _clip(p["off"], 400))
        w("  on:  " + _clip(p["on"], 400))
        if p["jitter_off"] != p["jitter_on"]:
            w(f"  jitter: {p['jitter_off']} | {p['jitter_on']}")
    c = d["clock"]
    w(f"clock offset (on - off at {c['anchor']}): {c['offset']:+.2f} s; first-hit delta: "
      + ("n/a" if c["first_hit_delta"] is None else f"{c['first_hit_delta']:+.2f} s"))

    cnt = d["counts"]["death_rows"]
    if cnt[0] == 0 or cnt[1] == 0:
        w(f"note: death rows {cnt[0]} / {cnt[1]}; `death row` lines need BSP_DEATH_TABLE=1 and a death")
    w("")
    w("GAMEPLAY: " + ("MOVED" if gameplay_moved(d) else "identical"))
    for k, a, b in d["headline"]:
        mark = "  " if a == b else "* "
        w(f"{mark}{k:<38} {_clip(a, 60):<40} {_clip(b, 60)}")

    for title, key in (("DEATH ROWS", "deaths"), ("PLANE DEATH MODES", "death_modes"),
                       ("UNIT TABLE", "units")):
        t = d[key]
        cnt = d["counts"]["death_rows" if key == "deaths" else
                          "plane_death_modes" if key == "death_modes" else "unit_rows"]
        if not (t["added"] or t["removed"] or t["changed"]):
            w(f"{title}: identical ({cnt[0]} rows)")
            continue
        w(f"{title}: {cnt[0]} -> {cnt[1]} rows, {len(t['added'])} only ON, "
          f"{len(t['removed'])} only OFF, {len(t['changed'])} changed")
        for k in t["removed"][:limit]:
            w(f"  - only OFF: {k}")
        for k in t["added"][:limit]:
            w(f"  + only ON:  {k}")
        for k, ch in t["changed"][:limit]:
            w(f"  ~ {k}: " + ", ".join(f"{f} {a} -> {b}" for f, a, b in ch[:6])
              + (" ..." if len(ch) > 6 else ""))
        rest = max(0, len(t["changed"]) - limit)
        if rest:
            w(f"  ... {rest} more changed rows (--json has all)")

    n = d["native"]
    cnt = d["counts"]["native_rows"]
    w("")
    w(f"NATIVE TABLE: {cnt[0]} -> {cnt[1]} rows; {len(n['added'])} added, {len(n['removed'])} removed, "
      f"{len(n['status'])} status changed, {len(n['calls'])} counts moved")
    for (name, addr), st, calls in n["added"][:limit]:
        w(f"  + {name} {addr} {st} calls={calls}")
    for (name, addr), st, calls in n["removed"][:limit]:
        w(f"  - {name} {addr} {st} calls={calls}")
    for (name, addr), s1, s2, c1, c2 in n["status"][:limit]:
        w(f"  ~ {name} {addr} {s1} -> {s2} calls {c1} -> {c2}")
    for (name, addr), c1, c2 in n["calls"][:limit]:
        w(f"  ~ {name} {addr} calls {c1} -> {c2}")
    for (name, addr), c1, c2 in n["noise"]:
        w(f"  (noise) {name} calls {c1} -> {c2}")

    s = d["summaries"]
    w("")
    w(f"SUMMARY LINES: {d['counts']['summary_lines'][0]} -> {d['counts']['summary_lines'][1]}; "
      f"{len(s['changed'])} changed, {len(s['added'])} only ON, {len(s['removed'])} only OFF")
    for k, fields, _a, _b in s["changed"][:limit]:
        if fields:
            w(f"  ~ {k}: " + ", ".join(f"{f} {_clip(a, 30)} -> {_clip(b, 30)}" for f, a, b in fields[:6])
              + (" ..." if len(fields) > 6 else ""))
        else:
            w(f"  ~ {k}")
    for k, body in s["added"][:limit]:
        w(f"  + {_clip(body)}")
    for k, body in s["removed"][:limit]:
        w(f"  - {_clip(body)}")

    o = d["other"]
    w("")
    w(f"OTHER LINES (masked multiset): {len(o['only_off'])} only OFF, {len(o['only_on'])} only ON")
    for line in o["only_off"][:limit]:
        w("  - " + _clip(line))
    for line in o["only_on"][:limit]:
        w("  + " + _clip(line))

    w("")
    if is_identical(d):
        w("VERDICT: identical apart from noise (exit 0)")
    elif gameplay_moved(d):
        w("VERDICT: DIFFERENT, gameplay moved (exit 3)")
    else:
        w("VERDICT: DIFFERENT, gameplay identical (exit 1)")
    return "\n".join(out)


def _jsonable(obj):
    if isinstance(obj, dict):
        return {(" ".join(k) if isinstance(k, tuple) else k): _jsonable(v) for k, v in obj.items()}
    if isinstance(obj, (list, tuple)):
        return [_jsonable(x) for x in obj]
    return obj


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("off")
    ap.add_argument("on")
    ap.add_argument("--json", help="write the whole comparison to this file")
    ap.add_argument("--limit", type=int, default=12, help="rows printed per section (default 12)")
    args = ap.parse_args(argv)
    try:
        off, on = Run(Path(args.off)), Run(Path(args.on))
    except OSError as exc:
        print(f"pair_diff: {exc}", file=sys.stderr)
        return 2
    if off.native_header is None or on.native_header is None:
        print("pair_diff: a log has no native table (`host methods ...`); did the run finish?",
              file=sys.stderr)
        return 2
    d = diff_runs(off, on)
    print(report(d, args.limit))
    if args.json:
        d["identical"] = is_identical(d)
        d["noise"] = [{"id": r["id"], "why": r["why"]} for r in NOISE]
        Path(args.json).write_text(json.dumps(_jsonable(d), indent=1), encoding="utf-8")
        print(f"json: {args.json}")
    if is_identical(d):
        return 0
    return 3 if gameplay_moved(d) else 1


if __name__ == "__main__":
    sys.exit(main())
