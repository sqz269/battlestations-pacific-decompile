"""Scripted x87-depth and ESP walk over a function read from the on-disk image.

Usage:
  python local/x87trace.py trace <start> <end> [--from A] [--to B]
  python local/x87trace.py callee <addr> [--n 40]

Walks the CFG (worklist over basic blocks), tracking
  * x87 stack depth (FLD/FILD/FLD1/... push, FSTP/FCOMIP/FADDP/... pop)
  * ESP displacement relative to function entry (so "frame base" = -esp_off)
and reports every join where two predecessors disagree.
"""
import json
import re
import struct
import sys
from pathlib import Path

import capstone

ROOT = Path(__file__).resolve().parent.parent


def load_image():
    cfg = json.loads((ROOT / "config" / "target.json").read_text())
    data = Path(cfg["binary"]).read_bytes()
    base = int(cfg["image_base"], 16)
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    tbl = pe + 24 + opt
    secs = []
    for i in range(nsec):
        o = tbl + 40 * i
        name = data[o:o + 8].rstrip(b"\0").decode("latin1")
        vsize, vaddr, rawsize, rawptr = struct.unpack_from("<IIII", data, o + 8)
        secs.append((name, base + vaddr, vsize, rawptr, rawsize))
    return data, secs


def off_of(secs, va):
    for name, vbase, vsize, rawptr, rawsize in secs:
        if vbase <= va < vbase + vsize:
            rel = va - vbase
            if rel >= rawsize:
                return None
            return rawptr + rel
    return None


# ---- x87 effect tables -------------------------------------------------
PUSH1 = {
    "fld", "fild", "fld1", "fldz", "fldpi", "fldl2e", "fldl2t", "fldlg2",
    "fldln2", "fbld", "fptan", "fsincos", "fdecstp",
}
POP1 = {
    "fstp", "fistp", "fisttp", "fbstp", "faddp", "fsubp", "fsubrp", "fmulp",
    "fdivp", "fdivrp", "fcomp", "ficomp", "fucomp", "fcomip", "fucomip",
    "fcompi", "fucompi",
    "fyl2x", "fyl2xp1", "fpatan", "fincstp", "ffreep", "fscale_dummy",
}
POP2 = {"fcompp", "fucompp"}
NEUTRAL = {
    "fst", "fist", "fadd", "fsub", "fsubr", "fmul", "fdiv", "fdivr", "fiadd",
    "fisub", "fisubr", "fimul", "fidiv", "fidivr", "fcom", "ficom", "fucom",
    "fcomi", "fucomi", "fabs", "fchs", "fsqrt", "frndint", "fxch", "fnop",
    "fscale", "fprem", "fprem1", "fsin", "fcos", "f2xm1", "fxam", "ftst",
    "fnstsw", "fstsw", "fnstcw", "fldcw", "fnclex", "fclex", "ffree",
    "fnsave", "frstor", "fnstenv", "fldenv", "fwait", "wait",
    "fcmovb", "fcmove", "fcmovbe", "fcmovu", "fcmovnb", "fcmovne",
    "fcmovnbe", "fcmovnu",
}
# fld st(0)-style: "fld st(1)" still pushes. fstp st(0)/st(1) still pops.


def x87_delta(mn):
    if mn in POP2:
        return -2
    if mn in POP1:
        return -1
    if mn in PUSH1:
        return +1
    if mn in NEUTRAL:
        return 0
    if mn.startswith("f") and mn not in ("fs", "fxrstor", "fxsave"):
        return None  # unknown float op
    return 0


COND = {
    "jo", "jno", "js", "jns", "je", "jz", "jne", "jnz", "jb", "jnae", "jc",
    "jnb", "jae", "jnc", "jbe", "jna", "ja", "jnbe", "jl", "jnge", "jge",
    "jnl", "jle", "jng", "jg", "jnle", "jp", "jpe", "jnp", "jpo", "jcxz",
    "jecxz", "loop", "loope", "loopne",
}


def main(argv):
    data, secs = load_image()
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = False

    mode = argv[0]
    if mode == "callee":
        addr = int(argv[1], 16)
        n = int(argv[2]) if len(argv) > 2 else 60
        o = off_of(secs, addr)
        for ins in md.disasm(data[o:o + 8 * n], addr):
            print("%08x  %-22s %s %s" % (ins.address, ins.bytes.hex(), ins.mnemonic, ins.op_str))
            if ins.mnemonic == "ret":
                break
        return 0

    if mode == "rets":
        a0 = int(argv[1], 16)
        a1 = int(argv[2], 16)
        o = off_of(secs, a0)
        prev = []
        for ins in md.disasm(data[o:o + (a1 - a0) + 16], a0):
            if ins.address > a1:
                break
            prev.append(ins)
            if ins.mnemonic in ("ret", "retn", "retf"):
                for p in prev[-4:]:
                    print("  %08x  %-16s %s %s" % (p.address, p.bytes.hex(), p.mnemonic, p.op_str))
                print("  ---")
        return 0

    start = int(argv[1], 16)
    end = int(argv[2], 16)
    # optional call-effect overrides: addr:xpush:retimm
    calls = {}
    show_from, show_to = start, end
    basefb = 0x58
    i = 3
    while i < len(argv):
        a = argv[i]
        if a == "--call":
            t, xp, ri = argv[i + 1].split(":")
            calls[int(t, 16)] = (int(xp), int(ri))
            i += 2
        elif a == "--icall":
            t, xp, ri = argv[i + 1].split(":")
            calls[("i", int(t, 16))] = (int(xp), int(ri))
            i += 2
        elif a == "--basefb":
            basefb = int(argv[i + 1], 0)
            i += 2
        elif a == "--from":
            show_from = int(argv[i + 1], 16)
            i += 2
        elif a == "--to":
            show_to = int(argv[i + 1], 16)
            i += 2
        else:
            i += 1

    o = off_of(secs, start)
    code = data[o:o + (end - start) + 16]
    insns = {}
    for ins in md.disasm(code, start):
        insns[ins.address] = ins

    state = {}          # addr -> (depth, esp)
    conflicts = []
    unknown_calls = set()
    notes = []
    work = [(start, 0, 0)]
    seen_edges = set()
    while work:
        pc, depth, esp = work.pop()
        while True:
            if pc > end or pc not in insns:
                break
            prev = state.get(pc)
            if prev is not None:
                if prev != (depth, esp):
                    conflicts.append((pc, prev, (depth, esp)))
                break
            state[pc] = (depth, esp)
            ins = insns[pc]
            mn = ins.mnemonic
            ops = ins.op_str
            nxt = pc + ins.size

            # --- ESP effects
            if mn == "push":
                esp -= 4
            elif mn == "pop":
                esp += 4
            elif mn == "pushfd":
                esp -= 4
            elif mn == "popfd":
                esp += 4
            elif mn in ("sub", "add") and ops.startswith("esp,"):
                try:
                    imm = int(ops.split(",")[1].strip(), 0)
                except ValueError:
                    notes.append((pc, "non-immediate esp adjust: " + ops))
                    imm = 0
                esp += -imm if mn == "sub" else imm

            # --- x87 effects
            d = x87_delta(mn)
            if d is None:
                notes.append((pc, "UNKNOWN x87 op: %s %s" % (mn, ops)))
                d = 0
            was = depth
            depth += d
            if depth < 0 <= was:
                notes.append((pc, "DEPTH WENT NEGATIVE after %s %s" % (mn, ops)))

            # --- control flow
            if mn == "call":
                if ops.startswith("0x"):
                    tgt = int(ops, 16)
                    if tgt in calls:
                        xp, ri = calls[tgt]
                        depth += xp
                        esp += ri
                    else:
                        unknown_calls.add(tgt)
                elif ("i", pc) in calls:
                    xp, ri = calls[("i", pc)]
                    depth += xp
                    esp += ri
                else:
                    notes.append((pc, "indirect call: " + ops))
                pc = nxt
                continue
            if mn == "jmp":
                if ops.startswith("0x"):
                    pc = int(ops, 16)
                    continue
                notes.append((pc, "indirect jmp: " + ops))
                break
            if mn in COND:
                if ops.startswith("0x"):
                    t = int(ops, 16)
                    work.append((t, depth, esp))
                pc = nxt
                continue
            if mn in ("ret", "retn", "retf"):
                if depth != 0:
                    notes.append((pc, "RET with x87 depth %d (float return if 1)" % depth))
                break
            if mn in ("int3", "ud2", "hlt"):
                break
            pc = nxt

    print("== conflicts ==")
    for pc, a, b in sorted(conflicts):
        print("  %08x  first=(depth %d, esp %+d) second=(depth %d, esp %+d)" % (pc, a[0], a[1], b[0], b[1]))
    if not conflicts:
        print("  none")
    print("== unknown call targets (assumed depth+0, esp+0) ==")
    for t in sorted(unknown_calls):
        print("  %08x" % t)
    print("== notes ==")
    for pc, m in sorted(notes):
        print("  %08x  %s" % (pc, m))
    print("== listing %08x..%08x ==" % (show_from, show_to))
    for a in sorted(insns):
        if a < show_from or a > show_to:
            continue
        ins = insns[a]
        st = state.get(a)
        if st is None:
            print("  %08x  [unreached]            %s %s" % (a, ins.mnemonic, ins.op_str))
        else:
            fb = -st[1]
            note = ""
            m = re.search(r"esp \+ (0x[0-9a-f]+|\d+)", ins.op_str)
            if m:
                n = int(m.group(1), 0)
                note = "   ; base [ESP+%02Xh]" % (n - fb + basefb)
            elif re.search(r"\[esp\]", ins.op_str):
                note = "   ; base [ESP+%02Xh]" % (basefb - fb)
            print("  %08x  d%-2d fb=0x%02x  %-18s %-34s%s"
                  % (a, st[0], fb, ins.bytes.hex(),
                     ins.mnemonic + " " + ins.op_str, note))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
