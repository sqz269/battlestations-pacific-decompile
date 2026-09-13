# Actual material program builder and compiler entry

Addresses: B3C3A0, B354D0, B3A7E0, B3B3C0, B35930.

This packet implements the actual B3C3A0 builder wrapper and advances its
B3B3C0 child through argument/name production and the real B35930 vertex-field
copies. It does not provide the remaining shader compiler. A required native
continuation receives the same retained builder and local name at B3B513 or
B3B536. There is no default continuation, generated raw pass, or conversion
from `CompiledMaterialPass` into a native owner.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B3C3A0..B3C4B1 | ECX effect, EDX root descriptor, seven stack arguments; EAX held pass; RET1C | Normal wrapper composed; B3B3C0 tail required |
| B354D0..B3553C | ECX fresh builder, EAX same, RET | Complete producer |
| B3A7E0..B3AE9B | ECX builder, RET | Normal destruction on valid accessible nonnegative counts; original FH3 excluded |
| B3B3C0..B3C390 | ECX builder; stack effect/root/mode descriptors, EAX pass; RET0C | Prefix through B3B50E's actual B35930 call; native continuations B3B513/B3B536 required |
| B35930..B35BD1 | ECX builder, RET | Complete normal raw field-copy producer; retained host failure differs from native unwind |

Names describe reconstruction hypotheses, not recovered symbols. These are
new MSVC Win32 C++ interfaces, not original ABI entry replacements.

## Actual builder and wrapper arguments

B354D0 writes the D5F314 profile, clears every DWORD04..68 and the actual
name header9C/A0, and writes FF at bytes6C/6D. The remaining bytes preserve
their incoming values. The B0h storage declaration reuses existing actual
12-byte array headers and 8-byte pooled strings. Five arrays own independent
field allocations;40 owns children deleted through current virtual0(flags1).
Source4C and name9C are strings;54 and60 contain two-byte rows. No semantic
source-builder object is installed over this storage.

B3C3A0 copies the supplied actual name header into9C, then writes fields in
the observed order: descriptor byteA9, generationAC, RTCountA4, mode byteA8,
policy byteAA, and unsigned `generation<3` byte98. Bytes6E/6F,99..9B andAB
remain unwritten. Its ECX effect and EDX root descriptor are captured in
callee-saved registers before builder construction. The two B45EE0 callers
are B4631E and B46795. Both supply generation3; the latter supplies mode1
and descriptor flag0. The former uses mode-descriptor45 and root44, so the
wrapper must preserve the actual low bytes rather than normalize booleans.

The retained operation copies argument values and borrows the original
descriptor/effect/name identities; it never keeps the address of a transient
`NativeMaterialProgramRequest`. All borrowed inputs and context services must
outlive a failed operation.

## Executed compiler prefix and field ownership

B3B3C0 publishes effect78, root70 and mode74, copies builder9C into its actual
local string, then optionally replaces that local with the substring after the
last `/`. The slash byte comes from CE7898. This preserves the complete builder
name. It calls the existing actual reverse-find/substring/string services,
including their allocation and reload rules. The flagF0 logging call4254B0
has a verified plain-RET body and adds no effect.

After rereading live0108D6F0/F1, a nonzero flag runs actual B35930. That body
walks current root then mode D0/D4 lists without clearing or deduplicating.
For each entry it allocates an independent1Ch record, reloads the source
descriptor, and captures index18, semantic14, component count0C and scalar08
before pooled-name copying. The new record gets mask10=0. The native source
name header remains borrowed across allocation; the copy helper reloads its
current length/data. Growth uses existing actual B34680 and native
`max(signed(capacity+5),10)` arithmetic. Publication precedes incrementing
the builder's current count. The next iteration reloads the descriptor and
its count. The acquired frame retains a not-yet-published field on host error.

Execution then requires B3B513 (next native child B35BE0) or B3B536 (next
native child B41820). The continuation receives the stable actual builder,
local-name header and an owned child slot for downstream acquisitions. It
must not replay the prefix. On normal return it has completed native local
cleanup and transfers a fully constructed, registered canonical pass reference
or an actual native compiler-null result. Native orphaned allocations on a
null result must stay in the canonical domain. An unavailable continuation
must raise an explicit error; it cannot return null as a substitute.

## Destruction and saved analysis limits

Normal B3A7E0 republishes D5F314, deletes40 children through their current
callable virtual0 with flags1, and clears each captured slot after return.
It then destroys/frees current field owners in04,10,28,1C,34 order, clearing
their captured slots after the free. It releases name9C, arrays60 and54,
source4C, then arrays40,34,28,1C,10,04. Each final array count is reduced to0
before the current data pointer is freed; stale data/capacity are preserved.
Negative-capacity repair uses minimum1 for two-byte arrays,6 for40 and10
for the five field arrays. Negative counts and inaccessible/corrupt extents
are outside the guarded host domain.

Saved Ghidra B3A7E0 ends atB3AA68, but the original disk body is1,724 bytes
through RET B3AE9B. Five post-BF65AC slot-clear blocks, one post-free growth
publication block and the tailB3AA69..B3AE9B have no saved containing function.
The report distinguishes these exact inclusive ranges and their raw CALLs
from verifier-eligible saved-body call rows. Read-only BSP wrapper bytes were
matched with the installed PE; no worker Ghidra mutation or saved-flow repair
is claimed.

On a C++ failure, the operation retains builder, temporary names, unpublished
field and tail child, with phase/call-site metadata. There is no destructor
rollback or retry. A live failed operation cannot be discarded. This differs
from original FH3 unwinding; failure paths inside existing string/reserve
services retain those services' documented limits. Normal builder cleanup may
leave stale freed headers, so it is also one-shot.

## Remaining source and validation boundary

The existing B35BE0/B36800/B34AA0/B372D0 field routines and B39110/B39880
source generation are semantic source projections. B60F60/B61280 use typed
compiler adapters, while B3AEA0 is a bounded reflection projection. The actual
pass pool/B44B10 constructor and state owners already exist, but B3B3C0's
D61810 raw88h reflection-owner construction, native source pipeline, compiled
cache cursor, COM wrappers/registration and the later ownership transitions
are not supplied by this packet. The header keeps that full tail required.

The ignored focused fixture uses the previous hash-verified actual Lua/VFS
composition and installed debug/alpha shaders. It relocates complete original
B354D0, B35930 and B3A7E0 machine bodies, reusing actual pooled-string,
reserve and CRT services through explicit ABI bridges. It compares raw producer
preimages, independent field copies into all five owning lists, and normal
destruction with canonical string-domain shutdown. It does not fabricate a
compiled pass or execute a successful compiler tail. Exact results and hashes
are recorded in reports/native_material_program_compiler.json.
The fixture passed all three original/source body comparisons (2,507 bytes,
855 instructions). The verifier checked162 eligible numeric call rows with
zero failures;23 raw destructor-tail CALLs remain outside saved-body coverage.

The new production translation unit is compiled directly with Win32 W4/WX.
The separate scripts/build.ps1 baseline passed, including CTest1/1, but does
not register this new source:
cmake/startup.cmake is held by another harness and must be appended by the
integrator before a combined build. No permanent test, original FH3 parity,
full cold material load, shader draw, or gameplay validation is claimed.

## Integrator body repair

Seven post-free Ghidra flow gaps were repaired under the write lock and the
B3A7E0 body recreated through its actual B3AE9B return, preserving prior
metadata. The verified full body contains 618 instructions and no gaps.
Twenty-three previously raw tail call rows now belong to this function;
the original worker status remains in the report as historical evidence.
