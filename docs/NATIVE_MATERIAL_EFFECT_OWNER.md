# Actual material-effect storage and lifetime

`native_material_effect_owner.hpp/.cpp` reconstruct the C4h effect base and
178h derived effect over their actual fields. The canonical companion borrows
the same atomic `+04` used by the material constructor, factory, and generic
owner release. Its current profile is `D5E534 / BD30E0 / B192D0` for the base
or `D61A00 / BD30E0 / B422D0` for the derived object. There is no semantic effect
copy, private reference count, effect cache, or shader-loading substitute.

The entry points and C++ names are descriptive hypotheses. Constructors use
native ECX=storage, EAX=same storage, RET; destructor/clear/name-array helpers
use ECX=storage/header array, RET. Scalar deleting wrappers take a flags DWORD
on the stack, return the original address even after free, and use RET4.
The added C++ interfaces carry the required application domains explicitly and
are not drop-in native calling conventions or original SEH implementations.

## Construction and preserved storage

`B18D60..B18E8F` publishes CEB130, sets the actual count to one, and publishes
D5E534. It clears the eleven texture pointers and signed count `+38`, constructs
eleven empty 8h name headers at `+3C`, clears count `+94` and the same headers,
then clears retained count `+A8` and the name header `+B8`. It builds a temporary
actual `error.tga` string through the same native string pool, samples the
current renderer, invokes its callable virtual `+64` with name/zero, and stores
the returned owned fallback directly at `+98`, without another retain.

Temporary-string release occurs before reading the shared DWORD `F8D3A8`.
The old value is written to `+C0`; bytes `+08` and `+B4` are cleared; the global
is then incremented modulo 2^32. Thus a callback during string return can affect
the sampled serial. This is one effect serial, not one serial per pass.
The three `+9C..A7` owner slots, `+AC/+B0`, and padding remain unwritten.

`B407A0..B40819` calls that base constructor once, clears descriptor `+C4`,
owner `+138`, byte `+13C`, publishes D61A00, and clears fourteen DWORDs at
`+140..174`. It does **not** initialize either fourteen-pointer pass array at
`+C8..FF` or `+100..137`. This corrects any interpretation of the earlier
"descriptor/pass storage" summary as zeroing those pointers. Caller/loader
initialization must make every slot valid before release or destruction.

Constructor states 2/1/0 clean the `+B8` name, eleven names in reverse order,
then the CEB130 base. State3 arms temporary-name cleanup only after resize/copy
and before texture acquisition. A failing acquisition does not release an
uninitialized fallback or derived members, and does not publish a serial.
The supporting funclets are `CBC570`, `CBC578`, `CBC583`, and `CBC591`;
`A81880` is the existing thunk to base cleanup `BD30F0`.

## Destruction and ownership

`B41B10..B41BCD` first invokes the descriptor's current callable virtual zero
with flags one. This is direct unique-owner deletion and does not decrement a
reference count. It clears `+C4` after the callback, then releases and clears
fourteen `+C8` owners, fourteen `+100` owners, and `+138`, in that order.
Each identity is read when its slot is reached; callback mutations of later
slots remain visible. The descriptor implementation itself is a required
callable binding, not reconstructed by this packet.

`B187A0..B18848` releases texture slots using the current signed `+38` end,
reloading it after each callback, then clears that count and repeats for the
three `+9C` slots using current `+A8`. Captured nonnull pointers clear only
after release. The C++ valid domain requires counts 0..11 and 0..3 with forward
live cursors; it reports corrupt/reentrant extents rather than walking memory.

`B18EB0..B18F6B` publishes D5E534, releases fallback `+98`, calls that owner
cleanup, then releases `+B8`, all eleven names in reverse order, and publishes
CEB130. Names are destroyed regardless of count `+94`, and headers are not
cleared by their storage destructor. Member guards preserve this cleanup on
exception exits. `B18D50..B18D5F` is the standalone eleven-header destructor
helper used by member unwind. `B41F80..B41FCE` publishes D61A00 and releases
derived members before base destruction, including the base-cleanup unwind.

`B192D0..B192ED` and `B422D0..B422ED` destroy their respective storage and free
through the shared allocation domain only when flags bit zero is set. Their
incorrect `_free` CALL_RETURN overrides were repaired under the Ghidra write
lock; both stored functions now include their reachable return continuations.

## Validation and remaining integration

MSVC Win32 and both existing CTest cases pass. One ignored fixture compares
complete base/derived constructor images and flags-zero destructor images
against original instructions. It checks preserved bytes, live serial sampling
and wrap, descriptor/owner callback order, live texture/retained count growth,
and member-name release order. A host acquisition-failure case checks the
established cleanup state. The standalone B18D50 wrapper is byte-reviewed and
its C++ body is exercised through cleanup; its original instructions are not
executed separately. Exact spans, source hashes, and artifacts are recorded in
`reports/native_material_effect_owner.json`.

The same fixture composes the actual material factory, actual effect companion,
and material companion in one owner domain. Dropping the last material reference
destroys the effect, its retained fallback, and returns the material slot; pool
trimming and native string-pool shutdown complete afterward. This composition
uses an explicitly valid zero pass preimage, not loaded shader passes.

The fixture fallback/retained children are real native render-context owners
used to verify the generic ownership protocol, not logical texture loading.
Descriptor deletion is an observed callable fixture boundary. Original string
pool/CRT/vector-helper boundaries use existing reconstructed bodies or host CRT;
original terminal context dispatch is relocated to the canonical rebuilt
context owner. The original scalar wrappers are compared with flags zero;
flags-one physical deletion is exercised through the rebuilt composition.
No real descriptor/shader loading, renderer acquisition implementation,
original SEH delivery, draw, or gameplay validation is claimed.
