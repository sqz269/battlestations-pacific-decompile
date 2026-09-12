# Raw singleton lifetime in the executable

The executable now owns a raw 20-byte singleton manager and a raw 16-byte
gameplay-effect manager. The menu borrows the application host; its memory
checkpoint reaches raw `004C1650` and `0086B0B0`. Normal WinMain shutdown reaches
raw `00BD0400`, frees the captured manager, and then clears its publication.
Both publication cells and the deletion bindings outlive menu destruction.

`FrontEndShellServices` expresses the effect probe operation. The existing
`FrontEndShellHost` supplies a concrete typed getter/probe adapter for existing
clients and tests. The executable implements the operation through the raw
host. It no longer constructs a typed `SingletonLifetimeDomain` or casts a raw
owner to a typed gameplay-effect object.

| Native entry | Reconstructed behavior | Native ABI / source change |
| --- | --- | --- |
| `00BD0400..00BD04C4` (197 bytes) | Validate captured last slot, pop current end before deleting, reread live count, release actual section, free current vector and zero fields | ECX manager, RET; source adds EDX stable deletion bindings and source C++ exception scope |
| `00412440..0041245E` (31 bytes) | Test low flags bit 0 before writing CE3818; optionally free captured owner; return captured pointer bits | ECX owner, stack flags, EAX captured owner, RET4; unused source EDX |
| `0086B0B0..0086B0F3` (68 bytes) | Walk checked raw tree iterators through actual `00869A20`, without payload reads or output | ECX effect owner, unused stack label, RET4; unused source EDX |
| `008F8449..008F846C` (36-byte shutdown; capture includes next byte) | Capture current manager, destroy it, free the capture, clear publication | Internal WinMain fragment; source host method has a new interface |

The native manager uses arbitrary owner vtable slot-zero dispatch. The source
adapter currently resolves four recovered profile identities to concrete
providers: CE3818 to `00412440`, D0DA64 to `008703E0`, D5E594 to `00B1B660`, and
D5E59C to `00B1B710`. It reads the current profile when each owner is popped.
Effect deletion requires the actual effect publication cell; registry deletion
requires the actual publication, string pool and invalid-parameter bindings.
Unknown profiles or missing bindings raise an explicit source admission error.
This map does not close every original singleton type. The executable's private
registration path currently admits only the D0DA64 effect owner.

The normal helper preserves native argument/local offsets with 24 bytes of
padding. A stable source binding replaces the native unwind-only owner spill.
The original FH3 registration is replaced by an outer source C++ scope. Native
FuncInfo DFF490 has state 0 -> CC5450 -> BD0220; the source catch invokes the
actual raw storage cleanup and rethrows. That cleanup frees and clears the
vector while retaining the manager allocation and its section. A popped owner
whose deletion throws is not silently retried or freed. Caller ownership and
failure handling remain explicit.

The evidence captures all 296 function bytes, 62 EH bytes, 20 profile-table
bytes and the 37-byte WinMain span from both saved BSP and the installed binary.
The emitted review compares all 99 probe/base-scalar bytes and 148 retained
manager normal bytes, with named call relocations and mapped branch targets.
It separately checks the source frame adaptation, concrete dispatch targets,
source catch metadata, actual archive providers and application object references.
Original function objects/comments were preserved. Ghidra's erroneous
CALL_RETURN overrides at BD04A3 and 412451 were cleared, the returning-free
continuations disassembled, and the new base-scalar function created and saved.
The existing `STL_inst_0086b0b0` name is retained; descriptive names are hypotheses.

The strict MSVC Win32 build, two existing CTests and eight reference seed spans
passed. One ad hoc source fixture covers a nonempty effect-tree probe, low-byte
base flags, mixed base/effect deletion, and a source exception crossing the naked
helper into BD0220 cleanup. It uses actual providers and an embedded executable
manifest; no persistent tests were added. A frozen rebuilt game ran 180 frames
and exited 0, logging exactly one registered effect owner and both publications
cleared through raw storage. The final audit pins the tested binaries, compiler
inputs, runtime log, annotations and any subsequent integration build separately.

These checks establish bounded source behavior and executable integration.
Only base/effect dispatch profiles were executed in this packet. Original caller
ABI, FH3/SEH/private-frame aliases, hardware-fault behavior, arbitrary owner
dispatch, visual correctness and full gameplay remain unproved. See
`reports/native_singleton_runtime_audit.json` for exact hashes and evidence.
