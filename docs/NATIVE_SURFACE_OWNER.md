# Native D3D9 surface owner

This packet reconstructs the eight owner and borrowed-array entries below from
the installed Battlestations Pacific Win32 executable. Names are descriptive
hypotheses. The interfaces in `include/bsp/native_surface_owner.hpp` use explicit
references to actual shared owners; they are not binary replacement entry points.

| Native entry | Complete extent | Original ABI | Reconstructed action |
| --- | --- | --- | --- |
| `00B3F630` | `[00B3F630,00B3F7AE)` | ECX raw slot; stack COM, flags, kind word; EAX owner; RET 0Ch | Construct surface owner |
| `00B3CC80` | `[00B3CC80,00B3CCE4)` | ECX owner; stack COM; RET 4 | Overwrite and initialize COM binding |
| `00B3F4E0` | `[00B3F4E0,00B3F588)` | ECX owner; RET | Complete owner destructor |
| `00B3F5B0` | `[00B3F5B0,00B3F5D0)` | ECX owner; stack flags; EAX original owner; RET 4 | Destructor then conditional pool return |
| `00B3D510` | `[00B3D510,00B3D544)` | ECX owner; RET | Release COM for reset |
| `00B3D550` | `[00B3D550,00B3D588)` | ECX owner; stack device; EAX HRESULT; RET 4 | Recreate directly into owner COM field |
| `00B25630` | `[00B25630,00B25697)` | ECX array; stack pointer to target pointer; EAX 0/1; RET 4 | Remove first matching borrowed pointer |
| `00B27D60` | `[00B27D60,00B27D73)` | ECX renderer; stack owner; EAX 0/1; RET 4 | Remove from renderer array at +1B0Ch |

## Storage and actual shared owners

`NativeSurfaceOwnerStorage` occupies exactly 34h bytes of a 38h slot supplied by
the real `D3D9SurfacePool`. Its profile word is `00D619A0`; destruction installs
the refcounted base profile `00CEB130`. The pool's DWORD slab ID at +34h and the
three bytes at owner +31h are untouched throughout this owner lifecycle.

| Offset | Actual field |
| --- | --- |
| +00h | Native profile word, not a C++ vptr |
| +04h | Actual four-byte atomic signed reference count; constructor writes one |
| +08h, +0Ch | Same bits from one live `00D7A24C` load; installed initial value `3F800000` |
| +10h, +14h | NativeString length and buffer pointer |
| +18h, +1Ch, +20h, +24h | Format, width, height, multisample type |
| +28h | Flags; any bit in mask 110h affects the shared tracking counter |
| +2Ch | Owned COM surface pointer |
| +30h | Exact low recreation-kind byte; every nonzero value chooses depth |
| +31h..+33h | Unwritten bytes |
| Slot +34h | Live slab index owned by the canonical surface pool |

`NativeSurfaceOwnerContext` receives the actual renderer publication `00F8D394`,
canonical surface pool `0108DB00`, shared string pool reached through `00419CC0`,
shared lifetime domain `01090AA0`, resource-support publication `0108FEDC`, and
tracking counter `0108DAFC`, and scalar word `00D7A24C`. It creates no independent registry, COM owner, pool,
reference count, or publication slot. The caller composes the already recovered
resource-support deleter into the real lifetime manager's dispatch callback.

The renderer storage view preserves the distinction between renderer identity
and its raw array header at +1B0Ch. The header is the actual data/count/capacity
triple. Removal captures data and count, tests the unsigned cursor range before
reading the target pointer, stops at the first match, swaps the captured final
entry into the hole, then decrements the live count. It neither changes capacity
nor clears the stale final entry, and performs no retain or release.

## Callback order and ownership

Construction writes the object fields, performs an AddRef/Release pair on the
captured input COM pointer, invokes the binding initializer, then repeats the
pair on that same captured input. The initializer publishes +2Ch before AddRef
and reloads +2Ch for GetDesc after AddRef returns. It does not release an old
binding. Its GetDesc output is an uninitialized native descriptor; HRESULT is
ignored and format/width/height/multisample are copied from the output. A null
input clears those metadata fields and flags while preserving the kind byte.

Construction also creates two real pooled strings containing `Surface`, including
the second deep copy in a borrowed-COM diagnostic record. It calls the concrete
resource-support getter, returns the record string and then the temporary string,
and only then reloads flags to conditionally increment `0108DAFC`. The diagnostic
record does not retain its borrowed COM pointer. `NativeString::release_to` is
deliberately not used for owner destruction because it clears fields; the native
destructor returns the buffer with length+1 and leaves the string words stale.

The complete destructor installs the surface profile, captures the renderer
publication once, removes the first matching borrowed entry, and calls the actual
resource-support singleton. It then reloads +2Ch, calls Release when nonnull, and
clears the field after the callback. It reloads flags after that callback for the
conditional counter decrement, returns the current name buffer, and installs
the base profile. No parent texture count or reference is stored in this owner.

The scalar deleting destructor runs that complete destructor before examining
flag bit zero. An odd flag returns the original address to the actual surface
pool; an even flag leaves the raw slot allocated. It does not ordinary-free the
owner. If destruction throws, this function does not return the pool slot.

Reset release first performs a balanced pair on the captured +2Ch pointer, then
reloads +2Ch for the final Release and clears it afterward. Recreation uses the
device passed on the stack: kind zero calls CreateRenderTarget with lockable
FALSE, and nonzero calls CreateDepthStencilSurface with discard TRUE. Both use
quality zero, null shared handle, and the actual +2Ch field as the output address.
The HRESULT is returned without a success check, old-slot preflight, extra AddRef,
rollback, or metadata update.

## Exception evidence

The constructor's four-state unwind map at `00DF779C`, FuncInfo `00DF77BC`, and
handler `00CBEF13` unwind the diagnostic record string, temporary string, owner
name, then base. States link 3 -> 2 -> 1 -> 0 -> -1. The leaves at `00CBEF0B`,
`00CBEF03`, `00CBEEF8`, and `00CBEEF0` are byte-verified. There is no COM release
or pool return in that constructor map; the outer factory owns pool-slot unwind.

The destructor's two-state map at `00DF7768`, FuncInfo `00DF7778`, and handler
`00CBEEE3` return the current name then install the base, through leaves
`00CBEED8` and `00CBEED0`. A throwing COM Release receives no second Release and
does not reach the normal field-clear or counter update. The C++ guards/catch
paths encode those cleanup boundaries for C++ exceptions. Native exception
dispatch and SEH/access-fault compatibility were not executed or established.

## Validation and limits

One ignored fixture, `local/surface_native_check.cpp`, runs original copied PE
instructions for all eight entries with the original surface pool, resource
support, lifetime manager and native string-resize functions. Preparation verifies
the correct Ghidra project/program for every byte query and matches 87 code, hook
and data spans against the installed PE. The isolated image commits only verified
span pages, fills gaps with trap bytes, and applies 87 preimage-checked address
relocations. The complete preimages and artifact hashes are retained in
`reports/native_surface_owner_audit.json`.

The fixture uses real CRT allocation/free and actual Win32 critical sections.
At the three native string-pool ABI boundaries, it calls the real shared
`SizedStoragePool` implementation with native configuration. This verifies the
owner and original string helper against that pool; it does not compare original
string-pool instructions or lazy string-pool startup. ABI-correct deterministic
COM callbacks represent the surface/device interfaces; no live D3D device runs.

The native and reconstructed paths match 29 ordered events and eight complete
38h slot snapshots after normalizing profile and data/COM pointer identities.
The sequence covers input aliasing during AddRef, failed GetDesc with a complete
descriptor, reset replacement callbacks, both recreation branches with failed
HRESULTs that write output, name/flag mutation during destruction, borrowed array
swap removal, odd/even delete flags, actual free-stack return/reuse, null binding,
exact kind bytes, a changed live scalar word copied bit-for-bit to both fields,
untouched owner padding and trailing slab ID. Each path ends
with 25 bytes used in the shared string arena and three reusable string blocks;
the actual lifetime manager destroys the resource-support owner and the surface
pool unlinks from the shared allocator list.

The module and fixture compile under MSVC Win32 with `/W4 /WX /fp:strict` and
`/EHsc`; the repository's existing build and test pass separately. CMake, Ghidra
annotations and ledgers are left to the primary integrator. This packet does not
implement texture caches/factories, render-target groups, renderer-wide reset
iteration, intrusive zero dispatch, or the remaining surface vtable methods.
No allocation-failure, native exception, binary drop-in, game, or GPU parity claim
is made by this fixture.
