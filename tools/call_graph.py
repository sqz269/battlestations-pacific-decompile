"""Call graph of the built bsp_game.exe: who calls each function, from which sites, and whether the
game's entry point reaches it.

Read-only over a build directory. Inputs: <build-dir>/bsp_game.map (the linker map: section
contributions, public and static symbols) and <build-dir>/Release/bsp_game.exe (base relocations,
TLS callbacks, code bytes).

Edges run between map symbols (functions in code, data symbols elsewhere):
  call  a CALL rel32 from one function into another (capstone over each function's bytes);
  jmp   a JMP rel32 into another function (a tail call);
  ref   a base relocation (an absolute address) inside a function or data symbol that points into
        another symbol: function pointers, vtables, callback and dispatch tables, data pointing at data.
Roots: the entry point, the TLS callbacks, and the pointers in the CRT/TLS initializer tables
(.CRT$X*, .tls$*, .rtc$*), which the CRT walks by bounds rather than by symbol.

What it measures is the compiler's view: `if constexpr` already dropped the arms that are off, and
/OPT:REF dropped code nothing references. A function reachable from the entry can run in the game; how
often it runs takes a game run. Inlined calls leave no edge, so the metric uses the analysis build
(--build: build/callgraph, compiled /Ob0). The regular build (build/win32) is fine for checking a call
across files, which MSVC does not inline without /GL.

Usage:
  python tools/call_graph.py [--build-dir build/win32]       write local/output/call_graph.json
  python tools/call_graph.py --build                          build build/callgraph (/Ob0), then analyse it
  python tools/call_graph.py who <address|name> [--limit N]   callers, call sites, reachability
  python tools/call_graph.py path <address|name>              one chain of callers from the entry
An <address> is the original-binary address in a reconstructed name (`foo_00a7b710`), 8 hex digits.
"""
import argparse
import bisect
import collections
import json
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "local" / "output" / "call_graph.json"
ANALYSIS_DIR = "build/callgraph"

SYM = re.compile(r"^\s*([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+(\S+)\s+([0-9a-fA-F]{8})\s+((?:[fi]\s+)*)(\S*)")
CONTRIB = re.compile(r"^\s*([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+([0-9a-fA-F]{8})H\s+(\S+)\s+(CODE|DATA)\s*$")
ADDR_SUFFIX = re.compile(r"_([0-9a-f]{8})(?=@|$)")
ROOT_CONTRIB = re.compile(r"^\.(CRT\$X|tls|rtc)", re.I)
FUNCLET = re.compile(r"^[_A-Za-z]*\$")
HOST_OBJ = re.compile(r"(^|:)game_hosts?[_.]", re.I)  # host files: their address-named methods are stand-ins


def short_name(mangled):
    """A readable name for display (not a full undecoration)."""
    m = mangled
    if m.startswith("??_7"):
        return "::".join(reversed(m[4:].split("@@")[0].split("@"))) + "::`vftable'"
    if m.startswith("??0") or m.startswith("??1"):
        parts = m[3:].split("@@")[0].split("@")
        cls = parts[0]
        return "::".join(reversed(parts)) + "::" + ("~" if m[2] == "1" else "") + cls
    if m.startswith("?"):
        parts = m[1:].split("@@")[0].split("@")
        return "::".join(reversed([p for p in parts if p])) or m
    m = m.lstrip("_")
    return m.split("@")[0] or mangled


def parse_map(path):
    contribs, syms = [], []
    in_syms = False
    for line in path.read_text(errors="replace").splitlines():
        if not in_syms:
            m = CONTRIB.match(line)
            if m:
                contribs.append((int(m.group(1), 16), int(m.group(2), 16), int(m.group(3), 16), m.group(4), m.group(5)))
                continue
            if "Publics by Value" in line:
                in_syms = True
            continue
        m = SYM.match(line)
        if not m or int(m.group(1), 16) == 0:
            continue
        syms.append((int(m.group(1), 16), int(m.group(2), 16), m.group(3), int(m.group(4), 16),
                     "f" in m.group(5), m.group(6)))
    return contribs, syms


def analyse(build_dir):
    import capstone
    import pefile

    bdir = (ROOT / build_dir) if not Path(build_dir).is_absolute() else Path(build_dir)
    map_path, exe_path = bdir / "bsp_game.map", bdir / "Release" / "bsp_game.exe"
    for p in (map_path, exe_path):
        if not p.is_file():
            sys.exit(f"missing {p}: build first (./scripts/build.ps1, or --build for the analysis build)")
    t0 = time.time()
    pe = pefile.PE(str(exe_path), fast_load=True)
    pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_BASERELOC"],
                                           pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_TLS"]])
    base = pe.OPTIONAL_HEADER.ImageBase
    img = pe.get_memory_mapped_image()
    sec_va = {i + 1: base + s.VirtualAddress for i, s in enumerate(pe.sections)}

    contribs, syms = parse_map(map_path)
    # section contributions as VA ranges
    cranges = sorted((sec_va[s] + off, sec_va[s] + off + ln, name, cls) for s, off, ln, name, cls in contribs if s in sec_va)
    cstarts = [c[0] for c in cranges]

    def contrib_of(va):
        i = bisect.bisect_right(cstarts, va) - 1
        return cranges[i] if i >= 0 and va < cranges[i][1] else None

    # nodes keyed by VA (identical-COMDAT folding puts several names on one VA)
    names, is_func, obj = collections.defaultdict(list), {}, {}
    for s, off, name, va, f, o in syms:
        c = contrib_of(va)
        if not c:
            continue
        names[va].append(name)
        is_func[va] = is_func.get(va, False) or f or c[3] == "CODE"
        obj.setdefault(va, o)
    starts = sorted(names)
    ends = {}
    for i, va in enumerate(starts):
        c = contrib_of(va)
        nxt = starts[i + 1] if i + 1 < len(starts) else c[1]
        ends[va] = min(nxt, c[1])

    def node_of(va):
        """(node VA or None, contribution name) for an address."""
        c = contrib_of(va)
        if not c:
            return None, None
        i = bisect.bisect_right(starts, va) - 1
        if i >= 0 and starts[i] >= c[0] and va < ends[starts[i]]:
            return starts[i], c[2]
        return None, c[2]

    edges = collections.defaultdict(lambda: [0, None])  # (src, dst, kind) -> [sites, first site]
    roots = set()

    def add(src, dst, kind, site):
        e = edges[(src, dst, kind)]
        e[0] += 1
        if e[1] is None:
            e[1] = site

    # direct calls and tail jumps
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    n_insn = 0
    for va in starts:
        if not is_func[va]:
            continue
        end = ends[va]
        code = img[va - base:end - base]
        off = 0
        while off < len(code):
            last = None
            for a, size, mn, ops in md.disasm_lite(code[off:], va + off):
                last = a + size
                n_insn += 1
                if (mn == "call" or mn == "jmp") and ops.startswith("0x"):
                    tgt = int(ops, 16)
                    dst, _ = node_of(tgt)
                    if dst is not None and dst != va and is_func.get(dst):
                        add(va, dst, mn, a)
            off = (last - va) if last is not None and last - va > off else off + 1

    # absolute references (base relocations)
    orphan_refs = collections.Counter()
    for block in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", []):
        for ent in block.entries:
            if ent.type != 3:
                continue
            loc = base + ent.rva
            val = int.from_bytes(img[ent.rva:ent.rva + 4], "little")
            dst, _ = node_of(val)
            if dst is None:
                continue
            src, cname = node_of(loc)
            if src is None:
                if cname and ROOT_CONTRIB.match(cname):
                    roots.add(dst)
                else:
                    orphan_refs[cname] += 1
                continue
            if src != dst:
                add(src, dst, "ref", loc)

    # roots: entry point, TLS callbacks
    entry, _ = node_of(base + pe.OPTIONAL_HEADER.AddressOfEntryPoint)
    if entry is not None:
        roots.add(entry)
    tls = getattr(pe, "DIRECTORY_ENTRY_TLS", None)
    if tls and tls.struct.AddressOfCallBacks:
        p = tls.struct.AddressOfCallBacks - base
        while p + 4 <= len(img):
            cb = int.from_bytes(img[p:p + 4], "little")
            if not cb:
                break
            n, _ = node_of(cb)
            if n is not None:
                roots.add(n)
            p += 4

    out_edges = collections.defaultdict(list)
    for (s, d, k) in edges:
        out_edges[s].append(d)
    parent = {r: None for r in roots}
    queue = collections.deque(sorted(roots))
    while queue:
        v = queue.popleft()
        for d in out_edges.get(v, ()):
            if d not in parent:
                parent[d] = v
                queue.append(d)

    incoming = collections.defaultdict(list)
    for (s, d, k), (n, first) in edges.items():
        incoming[d].append([f"{s:08x}", k, n, f"{first:08x}"])
    # the last field counts the names folded onto one address (identical-code folding, /OPT:ICF). Above 1, a
    # caller can't be told apart from its twins' callers; the analysis build links /OPT:NOICF to avoid it.
    nodes = [[f"{va:08x}", names[va][0], "f" if is_func[va] else "d", ends[va] - va, obj[va], int(va in parent),
              len(set(names[va]))] for va in starts]
    aliases = {f"{va:08x}": sorted(set(names[va]))[:12] for va in starts if is_func[va] and len(set(names[va])) > 1}
    by_addr = collections.defaultdict(list)
    for va in starts:
        if not is_func[va]:
            continue
        for nm in names[va]:
            if FUNCLET.match(nm):  # unwindfunclet$?foo_xxxxxxxx..., __catch$...: EH pieces of a function, not it
                continue
            for m in ADDR_SUFFIX.finditer(nm):
                if f"{va:08x}" not in by_addr[m.group(1)]:
                    by_addr[m.group(1)].append(f"{va:08x}")
    # a reconstructed address is "called" when one of its functions outside the host files (whose
    # address-named methods are stand-ins) is reachable and something reachable other than itself calls or
    # references it. tools/wire_readiness.py matches each row to the function built from its own source file.
    called = 0
    for a, vas in by_addr.items():
        if any(int(v, 16) in parent and not HOST_OBJ.search(obj[int(v, 16)]) and len(set(names[int(v, 16)])) == 1
               and any(int(s, 16) in parent and s != v for s, _, _, _ in incoming.get(int(v, 16), []))
               for v in vas):
            called += 1
    summary = {"functions": sum(1 for v in starts if is_func[v]),
               "reachable_functions": sum(1 for v in starts if is_func[v] and v in parent),
               "data_symbols": sum(1 for v in starts if not is_func[v]),
               "edges": len(edges), "roots": len(roots), "instructions": n_insn,
               "recon_addresses": sum(1 for vas in by_addr.values()
                                      if any(not HOST_OBJ.search(obj[int(v, 16)]) for v in vas)),
               "recon_called": called,
               "orphan_refs_ignored": sum(orphan_refs.values()),
               "folded_functions": len(aliases)}
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    meta = {"build_dir": str(bdir), "map_mtime": map_path.stat().st_mtime, "exe_mtime": exe_path.stat().st_mtime,
            "head": head, "no_inline": (bdir.name == Path(ANALYSIS_DIR).name), "seconds": round(time.time() - t0, 1),
            "orphan_contributions": dict(orphan_refs.most_common(8))}
    data = {"meta": meta, "summary": summary, "nodes": nodes,
            "incoming": {f"{k:08x}": v for k, v in incoming.items()},
            "parent": {f"{k:08x}": (f"{v:08x}" if v is not None else None) for k, v in parent.items()},
            "by_addr": by_addr, "aliases": aliases}
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(data, separators=(",", ":")), encoding="utf-8")
    return data


def load():
    if not OUT.is_file():
        sys.exit(f"no {OUT.relative_to(ROOT)}; run python tools/call_graph.py first")
    return json.loads(OUT.read_text(encoding="utf-8"))


def find(data, query):
    q = query.lower().removeprefix("0x")
    if re.fullmatch(r"[0-9a-f]{8}", q) and q in data["by_addr"]:
        return data["by_addr"][q]
    hits = [n[0] for n in data["nodes"] if q in n[1].lower() and n[2] == "f"]
    return hits


def who(data, query, limit):
    nodes = {n[0]: n for n in data["nodes"]}
    vas = find(data, query)
    if not vas:
        print(f"no function in this build matches {query!r}: not linked, inlined everywhere (use the --build "
              f"analysis build), or misspelt")
        return
    meta = data["meta"]
    print(f"build {meta['build_dir']} at {meta['head'][:10]}{' (no inlining)' if meta.get('no_inline') else ''}")
    for va in vas[:limit]:
        n = nodes[va]
        inc = data["incoming"].get(va, [])
        live = [e for e in inc if e[0] in data["parent"]]
        calls = sum(e[2] for e in live if e[1] in ("call", "jmp"))
        refs = sum(e[2] for e in live if e[1] == "ref")
        al = data.get("aliases", {}).get(va, [])
        q = query.lower().removeprefix("0x")
        shown = next((a for a in al if q in a.lower()), n[1])
        print(f"{short_name(shown)} @{va} ({n[4]}): {'REACHABLE' if n[5] else 'not reachable from the entry'}; "
              f"{calls} call sites and {refs} references from reachable code; {len(inc) - len(live)} callers unreachable")
        if al:
            print(f"   FOLDED with {len(al) - 1} other function(s) at this address (identical code): these callers may "
                  f"belong to any of them, e.g. {short_name([a for a in al if a != shown][0])}. The --build analysis "
                  f"build links /OPT:NOICF.")
        for s, k, cnt, site in sorted(inc, key=lambda e: (e[0] not in data["parent"], -e[2]))[:limit]:
            sn = nodes.get(s)
            print(f"   <- {k:4s} x{cnt} at {site} from {short_name(sn[1]) if sn else s}"
                  f"{'' if s in data['parent'] else '  (unreachable)'}")


def path(data, query):
    nodes = {n[0]: n for n in data["nodes"]}
    for va in find(data, query)[:3]:
        chain, v = [], va
        if v not in data["parent"]:
            print(f"{short_name(nodes[va][1])} @{va}: not reachable from the entry")
            continue
        while v is not None and len(chain) < 60:
            chain.append(v)
            v = data["parent"].get(v)
        print(" <- ".join(f"{short_name(nodes[c][1])}" for c in chain))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("command", nargs="?", choices=["who", "path"])
    ap.add_argument("query", nargs="?")
    ap.add_argument("--build-dir", default="build/win32")
    ap.add_argument("--build", action="store_true", help=f"build {ANALYSIS_DIR} (/Ob0, bsp_game only) and analyse it")
    ap.add_argument("--limit", type=int, default=12)
    args = ap.parse_args()
    if args.command:
        if not args.query:
            sys.exit("give an address or a name")
        data = load()
        (who(data, args.query, args.limit) if args.command == "who" else path(data, args.query))
        return
    build_dir = args.build_dir
    if args.build:
        build_dir = ANALYSIS_DIR
        r = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                            str(ROOT / "scripts" / "build.ps1"), "-BuildDir", build_dir.replace("/", "\\"),
                            "-NoInline", "-Target", "bsp_game", "-SkipTests"], cwd=ROOT,
                           capture_output=True, text=True)
        if r.returncode:
            sys.exit("analysis build failed:\n" + (r.stdout + r.stderr)[-1500:])
    data = analyse(build_dir)
    s = data["summary"]
    print(f"{s['reachable_functions']}/{s['functions']} functions reachable from the entry; "
          f"{s['recon_called']}/{s['recon_addresses']} reconstructed addresses called; {s['edges']} edges, "
          f"{s['roots']} roots, {data['meta']['seconds']}s -> {OUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
