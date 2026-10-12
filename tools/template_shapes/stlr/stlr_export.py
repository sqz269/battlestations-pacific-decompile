"""Write tools/template_shapes/stlr_decisions.txt from local/stlr_feat.json (rows outside other sweeps' leases)."""
import json, os
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))
EXCL = [(0xa50000, 0xbfffff), (0x980000, 0x98ffff), (0x670000, 0x6cffff), (0x700000, 0x73ffff)]
SINGLE = {0x99df87, 0x99e127, 0x9c659f, 0x9c6678, 0x9c6929}
lines = ['# cc13-stlreview1: decision per stl_probable bookmark row (docs/CC13_STL_PROBABLE_REVIEW.md)',
         '# addr|old_evidence|decision|category|name|evidence_key|anchors']
for x in sorted(d, key=lambda x: x['a']):
    a = int(x['a'], 16)
    if any(lo <= a <= hi for lo, hi in EXCL) or a in SINGLE:
        dec = 'deferred_leased'
    else:
        dec = x['dec']
    cat = name = key = ''
    if dec == 'upgrade_shape':
        cat, name, key = 'stl_shape', f"{x['shape'][0]}_{x['a']}", 'st1_' + x['shape'][1]
    elif dec == 'upgrade_named':
        cat, name = 'stl_instantiation', x['nm']
        key = 'st1_' + x['shape'][1] if x.get('shape') else 'cc13_stlreview1_named_instance'
    elif dec == 'reviewed_game':
        cat, name, key = 'game_code', x['nm'], 'cc13_stlreview1_game_named'
    elif dec == 'drop_game':
        key = 'cc13_stlreview1_game_unnamed'
    an = ';'.join(x.get('anch', []))[:120]
    lines.append('|'.join([x['a'], x['ev'], dec, cat, name, key, an]))
open(os.path.join(root, 'tools', 'template_shapes', 'stlr_decisions.txt'), 'w', encoding='utf-8', newline='\n').write('\n'.join(lines) + '\n')
import collections
print(collections.Counter((l.split('|')[2], l.split('|')[1]) for l in lines[2:]))
