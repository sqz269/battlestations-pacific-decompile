"""Census of x87 register-form arithmetic whose destination is ST(i), read from the PE on disk.

Usage:
  python tools/x87_dc_form_scan.py [--range LO-HI] [--forms dc,de,d8] [--ops fmul,fsub,..]
         [--by-function] [--reconstructed] [--limit N] [--json PATH] [--binary PATH]

Why: Ghidra's listing prints the D8 /r and DC /r register forms of FADD/FMUL with the same
short text (`FMUL ST1` is both D8 C9 = fmul st(0),st(1) and DC C9 = fmul st(1),st(0)), so a
reader of the listing cannot tell which register receives the result. The DC forms write ST(i)
and leave ST0 untouched. Note also the Intel encoding trap: DC E0+i is FSUBR st(i),st(0)
(st(i) = st(0) - st(i)) and DC E8+i is FSUB st(i),st(0) (st(i) = st(i) - st(0)); the same holds
for DC F0+i FDIVR / DC F8+i FDIV and for the DE pop forms. See docs/CC13_X87_DC_FORM_AUDIT.md.

Every instruction is decoded by capstone from a function start taken from the bsp index
(local/bsp_index.sqlite, `functions` table) and swept linearly up to the next function start,
so a desync after embedded data cannot run across a function boundary. Ranges outside every
indexed function are not scanned. The owner is the nearest preceding indexed start; a site after
RET/JMP + INT3 padding is printed with `?` after the owner (`past_end` in JSON), because it lies in
an undefined function the index does not know (00414FE0, 009BE590, 009D6D60 and 00B052F0 were found
this way).

  --range          hex addresses LO-HI (default: the whole .text)
  --forms          opcode bytes to report: dc (default), de, d8 (comma list)
  --ops            only these mnemonics (capstone names: fadd fmul fsub fsubr fdiv fdivr,
                   and their p forms)
  --include-st0    also report i = 0 (DC C8 is fmul st0,st0: same result as D8 C8, unambiguous)
  --by-function    one line per function with its op counts instead of one line per site
  --reconstructed  only functions that carry a config/reconstruction record (recon table)
  --context N      also print N capstone instructions before and after each site
  --limit          maximum printed lines (default 60); the full result goes to --json
  --json           write every site as JSON
"""
import argparse
import json
import sqlite3
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Intel SDM semantics of the register forms, keyed by (opcode byte, modrm >> 3 & 7).
SEMANTICS = {
    (0xD8, 0): ("fadd", "st0 = st0 + st(i)"),
    (0xD8, 1): ("fmul", "st0 = st0 * st(i)"),
    (0xD8, 2): ("fcom", "compare st0, st(i)"),
    (0xD8, 3): ("fcomp", "compare st0, st(i); pop"),
    (0xD8, 4): ("fsub", "st0 = st0 - st(i)"),
    (0xD8, 5): ("fsubr", "st0 = st(i) - st0"),
    (0xD8, 6): ("fdiv", "st0 = st0 / st(i)"),
    (0xD8, 7): ("fdivr", "st0 = st(i) / st0"),
    (0xDC, 0): ("fadd", "st(i) = st(i) + st0"),
    (0xDC, 1): ("fmul", "st(i) = st(i) * st0"),
    (0xDC, 2): ("fcom", "compare st0, st(i) (undocumented alias)"),
    (0xDC, 3): ("fcomp", "compare st0, st(i); pop (undocumented alias)"),
    (0xDC, 4): ("fsubr", "st(i) = st0 - st(i)"),
    (0xDC, 5): ("fsub", "st(i) = st(i) - st0"),
    (0xDC, 6): ("fdivr", "st(i) = st0 / st(i)"),
    (0xDC, 7): ("fdiv", "st(i) = st(i) / st0"),
    (0xDE, 0): ("faddp", "st(i) = st(i) + st0; pop"),
    (0xDE, 1): ("fmulp", "st(i) = st(i) * st0; pop"),
    (0xDE, 2): ("fcomp", "compare st0, st(i); pop (undocumented alias)"),
    (0xDE, 3): ("fcompp", "compare st0, st1; pop twice (DE D9 only)"),
    (0xDE, 4): ("fsubrp", "st(i) = st0 - st(i); pop"),
    (0xDE, 5): ("fsubp", "st(i) = st(i) - st0; pop"),
    (0xDE, 6): ("fdivrp", "st(i) = st0 / st(i); pop"),
    (0xDE, 7): ("fdivp", "st(i) = st(i) / st0; pop"),
}
FORM_BYTES = {"d8": 0xD8, "dc": 0xDC, "de": 0xDE}


def load_text(binary):
    import pefile

    if binary is None:
        with open(ROOT / "config" / "target.json", encoding="utf-8") as fh:
            binary = json.load(fh)["binary"]
    pe = pefile.PE(binary, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    for s in pe.sections:
        if s.Name.rstrip(b"\0") == b".text":
            return base + s.VirtualAddress, s.get_data()
    sys.exit("no .text section")


def load_functions(db, reconstructed):
    if not Path(db).is_file() or Path(db).stat().st_size == 0:
        raise SystemExit(f'{db} is missing or empty; run: python tools/bsp.py index')
    # read-only: a plain connect() on a missing path creates an empty file that breaks every later bsp.py query
    con = sqlite3.connect(f'file:{Path(db).as_posix()}?mode=ro', uri=True)
    names = {a: (ln or n) for a, n, ln in con.execute("select address, name, ledger_name from functions")}
    recon = defaultdict(list)
    for a, kind, name, status, src in con.execute("select address, kind, name, status, source from recon"):
        recon[a].append({"kind": kind, "name": name, "status": status, "source": src})
    starts = sorted(names)
    if reconstructed:
        keep = set(recon)
    else:
        keep = None
    return starts, names, recon, keep


def scan(args):
    import capstone

    tbase, text = load_text(args.binary)
    tend = tbase + len(text)
    lo, hi = tbase, tend
    if args.range:
        a, b = args.range.split("-")
        lo, hi = max(int(a, 16), tbase), min(int(b, 16), tend)
    forms = {FORM_BYTES[f.strip().lower()] for f in args.forms.split(",")}
    ops = {o.strip().lower() for o in args.ops.split(",")} if args.ops else None
    starts, names, recon, keep = load_functions(ROOT / "local" / "bsp_index.sqlite", args.reconstructed)

    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.skipdata = True
    sites = []
    for k, fs in enumerate(starts):
        fe = starts[k + 1] if k + 1 < len(starts) else tend
        if fe <= lo or fs >= hi or fs < tbase or fs >= tend:
            continue
        if keep is not None and fs not in keep:
            continue
        fe = min(fe, tend)
        code = text[fs - tbase: fe - tbase]
        insns = list(md.disasm(code, fs))
        past_end = None
        for n, ins in enumerate(insns):
            if (past_end is None and ins.mnemonic == "int3" and n
                    and insns[n - 1].mnemonic in ("ret", "jmp")):
                past_end = ins.address
            if ins.address < lo or ins.address >= hi:
                continue
            b = ins.bytes
            if len(b) != 2 or b[0] not in forms or b[1] < 0xC0:
                continue
            reg = (b[1] >> 3) & 7
            i = b[1] & 7
            op, sem = SEMANTICS[(b[0], reg)]
            if i == 0 and not args.include_st0:
                continue
            if ops and op not in ops:
                continue
            sites.append({
                "address": f"{ins.address:08X}", "function": f"{fs:08X}", "name": names.get(fs, ""),
                "bytes": b.hex(" "), "op": op, "i": i, "semantics": sem.replace("st(i)", f"st({i})"),
                "capstone": f"{ins.mnemonic} {ins.op_str}".strip(),
                "past_end": f"{past_end:08X}" if past_end else None,
                "reconstructed": [r["source"] or r["kind"] for r in recon.get(fs, [])],
                "context": [f"{x.address:08X}: {x.mnemonic} {x.op_str}".rstrip()
                            for x in insns[max(0, n - args.context): n + args.context + 1]] if args.context else [],
            })
    return sites


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--range")
    ap.add_argument("--forms", default="dc")
    ap.add_argument("--ops")
    ap.add_argument("--by-function", action="store_true")
    ap.add_argument("--reconstructed", action="store_true")
    ap.add_argument("--include-st0", action="store_true")
    ap.add_argument("--context", type=int, default=0)
    ap.add_argument("--limit", type=int, default=60)
    ap.add_argument("--json")
    ap.add_argument("--binary")
    args = ap.parse_args()
    sites = scan(args)
    if args.json:
        Path(args.json).write_text(json.dumps(sites, indent=1), encoding="utf-8")
    funcs = defaultdict(Counter)
    for s in sites:
        funcs[(s["function"], s["name"])][f'{s["bytes"][:2]}:{s["op"]}'] += 1
    print(f"{len(sites)} sites in {len(funcs)} functions; by op: "
          + ", ".join(f"{k}={v}" for k, v in sorted(Counter(f'{s["bytes"][:2]}:{s["op"]}' for s in sites).items())))
    lines = []
    if args.by_function:
        for (fa, name), c in sorted(funcs.items()):
            lines.append(f"{fa} {name[:48]:48} " + " ".join(f"{k}x{v}" for k, v in sorted(c.items())))
    else:
        for s in sites:
            lines.append(f'{s["address"]} {s["bytes"]} {s["capstone"]:18} {s["semantics"]:26} in {s["function"]}{"?" if s["past_end"] else ""} {s["name"][:40]}')
            lines.extend(("    > " if c.startswith(s["address"]) else "      ") + c for c in s["context"])
    for ln in lines[: args.limit]:
        print(ln)
    if len(lines) > args.limit:
        print(f"... {len(lines) - args.limit} more lines (raise --limit or use --json)")


if __name__ == "__main__":
    main()
