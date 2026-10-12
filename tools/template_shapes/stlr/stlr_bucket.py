"""Classify each stl_probable row.
named_tmpl  : ledger name says container/template instance and body has no game anchors -> upgrade
named_game  : ledger-named game code -> reviewed
raw_game    : unnamed, game anchors (strings, >=3 game callees, or >=2 game callees + data refs) -> drop
raw_mixed   : unnamed, weak game anchors -> manual
raw_pure    : unnamed, no data refs, no game callees -> stays a bookmark (library shaped)
"""
import json, os, sys, collections, re
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))
TMPL = re.compile(r'^(STL_|stl_|TMPL_|MSVC_|LIBSTL_|std::|_)|(PushBack|PopFront|PopBack|LookupOrInsert|FindOrInsert|GetOrInsert\w*|Subscript|Registry_Index|Map_Index|Iterator(Previous|Next|Increment|Decrement)|(increment|decrement)_\w*iterator|EraseRange|InsertN|CopyConstruct)(_|$)|(Vector|Tree|List|Map|Set|Deque|Array|Range|Iterator|Heap|Queue|Stack|Table|Pool|Buffer|Container|Pointers?)_?'
                  r'(PushBack|PopBack|Push|Pop|Insert\w*|Erase\w*|Reserve|Resize|Clear|CopyConstruct|Copy\w*|Assign\w*|'
                  r'Increment\w*|Decrement\w*|Inc|Dec|Append\w*|Remove\w*|Find\w*|LowerBound|UpperBound|Lbound|Ubound|IndexOf|Tidy|'
                  r'Construct\w*|Destroy\w*|Destruct\w*|Grow|Buy\w*|Fill|Uninit\w*|Iterator\w*|Next|Prev|At|Subscript|Swap|Sort\w*|Unique|Splice|Move\w*)'
                  r'(_|$)|^(resize|insert|erase|push|copy|assign|destroy)_\w*(vector|tree|list|map)', re.I)


def bucket(x):
    nm = x['ledger'] if x['ledger'] and not x['ledger'].startswith('FUN_') else (x['ghidra'] if not x['ghidra'].startswith('FUN_') else '')
    x['nm'] = nm
    if nm:
        if TMPL.search(nm) and x['nstr'] == 0 and x['ngs'] <= 1:
            return 'named_tmpl'
        return 'named_game'
    tl = [n for n in x['strong'] if not (TMPL.search(n) or 'GetSingleton' in n or 'GetOrCreate' in n)]
    rk = x.get('ck', {})
    x['score'] = (3 if rk.get('str') else 0) + 2 * min(rk.get('data', 0), 1) + min(rk.get('const', 0), 1) + len(tl)
    if x['score'] >= 2:
        return 'raw_game'
    if x['score'] == 1:
        return 'raw_mixed'
    return 'raw_pure'


c = collections.Counter()
for x in d:
    x['b'] = bucket(x)
    c[(x['b'], x['ev'])] += 1
for k, v in sorted(c.items()):
    print(v, k)
json.dump(d, open(os.path.join(root, 'local', 'stlr_feat.json'), 'w'), indent=0)
if len(sys.argv) > 1:
    b = sys.argv[1]
    pre = sys.argv[2] if len(sys.argv) > 2 else ''
    n = int(sys.argv[3]) if len(sys.argv) > 3 else 20
    skip = int(sys.argv[4]) if len(sys.argv) > 4 else 0
    sel = [x for x in d if x['b'] == b and x['a'].startswith(pre)]
    for x in sel[skip:skip + n]:
        print(x['a'], x['ev'][-5:], x['nm'] or '-', x.get('score'), x.get('ck'), x['size'], x['strs'][:2], x['drefs'][:3], x['strong'][:3], x['game_callers'][:2])
