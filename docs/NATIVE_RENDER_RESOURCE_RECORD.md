# Native renderer resource record storage

`destroy_native_render_resource_record_00b2f990` implements the complete
120-byte native storage destructor `[00B2F990,00B2FA08)`. It destroys the actual
2Ch record's alias nodes, sentinel and pooled name buffer while preserving
the record itself and its resource payload. The strict Win32 build and one
native differential lifetime case pass. Container removal, record assignment
and the full resource owner's lifetime remain separate work.

Source: `include/bsp/native_render_resource_record.hpp` and
`src/native_render_resource_record.cpp`. Original ABI: ECX points to the actual
record; no EDX input or stack arguments; `RET 0`; no semantic return value.
The descriptive C++ name is a hypothesis, not a recovered symbol. This typed
interface takes the actual shared `00419CC0` string-pool domain explicitly;
it is not a binary replacement for the original entry point.

| Offset | Actual storage and final behavior |
| --- | --- |
| +00h, +04h | Name length and pooled buffer; fields remain unchanged by the destructor |
| +08h | Unknown word, preserved |
| +0Ch | Actual 10h alias-list sentinel; ordinary-freed, then cleared |
| +10h | Alias count, set to zero before node cleanup |
| +14h through +24h | Five payload DWORDs, preserved |
| +28h | Opaque actual resource pointer, preserved without a call or reference operation |

The native list-clear helper `004D05E0` is inlined using the existing concrete
contract from `particle_clock_lifetime.cpp`. Each 10h node is next, previous,
string length and string data. The function captures the first node before
resetting both actual sentinel links and the count. Each iteration captures
next before returning the current nonnull string buffer with length+1, then
ordinary-frees the node through `singleton_lifetime_free`. It compares the
captured next pointer with the **current** record sentinel after those calls.
The concrete allocator/free operations keep the actual allocation identities;
there is no substitute vector or string container.

After the list drains, the function reloads the current sentinel, frees it,
then writes record+0Ch = null. Only afterward does it capture current name data
and return that buffer with the current length+1. Name length/data are left
alone after the callback, including any changes made by that callback.
`NativeString::release_to` would incorrectly clear those fields.

The complete post-free tail matters: current Ghidra discovery ended at
`00B2F9C8`, after the false no-return `_free` call. The original continues
through name cleanup, exception-chain restoration and `RET` at `00B2FA07`.
The integrator handles that function-body repair, annotation, ledgers and
build-source registration; this packet makes no Ghidra or shared metadata
mutation.

FuncInfo `00DF6100` and unwind map `00DF60F8` install a state-0 cleanup through
`00CBD840`/`0041DD20` on the saved original record. It returns the current name
buffer if list or sentinel cleanup unwinds. `RecordNameUnwind` retains that
boundary, and is disarmed at the equivalent of `00B2F9D8`, before the normal
name-buffer return. The existing pool release and ordinary-free interfaces
are nonthrowing; no artificial throwing allocator seam was added. Native SEH
registration and binary exception ABI are not reproduced or runtime-tested.

Validation used a fresh worktree based on `dcbc670`:

- `scripts/build.ps1` completed with MSVC Win32 `/W4 /WX /fp:strict`;
  CTest passed 1/1. An ignored local CMake top-level include registered the new
  source for this build without changing the integrator's CMake files.
- All eight seed byte checks passed before native execution. The six exact
  destructor, helper and EH spans in `native_render_resource_record_audit.json`
  were separately refreshed against the current verified Ghidra project and
  installed PE. The report contains the original bytes and SHA-256 hashes.
- One focused native differential case executed the complete installed
  destructor and list-clear bytes, with their seven dependency calls relocated
  to the same real pool and observed `malloc/free` boundaries used by the typed
  function. Both produced the exact six-event ownership sequence: first alias
  string, first node, second alias string, second node, current sentinel,
  current name.
- During that case, allocator callbacks changed a node's next link after it
  was captured, replaced the current sentinel, changed the name during
  sentinel free, and changed name fields during final string release. Both
  implementations preserved the required reloads, retained callback writes,
  freed each current-owned allocation once, and left all payload words and
  the actual opaque resource allocation untouched.

The ignored fixture is `local/native_resource_record_check.cpp`, built by
`local/build_native_resource_record_check.ps1`. Its source/build hashes and
native call relocations are recorded in the audit. This proves the selected
normal ownership path against native instructions; native exception injection,
renderer integration, multithreaded allocator behavior and game execution are
not claimed.
