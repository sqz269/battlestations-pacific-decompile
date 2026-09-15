# Native 20h post-effect construction (BR)

Address `00B4E470..00B4E83D`: 974 bytes, 325 saved instructions, no listing gaps.
`construct_native_post_effect20_00b4e470` covers the complete normal body and the
logical twelve-state cleanup schedule in the explicit provider domains below.
`BSP_PostEffect20h_Construct` is a descriptive hypothesis. This packet proposes
one native body; it does not add ledger credit or claim a native/source fixture,
binary ABI, FH3, GPU, renderer-startup or game validation.

The branch starts at `7d9a680444e0b5cb6406148b6dec51c232b02928`, including the
20h lifetime provider, BN draw record, BP raw model name/destruction, BO companion
pattern and BQ raw string overloads. Existing providers are reused and pinned in
the report. The BO source is read-only and retains its independent 24h contract.

## Native ABI and field schedule

Native ECX is the actual 20h/32-byte allocation. Three stack DWORDs are an actual
8-byte effect-name header, a vertex-count word, and an optional size input. ESI
captures ECX at `B4E48B`; EBX is zero from `B4E48D`; EBP/EDI capture successive
local creators. The original second-argument slot is reused only after its last
section-field read. EAX returns captured ESI; `B4E83B RET 0Ch` removes all three
words. The new C++ interface adds an allocation extent and an explicit prepared
host block; it is not a drop-in native signature.

| Native sites | Actual effects |
| --- | --- |
| B4E493, B4E49E, B4E4AF | CEB130 base table; placement construction of the actual `atomic<int32_t>` at +4 with **1**, once; D61EC0 table. No count0 or later reset. |
| B4E4B5..4BE | +0C, +10, +14 = 0, in order; original DWORD to +1C. +08/+18 preimages remain untouched until their later publication. |
| B4E4C1..4ED | CRT 40h frame allocation, B1FBB0 if nonnull, state disarm, publish +08 (including native null result). |
| B4E4F8..56A | Raw string length 17 and `pf44uf42uf41.mvfm`; current renderer+38 declaration; consumed normal raw string return; fresh renderer+5C stream allocation **4**, flags1000h, captured declaration. |
| B4E572..5D2 | Release local declaration; raw BCh/C0h mesh and B73D70; B73BB0(mesh,0,captured stream), then release the local stream creator with a fresh CE2220 read. There is no stream field in this family. |
| B4E5D8..62F | Raw named material, publish +14, parameter owner=this with retain0. Raw section; +0C=0,+10=original DWORD,+14=0, then +08=(word==3?4:5),+18=(word==3?1:2). Fresh +14 material assignment, real layout rebuild, mesh section append/retain. |
| B4E639..6C3 | Canonical 184h/188h raw model, raw `PostEffectSysObj`, BP raw model constructor, publish +10. Temporary data is captured before mask1 clears. One current D7A260 FLD, fresh +10 read, FST then FSTP; B75170(model,0,captured mesh,two spills). |
| B4E6CD..732 | Canonical 458h/45Ch camera; raw BQ `PostEffectSysCam - ` prefix; raw camera constructor with preadmitted initial viewport; publish +0C. Normal inline name return does not clear mask2. |
| B4E739..7B7 | CRT 34h replacement viewport; optional current +20 height getter then fresh +1C width getter; B1F940(width,height). Fresh +0C camera receives viewport; decrement creator with another current CE2220 read. |
| B4E7BB..7F5 | CRT 28h/40-byte raw draw record, BN B51BD0; FLD1, fresh model, FSTP visibility, fresh camera, FLDZ/FSTP leading0; captured section/mesh. Publish **+18**, with no counter ownership invented. BN preserves record +20/+24. |
| B4E7FF..825 | Capture CE2220 once, release captured section then captured mesh with the same function value. Each zero result resolves its current profile/slot and canonical companion; BD30E0 adds no decrement. |

All 27 observed calls across B107F0, B4F560, B51090, B540B0, B542D0, B544F0,
B546F0, B54940 and B54F90 pass count3 and optional0. Immediate PUSH0 or complete
register-writer scans prove this: EBP is zero at each relevant call; B4F560's EBX
is zero after its two-iteration SUB/JNZ loop. The implementation nevertheless
preserves all DWORD values and the full optional branch. Its nonnull domain is
current D619A0 with exact B3CD20/B3CD10 projections, +20 then +1C; both leaves were
already reconstructed by BO and receive no new credit. Other profiles and a
concrete nonnull native caller are unclosed. D61948 texture +1C/+20 is not used.

## Cleanup and host lifetime

CBFBAB..CBFBB4 loads FuncInfo DF86D4 and jumps BF6B43. FuncInfo maxState12 points
at DF86F8. The primary separately defined/saved this ten-byte handler, preserving
its default name. The worker made no Ghidra edits. The raw closure
CBFB30..CBFBAA is 123 bytes; three saved raw-free bodies omit their final POP/RET,
which are included in the disk/live evidence and in the following bounds.

| State | Next | Inclusive funclet | Action |
| --- | --- | --- | --- |
| 0 | -1 | CBFB30..37 | BD30F0 base table only |
| 1 | 0 | CBFB38..42 | CRT free captured raw40h frame |
| 2 | 0 | CBFB43..4A | 41DD20 declaration header at original frame-2C |
| 3 | 0 | CBFB4B..52 | B72F70 raw mesh at frame-34 |
| 4 | 0 | CBFB53..5A | B748C0 raw model at original frame+8 |
| 5 | 4 | CBFB5B..73 | Clear mask1, 41DD20 model header frame-24 |
| 6 | 0 | same CBFB5B..73 | Same conditional string action |
| 7 | 0 | CBFB74..7B | B71350 raw camera at frame+8 |
| 8 | 7 | CBFB7C..94 | Clear mask2, 41DD20 camera header frame-1C |
| 9 | 0 | same CBFB7C..94 | Same conditional string action |
| 10 | 0 | CBFB95..9F | CRT free failed raw34h viewport |
| 11 | 0 | CBFBA0..AA | CRT free failed raw28h draw record |

States6/9 have no normal assignment. Every transition, local release and return
is consumed before calling its action. Completed frame/mesh/material/section/
stream/model/camera creators are not reclaimed after later failures. Raw pool
slots are returned only when their native constructor did not complete. The
captured helper fields and surviving native ownership remain separate from host
metadata ownership; the source does not add compensating releases.

`NativePostEffect20ConstructionBlock` must be externally owned, immovable and
prepared before native effects. Fixed optional companions, scene/lifetime
admissions and **two** viewport records persist through failed attempts. Canonical
registration can allocate metadata; its transactional implementation must not
mutate native fields or publication cells. Borrowed completed-reference handles
are exposed before potentially throwing registration, so an unregistered child
can be disposed through its exact companion under a valid external ownership
plan. The block does not perform that disposal or invent orphan recovery.

The HOST-only `native_completed` marker is set after both final native releases
and before the 20h owner/reference bind. Later host diagnostics must not trigger
raw receiver free or base cleanup. The final owner uses the actual existing +4
atomic, registers without retain, and reaches B4E1F0/B4E430 lifetime: material,
nodes, raw draw, frame, base. Retire callbacks use captured keys and never read
ended allocation storage. Reset requires retired references, dead owners,
nonlive viewport records and external quiescence; it performs no native cleanup.
A failed camera can leave its completed first viewport live after raw camera
return; this remains an explicit external disposition boundary.

The ordinary C++ unwind policy continues remaining consumed-state actions after
a cleanup throws and propagates the newest cleanup exception. A second cleanup
failure can replace the first; this is deliberate source behavior, not native
FH3 double-exception equivalence. BQ nested assignment/concat/prefix has its own
documented consumed-action policy. Hardware faults, private-frame aliases and
compiler EH-state timing are excluded. Existing terminal interfaces are noexcept;
this does not prove native exception parity.

## Admission boundaries and verification

Every creator is an existing concrete pool/factory; GUI create_mesh/material/
section wrappers with rollback-on-binding-failure are not substituted. Canonical
pool/static wrapper identity, actual renderer/CE2220 publication cells, shared
CE4970/CE4ADC/D7A24C constants, D7A260 sentinel, table mappings, real layout and
draw-entry services must be supplied by the application. No synthetic manager,
fake callable or default pointer is introduced.

535320 still requires a real callable current renderer+48 provider in D5F0A8;
renderer construction/binding and service initialization are not closed by BR.
That factory may complete a material and then fail in its final effect release
without exposing the material identity. BR cannot identify or reclaim that
internal creator. B317E0/cache still uses its existing typed nonthrowing string
return domain; material/layout/node/camera/stream/BN providers retain their own
admission and materialization constraints. BP raw model and BQ raw prefix remove
the old nested model/prefix adapters, without proving injected cleanup failures.

The complete call rows, original ABI contracts, all caller argument sites,
source/provider hashes, original PE/live spans, build outcome and generated-code
evidence are in the report. No new tests are added. The worker build exercises
the checks actually configured in this isolated worktree. It does not execute
this constructor, establish full native FH3, prove a binary replacement, or
validate the runnable game. The renderer-service +70 producer/callable wiring
and failed-initialization preimage remain named integration work.
