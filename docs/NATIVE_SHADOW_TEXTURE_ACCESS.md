# Native shadow texture access

This bounded reconstruction supplies concrete shader texture access for the
observed directional-shadow profile `00D5B5D8`, its shared target profile
`00D5B5E8`, and D3D9 texture wrapper profile `00D61948`. The C++ interfaces have
a new ABI. This is not a shadow-owner factory, owner destructor, texture creator,
shadow render/update implementation, or game validation.

`NativeShadowMapView` derives from the existing `SystemShadowMapOwner`. Its four
matrices, direction, and limits remain references to the existing actual owner
fields. It also references the actual raw global slot `00F8BBF0` and the actual
shadow owner's raw fallback slot `+384`. It owns no second pointer value, texture,
image, dimension, or retention count. `SystemShadowOwnerResolver` can return this
companion for the actual pointer currently retained at a light's `+174`.

`NativeShadowDepthTargetFields` borrows the actual target's width/height `+04/+08`,
enable/capability bytes `+0C/+0D`, and raw color/depth texture slots `+10/+18`.
`NativeD3D9ShadowTexture` borrows the actual wrapper's mutable reported dimensions
`+28/+2C`. These are the words changed by `00B3CEB0`; they are not independent
nominal or physical COM dimensions. Binding construction does not initialize any
native fields. All actual owners and companions must remain alive through use.

## Evidence and ABI

The exact bytes were verified against both the selected Ghidra program
`C:/Users/sqz269/bsp.gpr` → `/battlestationspacific.exe` and the installed PE.
The audit records raw preimages, SHA-256 values, source hashes, and target identity.
Some entries are not yet defined as Ghidra functions; disk assembly plus matching
live Ghidra bytes supplies the complete bodies. No Ghidra or ledger mutation was
made in this worker packet. The names below are descriptive proposals.

| Native span, end exclusive | Proposed behavior | Original ABI and return |
|---|---|---|
| `00A8FCF0..00A8FD0F` | get shadow depth texture | ECX shadow, borrowed EAX, RET or tail JMP `00A8FDB0` |
| `00A8FD10..00A8FD2F` | get shadow color texture | ECX shadow, borrowed EAX, RET or tail JMP `00A8FD90` |
| `00A8FD90..00A8FD94` | get target color texture | ECX target, EAX `[this+10]`, RET |
| `00A8FDB0..00A8FDB4` | get target depth texture | ECX target, EAX `[this+18]`, RET |
| `00B3CE50..00B3CE54` | get reported texture width | ECX wrapper, unsigned EAX `[this+28]`, RET |
| `00B3CE60..00B3CE64` | get reported texture height | ECX wrapper, unsigned EAX `[this+2C]`, RET |
| `00B3CEB0..00B3CEC1` | set reported texture dimensions | ECX wrapper, two stack DWORDs, RET8; no semantic result |
| `00A8FDD0..00A8FE24` | toggle shared shadow target metadata | ECX target, low byte of stack argument, RET4; preserves ESI |

The two profile spans `00D5B5E8..00D5B5F4` and `00D61948..00D619A0` were also
verified. The former has virtual `+08 = 00A8FDD0`; the latter has virtual
`+3C = 00B3CE50` and `+40 = 00B3CE60`. There are no unresolved external calls in
these eight bounded routines and no discarded returning allocator tails.

## Ordering and binding contract

Each shadow getter captures `00F8BBF0` once, reads target `+0C`, and only if it is
nonzero reads `+0D`. Both nonzero select the captured target's `+18` or `+10`;
otherwise the getter reads the shadow's current `+384`. The result is borrowed.
A selected null texture remains null; the existing shader consumer diagnoses it
before its dimension call. A null target is an unavailable host binding, since
native code would dereference it rather than select the fallback.

The resolver must bind the exact supplied raw identity. Lookup is bookkeeping:
it must not mutate native state, invoke native callbacks, retain, or manufacture
an owner. The host requires the observed target and texture profiles and returns
an explicit error for unsupported profiles or unbound nonnull identities. The
texture's current profile is checked again at its dimension dispatch. These host
errors are not reconstructed native error handling. Callers must supply fields
from the actual objects at the documented offsets; the API cannot validate an
arbitrary reference's provenance.

`SystemLightingConstants` already invokes shadow virtual `+08` separately before
width virtual `+3C` and height virtual `+40`. This companion preserves that path;
it caches neither selected texture nor dimensions across calls. Every dimension
is an unsigned DWORD, including values above `INT32_MAX`. The existing shader
consumer performs its established unsigned floating-point conversion.

`00A8FDD0` compares `+0D` before writing the exact supplied low byte to `+0C`.
Its branch uses that captured comparison. If capability was zero it returns.
Otherwise a nonzero enable value reads height then width, captures color `+10`,
and calls `00B3CEB0`; it then reloads height, width, and depth `+18` for the second
call. Disabling captures color and writes `(8,8)`, then reloads depth and writes
`(8,8)`. It does not recheck capability between calls. Each setter receives both
DWORDs by value before storing width and then height. This ordering matters when
valid memory regions overlap. No COM method or reference operation is involved.

## Validation and limits

One ignored native/host sequence compiled under MSVC Win32 C++17 with
`/W4 /WX /fp:strict /O2` and passed. It maps the installed PE, leaves all eight
routine bodies, relative calls, virtual dispatch tables, and returns intact,
and relocates only the two absolute `00F8BBF0` operands at `00A8FCF1` and
`00A8FD11`. It executes the original virtual `+08/+0C`, texture `+3C/+40`, toggle,
and metadata setter against isolated valid storage. It does not load or start
the game or create Direct3D resources.

The first color wrapper's `+28/+2C` overlap target `+08/+0C`. Enabling therefore
changes target height during the first metadata call, making the second call's
height reload observable. The same sequence checks changing the global target,
changing fallback identity, mutating metadata between texture selection and
dimension dispatch, unsigned high-bit results, disabling despite the first write
clearing capability, and a later capability-zero enable. Unsupported and unbound
host texture bindings fail explicitly in the same fixture.

The required existing project build and seed differential checks are recorded
separately in the audit. The focused fixture compiles the new source directly;
the worker leaves shared CMake integration to the primary integrator. The fixture
and build scripts remain ignored under `local/`, with content hashes and commands
in the audit. There are no new tracked tests.

This establishes the bounded field-access and metadata sequence under valid,
single-threaded bindings. It does not establish texture creation, global-target
initialization, resource destruction, reference counts, complete shadow owner
lifetime, matrix updates, rendered shadows, or game behavior. Binding failures
may leave preceding native-ordered writes applied, and are not transactional.
