# Actual plane-squadron sorted insertion (cc11)

`native_plane_squadron_insert_sorted_007ed0d0` reconstructs the complete ordinary
`007ED0D0..007ED151` body through borrowed live fields. It also produces the SAME
plane+9D8h cell consumed by the existing actual leader provider and its Source
adopters. This is conditional Source reconstruction with a new C++ interface;
actual game objects, class profiles, callers and lifetime bindings remain unbound.

| Native routine | End exclusive | Bytes / instructions | Coverage / original convention |
| --- | --- | --- | --- |
| `007ED0D0` | `007ED151` | 129 / 42 | Complete normal body; ECX=squadron, stack(plane, signed index), RET 8 |
| Existing `007B8AD0` | `007B8ADC` | 12 / 4 | Reused unchanged Source provider; ECX=plane, full native EAX=0 or 1, RET |

The insertion body's disk/live SHA256 is
`25a888843fd7f3e017f78bfbd8429c4528096526da55f53b7805d4b184c420ee`.
There are zero native CALLs, absolute global references or table xrefs to this
entry. `007ED12B` jumps to `007ED130`, skipping the original three alignment bytes
`8D 49 00` at `007ED12D..007ED130`; this is no missing reachable continuation.
The original return value is unspecified scratch; the Source operation returns
void. Source leader returns bool, so it does not promise native full-EAX encoding.

The exact sequence is:

1. `007ED0D7` sets squadron+3ECh to 1, including for a null incoming plane.
   Null then returns without touching other represented fields or invoking views.
2. Nonnull `007ED0E6` writes incoming plane+9D4h=SAME squadron; `007ED0EC`
   writes its +9D8h=requested signed index; `007ED0F2` captures count+3CCh.
3. Starting at member array+3D0h, each occupied member is loaded and its actual
   signed +9D8h is read fresh. `007ED107/10D` stops at the FIRST strictly greater
   index. Equal indices remain ahead; there is no duplicate check or reindex.
4. `007ED130..13B` shifts the suffix backward from the captured tail, then
   `007ED13D` publishes the incoming member. `007ED144` increments the FRESH
   count field rather than publishing the captured count plus one.

The Source factories perform address/admission checks only, with no field-value
reads, allocation or callbacks. They borrow the following genuine live storage.

| Actual identity | Fields / minimum backing | Required mapping |
| --- | --- | --- |
| SAME squadron root | int32 +3CC; five pointer slots +3D0; byte +3EC; >=3EDh | Caller-owned live cells at exact offsets, root aligned for Win32 pointers |
| SAME incoming plane | pointer +9D4; int32 +9D8; >=9DCh | Required pure writable mapping to these same cells |
| Each current member plane | live int32 +9D8; >=9DCh | Existing required pure `NativeLandPlaneLeaderViews` mapping, followed by the operation's fresh signed read |

Mappings must return checked coherent views of the supplied identity: no
represented reads, effects, native calls, allocation, defaults or copied caches.
Address checks cannot prove that a supplied reference is a live object. Invalid
Source placement/identity reports logic_error; this does not recover native fault
or exceptional behavior. The nonnull ordinary domain admits count 0..4, live
occupied members, writable coherent cells and stable storage; squadron structural
backing is disjoint from reached plane backing. Repeated plane identities are not
rejected. No capacity, uniqueness, ownership, allocation, cleanup or reindex
policy is added. Invalid counts/storage, faults, concurrency and structural
reentry remain excluded. Native lack of guards is not a safe fallback contract.
`PlaneSquadronEntity` and its semantic member helpers have different storage and
policies and cannot substitute for these actual fields.

The actual direct caller is `007CDF82` inside complete native `007CDF20`
(`007CDF20..007CE03D`). Its selected arm resolves the real squadron, publishes
plane+9D4h/+9D8h, pushes the requested index and plane, and calls this entry with
ECX=squadron. The raw plane profile word `00D05F20+9Ch` (`00D05FBC`) is
`007CDF20`; plane constructor instruction `007CFD78` publishes `00D05F20`.
These are DATA/caller witnesses, not Source callable profiles or closure of that
higher routine. In particular `00D087C0+13Ch` targets `007ED010`, not this entry.
The larger plane constructor, resolver/world services, actual routing and
geometry remain separate dependencies.

The queue/lower-constructor identities also remain distinct: lower `009AFE70`
passes its actual block as ECX to `006C0B50`, with the fresh input plane+9D4h as
the incoming observer identity. The returned value stored at approach+34h /
task+42Ch is the first matching nonnull record+4 holder or FRESH block+80h after
notifications/message routing. It is not a queue-record address or cached
task+404h squadron. This packet produces a plane link; it creates no queue,
holder or retained-task ownership/lifetime binding.

One unique ignored Win32 probe compiled THREE fresh TUs: this operation, unchanged
`native_land_plane_leader.cpp`, and its probe. `/W4 /WX /fp:strict /O2 /MD`
compilation and embedded-manifest linking passed. Its 24 explicit checks compare
Source and unchanged, unrelocated original129B code on the same live shaped
backing: index 0 inserts before 1 and 4 with exact backward shift, existing whole
plane backing/indices stay untouched, and all padding/guards match. The SAME
published incoming cell then passes unchanged original12B full-EAX leader,
Source leader and the current final cruise-host leader adopter. The null branch
changes only dirty. No whole cruise/constructor caller is invoked; unexercised
required tuning/controller methods throw rather than provide defaults.

Both original bodies execute without code, CALL or table relocations. The fixture
reads the raw caller profile DWORD as DATA only. These shaped live objects and
qualified Source mappings are fixture storage, not native plane/squadron class
construction, profile or arena proof. The one focused case covers actual
front-insertion/backward-shift and null behavior; other admitted branch coverage
rests on complete native/Source/COFF inspection, not extra executed cases.

The complete generated Source COFF schedule preserves dirty+10 before null+13,
link+3D before index+49 before count+4E, fresh member+5B and signed CMP+77/JG+79,
backward load+96/store+9B, member+ A4, fresh count+AC then increment+ B1/store+B2.
Virtual calls at +27/+66 are required PURE Source view mappings, not native CALLs.
The existing leader cell CMP+07/SETE+0A and adopter mapping+13/CMP+21/SETE+24
were inspected in the same fresh compilation.

Artifact family: `local/cc11_plane_squadron_sorted_insert_*`; recipe:
`local/cc11_plane_squadron_sorted_insert_build.ps1`; schema-1 manifest:
`local/cc11_plane_squadron_sorted_insert_manifest.json` (SHA256
`24203afec3c832e8c6c95c4dcf260fa31572b6ec7cadb363ec4eef2002f30bf7`).
All 34 input hashes match before/after, including 27 quoted/actual compiler project
headers and three frozen current main libraries. Core SHA is
`40c16f99d6788c1444173bcf48353147b315fd00b28fd535dca0d9b5f53668b0`;
source-before=copy=source-after was established before root rebuilt. The executable
is PE32/I386 with embedded manifest resource 24/1 and asInvoker. No prior sealed
31/165-check or older family was changed or rerun. Three current direct-call rows
are checked separately: one insertion caller plus two existing leader callers;
neither leaf has native CALLs. `git diff --check` passes. Full main build,
CMake/ledger/profile annotations and integration belong to the primary; game,
native class/register ABI, arena, observer/death lifetimes and private EH remain
unbound. No numeric kernel or x87 geometry work was added.

Primary integration at `6bf930f3518af4d99b0fa9df6c0b3d2c10b7f698` passed the full MSVC Win32 build and all three existing CTests. Independent primary validation freshly compiled 3 actual TUs, pinned 30 Source/header/fixture inputs and 27 actual project compiler includes, plus 214 compiler headers, searched linker inputs, current BSP support libraries, installed PE and 136 unique historical worker paths. Whole Source COFF matches the worker. Complete129B42decoded7ED0D0 body, unreachable3B alignment12D..12F retained. Dirty3EC first includingNULL; nonnull sameplane9D4 link before9D8 requested signed index, captured3CC count, current3D0 member fresh signed9D8 scan FIRSTstrictlygreater, backward shift/member publish/FRESH countincrement. Genuine same live squadron+3CC/+3D0/+3EC and sameplane+9D4/+9D8 references, PURE checked identity mappings only; no default/cache/represented-read duringmapping or addedcapacity/duplicate/reindex policy. Ordinary count0..4/live occupied/coherent disjoint backing, no structural reentry/fault/overflow/concurrency. One24-check Source/original same-storage fixture0-before1/4 full backing/guards and unchanged otherindices, SAME producedcell through unchangedoriginal12B/fullEAX leader/Sourcebool/currentadopter, null dirty-only. Whole original129+12B unrelocated/no bridges, Source245B86 incl admission cold throw paths/leaf14B5/adopter75B25 exactworker; SourcePURE mapping calls distinct from native0CALL. Actualcaller7CDF82/D05F20+9C DATA and2leadercallers verified, no sortedtablexref (D087C0+13C is7ED010). Actualclass/wholehighercaller/ctor/queue/holder/world/arena/tuning/math/nativeABI/privateEH/game unbound; no olderfixture replay.
