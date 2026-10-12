"""List IsHint* call sites with enclosing function; list critical hints. Read-only."""
import os
import re
import sys

root = sys.argv[1]
mode = sys.argv[2] if len(sys.argv) > 2 else "calls"
CALL = re.compile(r"(?:(?<=[^\w])|^)(?:lua\w*?)?IsHint(Active|Critical|Stored)\s*\(([^)]*)\)")
FUNC = re.compile(r"^\s*(?:local\s+)?function\s+([\w.:]+)")

if mode == "crit":
    p = os.path.join(root, "scripts", "datatables", "hintsystemmessages.lua")
    text = open(p, encoding="latin-1").read()
    for m in re.finditer(r'\["id"\]\s*=\s*"([^"]+)"(.*?)(?=\["id"\]|\Z)', text, re.S):
        body = m.group(2)
        crit = re.search(r'\["critical"\]\s*=\s*(\w+)', body)
        dur = re.search(r'\["duration"\]\s*=\s*(-?\d+)', body)
        if (len(sys.argv) > 3 and m.group(1).startswith(sys.argv[3])) or (len(sys.argv) <= 3 and crit and crit.group(1) == "true"):
            print(m.group(1), "critical", "duration", dur.group(1) if dur else "?")
    sys.exit(0)

for dp, _, fns in os.walk(os.path.join(root, "scripts", "missions")):
    for fn in sorted(fns):
        if not fn.endswith(".lua"):
            continue
        path = os.path.join(dp, fn)
        lines = open(path, encoding="latin-1").read().split("\n")
        cur = "?"
        for i, ln in enumerate(lines, 1):
            f = FUNC.match(ln)
            if f:
                cur = f.group(1)
            code = ln.split("--", 1)[0]
            for c in CALL.finditer(code):
                print(f"{os.path.relpath(path, root)}:{i}\t{cur}\t{c.group(0).split(chr(40))[0]}({c.group(2)})\t{ln.strip()[:90]}")
            if "IsHint" in ln and not CALL.search(code):
                print(f"{os.path.relpath(path, root)}:{i}\t{cur}\t(def/comment)\t{ln.strip()[:90]}")
