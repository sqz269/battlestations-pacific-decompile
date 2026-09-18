"""Does a write to a stack slot actually reach a given read?

A backward trace that takes the nearest preceding write in ADDRESS order is
wrong whenever a branch jumps over that write.  This tool builds the function's
basic blocks from a Ghidra listing, computes dominators, and reports for each
candidate write whether its block dominates the read's block.  A write that
does not dominate cannot be assumed to be the producer: the slot's value at the
read is path-dependent, and any single-formula reconstruction of it is a guess.

    python tools/frame_slot_dominance.py <listing.txt> 009d22ff=009d214e,009d1fa5

The listing is whatever `python tools/bsp.py ghidra disasm <func> --lines N`
spilled to local/output/.  Pair each read address with a comma-separated list of
candidate writes, as many pairs as you like.

A "dominates=False" verdict is a finding, not a failure: it says the value is
produced by different code on different paths, so the enclosing routine has to
be reconstructed branch by branch rather than lifted out piecemeal.
"""
import re
import sys

ROW = re.compile(r"^([0-9a-f]{8}): (.*?)\s*$")
JCC = re.compile(r"^(J[A-Z]+)\s+0x([0-9a-f]+)")


def load(path):
    insns = []
    with open(path) as handle:
        for line in handle:
            m = ROW.match(line.strip())
            if m:
                insns.append((int(m.group(1), 16), m.group(2).strip()))
    insns.sort()
    return insns


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    insns = load(sys.argv[1])
    if not insns:
        print("no instructions parsed from the listing", file=sys.stderr)
        return 1
    addrs = [a for a, _ in insns]
    index = {a: i for i, a in enumerate(addrs)}
    text = dict(insns)

    targets, uncond = {}, set()
    for a, t in insns:
        m = JCC.match(t)
        if m:
            targets[a] = int(m.group(2), 16)
            if m.group(1) == "JMP":
                uncond.add(a)

    leaders = {addrs[0]}
    for a in targets:
        leaders.add(targets[a])
        if index[a] + 1 < len(addrs):
            leaders.add(addrs[index[a] + 1])
    leaders = sorted(x for x in leaders if x in index)
    leader_set = set(leaders)

    block_of, cur = {}, None
    for a, _ in insns:
        if a in leader_set:
            cur = a
        block_of[a] = cur

    succ = {b: set() for b in leaders}
    for bi, b in enumerate(leaders):
        end = leaders[bi + 1] if bi + 1 < len(leaders) else None
        last = addrs[index[end] - 1] if end is not None else addrs[-1]
        if last in targets:
            if targets[last] in block_of:
                succ[b].add(block_of[targets[last]])
            if last not in uncond and end is not None:
                succ[b].add(end)
        elif end is not None and not text[last].startswith("RET"):
            succ[b].add(end)

    entry = leaders[0]
    dom = {b: set(leaders) for b in leaders}
    dom[entry] = {entry}
    pred = {b: set() for b in leaders}
    for b, ss in succ.items():
        for s in ss:
            pred[s].add(b)
    changed = True
    while changed:
        changed = False
        for b in leaders:
            if b == entry:
                continue
            ps = [dom[p] for p in pred[b] if p in dom]
            new = (set.intersection(*ps) | {b}) if ps else {b}
            if new != dom[b]:
                dom[b], changed = new, True

    for spec in sys.argv[2:]:
        read_s, _, writes_s = spec.partition("=")
        read = int(read_s, 16)
        rb = block_of.get(read)
        if rb is None:
            print(f"read {read:08x} NOT FOUND in the listing")
            continue
        print(f"read {read:08x} in block {rb:08x}")
        for w in writes_s.split(","):
            if not w:
                continue
            wa = int(w, 16)
            wb = block_of.get(wa)
            if wb is None:
                print(f"  write {wa:08x} NOT FOUND")
                continue
            print(f"  write {wa:08x} block {wb:08x} dominates={wb in dom[rb]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
