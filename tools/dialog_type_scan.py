"""Scan Lua files for `sequence` tables and the `type` values of their elements.

Read-only over the game installation. Prints a summary and every non-standard occurrence.
Usage: python -I sc1_dialog_scan.py <root>
"""
import collections
import os
import re
import sys

KNOWN = {"msg", "setpanel", "hidepanel", "pause", "callback"}
SEQ = re.compile(r'(?:\[\s*"sequence"\s*\]|\bsequence)\s*=\s*\{', re.I)
TYPE = re.compile(r'(?:\[\s*"type"\s*\]|(?<![\w.])type)\s*=\s*([^,\n}]+)', re.I)


def strip_comments(text):
    # Replace comment bodies with spaces (keep newlines so line numbers stay right).
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                if text[j] == "\\":
                    j += 1
                if text[j] == "\n":
                    break
                j += 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("--[[", i) or text.startswith("--[==[", i):
            close = "]]" if text.startswith("--[[", i) else "]==]"
            j = text.find(close, i)
            j = n if j < 0 else j + len(close)
            out.append(re.sub(r"[^\n]", " ", text[i:j]))
            i = j
        elif text.startswith("--", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def block_end(text, start):
    depth = 0
    i = start
    n = len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return n


def main(root):
    counts = collections.Counter()
    odd = []
    files_with_seq = 0
    seq_blocks = 0
    elements_untyped = 0
    for dp, _, fns in os.walk(root):
        for fn in fns:
            if not fn.lower().endswith(".lua"):
                continue
            path = os.path.join(dp, fn)
            raw = open(path, "rb").read().decode("latin-1")
            text = strip_comments(raw)
            hit = False
            for m in SEQ.finditer(text):
                hit = True
                seq_blocks += 1
                s = m.end() - 1
                e = block_end(text, s)
                body = text[s:e]
                for t in TYPE.finditer(body):
                    val = t.group(1).strip()
                    line = text.count("\n", 0, s + t.start()) + 1
                    lit = re.fullmatch(r'"([^"]*)"|\'([^\']*)\'', val)
                    if lit:
                        name = (lit.group(1) or lit.group(2) or "")
                        counts[name.lower()] += 1
                        if name.lower() not in KNOWN:
                            odd.append((path, line, val))
                    else:
                        counts["<non-literal>"] += 1
                        odd.append((path, line, val))
                # Elements without a type key default to "msg" (reader default 00CE4AD8).
                elements_untyped += len(re.findall(r'\[\s*"message"\s*\]|\bmessage\s*=', body, re.I))
            files_with_seq += hit
    print("files with sequence blocks:", files_with_seq, "blocks:", seq_blocks)
    print("type counts:", dict(counts))
    print("message keys (msg default):", elements_untyped)
    print("non-standard:", len(odd))
    for p, l, v in odd:
        print(f"  {os.path.relpath(p, root)}:{l}: {v}")


if __name__ == "__main__":
    main(sys.argv[1])
