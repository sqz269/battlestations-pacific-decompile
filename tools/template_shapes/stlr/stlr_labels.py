"""Cluster -> (prefix, evidence key) for verified pseudocode clusters; prints unlabeled multi-member clusters."""
import json, os, collections, sys
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
LABELS = {
    '3529129d47': ('STL_TreeIter_Dec', 'iterdec'),
    '7bee2dc83d': ('STL_TreeIter_Inc', 'iterinc'),
    'fbf4e76353': ('STL_Vector_InsertOne', 'vecinsertone'),
    '33f554cf4c': ('STL_Vector_InsertOne', 'vecinsertone'),
    '6af7d8f6d8': ('STL_Vector_InsertOne', 'vecinsertone'),
    '19fdc91c3e': ('STL_Vector_InsertOne', 'vecinsertone'),
    '5538d072b7': ('STL_Tree_EraseRange', 'treeeraserange'),
    'd3f4746599': ('STL_Vector_PushBack', 'vecpushback'),
    'f3b59f5482': ('STL_Vector_PushBack', 'vecpushback'),
    '7d3ec05928': ('STL_Vector_PushBack', 'vecpushback'),
    'd1c5eec569': ('STL_Tree_Find', 'treefind'),
    'c2254d017f': ('STL_Map_Subscript', 'mapsubscript'),
    '13cb0c2883': ('STL_Map_Subscript', 'mapsubscript'),
    'd596f6ba30': ('STL_Vector_Resize', 'vecresize'),
    'f5625617e9': ('STL_Vector_Resize', 'vecresize'),
    '51692be268': ('STL_Vector_Resize', 'vecresize'),
    'b10ffa5637': ('STL_List_Remove', 'listremove'),
    'd6d984ae44': ('STL_Vector_CopyConstruct', 'veccopyctor'),
    '3caa99d0c0': ('STL_Vector_CopyConstruct', 'veccopyctor'),
    '879d27d29d': ('STL_List_Erase', 'listerase'),
    '8671004a6f': ('STL_List_InsertRange', 'listinsertrange'),
    '8e8e44e87d': ('STL_Vector_EraseRange', 'veceraserange'),
    '2bfe48a57e': ('STL_Vector_EraseRange', 'veceraserange'),
    '62cfb7ff8b': ('STL_Tree_InsertHint', 'treeinserthint'),
    '76ac0e735e': ('STL_Map_Subscript', 'mapsubscript'),  # was mislabelled treecopy from two wrong sweep60 names
}
GAME = {'7670104ecd'}  # virtual predicate chains over a vector of arguments: game code
for line in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'stlr_labels_extra.txt')) if os.path.exists(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'stlr_labels_extra.txt')) else []:
    p = line.strip().split('|')
    if len(p) == 3:
        if p[1] == 'GAME':
            GAME.add(p[0])
        else:
            LABELS[p[0]] = (p[1], p[2])
if __name__ == '__main__':
    d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))
    g = collections.defaultdict(list)
    for x in d:
        if 'nk' in x:
            g[x['nk']].append(x)
    cov = collections.Counter()
    for k, v in g.items():
        for x in v:
            if x['b'] in ('raw_pure', 'raw_mixed'):
                cov['label' if k in LABELS else 'game' if k in GAME else 'none'] += 1
    print(dict(cov))
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 30
    for k, v in sorted(g.items(), key=lambda kv: -len(kv[1])):
        if k in LABELS or k in GAME or len(v) < 2:
            continue
        raw = [x for x in v if x['b'].startswith('raw')]
        if not raw:
            continue
        print(len(v), k, [x['a'] for x in raw][:3], [x['nm'][:36] for x in v if x['nm']][:2], raw[0]['size'], raw[0].get('vcall'))
        n -= 1
        if not n:
            break
