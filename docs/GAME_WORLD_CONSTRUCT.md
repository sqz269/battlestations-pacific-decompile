# Mission world construction (`004DE610`)

Addresses: 004de610 004cb030 004cb0b0 006deca0 006dedc0 004d2bb0

Packet `game_world_construct`, worktree `agent/world-construct`. Ghidra was read-only for this
packet; every name below is a hypothesis, not a recovered symbol. Reconstruction in
`include/bsp/world_construct.hpp` and `src/world_construct.cpp`, machine-readable facts in
`reports/game_world_construct.json`.

The whole body was read from the raw listing (`disasm-raw 004DE610 --length 5400`), not from the
pseudocode. Ghidra aliases the allocation sizes into stack slots (`piStack_ac`, `local_88`), drops
the hidden `this` of the unprototyped callees, and marks `004DF846` unreachable, which is exactly
the block that materialises the `Ocean initialization failed` literal. `ghidra flow 004DE610`
reports 1370 instructions and no gaps.

## The correction this packet makes

The packet contract asked for "the `Ocean initialization failed` path: what fails, how it is
reported, what state the game is left in". **There is no such path.** The literal at `00CE7D3C` is
built and thrown away:

```
004df810: cmp dword ptr [esi + 0x5fc], edi   ; the scene record, not an ocean result
004df816: mov dword ptr [esp + 0x9c], ebp
004df81d: mov dword ptr [esi + 0x19f0], eax
004df823: je 0x4df8da                        ; no record -> skip the whole block
004df829: push 1
004df82b: push 0x1b                          ; 27 = strlen("Ocean initialization failed")
004df82d: lea ecx, [esp + 0x2c]
004df839: call 0x41dd40                      ; BSP_NativeString_Resize(&s, 1Bh, 1)
004df83e: mov ebx, dword ptr [esp + 0x28]
004df844: je 0x4df86c
004df84e: push 0xce7d3c                      ; the literal
004df854: call 0xbf7680                      ; memcpy into the pooled buffer
004df860: call 0x419cc0                      ; BSP_SizedStoragePool_GetSingleton
004df867: call 0xbd1510                      ; ReturnBlock  -- and that is all
```

No log call, no store, no branch on the result. The guard is `game+5FCh != 0`, so the string is
built on the path where the scene record **exists**, which is the success path. This is the same
shape `docs/MISSION_SCENE_LOAD.md` already recorded for `Scene initialization failed` at `004E014D`
and `docs/GAME_WORLD_ENTITIES.md` for the decal manager's compiled-out loop: a diagnostic whose
sink was inlined away, leaving only the allocation and the release.

The ocean bring-up has no error branch at all. Both branches of `004DF421` construct an ocean
owner with the same constructor `00BBDFF0`; they differ only in the second argument. What a caller
can observe as "no ocean" is a null `game+19E8h`, which happens only if `operator new` returns
null, and `docs/GAME_WORLD_OCEAN.md` shows the per-frame reader gating the whole ocean block on
exactly that pointer. `bsp::ocean_failure_is_reported()` returns `false` to state this.

## `004DE610` `BSP_Game_ConstructWorld`

`__fastcall void (GGame*)`, body `004DE610-004DFAFC`, `RET` with no stack cleanup (`004DFAFC`), so
`this` in ECX is the only argument. Sole caller `004DFB70` `BSP_Game_LoadMissionScene` at
`004E01DE`, inside the `1_` VFS block and immediately after the first `0046DF00` scene pass.

Two allocators appear. `00BF55BE` is used once, for the world object; everything else uses
`00BF681B` (cdecl, `add esp,4` after the call) except the Operator node, which comes from the
scene-node class allocator `00B71930` with its size in ECX. Every allocation follows the same
shape: `operator new`, a null test, the constructor on the non-null path, `XOR EAX,EAX` on the
other, then the store. A failed allocation therefore stores a null and the routine continues.

### Ordered construction steps

| # | Site | What is built | Size | Constructor | Slot |
| --- | --- | --- | --- | --- | --- |
| 0 | 004DE63C | `game+719Dh` set -> `[00F8D394]+18h = 20000000h` | | | |
| 1 | 004DE651 | world object, `memset 0` at 004DE664 first | `4BCh` | `004CB030` | `game+19CCh` |
| 2 | 004DE69C | `009037F0(world, 1, 1)` | | | |
| 3 | 004DE6A1 | scene root, name `World` (`00CE7E10`) | `24h` | `00B724E0` | `game+19ECh` |
| 4 | 004DE72A | `[00F8BBF0]->vtable[1](byte [00F88A04])` | | | |
| 5 | 004DE73F | Operator node, name `Operator` (`00CE7E04`) | `458h` | `00B71A80` | `game+19FCh` |
| 6 | 004DE7C3 | `DAT_00E188B0 = node`, then `00B6FBF0(node, [00CE380C])` | | | |
| 7 | 004DE7E4 | Operator child, attached by `00B71990`, then released | `34h` | `00B1F850` | `game+1A00h` |
| 8 | 004DE843 | `node->vtable[30h](&{0, [00CE3D08], 0})` | | | |
| 9 | 004DE872 | `00B6FE50(node, 0000506Eh)` | | | |
| 10 | 004DE896 | `00B1FFB0([00F8D394], 2 - [00F889E8])`, then `vtable[F0h]([00F889E4])` | | | |
| 11 | 004DE8C7 | `node+4Ch` = one of `[00CE3800]`, `[00CE3E18]`, `[00D7A24C]` indexed by `[00F889D4]`; overridden with `[00CE3804]` when `[00F8899C]` is set | | | |
| 12 | 004DE923 | `00AD71C0([00F8C210], byte [00F88A06])`, foliage enable | | | |
| 13 | 004DE936 | `004DCDF0(game)` | | | |
| 14 | 004DE93F | `004C9EC0(game)`, which creates the directional light at `game+19F8h` (its strings are `TestDirectionalLight` and `AllLights`) | | | |
| 15 | 004DE944 | **branch on the scene record `game+5FCh`** | | | |
| 16 | 004DF421 | ocean owner | `40h` | `00BBDFF0` | `game+19E8h` |
| 17 | 004DF6A3 | atmosphere, shared then released | `94h` | `00B84E50` | none |
| 18 | 004DF7B5 | sky | `B8h` | `0078DAA0` | `game+19F0h` |
| 19 | 004DF829 | the dead `Ocean initialization failed` literal | | | |
| 20 | 004DF911 | eight channel objects, `008DF900(i)` then `008DA160` | `30h` x8 | `008DF900` | `game+21A4h..+21C0h` |
| 21 | 004DF95B | `0091C560(game+21A0h)` | | | |
| 22 | 004DF966 | | `10h` | `008E2B90` | `game+21C8h`, follow-up `008E25F0` |
| 23 | 004DF9A2 | | `10h` | `009221E0` | `game+21CCh`, follow-up `00920EC0` |
| 24 | 004DF9DE | marker manager | `50h` | `006DECA0` | `game+21D4h`, follow-up `006D6200` |
| 25 | 004DFA1A | | `70h` | `00707480` | `game+21DCh`, follow-up `007018C0` |
| 26 | 004DFA56 | | `58h` | `00735030` | `game+21E8h`, follow-up `00733030` |
| 27 | 004DFA92 | | `10h` | `00945820` | `DAT_00F89B3C`, follow-up `00941310` |
| 28 | 004DFACD | `008D5B50(00F88980)` | | | |
| 29 | 004DFAD7 | `004BEC00(1Dh, 1)` then `00A933F0` | | | |

`game+21A0h` is not created here; `docs/MISSION_SCENE_LOAD.md` places it in `BSP_Game_OnInit` at
`004E3E74`, and step 21 only calls into it.

The eight-object loop is worth stating precisely, because the count matters for the teardown:

```
004df90f: xor edi, edi
004df911: lea ebx, [esi + 0x21a4]
004df917: push 0x30
004df919: call 0xbf681b
004df937: call 0x8df900        ; ctor(this, edi)
004df949: mov dword ptr [ebx], eax
004df94b: call 0x8da160        ; register
004df950: add edi, 1
004df953: add ebx, 4
004df956: cmp edi, 8
004df959: jl 0x4df917
```

### The scene-record branch

`004DE944` tests `game+5FCh`, the record `0046DF00` filled on the pass before this routine. The
record path runs `004DE952-004DEF42`, the default path `004DEF47` onward, and they converge at
`004DF372`. The two paths configure the same things from different sources.

Record fields the routine reads, with the site that reads each:

| Offset | Kind | Consumer |
| --- | --- | --- |
| `+990h` | struct | cloud description, `0078C9B0` and `00865900` (004DF878, 004DF88F) |
| `+A20h..+A5Fh` | 4 x `10h` | atmosphere layers, `00B84FA0(record+A20h+i*10h, i)` (004DF781-004DF7A3) |
| `+A78h` | float | Operator node `+178h` (004DE952) |
| `+A84h` | float | light scalar into `[00B7AAB0(light)]+8h` (004DF3FD) |
| `+A88h`, `+A8Ch`, `+A90h` | float, float, byte | foliage system `00AD56F0`, `00AD5710`, `00AD5750` |
| `+AB8h`, `+ABCh` | float, struct | caustics object `vtable[20h]` and `vtable[24h]` (004DEF1F, 004DEF3B) |
| `+ACCh` | struct | value of `CausticsTextureSource` (004DECB9, 004DEEE0) |
| `+C18h`, `+C1Ch` | dword | sky `0078DBA0` and `0078DCF0` (004DF8A2, 004DF8C1) |
| `+C20h` | byte | sky constructor flag (004DF7E1) |
| `+C44h` | struct | value of `WaterTracerColor` (004DF59B) |
| `+C54h` | struct | ocean owner constructor argument (004DF457) |
| `+C5Ch`, `+C60h` | float | ocean `00BBF1A0` and `00BBF200` |
| `+100Ch`, `+1010h` | float | terrain extent through `004B4D80` and the double at `00CE4C08` |
| `+1014h`, `+1024h`, `+1034h` | `10h` each | ocean vectors, `00BBCD90`, `00BBCDE0`, `00BBCD50` |
| `+1044h` | handle | terrain handle into `game+19D4h` via `00871BA0` and `00440580` |
| `+104Ch` | handle | shore handle into `game+19DCh` via `00871BA0` and `00440580` |

The default path installs literal constants instead: it normalises `{[00D7A260], [00D7A24C],
[00CE7D7C]}` with `00419510` into the light's direction at `light+1E0h..+1E8h`, writes
`light+1C4h..+1D4h` from `[00CE7804]`, `[00D7A2F0]`, `[00D7A24C]` and `[00CE7D78]`, and sets
`CausticsTextureSource` to the literal `CausticsDayLight` (`00CE77C4`).

Both paths then install the same three shore-wave pairs through `00B1B830` on the singleton
`00F8D434`: `ShoreWaveTextureSource0` = `ShoreWaves0`, and the same for 1 and 2.

### The ocean bring-up

```
004df421: cmp dword ptr [esi + 0x5fc], 0
004df428: push 0x40
004df42a: je 0x4df5eb                        ; no record -> the named branch
004df430: call 0xbf681b
004df44b: mov ecx, dword ptr [esi + 0x5fc]
004df451: mov edx, dword ptr [esi + 0x19ec]  ; the scene root
004df457: add ecx, 0xc54                     ; the record's ocean description
004df461: call 0xbbdff0                      ; ctor(this, sceneRoot, description)
004df47d: mov dword ptr [esi + 0x19e8], eax
```

and, on the other side, `004DF5EB` allocates the same `40h`, builds the literal `sky_001`
(`00CE7AB0`) into a temporary string and calls the same `00BBDFF0(sceneRoot, &name)`. So
`game+19E8h` is always attempted, and the ocean object proper is `owner+3Ch`
(`docs/GAME_WORLD_OCEAN.md`).

The setters use a compiler idiom that the pseudocode misattributes: the argument is pushed
**before** the accessor call, so `push X; call 00BBDDF0; mov ecx,eax; call 00BBCE30` is
`owner->GetOcean()->SetLight(X)`, not `00BBDDF0(X)`. The same idiom appears at `004DF88F` with
`00865CF0`, which `docs/GAME_WORLD_OCEAN.md` already identified as an argument-less static getter.

Setters, in order: `00BBCE30(game+19F8h)` (record path and default path both), then only on the
record path `00BBCD90`, `00BBCDE0` and `00BBCD50` with the three `10h` record vectors and
`00BBF1A0`/`00BBF200` with the two record floats, then on both paths `00BBCFE0(byte [00F889F4])`.

The `94h` atmosphere object at `004DF6A3` is handed to the ocean owner (`00BBDF20`, only when the
owner is non-null) and to the Operator node (`00B71940`), then released through its own
`InterlockedDecrement`, so those two hold the references. Its parameters are `00B84D00(0.0f)`,
`00B84D40(0.0f)` and `00B84C40(&{[00CE7D60], [00CE7D5C], [00CE7D58], colour 00A5A3ABh, 0.0})`.
`00B84xxx` is the `system_fog_prefix` band, so the object's own semantics belong to that packet.

## `game+19CCh`, the world object

`4BCh`, `operator new` `00BF55BE` at `004DE651`, `memset(block, 0, 4BCh)` at `004DE664`, then
`004CB030` `BSP_World_Construct`. The memset is load-bearing: the constructor never writes `+4h`,
and `+4h` is the intrusive activation-chain head that `00903670` walks every frame
(`docs/GAME_WORLD_ENTITIES.md`).

`__fastcall World* (World*)`, `RET 0`, returns `this` in EAX. Body `004CB030-004CB0AD`.

| Offset | Value | Site |
| --- | --- | --- |
| `+0h` | vtable `00CE7784`; slot 0 is the scalar deleting destructor `004CB0B0` | 004CB04C |
| `+4h` | 0 from the memset; the activation-chain head `00903670` reads | -- |
| `+8h` | 0 from the memset | -- |
| `+Ch`, `+10h`, `+14h` | 0 | 004CB057-004CB05D |
| `+18h..+4A3h` | `61h` elements of stride `0Ch`, element ctor `004C2D30`, element dtor `004B7EC0`, built by `eh_vector_constructor_iterator` | 004CB076 |
| `+4A4h`, `+4A8h` | 0 from the memset | -- |
| `+4ACh` | byte `1` | 004CB098 |
| `+4B0h` | `std::list` `_Myfirstiter`, not written | -- |
| `+4B4h` | `_Myhead` = `004C3080()` on `this+4B0h` | 004CB088, 004CB091 |
| `+4B8h` | `_Mysize` = 0 | 004CB094 |

`18h + 61h*0Ch = 4A4h` and `4B8h + 4 = 4BCh`, so the allocation is fully accounted for.

## `game+21D4h`, the marker manager

`50h`, `operator new` `00BF681B` at `004DF9DE`, constructor `006DECA0`
`BSP_MarkerManager_Construct`, follow-up `006D6200`. `__fastcall MarkerManager* (MarkerManager*)`,
`RET 0`, returns `this`.

| Offset | Value |
| --- | --- |
| `+0h` | vtable `00CF8FF8`; slot 0 is `006DEDC0`, `__thiscall(this, int flags)`, `RET 4`, which runs the real destructor `006DEAC0` and frees through `00BF65AC` when bit 0 is set |
| `+4h`, `+8h`, `+Ch` | 0 |
| `+10h` | byte 0 |
| `+14h`, `+20h`, `+2Ch`, `+38h`, `+44h` | five `std::list` members, 12 bytes each |

Each list is `{_Myfirstiter, _Myhead, _Mysize}`. The constructor leaves `_Myfirstiter` alone (a
release-build dead field), sets `_Myhead` to a self-linked sentinel and `_Mysize` to 0. The
sentinel allocators differ per value type: `006D8590` for the first list (node byte `+1Dh` = 1),
`006D85E0` for the second (`+25h` = 1), `006D8660` for the last three (`+11h` = 1). The head node
is linked to itself through `node+0h`, `node+4h` and `node+8h`.

The five offsets are exactly the five walks of the per-frame update `006DC1A0`
(`docs/GAME_WORLD_ENTITIES.md`), which `include/bsp/world_entities.hpp` already names as
`kMarkerGroupListOffset`, `kMarkerSecondGroupListOffset`, `kMarkerHighlightListOffset`,
`kMarkerScaledListOffset` and `kMarkerTintedListOffset`. `44h + 0Ch = 50h`, so the allocation is
fully accounted for and the constructor and the reader agree field for field.

## Teardown pairing

`004D2BB0` `BSP_Game_DestroyWorld`, `__thiscall void (GGame*)`, body `004D2BB0-004D303F`, sole
caller `004DA780`. Neither destructor is reachable except through its vtable: the only xref to
`004CB0B0` is the data reference from `00CE7784`, and the only xref to `006DEDC0` is from
`00CF8FF8`. Each slot is released as `p->vtable[0](1)` and then nulled.

The pairing is unambiguous at the ends of the two routines. `004DE610` closes with
`004BEC00(1Dh, 1)` then `00A933F0`; `004D2BB0` opens with `004BEC00(1Dh, 0)` then `00A933F0`.

Release order: `game+19E8h` (the ocean owner, first), `+21D0h`, `+21D4h`, a loop over `+21A4h`,
`+21A8h` and `+21ACh`, `+21C4h`, `+21C8h`, `+21CCh`, `+21DCh`, `+21E0h`, `+21E4h`, `+21E8h`, then
`+19CCh`, `+19D8h`, `+19E4h`, `+19F0h`, `+19F4h`, `+19F8h` and `+19ECh`.

Two asymmetries with the construction:

1. The loop counter is 3, not 8:
   ```
   004d2c95: lea edi, [esi + 0x21a4]
   004d2c9f: mov ebp, 3
   004d2cb4: add edi, 4
   004d2cb7: sub ebp, 1
   004d2cba: jne 0x4d2ca4
   ```
   `004DF911` fills eight slots, `+21A4h..+21C0h`. Five are never released here. Either
   `008DA160` transfers ownership to whatever it registers them with, or this leaks per mission.
   Not resolved: `008DA160` was not read (it is outside this packet).
2. `game+19FCh`, the Operator node, is not released anywhere in `004D2BB0`. `game+1A00h` needs no
   release, because `004DE610` already dropped its own reference after `00B71990` handed it to the
   node, but the node itself survives the teardown.

## Ghidra artefacts encountered

- The decompiler aliases the allocation sizes into stack slots (`piStack_ac`, `local_88`), so every
  size above comes from the listing.
- `004DF846` is reported as an unreachable block ("Removing unreachable block"). It is the `memcpy`
  of the `Ocean initialization failed` literal, and it is reached normally in the listing.
- The push-before-accessor idiom (`push X; call getter; mov ecx,eax; call setter`) is rendered as
  `getter(X)` in the pseudocode. Every ocean and cloud setter in this routine uses it.
- `ghidra flow 004DE610` reports 1370 instructions and **no** gaps.
- `ghidra flow 004D2BB0` reports **one** gap: `004D2D7B..004D2D84` (9 bytes) after the `CALL
  0x00BF65AC` at `004D2D76`. Reported, not repaired; Ghidra was read-only for this packet.
- `004CB030` still carries the tag `cg_array_ctor_helper` and the placeholder name
  `CG_array_ctor_helper_004cb030`. It is not an array-constructor helper; it is the world
  constructor, which happens to call `eh_vector_constructor_iterator` once.
- `00B71930` carries the placeholder name `CG_static_dtor_stub_00b71930`. It is the scene-node
  class allocator: `004DE73F` sets `ECX = 458h` (the size) before calling it. Not claimed by this
  packet.

## Uncertainties

1. Whether the five undestroyed objects at `game+21B0h..+21C0h` are owned by whatever `008DA160`
   registers them with, or leaked. `008DA160` and `008DF900` were not read.
2. The meaning of `00B6FE50(node, 0000506Eh)` at `004DE886` and of the second call at `004DF7A9`
   with the accumulated dword from the atmosphere block. The bytes are assembled one at a time
   (`6E`, `50`, `00`, `00`), which reads like a packed colour or a pair of 8-bit indices.
3. `game+19F4h` is released by the teardown but is never written by `004DE610`. Its producer was
   not traced; `004DCDF0` and `004C9EC0` are the candidates.
4. What `[00F8D394]+18h = 20000000h` under `game+719Dh` selects. The value looks like a byte
   budget, but no reader was traced.
5. Whether `game+19FCh` surviving `004D2BB0` is deliberate (the node is resident across missions)
   or a leak.
6. The index constant `[00F889D4]` at `004DE8CF` selects one of three floats into `node+4Ch`. Its
   range was not bounded, so a value above 2 would read past the three-element stack array.

## What remains, and follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `game_world_destroy` | 004d2bb0 004da780 008da160 008df900 | docs/GAME_WORLD_DESTROY.md, include/bsp/world_destroy.hpp | The mission teardown in full, and whether the five channel objects `004DF911` creates and `004D2BB0` skips are owned elsewhere; includes the `004D2D7B` flow gap |
| `game_scene_lighting_setup` | 004c9ec0 004dcdf0 00a8fd30 00b7bdf0 00b7aab0 | docs/GAME_SCENE_LIGHTING_SETUP.md | The directional light at `game+19F8h`: what `TestDirectionalLight` and `AllLights` name, and who writes `game+19F4h` |
| `world_shore_handles` | 00871ba0 00870cd0 00440580 004845d0 008685e0 00868420 | docs/WORLD_SHORE_HANDLES.md | The five embedded handles at `game+19D4h..+19E4h`: the two assign helpers, the request builders and the terrain extent scaling through `004B4D80` |
| `world_parameter_table` | 00b1b830 00b1b890 00b1b400 004de4b0 | docs/WORLD_PARAMETER_TABLE.md | The parameter singleton `00F8D434`: the key/value setter, the record-valued variant and the object `00B1B400` returns |
| `scene_node_allocator` | 00b71930 00b724e0 00b71a80 00b71990 00b71940 | docs/SCENE_NODE_ALLOCATOR.md | The node class allocator that takes its size in ECX, and the four node constructors and attach points `004DE610` uses |

## Reconstruction

`include/bsp/world_construct.hpp` and `src/world_construct.cpp`. The two object layouts are
structs with the offsets above; the construction order and the record/default branch are a
sequence routine over an injected `WorldConstructHost` with one method per native call site, in
the style of `bsp::run_application_frame`. The scene-graph, terrain, ocean-render, lighting,
atmosphere and resource internals stay behind host methods: they belong to other packets and are
contracts here, not behaviour. `bsp::ocean_failure_is_reported()` records the finding above.

The marker-manager list offsets are taken from `include/bsp/world_entities.hpp` rather than
restated, and the scene-record offsets this routine alone reads are added next to the ones
`include/bsp/mission_scene_load.hpp` already names.

One focused case was added to `tests/math_tests.cpp`: it runs the sequence with and without a
scene record and asserts that both produce an ocean owner, that the dead literal is built only on
the record path, and that nothing reports a failure.

## State reached

| Routine | State |
| --- | --- |
| `004DE610` `BSP_Game_ConstructWorld` | analyzed in full, reconstructed, build-tested |
| `004CB030` `BSP_World_Construct` | analyzed in full, reconstructed, build-tested |
| `004CB0B0` `BSP_World_ScalarDeletingDestructor` | analyzed (vtable slot and call site only) |
| `006DECA0` `BSP_MarkerManager_Construct` | analyzed in full, reconstructed, build-tested |
| `006DEDC0` `BSP_MarkerManager_ScalarDeletingDestructor` | analyzed in full |
| `004D2BB0` `BSP_Game_DestroyWorld` | analyzed for the teardown pairing only, not reconstructed |

Nothing here is fixture-tested, ABI-compatible or game-validated. The reconstruction exposes a new
C++ interface; it is not a drop-in binary replacement.

## Correction from docs/UNIT_WORLD_REGISTRATION_LIVE.md

The vector callbacks in the world layout table above were reversed. The native call at004CB076 receives constructor004B7EC0 (pushed at004CB065) and destructor004C2D30 (pushed at004CB060). The complete004B7EC0..004B7ECC body zeroes count, head and tail. The count97, stride12 and world+18 destination remain correct. The source header has been corrected; the original table remains as historical evidence.

