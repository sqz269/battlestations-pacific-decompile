# Whole scene property reference/sub-bag additions

The two complete Original leaves now have ordinary context-bearing C++ Source
interfaces in `native_scene_property_reference_subbag_add.hpp/.cpp`:

| Original | Whole body | Bytes / instructions | Ordinary API |
| --- | --- | --- | --- |
| `008F3AC0` | `[008F3AC0,008F3B56)` | 150 / 47 | `add_native_scene_property_reference_008f3ac0` |
| `008F3B60` | `[008F3B60,008F3BCA)` | 106 / 36 | `add_native_scene_property_subbag_008f3b60` |

Names are descriptive hypotheses. The root-admitted readiness report is
`reports/cc12_property_reference_subbag_readiness.json`; its historical two-header/
two-source proposal remains intact. This implementation uses one joint header/
source plus this document and its report. The primary owns registration and
integration metadata; this worker does not register the translation unit.

## Exact schedules and ownership

Reference allocates a canonical fresh uninitialized 38h root. Its physical DWORD
stores are phase`00CE89D4` at0, tag5 at4, full word08 at8, full word18 at18. It
calls the admitted raw ECX/RET0 duplicate`00438E40` with borrowed nullable target,
stores the full result pointer at1C, stores DWORDzero at20,24,34, then BYTE1 at2C.
Whole insertion`008F33F0` follows. Exactly33 root bytes are written;23 preimage
bytes remain in `[0C,18)`, `[28,2C)`, `[2D,34)`, including+C and+30.

Both word inputs retain all32 bits without conversion, enum restriction,
dereference or inferred ownership. Target is null or stable readable NUL-terminated
bytes; it need not be ASCII. Null produces no target allocation and result0;
nonnull empty text owns its one NUL byte. The input stays borrowed. On success,
the bag owns the fresh record and its distinct copied text+1C. Accepted type5
release frees that copy once, clears1C/18, and executes common reset4,C,28,8.

Sub-bag allocates a canonical fresh uninitialized38h root, then stores phase at0,
tag6 at4, full supplied child pointer atC, zero DWORDs18,1C,20,24,30. It stores
the full fresh root pointer into actual child+110 BEFORE recordzero34, recordzero8
and BYTE1 at2C, then invokes whole insertion. It writes41 root bytes plus4 child
bytes, preserving root`[10,18)`, `[28,2C)`, `[2D,30)`. Raw`008EF780` is not reused:
its final byte2C precedes word8, while this leaf writes word8 before byte2C.

This adopts the supplied actual child; it does not clone/copy/detach or supply a
borrow-only relationship. The child is a genuine canonical raw61-produced live
mutable114h owning bag from the accepted empty/whole clone provider. Require its
initial+110 backlink0 and a finite, unique, unshared, acyclic graph disjoint from
destination, replaced graph, fresh root, key/query/node/map/frame. These are
domain preconditions, not Original checks. Caller loses its owning claim on
successful publication. Accepted type6 lifetime scalar-deletes child with flags1;
child+110 is non-owning. The backlink is written before insertion and therefore
before replacement retires the old mapped graph. No same-child shortcut, old-owner
repair, detach, cycle check or rollback is introduced.

Independent noinline volatile DWORD/BYTE helpers preserve these physical write
widths and order under optimization. There is no root zero-fill, constructor
substitution, coalesced record store or callable phase-word dispatch. Source
argument/helper/register/stack/flags timing differs from Original instructions.

## Genuine provider domain and limits

Use the existing `NativeScenePropertyInsertionContext` and its SAME concrete
canonical string/physical processes, actual raw61 destination114h/map+4,
initialized CC8A30 E175B0 node pool/common E188B4 list, raw key context,
E177E4/E17654 permanent fallback cells, membership, generation and live return
gates. Complete explicit startup first; retire actual owning payloads/keys/nodes
before teardown. Keys are stable nonnull NUL-terminated closed ASCII in current
C locale. All address ranges are valid/nonwrapping and actual storage owners are
disjoint under accepted insertion/lifetime contracts. Serialize global mutation;
exclude alias, faults, overflow, allocation failure, provider throws, reentry and
concurrent trim. No callback, model/facade, allocator substitute or default closes
a provider. Direct Source calls use canonical `singleton_lifetime_allocate`,
actual `duplicate_native_string_00438e40`, and whole insertion`008F33F0`.

Both Original allocation-null branches structurally bypass every record store
(plus duplicate/backlink) and still pass0 to insertion. Whole insertion later
dereferences the record; this is outside the admitted successful nonnull domain.
A null child with nonnull root is also excluded and receives no added guard.

Reference Original registers FS handler`00CA49EB`/FuncInfo`00DD76C8`, state0 with
unwind map`00DD76C0` to action`00CA49E0`, which frees its saved raw root through
`00BF65AC`. It disarms construction cleanup before insertion. This ordinary
successful-domain implementation supplies neither that exceptional transport nor
invented cleanup/rollback. Sub-bag has no local FS/EH frame. Original ECX/RET10/
RET8/register/EFLAGS/FS/SEH/EH/class ABI, private CRT, parser/World/reference
resolution, startup/application binding, selected-entry runtime and game behavior
remain unproved.

## Evidence and validation

The owned ignored evidence family is
`local/WholeScenePropertyReferenceSubBagAddSource01/`. It preserves the prior
readiness family unchanged, exact Original byte/instruction pins, pre/post Source/
provider/compiler/tool inputs, actual command/launch/poll/terminal receipts,
complete object/COFF metadata and a self-excluding manifest over every nested file.
Its validation status and exact artifact pins are in the accompanying JSON report;
the post-commit family manifest pin is reported in the worker handoff.
The sole strict compile uses MSVC Win32 `/MD /W4 /WX /fp:strict`; one normal
`scripts/build.ps1` run executes existing repository checks while the new TU is
still unregistered. No new tests/probes or selected Original/Source entry, Native,
replay, runtime, startup or game execution is part of this worker packet.
