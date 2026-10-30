# Qualified raw Lua variant scalar deletion

Frozen base `879f5d855aa43e099fb349ea7b18d58c7f2d1a40` contains the exact admitted
`a4774a47499cf3ad0f38af058a53fef1e71cea61` retirement Source. Authorized readiness
`5ef322e05be0fa3197cc9ee6d952dcbe5c9dd142` supplies the complete scalar contract.
This packet adds one complete ordinary C++ scalar entry and an8-byte aggregate
result in the existing retirement header/CPP. Old declarations, implementations,
Frame120B/Operation24B/sidecar56B layouts and legacy Hosts remain preserved.
No callable table, caller dispatch, global, CMake entry or activation changes.

The new entry is `scalar_delete_native_lua_variant_00884fa0_cpp_cleanup`.
It borrows the actual receiver, a `const volatile uint32_t&` flags HOME, and the
same actual raw string context, operation and cleanup backing as the qualified
884AF0 child. `NativeLuaVariantScalarDeleteResult` has exactly two fields:
`completion` and `uintptr_t captured_receiver_bits`, with no default fields,
implicit Boolean conversion or pointer-success convention. Compilation asserts
aggregate/trivially-copyable properties, size8 and bits-field offset4.

## Complete Native contract and actual Source effects

Fresh installed-PE verification matches all three135-byte spans: the complete
scalar30B/11 instructions,884AF0 child101B and D0E6F4 word4B. Image SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The scalar is **00884FA0..00884FBD**, SHA256
`1e12378e15ffd6846e22c5321444c71e85ca37f9f502a575667414cc12a7f02b`.
Its final RET4 starts884FBB and occupies three bytes;28 bytes would truncate it.

Native captures ECX into saved ESI, calls884AF0, then tests the CURRENT LOW BYTE
of its incoming flags home with mask01. If set it passes captured ESI toBF65AC
`_free`. It returns captured receiver bits in EAX even after free, restores ESI
and RET4. The scalar adds no Native local FS/EH cleanup frame. D0E6F4 points at
this scalar; neither that profile nor an immediate Source profile store installs
or authenticates a Source callable table.

The new complete Source body captures V and its pointer bits before calling
actual `destruct_native_lua_variant_00884af0_cpp_cleanup`. It forwards the SAME
strings/operation/sidecars. The returned child Result determines completion;
`operation.result` is not consulted as a root-completion substitute. Returned
non-completion immediately returns that status plus diagnostic receiver bits,
before any flags read or free. It retains the child's stopped metadata.

Only after child completion does the body read exactly ONE CURRENT volatile
unsigned byte at flags-HOME offset0 and test mask01. Set bit calls actual
`singleton_lifetime_free` on captured V; clear bit skips free. Both normal paths
return completed status and captured bits. There is no receiver/profile/owner
reload, early flag snapshot, widened DWORD read, null/default guard or added
destructor action. The bits are identity, not a live-pointer guarantee: completed
bits may already be dangling. Non-completion bits do not imply Native success.

A genuine C++ exception escaping the qualified child has no ordinary scalar
result and bypasses flags/free. No catch, guard, owning destructor, extra frame,
retry, success conversion, fabricated exception or termination policy is added.
The child's retained primary/secondary captures and provider limits remain.

## Actual flags home and resource domain

V is genuine live20-byte variant backing satisfying the full qualified child
domain, including its actual nested payload/name/map and AA8/AA0/AA4 providers.
If the LATE free bit is set, captured V must be the exact root allocation base
in the canonical malloc/free domain. An entry flag value, numerical profile,
tag or size authenticates none of that. A retained embedded receiver cannot be
freed as a root. `singleton_lifetime_free` directly calls `std::free` and keeps
its existing noexcept/CRT boundary; no injected callback, pool, operator delete,
Native address cast or fallback substitutes for it.

The flags HOME must remain live/readable through its late byte read. Supported
raw-home/receiver aliases preserve this ordering; the HOME is not required to be
disjoint from every raw object. Legal aliases may change it during the child,
and the late read observes its current byte before any scalar receiver free.
Context, operation/sidecars and Source result metadata must stay independently
live/disjoint from mutable homes and raw payload, so diagnostic storage adds no
Native effects. Active metadata cannot be externally changed; reentry uses
independent operation/backing. Backing covers the actual child/nested-map chain.

This explicit Source HOME is not the Original private stack slot. Arbitrary
Original private-stack coincidence/alias replacement, asynchronous mutation,
fault/atomic behavior and physical ABI identity are not established. Volatile
supplies a Source read epoch, not synchronization or private-frame proof.

## Strict Win32 emission and complete old-code comparison

One MSVC19.51.36244.0 Win32 TU compile passed without warnings with
`/std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict /FAs`.
This is `/c` only: no executable link, new test, body/probe/DLL/game execution or
common build occurred. Root separately performs immutable review/common admission.

The new scalar is55B/21 operations with exactly two REL32 calls. The public
qualified destructor's simple forwarding body is inlined, so the actual child
target is the same complete qualified destructor kernel, not a normal kernel.
The other call is the genuine canonical free. Full scalar bytes are
`56ff7424188b74240cff742418ff74241856e80000000083c41085c075158b44240c8a00a801740956e80000000083c40433c08bd65ec3`.

| Strict scalar offset | Physical proof |
| --- | --- |
| 05h | Captured receiver enters preserved ESI before the child call. |
| 12h | Actual qualified destructor kernel receives captured V and same context/metadata. |
| 1Ah/1Ch | TEST child EAX; nonzero jumps directly to33h, skipping flags/free. |
| 1Eh/22h/24h | Load explicit HOME address; exactly one MOV AL,BYTE[EAX]; TEST AL,1. |
| 26h | Clear bit jumps to31h, skipping free. |
| 28h/29h | Push captured ESI and call actual singleton_lifetime_free. |
| 31h/33h/36h | Normal EAX=completed0; EDX=captured ESI; plain RET. Non-completion preserves child EAX. |

This strict compiler returns the8-byte aggregate in EAX=status/EDX=bits, with no
hidden result-buffer parameter or result-memory stores in the scalar. Its new
16-byte FPO record reports five DWORD parameters, one saved register, no locals
and no SEH flag. This observed C++ convention differs from Original EAX=receiver/
RET4. There is no general untouched-caller/hidden-buffer/private-ABI assertion;
other/common emission remains a fresh admission question.

Complete COFF decoding covers37 code sections5483B/1652 operations and75 direct
calls. Every one of the old36 code sections5428B/1631 operations matches immutable
a477 raw code and normalized relocations exactly, including all old public,
normal/qualified kernels, helpers and six compiler EH bodies. Old recursive
mode/status/cleanup schedules therefore have no new emitted body drift here.
The scalar alone adds55B/21 operations and two direct calls. The only new
externally defined function is this scalar; the seven unresolved provider/runtime/
cookie symbols are unchanged. There is no new allocator/callback/throw-policy import.

All six complete xdata tables632B and normalized relocations are a477-exact;
every byte was structurally decoded and every catch target is an instruction
start. SafeSEH remains24B with the same six handler identities. Raw symbol-index
words renumber, but no scalar handler or new EH/action table exists. All old24
FPO records are exact with normalized relocations; the scalar adds one record.

Other emitted metadata drift is explicit: CodeView debugS212 ->216B reflects the
different output-object path; compiler identity remains19.51. Section-checksum
payload grows568 ->584B alongside71 ->73 sections. Relocation/symbol indices,
file offsets, object layout and checksums are not whole-object equality proof.
The existing4B `__Avx2WmemEnabledWeakValue` BSS has PointerToRawData0 and ZERO file
payload; no file-header bytes are interpreted as BSS data. No Source global is added.

After the passing compile, Root requested a header-comment correction distinguishing
flags-HOME lifetime/allowed aliases from metadata disjointness. The compiled header
and final header are separately retained; stripped noncomment tokens/normalized
whitespace are identical. CPP did not change. No extra compile was needed for that
comment-only correction, and both exact header hashes are recorded.

## Custody and remaining admission

Six immutable implementation/review/readiness/admission inputs,21 unedited dependency
pins, all Source/baseline/object/assembly/COFF/EH/native metadata receipts and the
complete scalar listing are retained. Old Source prefixes and broad Hosts remain
exact. Existing CPP registration is reused without a CMake/shared metadata edit.

The positive behavior is qualified Source C++ composition over genuine objects,
homes and providers. Actual naked/register/provider failure delivery is not
proved by direct symbols or an ordinary parent catch. Existing noexcept/CRT/OS/
nonlocal exits, Native ECX/RET4/register/FH3/SEH/fault/private-home/FP epochs,
secondary-terminal and exception-object identity remain unproved. Full CF/D0
callable transport, genuine host/current-slot/producer adoption and required
runtime/gameplay/activation remain separate. This complete library composition
claims zero new unique Original/vendor/duplicate body, ABI, runtime or activation
credit. Only the four leased Source/evidence files change; no Ghidra, caller,
table, global, ledger, guard, peer or install mutation occurred.
