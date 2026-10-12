"""Compare hint-related functions of mission variants against a base file. Read-only."""
import re
import sys

FUNC = re.compile(r"^\s*(?:local\s+)?function\s+([\w.:]+)", re.M)


def funcs(path):
    text = open(path, encoding="latin-1").read()
    out = {}
    starts = [(m.start(), m.group(1)) for m in FUNC.finditer(text)]
    starts.append((len(text), None))
    for (s, name), (e, _) in zip(starts, starts[1:]):
        body = text[s:e]
        body = "\n".join(l.split("--", 1)[0].strip() for l in body.split("\n"))
        body = re.sub(r"\s+", " ", body).strip()
        out.setdefault(name, body)
    return text, out


def live_callers(text, name):
    n = 0
    for ln in text.split("\n"):
        code = ln.split("--", 1)[0]
        if re.search(r"(?<![\w])" + re.escape(name) + r"\s*\(", code) and not re.match(r"\s*function", code):
            n += 1
    return n


base = sys.argv[1]
names = sys.argv[2].split(",")
_, bf = funcs(base)
btext = open(base, encoding="latin-1").read()
for other in sys.argv[3:]:
    ot, of = funcs(other)
    res = []
    for n in names:
        if n not in of:
            res.append(f"{n}:MISSING")
        elif of[n] != bf.get(n):
            res.append(f"{n}:DIFF")
        cb, co = live_callers(btext, n), live_callers(ot, n)
        if cb != co:
            res.append(f"{n}:callers {cb}->{co}")
    print(other.split("missions\\")[-1], "same" if not res else " ".join(res))
