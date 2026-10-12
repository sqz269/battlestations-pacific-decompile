"""cc13-stlreview1: apply the stl_probable review (tools/template_shapes/stlr_decisions.txt) to config/tags.

Decisions per row (docs/CC13_STL_PROBABLE_REVIEW.md):
  upgrade_shape  -> stl_shape rename row <Prefix>_<addr>, evidence st1_<shape key>
  upgrade_named  -> stl_instantiation rename row keeping the existing ledger name
  reviewed_game  -> game_code "reviewed" row carrying the ledger name (not a template copy)
  drop_game      -> row removed (unnamed game code; the bookmark was a false positive)
  keep_* / deferred_leased -> unchanged
Writes local/stlr_ghidra.json (rows whose Ghidra bookmark must change) for the locked apply step.
"""
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = 'docs/CC13_STL_PROBABLE_REVIEW.md'
EV_NEW = {
    'st1_vecinsertone': 'cc13-stlreview1 shape: checked vector::insert(where, val): index=(where-first)/sizeof(T) behind _invalid_parameter_noinfo hooks, _Insert_n(where,1,val), checked iterator rebuilt from first+index (std::vector::insert)',
    'st1_vecpushback': 'cc13-stlreview1 shape: checked vector::push_back: in-place construct when size<capacity, else insert(end(),val) through _Insert_n; end-iterator hooks call _invalid_parameter_noinfo (std::vector::push_back)',
    'st1_vecresize': 'cc13-stlreview1 shape: vector::resize(n[,val]): size()<n -> _Insert_n(end,n-size,val), else erase(begin+n,end), checked-iterator hooks (std::vector::resize)',
    'st1_veccopyctor': 'cc13-stlreview1 shape: vector copy constructor: zero first/last/end, n=(last-first)/sizeof(T), _Buy(n) then _Ucopy under an EH state (std::vector::vector(const vector&))',
    'st1_veceraserange': 'cc13-stlreview1 shape: vector::erase(first,last): same-container hook, move [last,end) down onto first, destroy the tail, end=first+(end-last) (std::vector::erase(first,last))',
    'st1_listremove': 'cc13-stlreview1 shape: list::remove(val): walk the ring, compare each value, erase matches through checked iterators (std::list::remove)',
    'st1_listerase': 'cc13-stlreview1 shape: list::erase(where): checked hooks, unlink node, destroy value, free node, --size (std::list::erase)',
    'st1_listinsertrange': 'cc13-stlreview1 shape: list::_Insert(where,first,last): loop over source nodes, _Buynode(where,prev,value), _Incsize(1) length_error check, link (std::list::insert range)',
    'cc13_stlreview1_named_instance': 'cc13-stlreview1: bookmark-only stl_probable row upgraded; the function already carries a container-operation ledger name from an earlier sweep and has no string, game-global or game-call anchors',
    'cc13_stlreview1_game_named': 'cc13-stlreview1: not a template copy. Named game code (ledger name from an earlier sweep) that inlines checked-iterator hooks to _invalid_parameter_noinfo; the stl_probable bookmark (e0029 / st1_scl97) was a false positive',
}


def main():
    dec = {}
    for line in (ROOT / 'tools/template_shapes/stlr_decisions.txt').read_text(encoding='utf-8').splitlines():
        if not line or line[0] == '#':
            continue
        a, old, d, cat, name, key, anch = line.split('|')
        if d in ('upgrade_shape', 'upgrade_named', 'reviewed_game', 'drop_game'):
            dec[a] = (old, d, cat, name, key, anch)
    only = sys.argv[1:]  # optional band prefixes, e.g. 0080 0081
    if only:
        dec = {a: v for a, v in dec.items() if a[:4] in only}
    tagdir = ROOT / 'config/tags'
    ghidra, seen = [], set()
    for band in sorted({a[:4] + '0000' for a in dec}):
        path = tagdir / f'{band}.jsonl'
        out = []
        for line in path.read_text(encoding='utf-8').splitlines():
            if not line.strip():
                continue
            r = json.loads(line)
            a = r['address']
            if a in dec and r['category'] == 'stl_probable':
                old, d, cat, name, key, anch = dec[a]
                seen.add(a)
                if d == 'drop_game':
                    ghidra.append({'address': a, 'decision': d})
                    continue
                action = 'reviewed' if d == 'reviewed_game' else 'rename'
                conf = 'high' if d == 'reviewed_game' else 'medium'
                r = {'address': a, 'name': name, 'category': cat, 'confidence': conf, 'action': action,
                     'create': False, 'source': SOURCE, 'evidence_id': key}
                ghidra.append({'address': a, 'decision': d, 'row': r})
            out.append(json.dumps(r))
        path.write_text('\n'.join(out) + '\n', encoding='utf-8', newline='\n')
    missing = sorted(set(dec) - seen)
    evp = tagdir / 'evidence.json'
    ev = json.loads(evp.read_text(encoding='utf-8'))
    for g in ghidra:
        k = g.get('row', {}).get('evidence_id')
        if k and k not in ev:
            ev[k] = EV_NEW[k]
    evp.write_text(json.dumps(ev, indent=1, ensure_ascii=False) + '\n', encoding='utf-8', newline='\n')
    (ROOT / 'local/stlr_ghidra.json').write_text(json.dumps(ghidra))
    counts = {}
    for g in ghidra:
        counts[g['decision']] = counts.get(g['decision'], 0) + 1
    print(counts, 'missing rows:', len(missing), missing[:5])


if __name__ == '__main__':
    main()
