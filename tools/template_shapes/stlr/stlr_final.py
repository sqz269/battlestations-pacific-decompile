"""Final per-row decision: upgrade_shape / upgrade_named / reviewed_game / drop_game / keep."""
import json, os, re, sys, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stlr_labels import LABELS, GAME
from stlr_pat import TMPL, ELEM
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))


def pseudo(a):
    p = os.path.join(root, 'local', 'stlr_decomp', a + '.c')
    return open(p, encoding='utf-8').read() if os.path.exists(p) else ''


def anchors(x):
    t = pseudo(x['a'])
    out = []
    if re.search(r'"[^"]{2,}"|\bs_\w+', t):
        out.append('string')
    if re.search(r'\b_?(DAT|PTR)_00(e[0-9a-f]|f[0-9a-f]|10)[0-9a-f]{4}\b', t, re.I):
        out.append('global')
    if re.search(r'\b_?DAT_00(c[ef]|d[0-9a-f])[0-9a-f]{4}\b', t, re.I):
        out.append('rconst')
    names = set(re.findall(r'\b([A-Za-z]\w*_\w+)\s*\(', t)) - {'FUN_' + x['a']}
    sc = sorted(n for n in names if not re.match(r'(FUN_|STL_|TMPL_|TRIV_|CG_|LIBCRT|LIBSTL|MSVC_|operator|Unwind|EHHandler|_)', n) and not TMPL.search(n) and not ELEM.search(n) and 'GetSingleton' not in n and 'GetOrCreate' not in n and 'xlen' not in n)
    if sc:
        out.append('calls:' + ','.join(n[:40] for n in sc[:3]))
    if x.get('vcall'):
        out.append('vcall')
    m = re.search(r'\b(param_1|this)\s*\+\s*0x([1-9a-f][0-9a-f]{2,})\b', t)
    if m:
        out.append('field+' + m.group(2))
    return out


def game_enough(x):
    an = x['anch']
    nc = sum(len(a[6:].split(',')) for a in an if a.startswith('calls:'))
    big = any(a.startswith('field+') for a in an)
    return 'string' in an or 'global' in an or big or nc >= 2 or (nc >= 1 and ('rconst' in an or 'vcall' in an))


for x in d:
    k = x.get('nk')
    x['anch'] = anchors(x) if x['b'].startswith('raw') else []
    if k in LABELS:
        x['dec'] = 'upgrade_named' if x['nm'] else 'upgrade_shape'
        x['shape'] = LABELS[k]
    elif x['b'] == 'named_tmpl':
        x['dec'] = 'upgrade_named'
    elif x['b'] == 'named_game':
        x['dec'] = 'upgrade_named' if TMPL.search(x['nm']) and not x['ck'].get('str') and 'Objective' not in x['nm'] else 'reviewed_game'
    elif k in GAME:
        x['dec'] = 'drop_game'
        x['anch'] = ['vcall/registry cluster']
    elif x['b'] == 'raw_game':
        x['dec'] = 'drop_game' if game_enough(x) else 'keep_uncertain'
    elif x['b'] == 'raw_mixed':
        x['dec'] = 'drop_game' if game_enough(x) else 'keep_uncertain'
    else:
        x['dec'] = 'drop_game' if game_enough(x) else 'keep_uncertain' if x['anch'] else 'keep_library'
json.dump(d, open(os.path.join(root, 'local', 'stlr_feat.json'), 'w'), indent=0)
c = collections.Counter((x['dec'], x['ev']) for x in d)
for k, v in sorted(c.items()):
    print(v, k)
if len(sys.argv) > 1:
    for x in d:
        if x['dec'] == sys.argv[1] and x['a'].startswith(sys.argv[2] if len(sys.argv) > 2 else ''):
            print(x['a'], x['b'], x['nm'][:36] or '-', x['anch'][:3], x['size'])
