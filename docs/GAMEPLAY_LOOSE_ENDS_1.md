# Five gameplay loose ends (packet `cc2_loose_ends_1`)

Addresses: 007b9590, 007bcaa0, 0071c4a0, 008759b0, 008797b0, 006e1860, 009277f0, 00810dd0,
00810f60, 0071c470, 00864660, 008367d0, 008364f0, 004b79f0, 004cb030, 004de610, 004b7ec0,
00484540, 006fe620, 007f10b0, 00956c20, 009f9be0, 009f9bf0, 0091bda0, 0091d640, 00916980,
006b9450, 004bb440, 004b5670, 004b5680, 004d6ba0, 004ddb90, 0073d410.

Ghidra was **read-only** for this packet. Every descriptive name here is a hypothesis, not a
recovered symbol. Each hunt began with a linear Capstone sweep of the shipped `.text` on disk
(`local/scan_disp.py`, `local/scan_regex.py`), because `ghidra xrefs` under-reports the data
references and immediates these questions turn on; direct call sites were found by scanning for
`E8`/`E9` rel32 encodings (`local/find_calls.py`), which is the only way to see calls to an
address Ghidra has no function for.

## 1. The three unit-owned pointers: two are released, one leaks

`docs/UNIT_DESTRUCTOR_LEVELS.md`'s headline says the weapon director at `unit+738h`, the gunnery
pass at `unit+6DCh` and the `1ACh` damage-state instance at `unit+360h` are "never released by
any level" and that whether something outside the release owns them is `contract: unread`. The
release chain reading is right and this packet does not touch it. What is outside the chain:

| pointer | verdict | released by |
| --- | --- | --- |
| `unit+6DCh` gunnery pass | **released**, asynchronously | `007B95B5` / `007BCB0B` retire it, then `008759B0` deletes it on the node's next tick |
| `unit+360h` damage state | **released for eleven unit classes, not for the destroyer** | `008797E1`..`008797F4` |
| `unit+738h` weapon director | **leaks** | nothing |

### (a) `unit+6DCh`: retire-then-sweep, not a destructor

Neither release site frees the object itself. Both read the field, make one virtual call, and
drop the pointer:

| site | containing function | code |
| --- | --- | --- |
| `007B95B5`..`007B95C6` | `FUN_007b9590`, body `007B9590-007B95D7` | `ECX = [ESI+6DCh]`; if non-null `[[ECX]+10h]()`; `[ESI+6DCh] = 0` |
| `007BCB0B`..`007BCB1C` | the function at `007BCAA0`, body `007BCAA0-007BCB27`, which Ghidra has no function for | the same three steps |

The virtual is slot `+10h`, and for the gunnery pass class (`00D0D360`) that slot holds the
gun-bot base stub `0071C4A0`, whose whole body is

```
0071c4a0  MOV byte ptr [ECX + 0x10],1
0071c4a4  MOV byte ptr [ECX + 0x11],0
0071c4a8  RET
```

so the call frees nothing; it sets a retire flag on the sub-node. `docs/GUN_BOT_TICKS.md` lists
slot `+10h` as a stub, which is true of the body and wrong about the effect.

The free happens on the unit's tick node, by a mechanism `docs/FIXED_STEP_JOB_WAVES.md` already
established for sub-nodes in general: the ledger record for `008759B0` from packet
`cc_fixed_step` reads "one whose `+10h` byte is set is unlinked under the critical section from
`00875280`, decrements the count at `element+24h` and is destroyed through `sub->vtable[0](1)`".
What is new here is the link from `unit+6DCh` to that sweeper, and that the `+10h` byte is set by
a virtual the unit itself calls on the way down.

`00810E46`..`00810E62` in `FUN_00810dd0` stores the
new pass at `unit+6DCh` and immediately calls its slot `+4h` with `LEA ECX,[ESI+0x310]`, the
unit's tick element node (`docs/TICK_ELEMENT_OVERRIDES.md`); `00864BD0
BSP_UnitGunneryAi_AttachToTickElement` forwards to `008FBC80 BSP_GunBot_Attach` and `00876020`,
which links the object into the node's sub-list. `008759B0 BSP_TickElement_RunSubNodes` walks
that sub-list every step with `ESI` = the sub-node and `EBP` = the node:

| site | step |
| --- | --- |
| `008759B6` | `ESI = [node+1Ch]`, the sub-list head |
| `008759C4` | `[ESI+11h] != 0` gates the tick at slot `+0Ch` |
| `008759E1` | `[ESI+10h] != 0` (the flag `0071C4A0` set) falls into the removal path; `0` jumps to `00875A65` and continues |
| `008759FD`..`00875A3C` | unlink: `[ESI+8h]` is prev and `[ESI+0Ch]` next, the head is `node+1Ch`, the tail `node+20h` and the count `node+24h`, decremented at `00875A3C` |
| `00875A47`..`00875A4C` | the node's payload refcount is decremented and released through `[00CE2210]` |
| `00875A52`..`00875A5A` | `MOV EAX,[ESI]` / `MOV EDX,[EAX]` / `PUSH 1` / `MOV ECX,ESI` / `CALL EDX` — slot 0 with the MSVC deleting flag |

For the gunnery pass slot 0 is `00864660`, which calls the class body `00864380` and then
`operator delete` at `00BF65AC`, freeing the `558h` bytes `00810E1E` allocated.

**Rule.** A unit sub-node of the gun-bot family is never deleted by the unit; it is retired with
`subnode->vtable[10h]()` and deleted by the owning tick node's next `008759B0` pass. So the unit
destructor levels legitimately never mention `+6DCh`: by the time they run the field is already
`0`. The one thing this leaves open is the window: if a unit is destructed before its node's next
sub-node pass, the retired object is still on the sub-list and the unit's destructor chain was not
read here for a sub-list teardown. `contract: unread` on that window only.

`unit+DF4h` is a second pointer of the same shape, retired and dropped by the same two sites with
the same slot `+10h` call. Its class was not read.

### (b) `unit+360h`: released at vtable slot `+34Ch`, except on the destroyer

`FUN_008797b0` (body `008797B0-00879805`) ends with a real deleting destructor call:

```
008797e1  MOV ECX,dword ptr [ESI + 0x360]
008797e7  TEST ECX,ECX
008797e9  JZ 0x008797fe
008797eb  MOV EAX,dword ptr [ECX]
008797ed  MOV EDX,dword ptr [EAX + 0x4]
008797f0  PUSH 1
008797f2  CALL EDX
008797f4  MOV dword ptr [ESI + 0x360],0
```

The slot is `+4h`, not `0`, because that is where this class family keeps its scalar deleting
destructor: the instance's final vptr is `00CFD7B8` (installed at `007135FA`, over the base
`00CE89E8` from `007135E2`), and `00CFD7B8[4h]` is `00712FD0` while slot 0 is `004E6560` in both
tables.

`FUN_008797b0` is installed in eleven vtables (`00CF972C`, `00CF999C`, `00CFAB3C`, `00CFBDA4`,
`00CFBFDC`, `00CFC214`, `00CFDCDC`, `00CFE12C`, `00CFE38C`, `00CFE5CC`, `00D0DFF4`) and is also
tail-called directly from `FUN_0095d400` at `0095D487`. The slot is `+34Ch`: `00CFC710` and
`00CF9720` agree entry for entry at `+340h` (`0042B950`), `+348h` (`00779AF0`), `+350h`
(`00431410`) and `+354h` (`0042B9A0`), which fixes `00CF9720` at offset `+340h` of its table, and
`FUN_008797b0` sits four bytes later.

The destroyer's level-6 table installs `FUN_006E1860` at that slot instead (`00CFC3D0+34Ch`).
`FUN_006e1860` (body `006E1860-006E1925`) releases `+464h` the same way, and `+334h` and `+338h`
through an `[00CE2220]` refcount decrement, and **never touches `+360h`**. Both bodies tail-jump
to `009277F0`, a one-instruction thunk to `00923060`, and none of the three offsets `360h`, `6DCh`
or `738h` occurs anywhere in `00923060` (they are absent from the complete `.text` sweeps below).

**Rule.** The `1ACh` damage-state instance is released by the unit class's own slot `+34Ch`
implementation, so eleven unit classes free it and the destroyer's override does not. This
contradicts the "never released by any level" claim for `+360h`.

### (c) `unit+738h`: nothing releases it

Complete sweep of `.text` for every instruction mentioning `0x738`: 88 memory operands, 16 with
the memory operand as destination. `00810FA9` is the only store of a pointer into `unit+738h`
(`0083ED4E`, the other `MOV dword ptr [ESI+0x738],EAX`, is inside
`BSP_GameSettings_LoadFromLuaGlobals`, body `0083B5E0-00842951`, a different object). No site
anywhere nulls the field, and no window around any of the 88 sites contains a `PUSH 1` before an
indirect call or a call through slot `+4h`/`+10h`.

The director is shaped like a tick sub-node and is never enrolled as one. Its derived vtable
`00D09F58` holds the attach at slot `+4h` (`0071C470`: `PUSH ECX; MOV ECX,[ESP+8]; CALL
0x00876020; RET 4`, i.e. `00876020(node, this)`) and the retire stub at slot `+10h`
(`0071C4A0`), the same pair the gunnery pass uses. But `FUN_00810f60`, which allocates the `250h`
bytes at `00810F85`, constructs at `00810FA0` and stores at `00810FA9`, never calls either: the
rest of its body (`00810FAF`-`008110CF`) only reads `[ESI+538h]` and computes floats. So the
object never enters a node's sub-list, `008759B0` never sees it, and its deleting destructors
`008367D0` (derived) and `008364F0` (base) exist only as vtable entries with no direct call site.

**Rule.** The shipped game leaks one weapon director per unit.

## 2. `[[game+19CCh]+13Ch]` is the class-id-24 unit list, and plane squadrons are what is on it

`docs/SENSOR_TABLES.md` §8 and `docs/RECON_SLOT_LISTS.md` §10 leave this as an unidentified
"world list" whose head has no writer. It is not a separate list. The world registry's per-class
lists are `{count, head, tail}` triples at `registry + 18h + id*0Ch`, and

```
0x13C = 0x18 + 24*0xC + 4
```

so `registry+13Ch` is the **head of the list for class id 24 (`18h`)**, `registry+138h` its count
and `registry+140h` its tail. The reader's own step 5 uses the same array four instructions later:
`008074C7 LEA EAX,[EBX + EBX*0x2]` then `008074CA MOV EBP,dword ptr [EDX + EAX*0x4 + 0x1C]`, i.e.
`*(registry + 1Ch + id*0Ch)`, which is the same `+4h` head dword.

The array is 97 entries of 12 bytes, built by the vector constructor iterator at `004CB076`
(`PUSH 0x4C2D30` dtor, `PUSH 0x4B7EC0` ctor, `PUSH 0x61`, `PUSH 0xC`, `LEA EAX,[ESI+0x18]`) inside
`004CB030 BSP_World_Construct`; the element constructor `004B7EC0` zeroes exactly three dwords.
The registry block itself is `4BCh` bytes, allocated at `004DE656` and `memset` to zero at
`004DE664` before the constructor runs, so every head starts at `0`.

### The producer

The list primitive is `00484540 BSP_UnitList_PushBack`, `__thiscall void(List*, void* value)`,
`RET 4`: `operator new(0Ch)` node with `+0h` prev, `+4h` next and `+8h` value; on the empty branch
(`00484586`) it writes the head at `list+4h`, otherwise (`00484572`) it chains from the tail at
`list+8h`; both branches set the tail and `ADD dword ptr [ESI],0x1`, the count at `list+0h`.

The caller is an entity virtual at **slot `+130h`**, "register in the world's class-id lists":

| implementation | vtable slot | lists it joins |
| --- | --- | --- |
| `006FE620 BSP_UnitInstance_RegisterInWorldLists` | `00CFC3D0+130h` | ids 2, 4, 5, 6, 7 (`ADD ECX,0x30/0x48/0x54/0x60/0x6C` at `006FE62C`..`006FE65C`) |
| `FUN_007f10b0`, body `007F10B0-007F10D4` | `00D087C0+130h`, the plane squadron's primary vptr installed at `007F2CAD` | ids 2 and **24** (`ADD ECX,0x30` at `007F10BC`, `ADD ECX,0x138` at `007F10C8`) |

Both first call `00928560 BSP_GameEntity_RegisterInParentEntityList` and both reach the registry
as `[this+30h]`.

**Rule.** `[[game+19CCh]+13Ch]` holds the **plane squadrons**, pushed by the squadron's own
`+130h` override when it registers with the world, and step 4 of `BSP_Recon_RebuildSlotLists`
resets one detection record per squadron, not per unit. `00807480 MOV EAX,[ESI+0x28]` /
`MOV ECX,[EDI+0x8]` / `LEA ECX,[ECX + EAX*0x34 + 0x1E8]` therefore indexes a squadron's record
block.

Why the earlier scan found nothing: the insert never uses a `+13Ch` or `+138h` displacement. It
loads the registry from `entity+30h` and reaches the triple with an `ADD ECX, 0x138` **immediate**,
which a displacement scan cannot see. The complete `.text` sweep for `0x13c` (534 instructions)
confirms the negative it was meant to test: no instruction anywhere writes `registry+13Ch`
directly, and the only function that takes the address of a `+13Ch` field outside a constructor is
`004B79F0` (`LEA EAX,[ECX+0x13C]; RET`), which is dead code — no absolute reference in the image
and no `E8`/`E9` call site in `.text`.

## 3. The blast accumulator at `unit+460h + cat*4` has no live reader

`00956C20` sizes the array itself: the loop at `00956D59`..`00956EC3` runs `ESI = 0..0Bh`
(`00956EC0 CMP ESI,0xC`, `JL`), so there are **twelve** categories, and `unit+430h + cat*4`
(the per-category weapon range of `docs/GUNNERY_TABLES.md`) and `unit+460h + cat*4` are parallel
twelve-float arrays.

Every indexed access to the accumulator in `.text`:

| site | what |
| --- | --- |
| `00956D6C` | `MOVSS [EDI+ESI*4+0x460],XMM2`, the zero at the top of the pass |
| `00956E94`, `00956E9B` | the `FADD`/`FSTP` accumulation |
| `009F9BE4` | `FLD dword ptr [ECX+EAX*4+0x460]` in the accessor at `009F9BE0` |

`009F9BE0` is `__thiscall float(Unit*, int category)` — `MOV EAX,[ESP+4]; FLD [ECX+EAX*4+0x460];
RET 4` — and `009F9BF0` is `__thiscall float(Unit*)` returning `[this+474h] + [this+464h] +
[this+478h]`, i.e. categories 5 + 1 + 6. **Both are unreferenced**: zero absolute occurrences
anywhere in the image and zero `E8`/`E9` call sites in `.text`, so the compiler inlined every
call and the out-of-line copies are dead.

The sibling range array is the control: `unit+430h + cat*4` has two live indexed readers outside
`00956C20`, at `00863A34` and `0095EBC4`, and the gunnery triples at `unit+398h + cat*0Ch` have
two at `009520CC` and `0095212A`. The accumulator has none.

**Coverage: partial.** The indexed arm is complete. A live reader that inlined a *constant*
category would appear as a fixed displacement in `0x460`..`0x48C`, and that block is a common
float group in dozens of unrelated classes (roughly 200 float accesses across the twelve offsets,
most of them in `004F`-`0060` GUI code and in the gun and bomb classes); those were not resolved
per site. No function that reads `+430h + cat*4` or `+398h + cat*0Ch` also touches `+460h`.

## 4. The `entity -> int` map at `manager+1488h` is never inserted into, and `+44h` is `Params`

### (a) The map

Complete `.text` sweep for `0x1488`: nine instructions. Four compute the manager's map address,
two belong to an unrelated class (`FUN_008cb590` at `008CB59D`, and `008CC44E` inside
`FUN_008cc330`, which passes it to `0063F1E0`), and three are stack frames in library code.
The four are:

| site | containing function | what |
| --- | --- | --- |
| `0091D712` | `CG_array_ctor_helper_0091d640`, body `0091D640-0091D752` | construction: `map+4h = 0063B110()`, the head node, sentinel `+11h = 0` and parent/left/right pointed at itself |
| `009169B1` | `BSP_MissionPlayerRecords_Reset`, body `00916980-009169F5` | an inlined `std::map::clear`: walk from `[map+4h]->+4h`, `0063AD70` then `operator delete` per node, then reset the head links and `[map+8h] = 0` |
| `0091CCFF` | `CG_vector_deleting_dtor_0091ccd0`, body `0091CCD0-0091CD25` | destruction |
| `0091BE18` | `0091BDA0` | the `find` of `docs/KILL_CREDIT.md` |

**Rule.** The map is constructed, cleared and destroyed, and only ever read. Nothing in `.text`
inserts into it, so `_Mysize` at `map+8h` is `0` for the life of the process, the `0090C170` `find`
at `0091BE29`/`0091BE3F` always misses, and the early return it guards never fires.

### (b) `+44h` really is `Params`

Settled from the producer, which is what rule 4 of `docs/WORKER_VERIFICATION_CHECKLIST.md` asks
for. In `FUN_006b9450` (body `006B9450-006B9F4D`) the row is `ESI`, and the loop
`006B9BA0`..`006B9C49` runs `EDI = 1` (`006B9B92`) while `EDI < 16h` (`006B9C46`):

| site | step |
| --- | --- |
| `006B9BA0` | `PUSH 0xCF8530` — the literal string **`Params`** — and `00B67800 BSP_LuaObject_GetByName` |
| `006B9BC6` | `00B67720 BSP_LuaObject_GetByIndex(dest, EDI)`, so the column is a Lua array |
| `006B9C0C`, `006B9BDB` | validity checks (`00B65FB0`, `00B67690`) |
| `006B9C1D` | `00B66380 BSP_LuaReference_GetIntegerOrDefault` |
| `006B9C36` | when index 1 is absent, `0` is substituted instead |
| `006B9C3B`, `006B9C3E` | `LEA ECX,[ESI+0x44]` then `00442190`, a `push_back` |

So `row+44h` is a vector filled from `Params[1..21]` in order through Lua's one-based index, and
the consumer's `vector::at(0)` is the first `Params` entry. For comparison `row+3Ch` and `row+40h`
are single parsed columns, `+40h` from `XLastAchievement` (`00CF8538`).

## 5. `player+9h` has no writer, and the byte is `0` in every mission

Packet `cc2_unit_destructor_levels` already scanned `.text` for byte stores to `[reg+9h]` and
found none on a record, and already noted that `004B5680` has no `E8` call site. This hunt was
asked to check the three non-obvious writers it could not rule out. None of them exists, and the
zero now has a provenance rather than being assumed from "the record's zero-init".

The gate byte has a compiled setter and no surviving caller.

| address | ABI | body | status |
| --- | --- | --- | --- |
| `004B5670 BSP_ParticipantRecord_SetClaimed` | `__thiscall void(Record*, char)`, `RET 4` | `004B5670-004B5679` | `[this+8h] = arg`; **unreferenced** |
| `004B5680 BSP_ParticipantRecord_SetGateByte9` | `__thiscall void(Record*, char)`, `RET 4` | `004B5680-004B5689` | `[this+9h] = arg`; **unreferenced** |

Neither has an absolute occurrence anywhere in the image or an `E8`/`E9` call site in `.text`, so
every call was inlined and an inlined one would read `MOV byte ptr [reg+9], ...`. All fifty such
byte accesses below `00520000` were examined and none has a participant record as its base:

- `004D2E3F` and `004D2E85` are in `BSP_Game_DestroyWorld` (body `004D2BB0-004D303F`) on
  `[game+19D8h]`, a refcounted object released through `[00CE2220]`, not a record.
- `004DB143` (`= 0`) and `004DB17E` (`= 1`) are in `BSP_Game_TogglePauseMenu` (body
  `004DB030-004DB183`) on `[[00E198C4]+A8h]`. That is a different object: `00E198C4` is written at
  `0068CCFE` in the menu module with an instance of its own, and the game singleton is `00E188A8`.
  The `+8h`-then-`+9h` pair it tests is the same idiom `FUN_004bb770` uses at `004BB794`/`004BB79A`
  on a real record, which is why it reads like the record and is not.

The three non-obvious writers the packet asked about do not exist:

- **No whole-record copy.** The only `0x118` immediates paired with the record array are the two
  MSVC array iterators and the by-index accessors (`IMUL reg,reg,0x118` at `004671F4`, `004BB2B9`,
  `004BB554`, `004BB642`, `004BB674`, `004C6AB2`). There is no `REP MOVSD` over a record and no
  `memcpy(record, ..., 0x118)`.
- **No constructor write.** The record constructor is `004D6BA0`, run over the eight records by the
  vector constructor iterator at `004DDD6C` (`PUSH 0x4CB2F0` dtor, `PUSH 0x4D6BA0` ctor, `PUSH 8`,
  `PUSH 0x118`, `LEA ECX,[ESI+0x748]`); the destructor pass is the matching iterator at `004DD412`.
  Its body writes `+0h` (vptr `00CE7794`), `+10h`, `+14h`, `+1Ah`, `+28h`, `+2Ch`, `+38h`, `+3Ch`,
  `+40h`, `+44h`, `+48h`, `+4Ch`, `+50h`, `+88h`, `+89h`, `+8Ch`, `+90h`, `+94h` and on from `+98h`
  — **never `+8h` or `+9h`**.
- **No deserialization.** `004BB440 BSP_Game_ClaimParticipantRecord` writes `+Bh`, `+8h = 1`,
  `+19h = 0`, `+50h` and copies two strings to `+58h` and `+78h`; it does not write `+9h`.

The zero the claim loop at `004BB450` depends on comes from the session block, not from the record
constructor. `BSP_Application_Initialize` (body `0073D410-0073E52C`) allocates the game object and
clears it whole:

```
0073e150  PUSH 0x71a0
0073e155  CALL 0x00bf55be        ; allocate
0073e15a  PUSH 0x71a0
0073e15f  MOV ESI,EAX
0073e161  PUSH EDI
0073e162  PUSH ESI
0073e163  CALL 0x00bf79f0        ; _memset
```

`EDI` is zeroed once at `0073D433` and never written again anywhere in the function, so this is
`memset(game, 0, 0x71A0)`; `0073E141`..`0073E15F` was read from the disk bytes because the stored
listing has a gap there.

**Rule.** `player+9h` is zero-initialised with the game block, no code writes it, and it is `0`
for the whole process in single-player and in every other mode. Every reader of it
(`004BB79A`, `004BB7C1`, `004BB7E9`, `004BB811`, `004C6E6D`, `004C6E82`, `004C6E98`, `004C6EAE`,
`004B5516`, `004B5536`, `004D574A`, `004D581D`, `004D8932`, `004DB9E3`, `004DB9F4`, `004DBA06`,
`004DBA18`, `004E181F`) always sees `0`, so whatever second condition it was meant to express is
inert in the shipped build.

## 6. `no_ghidra_function`

| address | name | end address |
| --- | --- | --- |
| `004B79F0` | `world_class_list_head_getter_004b79f0` | `004B79F6` |
| `004B7EC0` | `world_class_list_entry_ctor_004b7ec0` | `004B7ECC` |
| `0071C4A0` | `gun_bot_retire_stub_0071c4a0` | `0071C4A8` |
| `007BCAA0` | `aircraft_deactivate_sibling_007bcaa0` | `007BCB27` |
| `009F9BE0` | `unit_blast_accumulator_get_009f9be0` | `009F9BED` |
| `009F9BF0` | `unit_blast_accumulator_sum_009f9bf0` | `009F9C0A` |

All six were read with `bsp.py disasm-raw` on the disk bytes. `FUN_007bca50`'s Ghidra body ends at
`007BCA98`, well before `007BCAA0`.

One Ghidra body is also **truncated**, which puts real code in no function at all:
`CG_vector_deleting_dtor_004dcf90`'s body stops at `004DD122`, but `004DD123` is `ADD ESP,4`, a
mid-stream continuation rather than a prologue, and the code runs unbroken to the `RET` at
`004DD5A6` with `int3` padding only from `004DD5A7`. The game destructor's whole tail therefore
lies outside any function, including the participant-record vector destructor iterator call at
`004DD412`, which is why that row of `reports/gameplay_loose_ends_1.json` carries no `native`
field. Ghidra was read-only here, so the body was not corrected.

## 7. Open questions

- The window in hunt 1(a): whether the unit's destructor chain tears down the tick node's
  sub-list, and so whether a unit destructed before its node's next `008759B0` pass leaks the
  retired gunnery pass after all.
- `unit+DF4h`, the second retired sub-node, and the class that owns the nine vtables holding
  `FUN_007b9590` (`00D000F0`, `00D00388`, `00D05FA0`, `00D066B8`, `00D069A0`, `00D0BB00`,
  `00D19DA8`, `00D1A080`, `00D1A358`). Their bases are not installed as vptrs at the offset the
  `+348h`/`+34Ch` alignment would predict, so the slot index of `FUN_007b9590` is **not** settled;
  what is settled is that it overrides the same virtual as `00779AF0`, because it tail-calls
  `00951FB0`, which clears `[ECX+4A4h]` and tail-calls `00779AF0
  BSP_MissionEntity_OnKilledPlayerUnit` with the same `this`.
- Whether the destroyer's missing `+360h` release is a shipped bug or whether `MDestroyer` never
  gets a damage-state instance. `0087BEA4` in `BSP_UnitInstance_InitHealthAndParts` (body
  `0087BCC0-0087BF73`) is level-3/4 code common to all units, which suggests it does.
- Which unit classes the eleven `FUN_008797b0` vtables are, and why `00D0DFF4` is among them.
- The fixed-displacement arm of hunt 3 (see the coverage note).
- What class id 24 is called in the game's own class table, and whether any entity other than the
  plane squadron installs a `+130h` override that joins it.
