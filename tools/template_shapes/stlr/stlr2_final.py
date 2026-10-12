"""cc13-stlreview2: second-pass overrides on top of stlr_final.py, then write tools/template_shapes/stlr2_decisions.txt.

Scope: every stl_probable row outside the cc13-sweep67 lease (00670000-006CFFFF, 00700000-0073FFFF).
Overrides come from reading the pseudocode (docs/CC13_STL_PROBABLE_REVIEW.md, section 9):
  NAMED   named rows whose names missed the TMPL regex but whose bodies are container operations
  DROP    unnamed game code the anchor rules missed: float math, indexed this[N] fields >= 100h,
          game calls (entity kill, GUI, input, Lua reader, net messages) or several virtual calls
"""
import json, os, collections
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))
EXCL = [(0x670000, 0x6cffff), (0x700000, 0x73ffff)]
NAMED = {'00b14480': ('STL_Vector_EraseRange', 'veceraserange'), '00b7cdf0': ('STL_TreeIter_Dec', 'iterdec')}
DROP = {
    # was keep_uncertain
    '0041e000': 'float math', '00429b00': 'BSP_BitCursor_WriteBits', '0042a480': 'BSP_BitCursor_ReadBits',
    '00443a30': 'float math', '004514a0': 'panel sequence map + string compare', '0045a320': 'float params',
    '00481430': 'BSP_MissionEntity_Kill', '00484a10': 'BSP_MissionEntity_Kill', '00487940': 'BSP_MissionEntity_Kill',
    '00491e70': 'BSP_LandingTroop_OnShipLeft', '004a5b90': 'BSP_MissionEntity_Kill', '004ab9a0': 'BSP_Random_UniformFloatRange',
    '0051e050': 'front-end screen teardown', '005251c0': 'BSP_InputManager', '00538a80': 'front-end screen teardown',
    '00540d50': 'free-camera HUD', '00542620': 'BSP_GuiText_SetLocalisedCString', '0059aa40': 'float math',
    '005f6ef0': 'BSP_InputManager', '006fbad0': 'float math + vcalls', '00797af0': 'float math', '00797ed0': 'BSP_MovieKeyframe_Tick',
    '007aed10': 'float math + sqrt', '007af740': 'BSP_Vector4f_Transform', '007b1fb0': 'float math + sqrt',
    '00884b60': 'BSP_MissionLuaVariant', '0088b120': 'BSP_MissionLuaVariant', '008ddbe0': '7 virtual calls',
    '0090cdd0': 'string formatting with ints', '0093b920': 'BSP_PointEffect_StopChildren', '00947bc0': 'BSP_SessionMessage + vehicle class',
    '009685b0': '8 virtual calls', '0097dc80': 'BSP_RandomThreads_NextU32', '00a02180': 'float math + vcalls',
    '00a12580': 'BSP_AiGroup_LeaderPoint', '00a12db0': 'BSP_EntityPose_RefreshWorld', '00a12fd0': 'BSP_EntityPose_RefreshWorld',
    '00ab1f60': 'GUI widget this[3Bh]+vcall', '00acf1d0': 'GUI widget this[3Bh]+vcall', '00ad51d0': 'BSP_Font_LoadGlyphDataAndTextures',
    '00ada810': 'BSP_Transform_RefreshWorldMatrix', '00adb6f0': 'float math', '00adf1b0': 'float math', '00aeab30': 'float math',
    '00aeb0b0': 'float grid stepping', '00aeb1f0': 'float math + fmod', '00aeb310': 'float math + fmod',
    '00bbad60': 'BSP_ProviderName_MatchesEnumerationFilter', '00bd8ec0': 'BSP_LuaReader_LookupChild',
    # was keep_library
    '0043dd80': 'float params', '0045a210': 'this[43h] field', '0045b220': 'this field +198h', '00741a40': 'BSP_NetEntityCreateMessage +130h',
    '007ae6d0': 'float math', '007b2950': 'BSP_NetEntityCreateMessage +11Ch', '0091a080': '4.7 KB body, field +1E8h, float math',
    '0095f1c0': 'field +130h, 4 vcalls', '00a9bf90': 'this[45h] field', '00a9c860': 'this[43h] field', '00acb9b0': 'this[41h] field',
    '00ad0680': 'GUI widget this[3Eh] + 2 vcalls',
    # third run (--s67), rows released by cc13-sweep67
    '006bf350': 'critical section + field +50h (air ops)', '006c7540': 'critical section + field +0ACh (air ops)',
    '006c77e0': 'critical section + field +0ACh (air ops)', '006cd6c0': 'BSP_VehicleClass_GetOrCreate + field +0E4h',
    '006acb40': 'BSP_GameEntity_Destruct + field +204h', '006cc800': 'BSP_VehicleClass_GetRegistryId under a critical section',
    '0071ba20': 'BSP_ModelNoteNameEquals_Predicate + field +84h', '00725380': 'float math (66 float ops)',
    '0072d950': 'BSP_UnitTickableEntity_Destruct + field +47Ch',
}
c = collections.Counter()
for x in d:
    a = x['a']
    if a in NAMED:
        x['dec'], x['shape'] = 'upgrade_named', NAMED[a]
    elif a in DROP:
        x['dec'], x['anch'] = 'drop_game', ['read: ' + DROP[a]]
import sys
OUT = 'stlr2_decisions.txt'
MINE = None
if '--s67' in sys.argv:
    # rows cc13-sweep67 held; cc13-leftovers1 still leases three ranges and 00675C40 belongs to cc9
    OUT = 'stlr3_decisions.txt'
    EXCL = [(0x73eba0, 0x73ffff), (0x721c00, 0x721dff), (0x7240e0, 0x7242ff), (0x675c40, 0x675c40)]
    MINE = {a for a in open(os.path.join(root, 'local', 's2_s67.txt')).read().split()}
lines = ['# cc13-stlreview2: second-pass decision per stl_probable bookmark row (docs/CC13_STL_PROBABLE_REVIEW.md section 9)',
         '# addr|old_evidence|decision|category|name|evidence_key|anchors']
for x in sorted(d, key=lambda x: x['a']):
    a = int(x['a'], 16)
    dec = 'deferred_leased' if any(lo <= a <= hi for lo, hi in EXCL) else x['dec']
    if MINE is not None and x['a'] not in MINE:
        dec = 'out_of_scope'
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
    c[(dec, x['ev'])] += 1
open(os.path.join(root, 'tools', 'template_shapes', OUT), 'w', encoding='utf-8', newline='\n').write('\n'.join(lines) + '\n')
for k, v in sorted(c.items()):
    print(v, k)
