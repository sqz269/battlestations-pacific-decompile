# Foliage loading and bush material setup, segment 80 (packet cc13-gui6)

Addresses: 00AD5590-00ADA240.

Code: `include/bsp/foliage_setup_cc13.hpp`, `src/foliage_setup_cc13.cpp`.
Names: `config/names/00ad0000.jsonl` (89 records, evidence prefixed "Hypothesis, not a
recovered symbol. cc13-gui6"), applied to Ghidra with `tools/ghidra_annotate.py --apply`.
Calls: `reports/cc13_gui6_foliage_calls.json` (42 rows, `tools/verify_report_calls.py`: 0 failed).

Already covered elsewhere and skipped here: the foliage draw (00AD7A30, 00AD7360, 00AD72A0)
in docs/CC13_SHADOW_CASCADES.md; the foliage type table parser (00AD9AC0 -> 00AE84E0) and the
Foliage setting (00AD71C0) in docs/APP_INIT_WORLD_EFFECTS.md; the bush and tree group kernels
of segment 81 (cull, LOD walk, quadtree loaders, `BSP_BushGroup_Initialise`,
`BSP_TreeGroup_Initialise`, the type constructors) in docs/CC13_TERRAIN_RENDER.md; the landscape
ground queries in docs/SCENE_CONTENTS_HOSTS.md.

## Status

| Level | What |
| --- | --- |
| Named in ledger and Ghidra | 89 of the 95 functions that were unnamed or carried generated names (82, plus 7 deleting destructors); 6 deleting destructors without an identified class keep their `CG_` names |
| Reconstructed, build-tested | 5 routines in `foliage_setup_cc13.cpp`; `./scripts/build.ps1` passes |
| ABI-compatible / game-validated | none |

## The foliage system's fields used here

The A8h object at [00F8C210] (published by its base constructor 00AD6950; the derived
destructor 00AD9570 ends in the base destructor 00AD69F0, which unregisters it).

| Offset | Field | Writer / reader |
| --- | --- | --- |
| +04h | bush textures dirty | set by 00AD7FE0, cleared by 00AD60D0 |
| +08h, +0Ch | scalars from [game+5FCh]+A88h / +A8Ch | 00AD56F0 / 00AD5710 from BSP_Game_ConstructWorld; +0Ch read by BSP_BushCell_CullAndEmitLod, +08h by BSP_ShaderConstants_GatherAndUpload |
| +10h | BushShadowAllowed | 00AD5750 from [game+5FCh]+A90h, which 004EA870 fills from the scene property (00CE9364); read by 00AE23F0 |
| +11h | BushNormalAllowed | 00AD5730 from 004EA870 (00CE9308); read by the shader constant gather |
| +24h, +30h | tree and bush quadtrees | 00AD7B90 removes groups from them |
| +3Ch | passed to every TreeGroupInstance | 00AD8C60 |
| +48h/+4Ch/+50h | TreeGroup types {data, count, capacity} | 00AD7030 |
| +54h (first +58h, last +5Ch, end +60h) | bush groups, keyed by landscape | 00AD8DD0, 00AD7140, 00AD7B90 |
| +64h (first +68h, last +6Ch, end +70h) | tree groups, keyed by (landscape, type) | 00AD8C60, 00AD7B90 |
| +78h | bush diffuse factor | 00AD7140 |
| +7Ch, +84h, +8Ch, +94h | bush texture, normal texture, atlas, normal atlas names | 00AD7FE0 |
| +9Ch, +A0h | bush texture and normal texture handles | 00AD60D0; read through 00AD57D0 / 00AD57E0 by BSP_BushGroup_Initialise |
| +A4h | bush effect (docs/CC13_SHADOW_CASCADES.md) | released by 00AD9570 |

## Scene properties to the foliage system (004EA870)

004EA870 (not renamed here; it is a scene-property reader outside this segment) reads, in
order, `AmbientLumScale` (00CE9354), `AmbientLumOffset` (00CE9340), `BushDiffuseFactor`
(00CE932C), `BushTextureName` (00CE931C) and `BushNormalAllowed` (00CE9308). It then calls
00AD7140 with BushDiffuseFactor (004EAA29: `FLD [ESP+18h]` is the third stored value),
00AD7FE0 with the BushTextureName string and 00AD5730 with the BushNormalAllowed byte, each on
[00F8C210]. `BushShadowAllowed` (00CE9364) goes to [game+5FCh]+A90h, which
BSP_Game_ConstructWorld forwards through 00AD5750.

## Bush material (00AD7FE0, 00AD60D0, 00AD7140)

00AD7FE0 ignores a null, empty or (case-insensitively) unchanged name. Otherwise it marks the
textures dirty and derives the other three names from the stem (the name without its last four
characters): `stem + "normal.dds"`, `"Effects\Foliage\" + stem + ".ats"` and
`"Effects\Foliage\" + stem + "normal.ats"`. A name shorter than four characters would hand a
negative length to the substring routine; the C++ keeps the whole name instead and says so.

00AD60D0 (caller 004ED910) does the loading when dirty: release each old atlas whose texture
handle is set, load the colour atlas and texture into +9Ch, load the normal atlas and texture
into +A0h, clear the flag. The old texture handles are overwritten, not released.

00AD7140 stores the factor at +78h and hands it to every bush group through 00AE22D0, which
keeps it at group +24h and registers it as the material float parameter `cDiffuseFactor`.

## Per-landscape files (00AD92C0, 00AD9050, 00AD91C0)

00AD92C0 runs when a landscape loads (only caller 00ADAC95). With the foliage enable byte
00F8C20C set it builds `terrain/<name>_bushes_pc.dat` and `terrain/<name>_trees_pc.dat` from
the landscape's name (+20h) and loads each one that the VFS ([0109CEEC] virtual +8) reports as
present. It then re-applies the diffuse factor whether or not anything loaded (00AD92E2 jumps
straight to 00AD94F1).

- Bushes (00AD9050): `count = read_int`; when non-zero, two strings, then the landscape's bush
  group (found or created by 00AD8DD0, keyed by the landscape pointer, constructed by
  `BSP_BushGroupType_Construct` with the two strings) is initialised from the rest of the stream
  by `BSP_BushGroup_Initialise` with the count. One bush group per file.
- Trees (00AD91C0): `v = read_int`; while `v` is non-zero: `x = read_int`, `model =
  read_string`, the TreeGroup type for the model (00AD7030, by name), the tree group for
  (landscape, type) (00AD8C60, constructing `BSP_TreeGroupInstance_Construct(landscape, type,
  [system+3Ch], x)`), `BSP_TreeGroup_Initialise` with the stream and `v`, then `v = read_int`.

Stream slots as in docs/SCENE_CONTENTS_HOSTS.md: +38h read_int, +48h read_string.

00AD7B90 is the inverse, called from BSP_Game_DestroyWorld and 00ADBF80 (landscape unload): it
erases every bush and tree group whose landscape has the same name as the argument (a name
compare, not a pointer compare), removing each from its quadtree first when it is registered.

## Other routines

- 00ADA0D0 `BSP_Landscape_LocalGridToWorldXZ`, a Landscape vtable slot (00D5D36C) beside
  BSP_Landscape_LocalHeight/LocalNormal: `(scale * u + offset_x, 0, scale * v + offset_z)`
  (+1Ch, +80h, +84h) through the node's world matrix; returns x and z.
- Singleton bases: the foliage type table (00AD6810, [00F8C264]), the foliage system (00AD6950,
  [00F8C210]) and the terrain render object (00AD9D00, [00F8C218]), each registered with the
  singleton lifetime manager and unregistered by its base destructor.
- 00AD55C0 is a font descriptor constructor called by BSP_FontRegistry_LoadLuaDescriptor; it
  sits in this segment but is not foliage code (`_Provisional`).
- Container helpers: pointer arrays `{data, count, capacity}` (00AD62A0, 00AD64D0, ...), the
  bush and tree group vector inserts (00AD8620, 00AD87E0), the 30h-byte foliage type record
  destroyers (00AD77E0 and the range walkers).

20 of the functions have no rel32 call and no absolute dword reference in the image (scan with
`local/gui6_absrefs.py`); their ledger evidence says so.

## Open questions

- What [game+5FCh]+A88h / +A8Ch (foliage system +08h / +0Ch) are: no store to +A88h was found
  by a byte scan, and 004EA870 does not write them.
- 00AD56C0 / 00AD56D0: a field at +3Ch of the object handled by the landscape load/unload code;
  meaning unknown.
- 00AD7880 (`_Provisional`): a destructor of an object with reference-counted members; its
  class is not identified.
- The deleting destructors 00AD5770, 00AD5790, 00AD79F0, 00AD7A10, 00AD9840 and 00AD9CE0 keep
  generated names.
- The bush file's two strings: which one is the model and which the texture is decided inside
  `BSP_BushGroupType_Construct`, which was not re-read here.

## Verification

- `./scripts/build.ps1`: build and both existing test targets pass.
- `python tools/verify_report_calls.py reports/cc13_gui6_foliage_calls.json`: 42 rows, 0 failed.
- No new tests.
