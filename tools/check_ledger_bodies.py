"""Compare 'body XXXXXXXX-YYYYYYYY' claims in the name ledger against Ghidra.

A wrong body range is a quiet defect. It reads as precision, and a range that
stops short hides whole branches from anyone who trusts it instead of re-reading
the listing. 0085DC80 carried 'body 0085DC80-0085DD30' against a real
0085DC80-0085DE93: the claimed end was mid-instruction and omitted the entire
degenerate arm and both exits, and a packet premise was built on the record.

Usage:
    python tools/check_ledger_bodies.py [--sample N] [--json OUT] [--batch N]

Verdicts, per address:
    ok       identical
    tail     the claim names the last INSTRUCTION's address and Ghidra names the
             last BYTE, verified by disassembling there - a convention
             difference, not an error, and by far the common case
    short    claimed end really is before the real end - branches hidden, and
             the case that cost a packet
    long     claimed end is past the real end
    start    the claimed start disagrees with Ghidra's entry (a malformed range,
             or a ledger address that is not a function entry)
    missing  Ghidra has no function there

Read-only: it queries Ghidra through `bsp.py ghidra proto --brief` and never
writes. Claims that name a body for an interior address are expected to come
back `start`, since the ledger address is then not a function entry.
"""
import argparse
import json
import pathlib
import random
import re
import subprocess
import sys

BODY = re.compile(r"body\s+([0-9A-Fa-f]{6,8})\s*-\s*([0-9A-Fa-f]{6,8})", re.I)
BRIEF = re.compile(r"^([0-9a-f]{8})\s+\S.*?\bbody\s+([0-9a-f]{8})\s*-\s*([0-9a-f]{8})\s*$")


def instruction_ends_at(root, address, last_byte):
    """True when the instruction at `address` ends exactly on `last_byte`.

    That is the signature of the last-instruction-address convention: the ledger
    named the final instruction, Ghidra named its final byte.
    """
    command = [sys.executable, str(root / "tools" / "bsp.py"), "ghidra", "disasm",
               address, "--limit", "2"]
    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=120)
    except subprocess.TimeoutExpired:
        return False
    starts = []
    for line in result.stdout.splitlines():
        match = re.match(r"^([0-9a-f]{8}):", line.strip())
        if match:
            starts.append(int(match.group(1), 16))
    if not starts or starts[0] != int(address, 16):
        return False
    if len(starts) >= 2:
        # A following instruction exists, so this one ends just before it.
        return starts[1] - 1 == int(last_byte, 16)
    # Ghidra returned ONE instruction for a two-instruction request, which means
    # the function ends here. Ghidra's body end is the final byte of that final
    # instruction, so the claim named its address and nothing is hidden. This is
    # the common shape - a function ending in RET imm16 gives a delta of 2.
    return int(address, 16) <= int(last_byte, 16)


def load_claims(root):
    claims = {}
    for shard in sorted((root / "config" / "names").glob("*.jsonl")):
        for line in shard.read_text(encoding="utf-8", errors="replace").splitlines():
            line = line.strip()
            if not line:
                continue
            try:
                record = json.loads(line)
            except ValueError:
                continue
            match = BODY.search(record.get("evidence") or "")
            if not match:
                continue
            address = (record.get("address") or "").lower().zfill(8)
            if not address.strip("0"):
                continue
            # Later shard lines win, matching the ledger's own last-write-wins read.
            claims[address] = {
                "address": address,
                "name": record.get("name", ""),
                "claim_start": match.group(1).lower().zfill(8),
                "claim_end": match.group(2).lower().zfill(8),
                "shard": shard.name,
            }
    return list(claims.values())


def query(root, addresses):
    """Ghidra's real bodies for these addresses, as {address: (start, end)}."""
    out = {}
    command = [sys.executable, str(root / "tools" / "bsp.py"), "ghidra", "proto"]
    command += addresses + ["--brief", "--lines", str(len(addresses) + 8)]
    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=600)
    except subprocess.TimeoutExpired:
        return out
    for line in result.stdout.splitlines():
        match = BRIEF.match(line.strip())
        if match:
            out[match.group(1)] = (match.group(2), match.group(3))
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sample", type=int, default=0, help="check N at random")
    parser.add_argument("--batch", type=int, default=60, help="addresses per Ghidra call")
    parser.add_argument("--json", default="", help="write the full result here")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parent.parent
    claims = load_claims(root)
    if args.sample and args.sample < len(claims):
        random.seed(20260914)
        claims = random.sample(claims, args.sample)

    counts = {"ok": 0, "tail": 0, "short": 0, "long": 0, "start": 0, "missing": 0}
    rows = []
    for start in range(0, len(claims), args.batch):
        chunk = claims[start:start + args.batch]
        bodies = query(root, [c["address"] for c in chunk])
        for claim in chunk:
            body = bodies.get(claim["address"])
            if body is None:
                claim["verdict"] = "missing"
            else:
                claim["real_start"], claim["real_end"] = body
                if claim["real_start"] != claim["claim_start"]:
                    claim["verdict"] = "start"
                elif claim["real_end"] == claim["claim_end"]:
                    claim["verdict"] = "ok"
                elif int(claim["claim_end"], 16) < int(claim["real_end"], 16):
                    claim["verdict"] = "short"
                    claim["delta"] = (int(claim["real_end"], 16)
                                      - int(claim["claim_end"], 16))
                else:
                    claim["verdict"] = "long"
                    claim["delta"] = (int(claim["claim_end"], 16)
                                      - int(claim["real_end"], 16))
            if claim["verdict"] == "short" and claim.get("delta", 99) <= 14:
                # Exact test rather than a size guess: disassemble at the claimed
                # end. If that instruction runs to Ghidra's last byte, the claim
                # named the final instruction's address and nothing is hidden.
                if instruction_ends_at(root, claim["claim_end"], claim["real_end"]):
                    claim["verdict"] = "tail"
            counts[claim["verdict"]] += 1
            rows.append(claim)
        print("  ... {}/{}".format(min(start + args.batch, len(claims)), len(claims)),
              file=sys.stderr)

    for row in rows:
        if row["verdict"] in ("ok", "tail", "start", "missing"):
            continue
        print("{verdict:6s} {address}  {name:42.42s} claimed {claim_start}-{claim_end}"
              "  real {real_start}-{real_end}".format(**row))

    print("\nchecked {} body claims: {}".format(
        len(rows), "  ".join("{}={}".format(k, counts[k]) for k in
                             ("ok", "tail", "short", "long", "start", "missing"))))
    if args.json:
        pathlib.Path(args.json).write_text(json.dumps(rows, indent=1), encoding="utf-8")
        print("full result: {}".format(args.json))


if __name__ == "__main__":
    main()
