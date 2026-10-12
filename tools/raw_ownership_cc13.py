"""Ownership map for raw functions (cc13-mopup1; rules from the cc13 band docs' triage tables, docs/CC13_MOPUP_1.md).

python tools/raw_ownership_cc13.py table            per-band counts by owner (raw only)
python tools/raw_ownership_cc13.py list <owner> [lo hi]   addresses for one owner
Owners: cc9, ng (non-gameplay), sweep79 / effects1 (leased to running cc13 workers), unk.
"""
import sqlite3
import sys

import os
DB = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'local', 'bsp_index.sqlite')

# (lo, hi, owner, source note); first match wins, so specific rows come first.
RULES = [
    (0x00790000, 0x007FFFFF, 'sweep79', 'lease cc13-sweep79'),
    (0x00865000, 0x00875FFF, 'effects1', 'lease cc13-effects1'),
    # 0040-0044 (docs/CC13_BAND_0040_0044.md)
    (0x0042A930, 0x0042CD00, 'cc9', '0040 doc: plane HUD and zoom fields'),
    (0x00415000, 0x00418FFF, 'cc9', '0040 doc: AvoidZone outline/segments'),
    (0x0041A000, 0x00424FFF, 'cc9', '0040 doc: AvoidZone groups/layers/manager'),
    (0x0043F000, 0x00443010, 'cc9', '0040 doc: gun, device class, descriptors, class-id predicates'),
    (0x0043B9C0, 0x0043B9C0, 'cc9', '0040 doc: water-height probe with a hit record'),
    # 0045-0048 (docs/CC13_BAND_0045_0048.md)
    (0x00453CC0, 0x004663FF, 'cc9', '0045 doc: engine movie, movie actions, entity-command session messages'),
    (0x004684F0, 0x004684F0, 'cc9', '0045 doc: squadron auto-target ordnance latch'),
    (0x00470350, 0x004707FF, 'cc9', '0045 doc: hit records'),
    (0x0047B600, 0x0047BBFF, 'cc9', '0045 doc: path/unit helpers, attack gate 0047B850'),
    (0x0048A010, 0x0048A010, 'cc9', '0045 doc: soldier/traffic behaviour on the shared RNG'),
    (0x0048A1E0, 0x0048A1E0, 'cc9', '0045 doc: soldier/traffic behaviour'),
    (0x0048A650, 0x0048A650, 'cc9', '0045 doc: soldier/traffic behaviour'),
    (0x0048AEC0, 0x0048AEC0, 'cc9', '0045 doc: soldier/traffic behaviour'),
    (0x0048C230, 0x0048C230, 'cc9', '0045 doc: soldier/traffic behaviour'),
    # 0049-004A: segment 8 head (land vehicle classes, soldier types, wrecks); see seg rule below
    # 004B-004F (docs/CC13_BAND_004B_004F.md)
    (0x004B0000, 0x004B3FFF, 'cc9', '004B doc: soldier registry, wreck class, explosion smoke'),
    (0x004F0000, 0x004F2FFF, 'cc9', '004B doc: spawn point, land convoy, unit owner entity'),
    (0x005F9580, 0x005FB7FF, 'cc9', 'mopup: HUD order screen unit commands (attackmove, regroup, attack, return to base)'),
    (0x004C7AA0, 0x004C7AA0, 'cc9', 'mopup: world border zone push-out'),
    (0x004CA760, 0x004CA930, 'cc9', 'mopup: world border zone layout/queries'),
    (0x004BFED0, 0x004BFED0, 'cc9', 'mopup: unit selection predicate'),
    (0x004C3A80, 0x004C3A80, 'cc9', 'mopup: multiplayer unit selection update'),
    (0x00428420, 0x00428420, 'cc9', 'mopup: projectile trace helper'),
    (0x0046B730, 0x0046B730, 'cc9', 'mopup: called by BSP_EngineMovie_Stop (cc9 movie wiring)'),
    (0x00487470, 0x00487470, 'cc9', 'mopup: traffic runtime view gate (check with cc9, 0045 doc)'),
    (0x00487A70, 0x00487A70, 'cc9', 'mopup: traffic behaviour update (shared RNG, 0045 doc)'),
    (0x004E7EF0, 0x004E7F90, 'cc9', 'mopup: scene game-entity constructors'),
    (0x004E8060, 0x004E8060, 'cc9', 'mopup: scene game-entity destructor'),
    (0x004E9520, 0x004EA7FF, 'cc9', 'mopup: scene unit entities, unit list registration, JapanList stock'),
    (0x00521F10, 0x00526FFF, 'cc9', 'mopup: HUD unit pick / target cycle / attack-move command logic (0050 doc: pick/command logic left for cc9)'),
    (0x00534690, 0x00534720, 'cc9', 'mopup: formation command messages'),
    (0x00535B20, 0x00535F50, 'cc9', 'mopup: formation command issue / update'),
    (0x00545370, 0x00545370, 'cc9', 'mopup: weapon group availability'),
    (0x005457C0, 0x005457C0, 'cc9', 'mopup: weapon director target predicate'),
    (0x00545B80, 0x00545E50, 'cc9', 'mopup: weapon group availability'),
    (0x00547480, 0x00549430, 'cc9', 'mopup: weapon aim / group firing, command-building respawn'),
    (0x00604A20, 0x00604A50, 'cc9', 'mopup: plane state predicates (0060 doc)'),
    (0x006058A0, 0x006062B0, 'cc9', 'mopup: plane instrument/effects maths'),
    (0x0060C5B0, 0x0060C5B0, 'cc9', 'mopup: plane collision sweep predicate'),
    (0x006441B0, 0x006441B0, 'cc9', 'mopup: unit ordnance predicate'),
    (0x00656250, 0x00673FFF, 'cc9', 'mopup: support manager purchases, slots, spawns and its session messages (0060 doc: cc9 for the support-manager logic)'),
    (0x00778860, 0x00778860, 'cc9', 'mopup: AI command predicate'),
    (0x0085E4D0, 0x0085E4D0, 'cc9', 'mopup: intercept solution maths (plane/gun bot)'),
    (0x008E34F0, 0x008E36FF, 'cc9', 'mopup: command building / AI party / power-up predicates'),
    (0x008EF7C0, 0x008EF7C0, 'cc9', 'mopup: command building garrison respawn setter'),
    # 0060-0066 (docs/CC13_BAND_0060_0066.md)
    (0x0066DD00, 0x0066EC1B, 'cc9', '0060 doc: support-manager purchases/commands/messages'),
    (0x00675C40, 0x00675C40, 'cc9', 'cc9 reserved: support-manager update'),
    # 0067-0073 (docs/CC13_BAND_0067_0073.md): remaining raw are cc9 except the decal helpers
    (0x0073EBA0, 0x0073EBA0, 'ng', '0067 doc: decal triangle clip'),
    (0x0073F1B0, 0x0073F1B0, 'ng', '0067 doc: decal tree walk'),
    (0x0073F6E0, 0x0073F6E0, 'ng', '0067 doc: decal tree walk'),
    (0x00725550, 0x00725550, 'ng', 'leftovers doc: GeomMesh debug wireframe'),
    (0x006AC000, 0x006ACFFF, 'cc9', '0067 doc: ship AI landing approach / command-building landing paths'),
    (0x00696470, 0x006BA61F, 'ng', '0067 doc: seg 29 up to 006BA620 (air-ops sites listed separately)'),
    (0x006BA620, 0x006CFFFF, 'cc9', '0067 doc: air ops'),
    (0x00670000, 0x00696450, 'ng', '0067 doc: segs 27 tail-28 front end'),
    # 006D-006F (docs/CC13_BAND_006D_006F.md)
    (0x006D6000, 0x006DEDFF, 'ng', '006D doc: world markers, GUI highlights'),
    (0x006EF000, 0x006F1F1F, 'ng', '006D doc: fog-cloud model'),
    (0x006FB300, 0x006FB3CF, 'cc9', '006D doc: cruiser constructor 006FB300'),
    (0x006F9A00, 0x006FB2FF, 'ng', '006D doc: ConvexObject resource'),
    (0x006D0000, 0x006FFFFF, 'cc9', '006D doc: air-ops sites, bombs/projectiles, buildings, squadron commands, ships'),
    (0x00700000, 0x00701AD0, 'cc9', '0067 doc: door/cargo/bomb-carrier units'),
    (0x00701B70, 0x0070BFFF, 'ng', '0067 doc: message maps, fish school, flag model'),
    (0x0070C000, 0x00714030, 'cc9', '0067 doc: flak bullet, ship AI unit lists, unit part effects'),
    (0x00714060, 0x0071B710, 'ng', '0067 doc: resource items and parsers'),
    (0x0071BA20, 0x00722FB0, 'cc9', '0067 doc: weapon director, command controllers'),
    (0x00727B20, 0x00728960, 'cc9', '0067 doc: hit records, sensor state, gun construction'),
    (0x00723030, 0x00727B1F, 'ng', '0067 doc: geom-mesh resource'),
    (0x00728A90, 0x00735EC0, 'cc9', '0067 doc: guns, gun messages, mission countdown'),
    (0x00737710, 0x00737730, 'cc9', '0067 doc: Bink movie wiring'),
    (0x0073F9C0, 0x0073F9C0, 'cc9', '0067 doc: bullet class Lua fields'),
    (0x00735F30, 0x0073FFFF, 'ng', '0067 doc: MSAR mount, singleton holders, decal manager'),
    # 0074-0078 (docs/CC13_BAND_0074_0078.md)
    (0x00740000, 0x0076A9BF, 'cc9', '0074 doc: land units, carrier deck, session-message classes'),
    (0x0076A9C0, 0x00778FFF, 'ng', '0074 doc: MMultiPlayer session'),
    (0x00779000, 0x00781FFF, 'cc9', '0074 doc: entity replication'),
    (0x0078D3D0, 0x0078D84F, 'cc9', '0074 doc: plane water-surface law'),
    (0x00782000, 0x0078FFFF, 'ng', '0074 doc: transport, packet statistics, ocean container'),
    # 0080-008F (docs/CC13_BAND_0080_008F.md, docs/CC13_LEFTOVERS_1.md)
    (0x0085C000, 0x0085E8FF, 'ng', '0080 doc: matrix and collision geometry'),
    (0x00880080, 0x00880080, 'cc9', 'leftovers doc: avoid-zone byte grid'),
    (0x00880220, 0x00880420, 'cc9', 'leftovers doc: avoid-zone byte grid / shallow water'),
    (0x00880000, 0x00883FFF, 'ng', '0080 doc: landscape terrain'),
    (0x008D8F00, 0x008D8FFF, 'ng', '0080 doc: scene tokenizer'),
    (0x008DC000, 0x008E37FF, 'ng', '0080 doc: objective-sync messages, map ping'),
    (0x008EE5F0, 0x008F7DFF, 'ng', '0080 doc: scene property bag'),
    (0x00800000, 0x008FFFFF, 'cc9', '0080 doc: units, weapons, Lua host and natives, power-ups, bots'),
    # 0090-00A4 (docs/CC13_BAND_0090_00BF.md, docs/CC13_BAND_00A1_00A4.md)
    (0x0098C630, 0x00996060, 'ng', '0090 doc: seg 66 system (hardware, registry, content package)'),
    (0x00900000, 0x00A38DA0, 'cc9', '0090/00A1 docs: bots, entities, vehicle classes, tasks, AI'),
    (0x00A38DA1, 0x00A4FFFF, 'ng', '00A1 doc: network console, XLive, telemetry'),
    # 00A5-00CE (docs/CC13_BAND_0090_00BF.md, docs/CC13_TERRAIN_RENDER.md)
    (0x00AE9BD0, 0x00AED100, 'cc9', 'terrain doc: segment line test (gameplay lane)'),
    (0x00ADEB40, 0x00ADEB40, 'cc9', 'terrain doc'),
    (0x00ADEB80, 0x00ADEB80, 'cc9', 'terrain doc'),
    (0x00ADF1B0, 0x00ADF1B0, 'cc9', 'terrain doc'),
    (0x00A50000, 0x00CEFFFF, 'ng', 'engine, render, GUI widgets, Lua, CRT, static init'),
]

# segment defaults where no rule matched (segment ids from bsp.py segment)
SEG = {}
for s in range(0, 8):
    SEG[s] = 'ng'
SEG[8] = 'cc9'  # 00491170-004BA200 soldier types, land vehicle classes, wreck class (gameplay classes)
for s in range(9, 30):
    SEG[s] = 'ng'


def owner(a, seg):
    for lo, hi, o, note in RULES:
        if lo <= a <= hi:
            return o, note
    if seg in SEG:
        return SEG[seg], 'segment %s default' % seg
    return 'unk', ''


def raw_rows(c):
    return c.execute("""SELECT address, segment FROM functions
      WHERE (name LIKE 'FUN\\_%' ESCAPE '\\' OR name LIKE 'thunk\\_FUN\\_%' ESCAPE '\\')
      AND COALESCE(tag_category,'')='' AND COALESCE(ledger_name,'')='' ORDER BY address""").fetchall()


def main():
    c = sqlite3.connect(DB)
    rows = raw_rows(c)
    if sys.argv[1] == 'table':
        bands = {}
        tot = {}
        for a, s in rows:
            o, _ = owner(a, s)
            bands.setdefault(a >> 16, {}).setdefault(o, 0)
            bands[a >> 16][o] += 1
            tot[o] = tot.get(o, 0) + 1
        print('total', len(rows), tot)
        for b in sorted(bands):
            print('%04X %s' % (b, ' '.join('%s=%d' % kv for kv in sorted(bands[b].items()))))
    elif sys.argv[1] == 'list':
        lo = int(sys.argv[3], 16) if len(sys.argv) > 3 else 0
        hi = int(sys.argv[4], 16) if len(sys.argv) > 4 else 0xFFFFFFFF
        for a, s in rows:
            o, note = owner(a, s)
            if o == sys.argv[2] and lo <= a <= hi:
                print('%08X %s %s' % (a, s, note))


if __name__ == '__main__':
    main()
