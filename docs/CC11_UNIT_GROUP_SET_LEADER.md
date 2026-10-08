# Whole raw UnitGroup leader assignment, 0070D0C0

`set_native_unit_group_leader_0070d0c0` reconstructs the complete
`[0070D0C0,0070D0E4)` body: **36 decimal bytes (0x24), 16 instructions**.
The descriptive name is a hypothesis, not a recovered source symbol.
Native ECX is the actual group, the two stack arguments are next and previous,
and the epilogue is RET 8. The unused fastcall EDX parameter preserves that
argument placement. No semantic return value is claimed.

The worker's first and only native run passed **383 checks, four cases,
eight Original/Source calls, zero failures**. All six complete raw bodies and
seven call edges were checked; the Source family built from eight fresh TUs
without BSP archives. This establishes conditional raw-entry and borrowed
storage behavior. Original class construction/lifetime, full class ABI,
startup and gameplay remain unvalidated. The primary integrator owns shared
registration, annotation, CMake and the full repository build.

## Exact behavior and native state

The body captures previous in EAX and next in ESI, then compares those two
arguments. It does not compare against group+14. Equal arguments return
without publishing or invoking wake, even when group+14 holds a third,
different actual leader. With different arguments, null previous skips the
wake call and still publishes next to group+14. Different nonnull previous
calls complete raw `00815E20` with ECX=next and stacked previous, then stores
next to group+14 only after that call returns.

The assembly preserves this order literally. There is no null-next default,
replacement provider, callback, profile dispatch, class-field substitution
or repair guard. A nonnull previous requires a usable actual next and valid
FAC-byte unit prefixes with every reached pose ancestor valid. The full
caller at 0077BD70 and detach/type/speed routing are separate contracts.

ESI and EDI are pushed and restored, while EBX and EBP survive unchanged.
Volatile exits depend on the path:

| Path | EAX | ECX | EDX | Floating state |
| --- | --- | --- | --- | --- |
| Equal arguments | Previous/next argument | Actual group | Entry EDX | Entire x87, XMM0 and MXCSR unchanged |
| Different, previous null | Zero | Actual group | Entry EDX | Entire x87, XMM0 and MXCSR unchanged |
| Real wake | Actual next+F9C | Zero | Actual previous+FA4 | Complete wake effects, then group publication |

On wake paths the last x87 operation remains the handoff FSTP at
`00815E20+A9`, with data pointer actual next+FA8. XMM0 is all zero, residual
Y is positive zero and MXCSR is unchanged. The setter's final MOV, POPs and
RET do not replace the callee's volatile or flag effects.

No-wake EFLAGS was 00000246 and wake EFLAGS was 00000206 in this run. These
are observed values at this fixture's actual physical stack. Each
Original/Source pair matched at the same callsite and ESP; wake EFLAGS is
not an intrinsic function constant. The earlier handoff fixture's different
physical stack therefore need not produce this run's wake flag value.

## Complete graph and shared constructor setup

| Original entry | Complete bytes | Instructions | Production Source |
| --- | ---: | ---: | --- |
| 0070D0C0 | 36 | 16 | `native_unit_group_set_leader.cpp` |
| 00815E20 | 182 | 42 | `native_unit_wake_handoff.cpp` |
| 00414DB0 | 89 | 28 | `native_entity_pose.cpp` |
| 00413920 | 874 | 290 | `native_camera_matrix_math.cpp` |
| 004134F0 | 103 | 35 | `native_camera_matrix_copy.cpp` |
| 00815680 | 456 | 135 | `native_unit_wake_copy.cpp` |

These six complete Original bodies total 1740 bytes. They occupy their
original relative offsets in one RX island from 004134F0 through 00815ED5,
extent 0x4029E6 (4,205,030 bytes). **All seven Original CALL operands remain
unchanged**, including pose recursion. There is no Original call patch,
trampoline, source helper substituted for native code or bridge. The exact
matrix-copy extent is 103 bytes; the following CC padding is excluded.

All seven Source call edges bind the corresponding complete canonical raw
functions directly. The new leader Source COFF is 36 bytes/16 instructions,
SHA-256 `b35de2a7f8b9951c4be1b377bb90e2338025bc81326002a854da03821447e161`.
Only the natural CALL operand at +18h relocates; the remaining 32 bytes equal
Original, SHA-256
`c8663953b174916475ec8175c41d653c92f494f2ea3ec42768d762cdab158787`.
All six full live Ghidra/installed-PE extents, COFF and linked bodies were
checked. All six Original and Source bodies remained unchanged after the run.

The seventh production TU is the current genuine Source
`construct_native_unit_group_0070dab0`. It initializes the actual guarded
508-byte group once per case before the common backing snapshot. Both sides
then restore that same backing in place. Its complete compiled Source body
is 132 bytes/39 instructions, no relocations, SHA-256
`cc195aa9f77f61c070ce1d558dafe1a2b0edf6b9d89084ad0a24d87dc53c362e`.
The fixture's direct call binds that real function; its two additional
address references are for whole-body snapshots. The complete constructor
body also remained unchanged.

The Source constructor borrows the actual CF4888 constant bits, 4479C000
(float 999.0), pinned from the original PE and verified against live Ghidra
data. The fixture preserves this word and its guards. CF4888 is distinct
from the constructor's CFD6F8 raw vtable stamp. That stamp is stored as
uncallable data and is never dispatched. All 989 constructor write bytes,
the other 299 group bytes and every outer guard were checked. **The Native
constructor is not executed**, and Source setup is not proof of original
class construction, virtual ABI or lifetime.

## Four cases and complete state comparison

| Case | Actual arguments and pose graph | CW / TOP / tag | Outcome |
| --- | --- | --- | --- |
| 0 | Equal nonnull arguments; group stores a third actual leader | 037F / 6 / 0FFF | Third leader and all backing unchanged |
| 1 | Different next, previous null | 0A7F / 6 / 0FFF | Only group+14 may change; no wake state touched |
| 2 | Different actual units, both dirty roots | 027F / 6 / 0FFF | Publish next after real wake; residual bits 41400000 / 00000000 / 40800000 |
| 3 | Next.parent=actual previous; previous.parent=dirty root A | 0A7F / 3 / FFFF | Real recursion makes previous valid before the fresh old-C8 test; residual bits 40000000 / 00000000 / C0400000 |

Cases 0-2 retain two live 80-bit x87 canaries. Case 2 needs one free slot;
case 3 reaches the eight-slot matrix multiply and therefore starts with an
empty stack. Empty TOP may be nonzero, as case 3 demonstrates. Production
does not reset floating state. The fixture alone saves, seeds and restores
the caller's ambient floating environment.

The same 10,368-byte aligned backing contains actual FAC-byte units at 20
and 1020, pose ancestors at 2020 and 2160, and the actual 508-byte group at
2320. Both targets receive identical addresses and parent identities after
restoring the backing in place. Every byte compares, including all guards,
the complete group and the old inline ring. No byte outside the precise
leader/pose/ring/residual write mask changes. The wake ring at unit BD0 and
FA0/FA4/FA8 residual fields remain one actual aliased representation.

Each call saves 216 bytes: register/stack fields, the full 108-byte x87 state,
all XMM0 bits, MXCSR, actual entry ESP, 32 physical stack guard bytes and
the next/previous/group argument values. The entire saved record matches
on no-wake paths. On wake paths only the natural code-address FPIP DWORD
differs. All 80 x87 payload bytes compare in every case; both live canaries
survive where admitted. All nonvolatiles, RET 8 stack balance, actual
arguments and all 32 stack guard bytes pass independently.

No-wake paths preserve the entire x87 state, including the seeded FPIP, and
all 128 XMM0 bits. SW is 7520 -> 7520 there, 7520 -> 7523 on dirty roots,
and 5D20 -> 5D23 on recursive ancestry. CW, TOP, tag and MXCSR 00001FA0 are
preserved. The real wake cases retain the native invalid/denormal sticky
effects, signalling-NaN quieting and subnormal bits. Publication order is
established by complete instruction/call inspection; the fixture executes
that real wake and checks the resulting leader and whole storage without
injecting an observer or replacement dependency.

## Evidence, reproduction and remaining scope

`reports/cc11_unit_group_set_leader.json` indexes the sealed local directory
`local/cc11_unit_group_set_leader_20261007_a`, its recipe, exact inputs,
complete body/adapter evidence, runtime snapshots, logs and manifest. The
check count includes setup preconditions, artifact I/O and preservation
checks as well as behavior/state comparisons.

Both the Source-only compile and the eight-TU connected build passed on
their first attempts with `/MD /O2 /Gy /W4 /WX /fp:strict /showIncludes`.
The I386 PE embeds an `asInvoker` manifest using `/MANIFEST:EMBED`. Actual
pins cover 16 project/fixture inputs, 266 host headers, seven searched system
libraries and three selected tool files. No BSP archive or mutable main
build library is consumed. The recorded main-core hash is context only.

One preparation command used the wrong byte-query syntax, and two pre-run
inspection scripts failed on constructor-reference classification and a
manifest attribute spelling. All three failed scripts/receipts remain in
the artifact set. Their corrected checks passed before execution. There
were no compile failures, no failed native runs and no earlier family replay.
An orientation-only packet-list command also rejected unsupported options;
its separate receipt records that rejection before the packet was claimed.
Reproduction must use a new artifact directory and preserve this receipt.

All 581 current prior paths stayed unchanged, including the previous 318
path baseline and all 159 sealed wake-handoff artifacts. The authorized
sync to published main had already changed the handoff documentation and
report; their exact worker versions are preserved in this new receipt.
Earlier disclosed wake-copy publication changes remain separately recorded.
All current inputs, toolchain/system files and the original image passed
post-run hashes. Ghidra remained read-only; no tracked tests were added.

Actual class allocation/construction/destruction, usable owner lifetimes,
invalid inputs, cyclic ancestry, unmasked faults, concurrency/reentry,
the enclosing 0077BD70 caller, full detach/type/speed behavior, world
integration and gameplay remain external. The caller at 0077BE40 was
verified statically and was not executed by this family.

## Primary integration

Whole70D0C036B16 compares SUPPLIED next/previous arguments, not storedgroup14; equality is inert even if storedleader differs, unequalpreviousNULL publishes next without wake, unequalnonnullprevious invokes genuinewhole815E20182 BEFORE publish. Only natural CALL operand18h relocates;32 otherbytes Original-exact, fullphysical Win32RET8/register/flags contract retained. Independent eight-current-TU383-check/4-case/8-call family uses genuinewhole SourceCtor132B39 on sameactual508group; all six complete raw bodies1740B and seven canonical Sourceedges match original exact-relative4205030-byte island with ALLseven nativeCALL operands unchanged. ActualCF4888bits4479C000 are nativeDATA borrowed into Source; nativeCtor neverexecuted/profileCFD6F8 uncallableDATA. Full10368actual backing/32physicalframeguards/216statebytes match except naturalwakeFPIP. RootEFLAGS246 early/202wake versusworker246/206wake are common-callsite/ESP observations, not intrinsicconstants. Dirtyparentmultiply needs EMPTYtagFFFF (nonzeroemptyTOP valid); otherpaths admitonefreeslot/two80bitcanaries. Original caller77BD70/class6virtualadmission/detach/fullclass/lifetimes/allocator/privateEH/concurrency/world/game remain external.

Main Source `273980cf3` passed full MSVC Win32 and all three existing CTests. Independent eight-current-TU receipt `local/cc11_set_leader_current_primary/inputs_after.json` pins seven project headers,one generated header,266 host headers,seven system libraries,four compiler/tool files and784 historical worker paths; zero BSP archives. All seven genuine raw Source calls and SourceCtor132B39 are COFF/linked verified. Root preparation retained a metadata-key mismatch before compilation; the strict compile/link then passed, while its initial invocation rejected a missing constructor-size argument before native/source calls. Exact corrected argv132/fresh runtime directory and pre-run whole-COFF inspection produced the first and only native body run383/0; source and compiled artifacts stayed unchanged. Saved comments,exports and snapshot follow in report.
