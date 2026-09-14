# Native occlusion-query poll

`poll_native_occlusion_query_00b5fca0` reconstructs the complete original
`B5FCA0..B5FCD3` body over the actual query owner. It uses the same ECX-only
entry, plain RET and AL result as the native 52-byte function. It adds no owner
projection, storage allocation, dependency context or virtual-profile check.

## Body and caller contract

The routine captures the current COM pointer at owner+10h once. A null pointer
returns true without writing any owner word. A nonnull pointer receives exactly
one stdcall through its current vtable+1Ch:

`GetData(captured_query, actual_owner+0Ch, 4, 1)`

Only exact HRESULT zero stores DWORD 2 at owner+8 and returns true. S_FALSE and
errors both return false. Any samples written by GetData remain in the actual
owner on every result. The method does not inspect the old state, retry, wait,
retain/release COM ownership, clear the query pointer or translate an error.
An existing nonzero state does not suppress the call when this entry is invoked
directly.

The instruction sequence also preserves native incidental return behavior:
nonnull EAX keeps the HRESULT's upper 24 bits and replaces AL with the boolean;
null EAX is 1. Only AL is the semantic result. ESI is saved/restored, the COM call
cleans its four arguments, and no source or native EH frame is introduced.
Owner/query/vtable lifetimes and all reached reads/writes must be valid. No
complete owner type or allocation size is inferred from the accessed 14h prefix.

The original `D62AD0` profile's slot+10 contains `B5FCA0`. In the EndFrame loop
`B2DADB..B2DB14`, EBX is zero (initialized at `B2D8FC`). The loop reloads the
current renderer+19A0h array, captures each actual query pointer, and invokes
its current vtable+10 only when query+8 is zero. It increments an unsigned index
and compares the current renderer+19A4h count after each iteration. AL is ignored.
There is no wait/retry/lost-device gate inside that loop; enclosing EndFrame
gates still apply. The poll does not move caller scheduling policy into itself.

The installed PE and saved Ghidra bytes agree for the full poll, original
profile slots and caller loop. The body is already complete and needs no
returning-call or function-boundary repair. The saved generic prototype reports
`undefined ... (void)` while decompilation recovers the ECX input and boolean
result; this worker leaves any database prototype/comment correction to the
primary integrator and performs no Ghidra mutation.

## Existing source and integration

`native_occlusion_query_device_reset.cpp` already accesses the same actual
owner+10h COM word for release and reset. The new poll accepts the same raw owner
pointer. `d3d9_query.cpp` instead implements `D3D9OcclusionQuery`, a projected
owner with different offsets and its own lifetime. Its poll behavior is useful
prior evidence, but passing an actual native owner through that projected type
would address the wrong words. No projected source or reset API was changed.

The implementation is a literal MSVC Win32 naked assembly entry in
`src/native_occlusion_query_poll.cpp`, declared by
`include/bsp/native_occlusion_query_poll.hpp`. One source registration is appended
to `cmake/startup.cmake` under a short coordination lease. EndFrame can call this
entry with the actual owner after selecting its real supported query profile;
this packet does not implement the enclosing EndFrame dispatch or raw owner
construction/destruction.

## Validation and limits

The accompanying JSON report records immutable native inputs, source and build
artifacts, exact tool identities, and the focused probe result. Its validation
fields are authoritative for the final build and probe run.

The strict Win32 build passed, both existing CTests passed, all 52 linked entry
bytes matched the original, and all four original/source differential cases
passed. Four native spans totaling 164 bytes matched the saved program and PE,
including the caller's zero-register setup.

The single local probe links the built `bsp_core.lib`, compares the loaded source
entry against all 52 native bytes, and executes both that entry and an executable
copy of the verified position-independent native body. A synthetic stdcall COM
boundary checks the exact receiver/output/size/flags, one-call behavior,
S_OK/S_FALSE/error/null results, samples written on failure, owner mutations
during GetData, untouched adjacent words, and the complete returned EAX value.
Its four cases form one bounded original/source differential; it is not a
substitute production provider or a real GPU query test.

The probe is built with `/MD /link /MANIFEST:EMBED`. Runtime evidence identifies
modules actually loaded by that process. `bsp_core.lib` is statically linked;
its hash is a linked artifact identity, not a claim that the archive was loaded
as a runtime module. The source entry is checked after linking into the probe.
The 32-bit process resolves each reported module filename through an open file
handle before hashing: reported System32 DLL paths resolve to physical SysWOW64
files. Their on-disk PE machine values match the actually loaded I386 images.

Strict `scripts/build.ps1` validation is run after `verify-seeds` creates the
native reference header, enabling both existing CTests. No tracked test or
framework is added. Byte identity and this synthetic-COM differential do not
prove a running original game, a real driver result, renderer lifetime, original
vtable publication or an installed binary replacement. No installation or
gameplay validation is claimed.
