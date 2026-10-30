#!/usr/bin/env python3
"""Attribute supplied drcov observations; absence of a trace remains unknown.

No games are launched. Outputs describe reach in supplied logs, not correctness
or call counts. See docs/COVERAGE_ATTRIBUTION.md for scope and PDB limitations.
"""
from __future__ import annotations

import argparse
import bisect
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import sqlite3
import struct
import subprocess
import sys
import uuid

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import progress_board as board


class CensusError(RuntimeError):
    pass


def sha256(path):
    result = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            result.update(block)
    return result.hexdigest()


def number(value):
    if isinstance(value, int):
        return value
    return int(value, 0) if value.lower().startswith("0x") else int(value, 16)


def basename(path):
    return path.replace("\\", "/").rsplit("/", 1)[-1].casefold()


def merged_ranges(ranges):
    result = []
    for lo, hi in sorted(ranges):
        if hi <= lo:
            continue
        if result and lo <= result[-1][1]:
            result[-1] = (result[-1][0], max(hi, result[-1][1]))
        else:
            result.append((lo, hi))
    return result


def uncovered(lo, hi, covered):
    cursor = lo
    for start, stop in merged_ranges(covered):
        if start > cursor:
            yield cursor, min(start, hi)
        cursor = max(cursor, stop)
    if cursor < hi:
        yield cursor, hi


@dataclass(frozen=True)
class Observation:
    # Normalized image RVA. A future entry-count source can supply entry_count;
    # drcov never supplies it, and record multiplicity is never used as a count.
    rva: int
    size: int
    entry_count: int | None = None

    def __post_init__(self):
        if not (0 <= self.rva < 0x100000000 and 0 < self.size <= 0xFFFF
                and self.rva + self.size <= 0x100000000):
            raise CensusError("invalid normalized observation span")
        if self.entry_count is not None and (type(self.entry_count) is not int or self.entry_count <= 0):
            raise CensusError("entry-count providers must omit zero observations and reject negative counts")


class RangeIndex:
    def __init__(self, rows):
        self.rows = sorted(rows, key=lambda row: (row[0], row[1]))
        self.starts = [row[0] for row in self.rows]
        self.max_stops = []
        largest = 0
        for row in self.rows:
            largest = max(largest, row[1])
            self.max_stops.append(largest)

    def overlaps(self, lo, hi):
        end = bisect.bisect_left(self.starts, hi)
        start = bisect.bisect_right(self.max_stops, lo, 0, end)
        return [row for row in self.rows[start:end] if row[1] > lo]


def pe_info(path):
    raw = Path(path).read_bytes()
    if raw[:2] != b"MZ" or len(raw) < 64:
        raise CensusError(f"not a PE image: {path}")
    header, = struct.unpack_from("<I", raw, 0x3C)
    if raw[header:header + 4] != b"PE\0\0":
        raise CensusError(f"bad PE signature: {path}")
    machine, count, timestamp, _, _, optional_size, _ = struct.unpack_from("<HHIIIHH", raw, header + 4)
    optional = header + 24
    magic, = struct.unpack_from("<H", raw, optional)
    if magic not in (0x10B, 0x20B):
        raise CensusError("unsupported PE optional header")
    image_base, = struct.unpack_from("<I" if magic == 0x10B else "<Q", raw,
                                   optional + (28 if magic == 0x10B else 24))
    size, = struct.unpack_from("<I", raw, optional + 56)
    sections = []
    for index in range(count):
        offset = optional + optional_size + 40 * index
        virtual_size, rva, file_size, file_offset = struct.unpack_from("<IIII", raw, offset + 8)
        flags, = struct.unpack_from("<I", raw, offset + 36)
        sections.append({"name": raw[offset:offset + 8].rstrip(b"\0").decode("ascii", "replace"),
                         "rva": rva, "virtual_size": virtual_size, "file_size": file_size,
                         "file_offset": file_offset, "flags": flags})

    def file_offset(rva):
        for section in sections:
            if section["rva"] <= rva < section["rva"] + section["file_size"]:
                return section["file_offset"] + rva - section["rva"]
        raise CensusError("PE debug directory is outside file-backed sections")

    codeview = None
    directories = optional + (96 if magic == 0x10B else 112)
    debug_rva, debug_size = struct.unpack_from("<II", raw, directories + 6 * 8)
    if debug_rva:
        start = file_offset(debug_rva)
        for offset in range(start, start + debug_size, 28):
            _, _, _, _, kind, data_size, _, data_offset = struct.unpack_from("<IIHHIIII", raw, offset)
            if kind == 2 and raw[data_offset:data_offset + 4] == b"RSDS" and data_size >= 24:
                age, = struct.unpack_from("<I", raw, data_offset + 20)
                codeview = {"guid": str(uuid.UUID(bytes_le=raw[data_offset + 4:data_offset + 20])),
                            "age": age, "pdb_path": raw[data_offset + 24:data_offset + data_size].split(b"\0")[0].decode("utf-8", "replace")}
    return {"path": str(Path(path).resolve()), "sha256": hashlib.sha256(raw).hexdigest(),
            "machine": machine, "image_base": image_base, "image_size": size,
            "timestamp": timestamp, "sections": sections, "codeview": codeview}


def read_drcov(path, module_name, image=None, original=False):
    """Read drcov v2/v3, module-table v2..5, binary or text BB tables.

    Module records, not assumptions about module zero, choose the target image.
    Segmented module RVAs use the containing module's runtime base.
    """
    path = Path(path)
    with path.open("rb") as stream:
        def line():
            raw = stream.readline(65537)
            if not raw or len(raw) > 65536:
                raise CensusError(f"missing/oversize drcov header in {path}")
            return raw.decode("utf-8", "strict").rstrip("\r\n")
        version_line = line()
        version = re.fullmatch(r"DRCOV VERSION: ([23])", version_line)
        if not version:
            raise CensusError(f"unsupported drcov version: {path}")
        file_version = int(version.group(1))
        flavor = line()
        if flavor not in ("DRCOV FLAVOR: drcov", "DRCOV FLAVOR: drcov-32", "DRCOV FLAVOR: drcov-64"):
            raise CensusError(f"unsupported drcov flavor: {flavor}")
        match = re.fullmatch(r"Module Table: version ([2-5]), count (\d+)", line())
        if not match:
            raise CensusError("unsupported module-table header (legacy v1 is not guessed)")
        table_version, count = map(int, match.groups())
        columns_line = line()
        if not columns_line.startswith("Columns: "):
            raise CensusError("missing module Columns header")
        columns = [part.strip().casefold() for part in columns_line[9:].split(",")]
        if columns[-1] != "path" or "id" not in columns or "end" not in columns or not ({"base", "start"} & set(columns)):
            raise CensusError("unsupported module columns")
        if "(custom fields)" in columns:
            raise CensusError("custom module columns require an explicit parser")
        modules = {}
        for _ in range(count):
            values = [part.strip() for part in line().split(",", len(columns) - 1)]
            if len(values) != len(columns):
                raise CensusError("incomplete module row")
            row = dict(zip(columns, values))
            mid = int(row["id"])
            if mid in modules:
                raise CensusError("duplicate drcov module id")
            start = number(row.get("start", row.get("base")))
            stop = number(row["end"])
            if stop <= start:
                raise CensusError("invalid module span")
            modules[mid] = {"id": mid, "containing_id": int(row.get("containing_id", mid)),
                            "start": start, "end": stop, "path": row["path"],
                            "timestamp": number(row["timestamp"]) if "timestamp" in row else None,
                            "preferred_base": number(row["preferred_base"]) if "preferred_base" in row else None}
        target_ids = {mid for mid, row in modules.items() if basename(row["path"]) == basename(module_name)}
        if not target_ids:
            raise CensusError(f"module {module_name!r} absent from {path}")
        for mid in target_ids:
            row = modules[mid]
            root = modules.get(row["containing_id"])
            if root is None or root["containing_id"] != root["id"]:
                raise CensusError("unresolved containing module")
            if original and root["start"] != 0x400000:
                raise CensusError(f"original main module base is {root['start']:#x}, expected 0x400000")
            if image:
                if root["end"] - root["start"] != image["image_size"]:
                    raise CensusError("trace/image SizeOfImage mismatch")
                if root["timestamp"] is not None and root["timestamp"] != image["timestamp"]:
                    raise CensusError("trace/image PE timestamp mismatch")
        match = re.fullmatch(r"BB Table: (\d+) bbs", line())
        if not match:
            raise CensusError("missing BB table")
        total = int(match.group(1))
        if total > 100_000_000:
            raise CensusError("unreasonably large BB table")
        remaining = stream.read()
        entries = []
        if remaining.startswith(b"module id, start, size:"):
            records = remaining.splitlines()[1:]
            if len(records) != total:
                raise CensusError("truncated/extra text BB records")
            for record in records:
                match = re.fullmatch(rb"module\[\s*(\d+)\]:\s*(0x[0-9a-fA-F]+),\s*(\d+)", record.strip())
                if not match:
                    raise CensusError("bad text BB record")
                mid, rva, size = match.groups()
                entries.append((int(rva, 16), int(size), int(mid)))
        else:
            if len(remaining) != total * 8:
                raise CensusError(f"truncated/extra binary BB bytes: expected {total * 8}, found {len(remaining)}")
            entries = struct.iter_unpack("<IHH", remaining)
        observations = set()
        unknown_module_records = other_module_records = 0
        for offset, size, mid in entries:
            if not size:
                raise CensusError("zero-size drcov block")
            if mid == 0xFFFF:
                unknown_module_records += 1
                continue
            if mid not in modules:
                raise CensusError("BB references an absent module id")
            if mid not in target_ids:
                other_module_records += 1
                continue
            segment = modules[mid]
            root = modules[segment["containing_id"]]
            # Official DRCOV_VERSION_SEGMENT_OFFSETS=3 changed the offset
            # origin; v2 offsets remain relative to the containing module.
            address = (segment["start"] if file_version == 3 else root["start"]) + offset
            if not segment["start"] <= address < address + size <= segment["end"]:
                raise CensusError("BB exceeds its module segment")
            observations.add(Observation(address - root["start"], size))
    return observations, {"path": str(path.resolve()), "sha256": sha256(path), "drcov_version": file_version,
                          "flavor": flavor[14:], "module_table_version": table_version,
                          "bb_records": total, "distinct_selected_blocks": len(observations),
                          "selected_bb_records": total - other_module_records - unknown_module_records,
                          "duplicate_selected_bb_records": total - other_module_records - unknown_module_records - len(observations),
                          "bb_table_complete": True,
                          "other_module_records": other_module_records,
                          "unknown_module_records": unknown_module_records,
                          "selected_modules": [modules[mid] for mid in sorted(target_ids)]}


class Catalog:
    def __init__(self, path):
        self.path = Path(path)
        db = sqlite3.connect(self.path.resolve().as_uri() + "?mode=ro", uri=True)
        db.row_factory = sqlite3.Row
        self.meta = dict(db.execute("select key,value from meta"))
        categories = {row["id"]: board.category_for(row["start"], (row["keywords"] or "").split(",")[:8])
                      for row in db.execute("select id,start,keywords from segments")}
        self.parents = dict(db.execute("select funclet,parent from funclet_parent"))
        self.functions = {}
        for row in db.execute("select * from functions"):
            item = dict(row)
            address, name = item["address"], item["name"]
            item["eh_funclet"] = address in self.parents or name.startswith(("Unwind@", "Catch_All@", "EHHandler@"))
            item["board_game"] = (not name.startswith(board.FUNCLET) and board.is_game(address, item["tag_category"])
                                  and not name.startswith(board.LIBRARY_NAME_PREFIXES)
                                  and item["recon_status"] not in board.OUT_OF_SCOPE_STATUSES)
            item["category"] = categories.get(item["segment"], "other")
            self.functions[address] = item
        self.ranges = RangeIndex((row[0], row[1], row[0]) for row in db.execute("select func,stop from ranges"))
        self.range_starts = set(self.ranges.starts)
        self.undefined = RangeIndex((row[0], row[1], row[0]) for row in db.execute("select address,stop from undefined"))
        self.callers = dict(db.execute("select callee,count(distinct caller) from calls group by callee"))
        db.close()


def attribute_original(observations, catalog):
    hits, undefined_hits, unknown, counts = defaultdict(set), defaultdict(set), [], Counter()
    unknown_blocks = set()
    entries = Counter()
    for observation in observations:
        lo, hi = 0x400000 + observation.rva, 0x400000 + observation.rva + observation.size
        covered = []
        for start, stop, address in catalog.ranges.overlaps(lo, hi):
            hits[address].add(observation)
            covered.append((max(lo, start), min(hi, stop)))
        for start, stop, address in catalog.undefined.overlaps(lo, hi):
            undefined_hits[address].add(observation)
            covered.append((max(lo, start), min(hi, stop)))
        if lo in catalog.functions and lo not in catalog.range_starts:
            hits[lo].add(observation)
            covered.append((lo, lo + 1))
        pieces = list(uncovered(lo, hi, covered))
        if pieces:
            unknown_blocks.add(observation)
            unknown.extend(pieces)
        if observation.entry_count is not None:
            if lo in catalog.functions:
                entries[lo] += observation.entry_count
            else:
                counts["entry_count_records_without_exact_function_start"] += 1
    return {"hits": hits, "undefined_hits": undefined_hits, "unattributed_ranges": merged_ranges(unknown),
            "unattributed_block_count": len(unknown_blocks), "entry_counts": entries, "counts": dict(counts)}


def identifier(name):
    name = name.split("(", 1)[0].strip().rsplit(" ", 1)[-1]
    name = re.sub(r"<.*>", "", name).rsplit("::", 1)[-1]
    return name if re.fullmatch(r"~?[A-Za-z_]\w*", name) else None


def wire_identifier_known(row):
    # Legacy wire_readiness serializes ident/ident_via for fallback methods,
    # but omits them for its ordinary address-suffix resolution. Its state
    # still records whether a source identifier was actually resolved.
    if row.get("state") is not None:
        return row["state"] != "unknown_no_identifier"
    return bool(row.get("ident"))


class Resolver:
    def __init__(self, wire):
        self.rows = {int(row["address"], 16): row for row in wire.get("rows", [])}
        self.by_ident = defaultdict(set)
        for address, row in self.rows.items():
            for name in row.get("ident", []):
                self.by_ident[name.rsplit("::", 1)[-1]].add(address)

    def body_identity(self, name, source=None):
        ident = identifier(name)
        candidates = set(self.by_ident.get(ident, ()))
        if not candidates and ident:
            # A suffix identifies a candidate address, never the executing body.
            suffix = re.search(r"_([0-9a-fA-F]{8})$", ident)
            if suffix and int(suffix.group(1), 16) in self.rows:
                candidates.add(int(suffix.group(1), 16))
        qualified = name.split("(", 1)[0].strip()
        normalized = source.replace("\\", "/").casefold() if source else None
        evidence = {}
        for address in candidates:
            row = self.rows[address]
            qualified_match = any("::" in value and value == qualified for value in row.get("ident", []))
            expected = row.get("implementation_source") or row.get("source")
            expected_normalized = str(expected).replace("\\", "/").casefold() if expected else None
            if normalized and expected_normalized:
                matches = normalized == expected_normalized or normalized.endswith("/" + expected_normalized)
                status = "verified_implementation_source" if matches else "implementation_source_mismatch"
            elif qualified_match:
                status = "exact_qualified_body_identifier"
            else:
                status = "implementation_source_unverified"
            evidence[address] = {"status": status, "expected_source": expected,
                                 "exact_qualified_body_identifier": qualified_match}
        return evidence

    def resolve(self, name, source=None, allow_unverified=False):
        evidence = self.body_identity(name, source)
        if allow_unverified:
            return set(evidence)
        return {address for address, item in evidence.items() if item["status"] in
                ("verified_implementation_source", "exact_qualified_body_identifier")}


def host_adapter_hint(row):
    """A review aid; host-file/name membership does not prove a stand-in body."""
    source = (row.get("source") or "").replace("\\", "/").casefold()
    return "/game_hosts" in source or bool(re.search(r"(?:Adapter|Binding|Host)::", row["name"]))


class PdbSymbols:
    """Offline, private DbgHelp session; never opens or attaches to a game.

    ctypes layouts follow the installed Windows SDK DbgHelp.h. Inline records
    use the documented inline APIs, not mangled-name or source-text guesses.
    """
    def __init__(self, image, pdb, dll="dbghelp.dll"):
        if os.name != "nt":
            raise CensusError("PDB attribution requires Windows DbgHelp")
        import ctypes as c
        self.c = c
        U32, U64, BOOL, PTR = c.c_uint32, c.c_uint64, c.c_int32, c.c_void_p

        class Symbol(c.Structure):
            _fields_ = [("SizeOfStruct", U32), ("TypeIndex", U32), ("Reserved", U64 * 2),
                        ("Index", U32), ("Size", U32), ("ModBase", U64), ("Flags", U32),
                        ("Value", U64), ("Address", U64), ("Register", U32), ("Scope", U32),
                        ("Tag", U32), ("NameLen", U32), ("MaxNameLen", U32), ("Name", c.c_char * 1)]

        class Line(c.Structure):
            _fields_ = [("SizeOfStruct", U32), ("Key", PTR), ("LineNumber", U32),
                        ("FileName", c.c_char_p), ("Address", U64)]

        class Module(c.Structure):
            _fields_ = [("SizeOfStruct", U32), ("BaseOfImage", U64), ("ImageSize", U32),
                        ("TimeDateStamp", U32), ("CheckSum", U32), ("NumSyms", U32), ("SymType", U32),
                        ("ModuleName", c.c_char * 32), ("ImageName", c.c_char * 256),
                        ("LoadedImageName", c.c_char * 256), ("LoadedPdbName", c.c_char * 256),
                        ("CVSig", U32), ("CVData", c.c_char * 780), ("PdbSig", U32),
                        ("PdbSig70", c.c_ubyte * 16), ("PdbAge", U32),
                        ("PdbUnmatched", BOOL), ("DbgUnmatched", BOOL), ("LineNumbers", BOOL),
                        ("GlobalSymbols", BOOL), ("TypeInfo", BOOL), ("SourceIndexed", BOOL),
                        ("Publics", BOOL), ("MachineType", U32), ("Reserved", U32)]

        self.Symbol, self.Line = Symbol, Line
        self.api = c.WinDLL(dll, use_last_error=True)
        self.callback_type = c.WINFUNCTYPE(BOOL, c.POINTER(Symbol), U32, PTR)
        definitions = {
            "SymSetOptions": ([U32], U32), "SymInitializeW": ([PTR, c.c_wchar_p, BOOL], BOOL),
            "SymCleanup": ([PTR], BOOL),
            "SymLoadModuleExW": ([PTR, PTR, c.c_wchar_p, c.c_wchar_p, U64, U32, PTR, U32], U64),
            "SymGetModuleInfo64": ([PTR, U64, c.POINTER(Module)], BOOL),
            "SymEnumSymbolsEx": ([PTR, U64, c.c_char_p, self.callback_type, PTR, U32], BOOL),
            "SymFromAddr": ([PTR, U64, c.POINTER(U64), c.POINTER(Symbol)], BOOL),
            "SymAddrIncludeInlineTrace": ([PTR, U64], U32),
            "SymQueryInlineTrace": ([PTR, U64, U32, U64, U64, c.POINTER(U32), c.POINTER(U32)], BOOL),
            "SymFromInlineContext": ([PTR, U64, U32, c.POINTER(U64), c.POINTER(Symbol)], BOOL),
            "SymGetLineFromAddr64": ([PTR, U64, c.POINTER(U32), c.POINTER(Line)], BOOL),
            "SymGetLineFromInlineContext": ([PTR, U64, U32, U64, c.POINTER(U32), c.POINTER(Line)], BOOL),
        }
        for name, (args, result) in definitions.items():
            try:
                method = getattr(self.api, name)
            except AttributeError:
                raise CensusError(f"DbgHelp lacks required inline/PDB API {name}") from None
            method.argtypes, method.restype = args, result
        # Per SymInitialize, use a unique nonzero token, not GetCurrentProcess.
        self.handle = PTR(id(self))
        self.opened = False
        self.image, self.pdb, self.base = image, Path(pdb).resolve(), image["image_base"]
        self.api.SymSetOptions(0x2 | 0x10 | 0x200 | 0x400 | 0x1000 | 0x80000)
        if not self.api.SymInitializeW(self.handle, str(self.pdb.parent), False):
            raise CensusError(f"SymInitializeW: {c.get_last_error()}")
        self.opened = True
        try:
            loaded = self.api.SymLoadModuleExW(self.handle, None, image["path"], None,
                                               self.base, image["image_size"], None, 0)
            if not loaded:
                raise CensusError(f"SymLoadModuleExW: {c.get_last_error()}")
            info = Module()
            info.SizeOfStruct = c.sizeof(Module)
            if not self.api.SymGetModuleInfo64(self.handle, loaded, c.byref(info)):
                raise CensusError(f"SymGetModuleInfo64: {c.get_last_error()}")
            guid = str(uuid.UUID(bytes_le=bytes(info.PdbSig70)))
            loaded_pdb = bytes(info.LoadedPdbName).decode("mbcs", "replace")
            cv = image.get("codeview")
            if (info.SymType not in (3, 7) or info.PdbUnmatched or info.DbgUnmatched or not cv
                    or guid != cv["guid"] or info.PdbAge != cv["age"]):
                raise CensusError("matching full PDB not established (symbol kind/GUID/age mismatch)")
            if os.path.normcase(str(Path(loaded_pdb).resolve())) != os.path.normcase(str(self.pdb)):
                raise CensusError(f"DbgHelp loaded a different PDB: {loaded_pdb}")
            self.metadata = {"pdb": str(self.pdb), "pdb_sha256": sha256(self.pdb), "guid": guid,
                             "age": info.PdbAge, "dbghelp": dll, "matching_pdb": True,
                             "line_records": bool(info.LineNumbers), "global_symbols": bool(info.GlobalSymbols)}
        except BaseException:
            self.close()
            raise

    def close(self):
        if self.opened:
            self.api.SymCleanup(self.handle)
            self.opened = False

    def line(self, address, context=None):
        c = self.c
        line = self.Line()
        line.SizeOfStruct = c.sizeof(line)
        displacement = c.c_uint32()
        if context is None:
            success = self.api.SymGetLineFromAddr64(self.handle, address, c.byref(displacement), c.byref(line))
        else:
            success = self.api.SymGetLineFromInlineContext(self.handle, address, context, self.base,
                                                           c.byref(displacement), c.byref(line))
        return line.FileName.decode("mbcs", "replace") if success and line.FileName else None

    def symbol(self, pointer, inline=False, source=None):
        value = pointer.contents
        name = self.c.string_at(self.c.addressof(value) + self.Symbol.Name.offset, value.NameLen).decode("mbcs", "replace")
        return {"name": name, "rva": int(value.Address) - self.base, "size": int(value.Size),
                "tag": int(value.Tag), "index": int(value.Index), "inline": inline, "source": source}

    def inventory(self):
        rows, errors = [], []
        for options, inline in ((1, False), (2, True)):
            @self.callback_type
            def callback(pointer, size, user):
                try:
                    value = pointer.contents
                    if inline or value.Tag == 5 or value.Flags & 0x800:
                        rows.append(self.symbol(pointer, inline))
                    if len(rows) > 2_000_000:
                        errors.append("symbol inventory limit exceeded")
                        return False
                    return True
                except BaseException as error:
                    errors.append(str(error))
                    return False
            if not self.api.SymEnumSymbolsEx(self.handle, self.base, b"*", callback, None, options):
                errors.append(f"SymEnumSymbolsEx options={options}: {self.c.get_last_error()}")
        return rows, errors

    def at(self, address, context=None):
        c = self.c
        buffer = c.create_string_buffer(c.sizeof(self.Symbol) + 4096)
        pointer = c.cast(buffer, c.POINTER(self.Symbol))
        pointer.contents.SizeOfStruct, pointer.contents.MaxNameLen = c.sizeof(self.Symbol), 4096
        displacement = c.c_uint64()
        if context is None:
            success = self.api.SymFromAddr(self.handle, address, c.byref(displacement), pointer)
        else:
            success = self.api.SymFromInlineContext(self.handle, address, context, c.byref(displacement), pointer)
        return self.symbol(pointer, context is not None, self.line(address, context)) if success else None

    def inline_at(self, address):
        count = self.api.SymAddrIncludeInlineTrace(self.handle, address)
        if count == 0:
            return ()
        if count > 256:
            raise CensusError("invalid inline-frame count")
        context, frame = self.c.c_uint32(), self.c.c_uint32()
        if not self.api.SymQueryInlineTrace(self.handle, address, 0, address, address,
                                            self.c.byref(context), self.c.byref(frame)):
            raise CensusError(f"SymQueryInlineTrace: {self.c.get_last_error()}")
        rows = []
        for index in range(count):
            # Inline context numbers are interpreted relative to Address;
            # they are not persistent inline-site IDs across addresses.
            row = self.at(address, context.value + index)
            if row is None:
                raise CensusError("SymFromInlineContext returned no symbol")
            rows.append(row)
        return tuple(rows)


def attribute_bsp(observations, symbols, resolver, max_addresses):
    inventory, inventory_errors = symbols.inventory()
    present, inventory_ambiguous = set(), set()
    outer = []
    aliases = defaultdict(set)
    unverified_inventory = set()
    mismatched_inventory = set()
    mismatched_inventory_records = 0
    for row in inventory:
        loose = resolver.resolve(row["name"], allow_unverified=True)
        if loose and not row.get("source"):
            if row["inline"]:
                try:
                    frames = symbols.inline_at(symbols.base + row["rva"])
                    row["source"] = next((frame.get("source") for frame in frames
                                          if frame["index"] == row["index"]), None)
                except CensusError:
                    row["source"] = None
            else:
                row["source"] = symbols.line(symbols.base + row["rva"])
        evidence = resolver.body_identity(row["name"], row.get("source"))
        candidates = resolver.resolve(row["name"], row.get("source"))
        unverified = {a for a, item in evidence.items() if item["status"] == "implementation_source_unverified"}
        mismatched = {a for a, item in evidence.items() if item["status"] == "implementation_source_mismatch"}
        unverified_inventory.update(unverified)
        inventory_ambiguous.update(unverified)
        mismatched_inventory.update(mismatched)
        mismatched_inventory_records += bool(mismatched)
        if len(candidates) == 1:
            present.update(candidates)
        elif candidates:
            inventory_ambiguous.update(candidates)
        if not row["inline"] and row["size"] > 0 and row["rva"] >= 0:
            outer.append((row["rva"], row["rva"] + row["size"], row))
            aliases[row["rva"]].add(row["name"])
    outer_index = RangeIndex(outer)
    hits, possible, entry_counts = defaultdict(set), set(), Counter()
    entry_seen = set()
    unresolved, runtime = {}, {}
    blocks_without_outer = []

    def hit(row, observation, shared_rva=False):
        key = (row["name"], row["rva"], row["index"], row["inline"])
        if key not in runtime:
            if not row.get("source") and not row["inline"]:
                row = {**row, "source": symbols.line(symbols.base + max(row["rva"], observation.rva))}
            evidence = resolver.body_identity(row["name"], row.get("source"))
            candidates = resolver.resolve(row["name"], row.get("source"))
            runtime[key] = {**row, "original_candidates": [f"{a:08x}" for a in sorted(candidates)],
                            "candidate_reaches": [f"{a:08x}" for a in sorted(evidence)],
                            "body_identity": {f"{a:08x}": item for a, item in sorted(evidence.items())},
                            "host_adapter_hint": host_adapter_hint(row),
                            "mapping": "unique_identifier" if len(candidates) == 1 else
                                       "ambiguous_identifier" if candidates else "no_wire_identifier_match"}
            possible.update(a for a, item in evidence.items() if item["status"] == "implementation_source_unverified")
            if not candidates:
                if evidence:
                    runtime[key]["rejected_leaf_candidates"] = [f"{a:08x}" for a in sorted(evidence)]
                    runtime[key]["mapping"] = ("source_or_qualification_mismatch" if any(
                        item["status"] == "implementation_source_mismatch" for item in evidence.values())
                        else "source_identity_unverified")
        candidates = {int(a, 16) for a in runtime[key]["original_candidates"]}
        if shared_rva and candidates:
            possible.update(candidates)
            runtime[key]["mapping"] = "shared_rva_logical_caller_unknown"
        elif len(candidates) == 1:
            address = next(iter(candidates))
            hits[address].add(observation)
            if (observation.entry_count is not None and not row["inline"]
                    and observation.rva == row["rva"] and (address, observation) not in entry_seen):
                entry_seen.add((address, observation))
                entry_counts[address] += observation.entry_count
        elif candidates:
            possible.update(candidates)
        else:
            unresolved[key] = runtime[key]

    for observation in observations:
        rows = outer_index.overlaps(observation.rva, observation.rva + observation.size)
        if not rows:
            row = symbols.at(symbols.base + observation.rva)
            if row and row["size"] > 0 and row["rva"] <= observation.rva < row["rva"] + row["size"]:
                rows = [(row["rva"], row["rva"] + row["size"], row)]
            else:
                blocks_without_outer.append({"rva": observation.rva, "size": observation.size})
        for _, _, row in rows:
            hit(row, observation, len(aliases[row["rva"]]) > 1)

    inline_spans, scanned, inline_error = [], 0, None
    ranges = merged_ranges((o.rva, o.rva + o.size) for o in observations)
    # Scan every covered byte, not only BB starts: inline ranges can begin in
    # the middle of a basic block. Compress adjacent equal contexts afterward.
    for lo, hi in ranges:
        previous, previous_key, start = (), (), lo
        stop = lo
        for rva in range(lo, hi):
            if scanned >= max_addresses:
                inline_error = "inline address budget exhausted; raise --max-pdb-addresses"
                break
            try:
                rows = symbols.inline_at(symbols.base + rva)
            except CensusError as error:
                inline_error = str(error)
                break
            key = tuple((row["name"], row["index"], row["rva"]) for row in rows)
            if key != previous_key:
                if previous:
                    inline_spans.append((start, rva, previous))
                previous, previous_key, start = rows, key, rva
            scanned += 1
            stop = rva + 1
            if scanned % 100_000 == 0:
                print(f"coverage_census: PDB inline scan {scanned} covered bytes", file=sys.stderr, flush=True)
        if previous and stop > start:
            inline_spans.append((start, stop, previous))
        if inline_error:
            break
    inline_index = RangeIndex(inline_spans)
    for observation in observations:
        shared = any(len(aliases[row[2]["rva"]]) > 1
                     for row in outer_index.overlaps(observation.rva, observation.rva + observation.size))
        for _, _, rows in inline_index.overlaps(observation.rva, observation.rva + observation.size):
            for row in rows:
                hit(row, observation, shared)
    complete = not inventory_errors and inline_error is None
    inventory_present = present | inventory_ambiguous
    absent = [f"{address:08x}" for address, row in resolver.rows.items()
              if wire_identifier_known(row) and address not in inventory_present] if not inventory_errors else None
    host_records = [row for row in runtime.values() if row["host_adapter_hint"] and row["candidate_reaches"]]
    host_candidates = {int(a, 16) for row in host_records for a in row["candidate_reaches"]}
    source_mismatches = [row for row in runtime.values() if any(
        item["status"] == "implementation_source_mismatch" for item in row["body_identity"].values())]
    return {"hits": hits, "possible": possible, "present": present, "complete": complete,
            "entry_counts": entry_counts,
            "metadata": {**symbols.metadata, "inventory_records": len(inventory),
                         "identifier_resolution": "suffix/leaf creates candidates only; verified frozen implementation source or exact qualified body identifier required; known source mismatch rejects",
                         "outer_inventory_records": sum(not r["inline"] for r in inventory),
                         "inline_inventory_records": sum(r["inline"] for r in inventory),
                         "inventory_errors": inventory_errors, "inline_scan_error": inline_error,
                         "source_unverified_inventory_candidates": [f"{a:08x}" for a in sorted(unverified_inventory)],
                         "source_unverified_inventory_candidate_count": len(unverified_inventory),
                         "source_mismatched_inventory_candidates": [f"{a:08x}" for a in sorted(mismatched_inventory)],
                         "source_mismatched_inventory_candidate_count": len(mismatched_inventory),
                         "source_mismatched_inventory_record_count": mismatched_inventory_records,
                         "covered_bytes": sum(hi - lo for lo, hi in ranges), "inline_bytes_scanned": scanned,
                         "inline_scan_complete": inline_error is None,
                         "shared_rva_groups": {f"{rva:x}": sorted(names) for rva, names in aliases.items() if len(names) > 1},
                         "blocks_without_outer_symbol": blocks_without_outer,
                         "blocks_without_outer_symbol_count": len(blocks_without_outer),
                         "unresolved_executed_symbols": list(unresolved.values()),
                         "unresolved_executed_symbol_count": len(unresolved),
                         "rejected_leaf_name_record_count": sum(row["mapping"] == "source_or_qualification_mismatch" for row in runtime.values()),
                         "source_unverified_executed_record_count": sum(row["mapping"] == "source_identity_unverified" for row in runtime.values()),
                         "source_mismatched_executed_record_count": len(source_mismatches),
                         "host_adapter_candidate_record_count": len(host_records),
                         "host_adapter_candidate_addresses": [f"{a:08x}" for a in sorted(host_candidates)],
                         "host_adapter_candidate_address_count": len(host_candidates),
                         "host_adapter_candidates_without_positive_body_hit": [f"{a:08x}" for a in sorted(host_candidates - hits.keys())],
                         "host_adapter_candidates_without_positive_body_hit_count": len(host_candidates - hits.keys()),
                         "host_adapter_source_mismatched_record_count": sum(row["host_adapter_hint"] for row in source_mismatches),
                         "host_adapter_hint_semantics": "PDB source/name review hint, not a stand-in classification; candidate reach alone gives no reconstruction-body credit",
                         "ambiguous_executed_candidates": [f"{a:08x}" for a in sorted(possible)],
                         "ambiguous_executed_candidate_count": len(possible),
                         "ambiguous_candidates_without_positive_hit": [f"{a:08x}" for a in sorted(possible - hits.keys())],
                         "ambiguous_candidates_without_positive_hit_count": len(possible - hits.keys()),
                         "unresolved_executed_unique_symbol_name_count": len({row["name"] for row in unresolved.values()}),
                         "symbol_count_semantics": "PDB symbol/site records; unique names are reported separately",
                         "wire_rows_without_identifier": [f"{a:08x}" for a, row in resolver.rows.items() if not wire_identifier_known(row)],
                         "wire_rows_without_identifier_count": sum(not wire_identifier_known(row) for row in resolver.rows.values()),
                         "wire_rows_without_explicit_ident_field_count": sum(not row.get("ident") for row in resolver.rows.values()),
                         "wire_rows_without_symbol_or_inline_record": absent,
                         "wire_rows_without_symbol_or_inline_record_count": len(absent) if absent is not None else None,
                         "absent_record_cause": "unknown; ICF, removal, optimization, or identifier limitations are not distinguished",
                         "executed_symbol_records": list(runtime.values())}}


def host_mentions(catalog):
    result = defaultdict(set)
    for path in (ROOT / "src").glob("game_hosts*.cpp"):
        for match in re.finditer(r"(?<![0-9a-fA-F])([0-9a-fA-F]{8})(?![0-9a-fA-F])",
                                 path.read_text(encoding="utf-8", errors="replace")):
            address = int(match.group(1), 16)
            if address in catalog.functions:
                result[address].add(path.relative_to(ROOT).as_posix())
    return result


def report(catalog, resolver, original, bsp, originals, bsps):
    mentions = host_mentions(catalog)
    rows = []
    for address, function in sorted(catalog.functions.items()):
        wire = resolver.rows.get(address, {})
        orig_hit = address in original["hits"] if original is not None else None
        if original is not None and address not in catalog.range_starts and not orig_hit:
            orig_hit = None
        bsp_hit = None
        if bsp is not None:
            if address in bsp["hits"]:
                bsp_hit = True
            elif bsp["complete"] and address in bsp["present"] and address not in bsp["possible"]:
                bsp_hit = False
        rows.append({"address": f"{address:08x}", "name": function["name"],
                     "board_game": function["board_game"], "category": function["category"],
                     "tag_category": function["tag_category"],
                     "scope": "game" if function["board_game"] else "eh_funclet" if function["eh_funclet"] else "excluded_by_board",
                     "eh_funclet": function["eh_funclet"], "funclet_parent":
                         f"{catalog.parents[address]:08x}" if address in catalog.parents else None,
                     "has_function_ledger": function["recon_kind"] == "function",
                     "reconstruction_status": function["recon_status"],
                     "orig_executed": orig_hit, "bsp_executed": bsp_hit,
                     "orig_distinct_blocks": len(original["hits"].get(address, ())) if original is not None else None,
                     "bsp_distinct_blocks": len(bsp["hits"].get(address, ())) if bsp is not None else None,
                     "orig_entry_count": original["entry_counts"].get(address) if original is not None else None,
                     "bsp_entry_count": bsp["entry_counts"].get(address) if bsp is not None else None,
                     "original_caller_count": catalog.callers.get(address, 0),
                     "wire_state": wire.get("state"), "ident": wire.get("ident"),
                     "ident_via": wire.get("ident_via"), "source": wire.get("source", function["recon_source"]),
                     "implementation_source": wire.get("implementation_source"),
                     "stand_in_hosts": sorted(mentions.get(address, ()))})

    def cross_key(row):
        return f"orig_{'unknown' if row['orig_executed'] is None else 'yes' if row['orig_executed'] else 'no'}_bsp_{'unknown' if row['bsp_executed'] is None else 'yes' if row['bsp_executed'] else 'no'}"

    game_rows = [row for row in rows if row["board_game"]]
    cross = Counter(cross_key(row) for row in game_rows)
    frontier = sorted((row for row in game_rows if row["orig_executed"] is True and row["bsp_executed"] is False),
                      key=lambda row: (-row["orig_distinct_blocks"], -row["original_caller_count"], row["address"]))
    frontier_category, frontier_host = defaultdict(list), defaultdict(list)
    for row in frontier:
        frontier_category[row["category"]].append(row["address"])
        for host in row["stand_in_hosts"] or ["no_textual_host_mention"]:
            frontier_host[host].append(row["address"])
    def side_summary(side, data, sources):
        if data is None:
            return {"status": "not_measured", "board_game_functions_executed": None, "inputs": sources}
        executed = [row for row in rows if row[f"{side}_executed"] is True]
        scopes = defaultdict(Counter)
        for row in executed:
            scopes[row["scope"]][row["category"]] += 1
        return {"status": "attributed" if side == "orig" or data["complete"] else "attribution_incomplete",
                "board_game_functions_executed": sum(row["board_game"] for row in executed),
                "board_game_with_function_ledger_executed": sum(row["board_game"] and row["has_function_ledger"] for row in executed),
                "all_identified_functions_executed": len(executed),
                "eh_funclets_executed": sum(row["eh_funclet"] for row in executed),
                "executed_without_function_ledger": sum(not row["has_function_ledger"] for row in executed),
                "board_game_executed_without_function_ledger": sum(row["board_game"] and not row["has_function_ledger"] for row in executed),
                "by_category": dict(Counter(row["category"] for row in executed if row["board_game"])),
                "by_scope": dict(Counter(row["scope"] for row in executed)),
                "by_scope_category": {scope: dict(counts) for scope, counts in scopes.items()},
                "board_game_unknown_cells": sum(row[f"{side}_executed"] is None for row in game_rows),
                "mapped_addresses_without_index_function": sum(address not in catalog.functions for address in data["hits"]),
                "count_semantics": ("PDB source/qualified-identifier verified reconstruction-body mappings in supplied observations; candidate/host reaches alone are excluded"
                                    if side == "bsp" else "identified mapped reach in supplied observations; unknown/ambiguous mappings remain outside this count"),
                "inputs": sources}
    original_details = None
    if original is not None:
        original_details = {"undefined_code": [{"address": f"{a:08x}", "distinct_blocks": len(blocks)}
                                               for a, blocks in sorted(original["undefined_hits"].items())],
                            "undefined_chunks_executed": len(original["undefined_hits"]),
                            "distinct_blocks_with_undefined_overlap": len(set().union(*original["undefined_hits"].values()))
                                if original["undefined_hits"] else 0,
                            "unattributed_ranges": [[f"{lo:08x}", f"{hi:08x}"] for lo, hi in original["unattributed_ranges"]],
                            "unattributed_bytes": sum(hi - lo for lo, hi in original["unattributed_ranges"]),
                            "distinct_blocks_with_unattributed_bytes": original["unattributed_block_count"],
                            "distinct_blocks_with_eh_overlap": len(set().union(*(blocks for address, blocks in original["hits"].items()
                                if catalog.functions[address]["eh_funclet"])))
                                if any(catalog.functions[address]["eh_funclet"] for address in original["hits"]) else 0,
                            "other_counts": original["counts"]}
    return {"schema_version": 1, "kind": "execution_coverage_census", "mission": None,
            "generated_utc": datetime.now(timezone.utc).isoformat(),
            "summary": {"original": side_summary("orig", original, originals),
                        "bsp": side_summary("bsp", bsp, bsps), "board_game_cross_table": dict(cross)},
            "original_details": original_details, "bsp_details": bsp["metadata"] if bsp else None,
            "unindexed_original_rows": [{"address": f"{address:08x}", "name": wire.get("name"),
                                          "source": wire.get("source"), "ident_via": wire.get("ident_via"),
                                          "wire_state": wire.get("state"), "orig_executed": None,
                                          "bsp_executed": True if bsp and address in bsp["hits"] else None,
                                          "scope": "unknown_no_index_function"}
                                         for address, wire in sorted(resolver.rows.items()) if address not in catalog.functions],
            "per_original_address": rows, "wiring_frontier": [row["address"] for row in frontier],
            "frontier_by_category": dict(frontier_category), "frontier_by_stand_in_host": dict(frontier_host),
            "orig_yes_bsp_unknown": [row["address"] for row in game_rows if row["orig_executed"] is True and row["bsp_executed"] is None],
            "orig_no_bsp_yes": [row["address"] for row in game_rows if row["orig_executed"] is False and row["bsp_executed"] is True],
            "orig_no_bsp_no": [row["address"] for row in game_rows if row["orig_executed"] is False and row["bsp_executed"] is False],
            "limits": ["Coverage establishes reach in supplied logs, not correctness, parity, or call frequency.",
                       "No supplied trace or unavailable attribution yields unknown cells, not no-execution.",
                       "Distinct drcov block records are deduplicated; multiplicity is never an entry/call count.",
                       "Original ranges are the index's contiguous swept ranges, not arbitrary Ghidra body unions.",
                       "EH/undefined overlap counts are not additive partitions of block counts.",
                       "Board categories reuse current rules; the historical/reference target gate is a separate denominator.",
                       "Shared PDB RVAs leave logical callers ambiguous; ICF/removal/optimization causes are not inferred.",
                       "PDB suffix/leaf and host/adapter candidate reaches alone do not credit reconstruction-body execution; known source mismatch rejects.",
                       "wire_readiness ident_via and file-level OFF-switch caveats remain; source wiring is not runtime reach.",
                       "False means not observed in supplied usable logs, not dead code or unreachable in other missions.",
                       "Drcov has no per-block game timestamps; clock markers do not filter startup/menu blocks out of these logs."]}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original-log", type=Path, action="append", default=[])
    parser.add_argument("--bsp-log", type=Path, action="append", default=[])
    parser.add_argument("--original-module", default="battlestationspacific.exe")
    parser.add_argument("--bsp-module", default="bsp_game.exe")
    parser.add_argument("--original-image", type=Path)
    parser.add_argument("--bsp-image", type=Path)
    parser.add_argument("--pdb", type=Path)
    parser.add_argument("--dbghelp", default="dbghelp.dll")
    parser.add_argument("--max-pdb-addresses", type=int, default=5_000_000)
    parser.add_argument("--index", type=Path, default=ROOT / "local/bsp_index.sqlite")
    parser.add_argument("--wire", type=Path, default=ROOT / "local/output/wire_readiness.json")
    parser.add_argument("--mission", default="unspecified")
    parser.add_argument("--output", type=Path, default=ROOT / "local/output/coverage_census.json")
    args = parser.parse_args(argv)
    if args.max_pdb_addresses <= 0:
        parser.error("--max-pdb-addresses must be positive")
    try:
        if args.output.exists():
            raise CensusError(f"refusing to overwrite output: {args.output}")
        catalog = Catalog(args.index)
        wire = json.loads(args.wire.read_text(encoding="utf-8")) if args.wire.exists() else {}
        resolver = Resolver(wire)
        images = {"original": pe_info(args.original_image) if args.original_image else None,
                  "bsp": pe_info(args.bsp_image) if args.bsp_image else None}
        sources, observations = {}, {}
        for side, logs, module in (("original", args.original_log, args.original_module), ("bsp", args.bsp_log, args.bsp_module)):
            sources[side], observations[side] = [], set()
            if len({str(path.resolve()) for path in logs}) != len(logs):
                raise CensusError("duplicate input log path")
            for path in logs:
                found, receipt = read_drcov(path, module, images[side], original=side == "original")
                observations[side].update(found)
                sources[side].append(receipt)
        original = attribute_original(observations["original"], catalog) if args.original_log else None
        bsp, bsp_error = None, None
        if args.bsp_log:
            if not args.bsp_image or not args.pdb:
                bsp_error = "--bsp-image and matching --pdb are required for PDB attribution"
            elif not wire:
                bsp_error = "wire_readiness.json unavailable; original identifier attribution unknown"
            else:
                symbols = None
                try:
                    symbols = PdbSymbols(images["bsp"], args.pdb, args.dbghelp)
                    bsp = attribute_bsp(observations["bsp"], symbols, resolver, args.max_pdb_addresses)
                except (CensusError, OSError, ValueError) as error:
                    bsp_error = str(error)
                finally:
                    if symbols:
                        symbols.close()
        result = report(catalog, resolver, original, bsp, sources["original"], sources["bsp"])
        result["mission"] = args.mission
        for side, logs in (("original", args.original_log), ("bsp", args.bsp_log)):
            result["summary"][side]["distinct_selected_blocks"] = len(observations[side]) if logs else None
        if bsp_error:
            result["summary"]["bsp"].update(status="attribution_unavailable", reason=bsp_error)
        result["provenance"] = {"head": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                                "index": str(args.index.resolve()), "index_meta": catalog.meta,
                                "wire": str(args.wire.resolve()) if args.wire.exists() else None,
                                "wire_sha256": sha256(args.wire) if args.wire.exists() else None,
                                "wire_inputs": wire.get("inputs"), "images": images,
                                "tool_sha256": sha256(__file__), "board_tool_sha256": sha256(board.__file__)}
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("x", encoding="utf-8") as stream:
            json.dump(result, stream, indent=1)
            stream.write("\n")
        compact = {side: {key: value for key, value in result["summary"][side].items()
                          if key not in ("inputs", "by_category", "by_scope_category")} for side in ("original", "bsp")}
        print(json.dumps({"output": str(args.output.resolve()), "summary": compact,
                          "cross_table": result["summary"]["board_game_cross_table"]}, indent=1))
        return 0 if not bsp_error else 2
    except (CensusError, OSError, ValueError, sqlite3.Error, struct.error) as error:
        print(f"coverage_census: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
