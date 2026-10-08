# Native unit wake ownership readiness

Packet `cc12_native_unit_wake_owner_readiness` admits **zero new Source
implementations and zero ready owner/wiring packets**. The raw wake constructor,
plain destructor, fill and append APIs exist, but the current application has no
genuine writable 988-byte wake producer. Calling them on the existing semantic
trail or on a newly fabricated scratch block would not close that gap.

The decisive upstream boundary is `SceneUnitCreatorBinding` in
`src/game_hosts_scene_contents.cpp:889`: `create_instance_from_descriptor`
explicitly records native `006FE590` as unimplemented, sets the scene record's
`created` flag, and returns `&entity_`. Its existing whole compiled body is
40 bytes / 12 instructions: its sole call relocation targets
`GameHostLog::unimplemented`, followed by the flag store and record-pointer
return. It does not allocate or construct a native unit. The mission frame later
creates a separate `GameUnitSlot` through `GameUnitsHost::create_units`.

## Original enclosing contract

Fresh read-only exports verified project `bsp`, program
`/battlestationspacific.exe`. The entire `0081ED40..0081F354` constructor is
retained: **1,557 bytes / 277 instructions**, with every decoded instruction
start equal to the fresh Ghidra listing. `MOV ESI,ECX` at `0081ED63` preserves
the caller's full-unit receiver; ESI is unchanged until its epilogue restore.
The function constructs the base and intervening subobjects, then performs:

| Address | Original operation |
| --- | --- |
| `0081F02A` | `LEA ECX,[ESI+BD0h]` |
| `0081F030` | Store constructor EH state `14h` |
| `0081F035` | Store the preceding unit field `+BCCh` |
| `0081F03D` | Direct `CALL 00815600` |
| `0081F15A` | Store EH state `18h` before the next tracked subobject call |
| `0081F343` / `0081F352` | Return the same full-unit pointer in EAX / `RET 4` |

`0081F02A` is the receiver-address instruction; the call itself is `0081F03D`.
The constructor receives its enclosing allocation. Its explicit `new(2Ch)` at
`0081F200` creates a different side block stored at unit `+73Ch`, not the unit
or wake. The existing `006FE590` allocation model documents a zero-filled
`1188h` Destroyer allocation; that outer factory and the eight constructor
callers were not newly analyzed here. Do not generalize that allocation size
to every derived vehicle class.

The accepted raw wake layout occupies `[unit+BD0h, unit+FACh)`:

| State | Wake offset | Full-unit offset |
| --- | --- | --- |
| Base vptr / owned tracked section | `0` / `4` | `BD0h` / `BD4h` |
| Forty samples, stride `18h` | `8` | `BD8h` |
| Head / full-byte flag | `3C8h` / `3CCh` | `F98h` / `F9Ch` |
| Three live residual lanes | `3D0h..3D8h` | `FA0h..FA8h` |

Whole `00825F20..00826D6B` motion bytes and listing are also retained:
**3,660 bytes / 946 instructions**. This packet reviews its receiver provenance
and relevant prefix/tail, not whole motion behavior. Entry ECX is copied to EDI;
ESI becomes `EDI-310h`. The only later writes to those two registers are the
two return epilogues' POPs, including the early return at `00826638`. Thus
`00826CE8 LEA ECX,[EDI+8C0h]` followed by `00826CEE CALL 00810190` addresses
the same full-unit `+BD0h` wake. Its point argument is the actual unit `+FCh`
address; heading comes from the unit's `+50h` virtual slot. No complete motion
x87/ABI equivalence is claimed.

Existing lifetime Source evidence establishes the plain wake destructor's
owned-section release and lack of allocation-root free. Separately, historical
`docs/UNIT_DESTRUCTOR_LEVELS.md` records the parent destructor `0081F3A0`
inlining `+BD0h` vptr reset and `+BD4h` recursive leave/delete/free/clear at
`0081F56C..0081F5A4`. Its current Source is a semantic `UnitReleaseHost` plan,
with no application implementation or caller found. That older parent receipt
is not a fresh whole Original teardown proof. The constructor's SEH handler
`00C9141D` and its unwind table were not analyzed; the observed state stores
alone do not establish failure cleanup.

## Current persistent owners and consumers

`GameMissionFrameHost::Impl` genuinely owns `GameUnitsHost`, and its
`release_units` withdraws HUD, Lua, subsystem, AI and world borrowers before
`units.reset()`. `GameUnitsHost::Impl::slots` owns stable
`unique_ptr<GameUnitSlot>` instances. Each slot really owns its motion/pose
state and a semantic `ShipAiWakeTrail`. Those are persistent application
objects, but none is the raw embedded wake.

The slot's existing `NativeUnitObserverPrefixStorage` is a genuine `20h` prefix
with its own producer; it provides no allocation extending to `+BD0h`. The
current `GameUnitsHost` destructor tears down observer endpoints and then C++
member ownership. It has no wake tracked section to release.

Two existing models must not be overlaid:

| Current type | Actual Source layout | Why it cannot supply the raw wake |
| --- | --- | --- |
| `ShipAiWakeTrail` | Samples at `0`, head `3C0h`, flag `3C4h`, residual `3C8h`; four extra diagnostic counters | No native vptr/section prefix; existing compiled fill stores flag/head at these shifted offsets |
| `UnitPoseHistoryRing` | Five-float slots of `14h`, head `328h`, residual `330h`; 828 bytes under the configured layout | Existing compiled constructor increments by `14h` and stores residual at `330h/334h/338h`; the declared Native stride constant does not pad the C++ slots |

`create_unit_instance` and `construct_unit_instance_sub_objects_0081ed40` remain
unbound semantic host interfaces. Their abstract allocation, subobject and
critical-section callbacks do not constitute an application producer. The
`UnitReleaseHost` destructor plan has the same boundary.

All current game-host wake uses remain connected to the semantic trail:

| Use | Current `game_hosts_units.cpp` sites |
| --- | --- |
| Spawn fill / placement refill | `14937`, `31838` |
| Motion-tail append / fallback append | `8752`, `31463` |
| Leader handoff | `2785`, whole C++ trail assignment |
| Decomposition / distance sampling | `3456`, `32046`, `32231` |
| Direct sample/head and diagnostic reads | `32251`, `32262`, `34121` |

The existing whole `game_hosts_units.obj` contains seven semantic wake
relocations and **zero raw wake API relocations**. A Source/header search over
4,025 files finds the raw API declarations/definitions but no application
caller. That search is corroborating scoped evidence, not a proof that every
arbitrary unnamed allocation has been understood. Keeping a second semantic
trail synchronized by copies would introduce another authority and does not
qualify as wiring the real raw owner.

## Gate for further work

There is no ready Source owner/wiring packet to authorize from this evidence.
First establish an actual unit-storage producer and allocation-root lifetime
that reaches this subobject, or explicitly review a genuine persistent owner
contract against that Native producer. That work must close creator/class
eligibility, original preimage provenance, construction publication and
withdrawal, real section ownership, quiescent destruction, and allocation-root
free. Current uniform semantic trail initialization across game-host unit
kinds is not evidence that all those Original classes contain this raw wake.

A subsequent bounded readiness investigation can follow the real enclosing
factory/constructor `006FE590/006FE460`, parent destruction
`0081F3A0/00822700`, and constructor unwind metadata beginning at `00C9141D`.
These are dependency identifiers from existing records, not addresses claimed
or newly analyzed by this packet. The exact Source admission boundary is the
record return in `game_hosts_scene_contents.cpp`, its handoff through the
mission frame, and the independent slot creation in `game_hosts_units.cpp`.
No whole vehicle-class ABI implementation is proposed here.

Once a real producer is admitted, a separate packet must map every fill,
append, handoff, decomposition, sampling and diagnostic consumer to the same
live state, preserving the actual Native/Source interfaces and canonical
geometry context. A typed-trail cast, 988-byte scratch/reset array, public
callback owner, fabricated failure rollback, or bare leaf call does not
satisfy this gate.

## Evidence and validation boundary

The report is `reports/cc12_native_unit_wake_owner_readiness.json`. Ignored
evidence is retained in
`local/cc12_native_unit_wake_owner_readiness_evidence` and its sealed ZIP.
It includes whole Original parents; complete current Source inputs; seventeen
whole existing objects; fourteen unique matching core members in the complete
1,905-member archive; three directly linked application objects; and 646
complete selected COFF function extents, including ten application methods.
Configured compiler binaries and seventeen actual historical command/read
records with their currently available physical dependency files are pinned.

This is a read-only readiness audit. Existing object/archive evidence is not
a fresh build or proof that every current dependency byte was consumed by
those earlier compilations. No Source implementation, test, API invocation,
Original execution, Ghidra mutation, final EXE linkage proof, ABI compatibility
claim or gameplay claim was added. Existing last-test records are retained as
historical evidence only. Implementation and ready-Source counts remain zero.

## Primary acceptance

Root independently verified all2,747 retained artifacts /2,748 ZIP entries and CRCs, both whole installed Native parent spans, and all646 selected current complete extents in seventeen whole objects. Fourteen current whole core objects match unique current archive members. The actual40B/12-instruction factory has only the logger call; the three application objects contain no raw wake API relocations. This confirms the Source producer boundary while preserving the older compile-record qualification. Receipt: `local/cc12_native_unit_wake_owner_primary_review/receipt.json`, SHA256 `8e734c74b4e70514f149d1dc4585305856cf87488285f16b7ac50ed813c17b3a`.

No new Source, Original credit, build, entry execution, whole-class/lifetime ABI or gameplay admission is added. The next independent producer investigation can inspect006FE590/006FE460 and the actual application factory handoff; it cannot wire the semantic trail to the raw988B API.
