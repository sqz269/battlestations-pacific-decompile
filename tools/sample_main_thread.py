#!/usr/bin/env python3
"""Sample where bsp_game's main thread spends a run's wall time. Read-only, no elevation.

Usage (start it, then launch the run through tools/run_game.ps1 with the same -Log path):
    python tools/sample_main_thread.py --log local\\X.log [--out local\\X_samples.jsonl] [--map build\\win32\\bsp_game.map] [--period-ms 5]
    python tools/sample_main_thread.py --log local\\X.log --report-only   (re-read an existing --out)

It waits for the bootstrap child whose command line carries the --log path, then until the
process exits: suspends each sampled thread, reads EIP/ESP (Wow64GetThreadContext) and 32 KB of
stack, resumes it. Callers come from stack scanning (a dword in a module's code preceded by a
CALL), symbolized from the MSVC map file and the system DLLs' export tables. The main thread
(earliest created) gets full stacks every round, other threads a leaf every 20th round. The
report splits the main thread's mission-phase samples into logging / rendering / interface /
simulation / other by the innermost matching frame (docs/TOOLING.md section 5).
"""
import ctypes
import ctypes.wintypes as wt
import bisect
import collections
import json
import os
import re
import struct
import subprocess

import time

k32 = ctypes.WinDLL("kernel32", use_last_error=True)
psapi = ctypes.WinDLL("psapi", use_last_error=True)
k32.OpenProcess.restype = ctypes.c_void_p
k32.OpenThread.restype = ctypes.c_void_p
k32.CreateToolhelp32Snapshot.restype = ctypes.c_void_p
for _f in ("Wow64SuspendThread", "ResumeThread", "Wow64GetThreadContext", "CloseHandle", "GetExitCodeProcess"):
    getattr(k32, _f).argtypes = None
k32.Wow64SuspendThread.argtypes = [ctypes.c_void_p]
k32.Wow64SuspendThread.restype = wt.DWORD
k32.ResumeThread.argtypes = [ctypes.c_void_p]
k32.ResumeThread.restype = wt.DWORD
k32.Wow64GetThreadContext.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
k32.GetExitCodeProcess.argtypes = [ctypes.c_void_p, ctypes.POINTER(wt.DWORD)]
k32.ReadProcessMemory.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)]
k32.GetThreadTimes.argtypes = [ctypes.c_void_p] + [ctypes.POINTER(ctypes.c_ulonglong)] * 4
k32.Thread32First.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
k32.Thread32Next.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
k32.CloseHandle.argtypes = [ctypes.c_void_p]
psapi.EnumProcessModulesEx.argtypes = [ctypes.c_void_p, ctypes.c_void_p, wt.DWORD, ctypes.POINTER(wt.DWORD), wt.DWORD]
psapi.GetModuleFileNameExW.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_wchar_p, wt.DWORD]
psapi.GetModuleInformation.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, wt.DWORD]

PROCESS_ALL = 0x0400 | 0x0010  # QUERY_INFORMATION | VM_READ
THREAD_ACCESS = 0x0002 | 0x0008 | 0x0040  # SUSPEND_RESUME | GET_CONTEXT | QUERY_INFORMATION
WOW64_CONTEXT_CONTROL = 0x00010001
WOW64_CONTEXT_SIZE = 716


class THREADENTRY32(ctypes.Structure):
    _fields_ = [("dwSize", wt.DWORD), ("cntUsage", wt.DWORD), ("th32ThreadID", wt.DWORD),
                ("th32OwnerProcessID", wt.DWORD), ("tpBasePri", wt.LONG),
                ("tpDeltaPri", wt.LONG), ("dwFlags", wt.DWORD)]


def find_child(log_path):
    """The bootstrap child carries --bsp-native-data-handoff and our --log path."""
    name = os.path.basename(log_path).lower()
    ps = ("Get-CimInstance Win32_Process -Filter \"Name='bsp_game.exe'\" | "
          "ForEach-Object { \"$($_.ProcessId)`t$($_.CommandLine)\" }")
    while True:
        out = subprocess.run(["powershell", "-NoProfile", "-Command", ps],
                             capture_output=True, text=True).stdout
        for line in out.splitlines():
            if "\t" not in line:
                continue
            pid, cmd = line.split("\t", 1)
            if "--bsp-native-data-handoff" in cmd and name in cmd.lower():
                return int(pid)
        time.sleep(0.5)


def threads_of(pid):
    snap = k32.CreateToolhelp32Snapshot(0x4, 0)
    te = THREADENTRY32()
    te.dwSize = ctypes.sizeof(te)
    tids = []
    if k32.Thread32First(snap, ctypes.byref(te)):
        while True:
            if te.th32OwnerProcessID == pid:
                tids.append(te.th32ThreadID)
            if not k32.Thread32Next(snap, ctypes.byref(te)):
                break
    k32.CloseHandle(snap)
    return tids


def modules_of(hproc):
    arr = (ctypes.c_void_p * 1024)()
    needed = wt.DWORD()
    psapi.EnumProcessModulesEx(hproc, arr, ctypes.sizeof(arr), ctypes.byref(needed), 0x01)  # 32-bit
    mods = []
    for i in range(needed.value // ctypes.sizeof(ctypes.c_void_p)):
        h = arr[i]
        if not h:
            continue
        name = ctypes.create_unicode_buffer(520)
        psapi.GetModuleFileNameExW(hproc, ctypes.c_void_p(h), name, 520)

        class MI(ctypes.Structure):
            _fields_ = [("base", ctypes.c_void_p), ("size", wt.DWORD), ("entry", ctypes.c_void_p)]
        mi = MI()
        psapi.GetModuleInformation(hproc, ctypes.c_void_p(h), ctypes.byref(mi), ctypes.sizeof(mi))
        mods.append((mi.base, mi.base + mi.size, name.value))
    return sorted(mods)


def pe_sections_and_exports(path):
    data = open(path, "rb").read()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    optsz = struct.unpack_from("<H", data, pe + 20)[0]
    opt = pe + 24
    exp_rva = struct.unpack_from("<I", data, opt + 96)[0]
    secs = []
    for i in range(nsec):
        off = opt + optsz + 40 * i
        vsz, va, rawsz, rawptr, chars = struct.unpack_from("<IIII", data, off + 8) + (struct.unpack_from("<I", data, off + 36)[0],)
        secs.append((va, vsz, rawptr, rawsz, chars))

    def rva2off(rva):
        for va, vsz, rp, rs, _ in secs:
            if va <= rva < va + max(vsz, rs):
                return rva - va + rp
        return None
    exports = []
    if exp_rva:
        o = rva2off(exp_rva)
        if o is not None:
            nfunc, nnames, afunc, aname, aord = struct.unpack_from("<IIIII", data, o + 20)
            fo, no, oo = rva2off(afunc), rva2off(aname), rva2off(aord)
            for i in range(nnames):
                nrva = struct.unpack_from("<I", data, no + 4 * i)[0]
                ordv = struct.unpack_from("<H", data, oo + 2 * i)[0]
                frva = struct.unpack_from("<I", data, fo + 4 * ordv)[0]
                noff = rva2off(nrva)
                nm = data[noff:data.index(b"\0", noff)].decode("ascii", "replace")
                exports.append((frva, nm))
    return data, secs, rva2off, sorted(exports)


class ModuleSyms:
    def __init__(self, base, end, path, map_syms=None):
        self.base, self.end, self.path = base, end, path
        self.name = os.path.basename(path)
        try:
            self.data, self.secs, self.rva2off, exports = pe_sections_and_exports(path)
        except Exception:
            self.data, self.secs, self.rva2off, exports = b"", [], lambda r: None, []
        if map_syms is not None:
            self.syms = map_syms
        else:
            self.syms = [(base + rva, nm) for rva, nm in exports]
        self.keys = [a for a, _ in self.syms]
        self.code = [(base + va, base + va + vsz) for va, vsz, _, _, ch in self.secs if ch & 0x20]

    def in_code(self, a):
        return any(lo <= a < hi for lo, hi in self.code)

    def byte_at(self, a):
        off = self.rva2off(a - self.base)
        return None if off is None or off >= len(self.data) else self.data[off]

    def preceded_by_call(self, a):
        def b(k):
            return self.byte_at(a - k)
        if b(5) == 0xE8:
            return True
        if b(6) == 0xFF and b(5) is not None and (b(5) & 0x38) == 0x10:
            return True
        if b(2) == 0xFF and b(1) is not None and (b(1) & 0xF8) == 0xD0:
            return True
        if b(3) == 0xFF and b(2) is not None and (b(2) & 0x38) == 0x10:
            return True
        return False

    def sym(self, a):
        i = bisect.bisect_right(self.keys, a) - 1
        return self.syms[i][1] if i >= 0 else "?"


def load_map(path, base):
    syms = []
    pat = re.compile(r"^\s*[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s", re.I)
    for line in open(path, encoding="latin-1"):
        m = pat.match(line)
        if m:
            addr = int(m.group(2), 16)
            if addr >= 0x10000000:
                syms.append((addr - 0x10000000 + base, m.group(1)))
    return sorted(syms)


def sample(log_path, map_path, out_path, period):
    pid = find_child(log_path)
    hproc = k32.OpenProcess(PROCESS_ALL, False, pid)
    time.sleep(3.0)
    mods, loaded_names = [], set()

    def refresh_modules():
        for lo, hi, path in modules_of(hproc):
            if path in loaded_names:
                continue
            loaded_names.add(path)
            ms = None
            if os.path.basename(path).lower() == "bsp_game.exe":
                ms = ModuleSyms(lo, hi, path, load_map(map_path, lo))
            else:
                ms = ModuleSyms(lo, hi, path)
            mods.append(ms)
        mods.sort(key=lambda m: m.base)
        bases[:] = [m.base for m in mods]
    bases = []
    refresh_modules()
    handles = {}
    ctx = ctypes.create_string_buffer(WOW64_CONTEXT_SIZE)
    buf = ctypes.create_string_buffer(65536)
    got = ctypes.c_size_t()
    out = open(out_path, "w", encoding="utf-8")
    t0 = time.perf_counter()
    last_refresh = t0
    main_tid = render_tid = None
    rounds = 0
    deep = set()

    def earliest(tids):
        best = None
        for tid in tids:
            h = k32.OpenThread(0x0040, False, tid)
            if not h:
                continue
            c, e, kt, ut = (ctypes.c_ulonglong() for _ in range(4))
            if k32.GetThreadTimes(h, ctypes.byref(c), ctypes.byref(e), ctypes.byref(kt), ctypes.byref(ut)):
                if best is None or c.value < best[0]:
                    best = (c.value, tid)
            k32.CloseHandle(h)
        return best[1] if best else None
    n = 0
    while True:
        code = wt.DWORD()
        k32.GetExitCodeProcess(hproc, ctypes.byref(code))
        if code.value != 259:
            break
        now = time.perf_counter()
        if now - last_refresh > 2.0:
            refresh_modules()
            last_refresh = now
        tids = threads_of(pid)
        if main_tid is None and tids:
            main_tid = earliest(tids)
            deep.add(main_tid)
        rounds += 1
        try:
            log_size = os.path.getsize(log_path)
        except OSError:
            log_size = 0
        for tid in tids:
            if tid not in deep and rounds % 20:
                continue
            h = handles.get(tid)
            if h is None:
                h = k32.OpenThread(THREAD_ACCESS, False, tid)
                handles[tid] = h
            if not h:
                continue
            if k32.Wow64SuspendThread(h) == 0xFFFFFFFF:
                continue
            try:
                struct.pack_into("<I", ctx, 0, WOW64_CONTEXT_CONTROL)
                if not k32.Wow64GetThreadContext(h, ctx):
                    continue
                eip = struct.unpack_from("<I", ctx, 184)[0]
                esp = struct.unpack_from("<I", ctx, 196)[0]
                size = 32768 if tid in deep else 0
                while size >= 4096:
                    if k32.ReadProcessMemory(hproc, ctypes.c_void_p(esp), buf, size, ctypes.byref(got)):
                        break
                    size //= 2
                stack = buf.raw[:got.value] if size >= 4096 else b""
            finally:
                k32.ResumeThread(h)
            frames = []

            def modof(a):
                i = bisect.bisect_right(bases, a) - 1
                if i >= 0 and a < mods[i].end:
                    return mods[i]
                return None
            lm = modof(eip)
            leaf = f"{lm.name}!{lm.sym(eip)}" if lm else f"?{eip:08x}"
            if tid not in deep:
                stack = b""
            for i in range(0, len(stack) - 3, 4):
                a = struct.unpack_from("<I", stack, i)[0]
                m = modof(a)
                if m is not None and m.in_code(a) and m.preceded_by_call(a):
                    frames.append(f"{m.name}!{m.sym(a)}")
                    if len(frames) >= 60:
                        break
            role = "main" if tid == main_tid else "render" if tid == render_tid else "other"
            out.write(json.dumps({"t": round(now - t0, 4), "tid": tid, "role": role, "log": log_size, "leaf": leaf, "frames": frames}) + "\n")
            n += 1
        time.sleep(period)
    out.close()
    print(f"samples {n} over {time.perf_counter() - t0:.1f} s, pid {pid}, main {main_tid}, render {render_tid}")



RENDER_MOD = re.compile(r"^(d3d9|d3dx9|nvd3dum|nvldumd|nvwgf|igd|aticfx|amdxc|atiu|dxgi|d3d1|nvapi|nvgpucomp)", re.I)
LOG_SYM = re.compile(r"GameHostLog|fprintf|fwrite|fflush|_write|WriteFile|stdio_common")
RENDER_SYM = re.compile(r"Render|render|Present|present|D3D|Direct3D|Draw|draw|Shader|shader|GameDeviceHost|Texture|texture")
UI_SYM = re.compile(r"Gui|gui|Hud|hud|Text|Widget|widget|Font|font|Minimap|minimap|Menu|menu")
SIM_SYM = re.compile(r"run_mission_frame_004e4a40|fixed_step|FixedStep")
WAIT_SYM = re.compile(r"Wait|Delay|Yield|Sleep|SignalAndWait|ZwRemoveIoCompletion|NtRemoveIoCompletion")


def classify(r):
    chain = [r["leaf"]] + r["frames"]
    for f in chain:
        mod, _, sym = f.partition("!")
        if LOG_SYM.search(sym):
            return "logging"
        if RENDER_MOD.match(mod) or RENDER_SYM.search(sym):
            return "rendering"
        if UI_SYM.search(sym):
            return "interface (gui/hud/text)"
    if any(SIM_SYM.search(f) for f in chain):
        return "simulation"
    return "other"



def report(samples_path, log_path):
    log = open(log_path, "rb").read()
    start = log.find(b"\n  mission frame 1 ")
    end = log.find(b"frame limit ")
    print(f"log bytes {len(log)}; mission starts at byte {start}, frame limit at byte {end}")
    rows = [json.loads(l) for l in open(samples_path)]
    phase = lambda r: "load" if r["log"] < start else ("mission" if end < 0 or r["log"] < end else "shutdown")
    for role in ("main", "render"):
        sel = [r for r in rows if r["role"] == role]
        if not sel:
            continue
        t = [r["t"] for r in sel]
        print(f"\n== {role} thread: {len(sel)} samples over {t[-1] - t[0]:.1f} s")
        by_phase = collections.defaultdict(list)
        for r in sel:
            by_phase[phase(r)].append(r)
        for ph, rs in by_phase.items():
            span = rs[-1]["t"] - rs[0]["t"]
            c = collections.Counter(classify(r) for r in rs)
            w = sum(1 for r in rs if WAIT_SYM.search(r["leaf"]))
            print(f"  {ph}: {len(rs)} samples, {span:.1f} s wall; leaf in a wait {w} ({100*w/len(rs):.1f}%)")
            for k, v in c.most_common():
                print(f"    {k:<28} {v:6d}  {100*v/len(rs):5.1f}%")
            if ph == "mission":
                leafs = collections.Counter(r["leaf"].split("@@")[0][:90] for r in rs)
                print("    top leaves:")
                for k, v in leafs.most_common(12):
                    print(f"      {v:6d} {100*v/len(rs):5.1f}%  {k}")
                logsplit = collections.Counter()
                for r in rs:
                    if classify(r) == "logging":
                        chain = [r["leaf"]] + r["frames"]
                        first = next(f for f in chain if LOG_SYM.search(f.partition("!")[2]))
                        logsplit[first.split("@@")[0][:80]] += 1
                print("    logging by innermost matching frame:")
                for k, v in logsplit.most_common(6):
                    print(f"      {v:6d}  {k}")


def cli():
    import argparse
    ap = argparse.ArgumentParser(description="read-only main-thread sampler for bsp_game")
    ap.add_argument("--log", required=True, help="the -Log path the run writes")
    ap.add_argument("--out", help="samples file (default: <log>_samples.jsonl)")
    ap.add_argument("--map", default=r"build\win32\bsp_game.map")
    ap.add_argument("--period-ms", type=float, default=5.0)
    ap.add_argument("--report-only", action="store_true")
    args = ap.parse_args()
    out = args.out or re.sub(r"\.log$", "", args.log) + "_samples.jsonl"
    if not args.report_only:
        sample(args.log, args.map, out, args.period_ms / 1000.0)
    report(out, args.log)


if __name__ == "__main__":
    cli()
