# Authored environment fog source

`copy_authored_fog_0078caa4` reconstructs only `[0078CAA4,0078CCE0)` inside
`0078C9B0`. It reads the actual authored block, writes the actual environment
regions, and updates that environment's existing private `SystemFogOwner`.
The same storage is exposed to `apply_environment_fog_0078d076` through
`MutableEnvironmentFogFields::read_only()`. Neither view owns copied state.

This is a new MSVC Win32 C++ interface. The enclosing native method has ECX
environment, one stack authored pointer, and `RET 4`; this interior has live
ESI authored source and EDI environment and continues into unrelated work at
`0078CCE0`. It is not a callable native entrypoint or a whole-method replacement.
Names and the interpretation of authored fields as fog remain descriptive
hypotheses. The addresses, offsets, instructions and call targets are recovered
evidence.

## Ordered writes

All offsets below are hexadecimal. The first three groups use one integer
read/store pair per DWORD, in forward order. They are neither float conversions
nor snapshot/memmove copies; an earlier write can change a later source read.

| Native start | Authored bytes | Environment bytes | Behavior |
| --- | --- | --- | --- |
| 0078CAA4 | 80..8F | 34..43 | Four raw color words |
| 0078CACB | 118..127 | 84..93 | Four raw underwater words |
| 0078CAFB | 90..CF | 44..83 | Sixteen raw directional words |

The next eleven copies each execute `FLD m32; FSTP m32`, including NaN quieting
and x87 exception effects. The table gives the copy order and eventual private
owner field; those private setters run later in a different order.

| Authored offset | Environment offset | Private owner offset |
| --- | --- | --- |
| D0 | 08 | 68 |
| D4 | 0C | 6C |
| D8 | 10 | 70 |
| DC | 14 | 78 |
| E0 | 18 | 74 |
| E4 | 1C | 7C |
| 104 | 2C | 80 |
| 108 | 30 | 84 |
| 10C | 20 | 88 |
| 110 | 28 | 8C |
| 114 | 24 | 90 |

After the first `D0 -> 08` store at `0078CB97`, `0078CB9A` captures the private
owner from environment+B4. The following ten scalar copies still run before
`0078CBFB` consumes that captured owner in `00B84C40`. Moving the read next to
the call would change its ordering relative to those x87 accesses. The host
implementation captures the same live projection at that boundary and delays
its null-owner check until the first setter.

Four calls to `00B84FA0` then transfer environment+44 records with indices 0..3.
Each iteration reloads the actual B4 projection slot at its consumption point.
Eleven scalar calls follow in environment-offset order
`18,14,08,0C,10,1C,2C,30,20,28,24`. Each preserves
`FLD environment scalar -> reload B4 -> FSTP argument DWORD -> raw setter`.
The scalar setter addresses are `00B84D30,D40,D00,D10,D20,D50,D60,D80,DC0,DE0,E00`.
The eleven values are consequently converted once during the environment copy
and again when they are read for the private owner. There is no scaling or clamp.

There is **no private underwater setter** in this fragment. Authored+118 is
copied to environment+84, and that same storage is available to the later camera
apply fragment. The private owner's underwater field is untouched here.

## Actual bindings and lifetime boundary

The caller supplies at least 128h readable bytes starting at the actual authored
block, mutable references to the four actual environment regions, and a reference
to its actual B4 projection slot. The slot uses the established host representation
`SystemFogOwner::fields_08`; it maps back to that same concrete owner without a
registry or second fog state. A standalone diagnostic `SystemFogState` is not a
valid owner. These objects and references must remain live throughout the call.

The host rejects a null source or short span before writes. It checks null owners
only when they would be consumed, retaining previous writes and x87 effects.
These host errors do not reproduce native access violations or establish an
unmasked floating-point exception ABI. Success leaves the supplied error string
unchanged, following the existing environment-apply convention.

The enclosing environment construction/destruction is evidence-only:

- `[0078DB40,0078DB6D)` requests 94h bytes from `00BF681B`, calls the concrete fog
  initializer `00B84E50`, and publishes its result to environment+B4 at `0078DB67`.
  There is no extra retain between construction and publication; the constructor's
  initial reference is the environment's reference.
- Environment vtable D04390 begins with `00BD30E0,0078D6A0`. Its deleting wrapper
  calls `0078D5D0`; within `[0078D5F5,0078D621)`, the nonnull B4 owner is decremented
  through `InterlockedDecrement` at `0078D609`, destroyed through its vtable+0
  when the result is zero, and only then is B4 cleared at `0078D61B`.
- This interior merely borrows that owner. The environment constructor,
  destructor, sky/water owners, whole authored type and surrounding loader calls
  have not been reconstructed by this packet.

At world caller `004DF87F`, authored input is the already available object at
world+5FC plus 990h; `004DF810/004DF823` guard that object before this path.
Another caller, `0067F55C`, passes stack storage initialized by `004CB420` and
later destroyed by `004C6520`. Those observations establish concrete source
provenance, not a recovered schema, constructor defaults or independent lifetime
implementation. `00503050` is another recorded callsite in `00502480`.

## Validation and remaining boundary

The audit records the installed executable hash and current live Ghidra/disk
hashes for the 572-byte fragment plus the bounded allocation/publication and
release fragments. Live queries verified project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base before each batch.
Assembly, rather than inferred C++ float expressions, establishes the operation
order and private-owner capture boundary. No Ghidra or ledger writes were made.

A single ignored local fixture executes the original interior and its actual
native leaf setters, mapped together at a new image base. These paths need no
absolute relocation or import hooks. The only byte change is the explicit
continuation boundary `0078CCE0: D9 -> C3`; the 572-byte owned span is unchanged.
The fixture saves the live integer registers required by its host call boundary.

It compares all owner bytes and all environment/source backing bytes with the
host result, excluding only the B4 owner-pointer versus fields-pointer ABI
representation. Its two inputs use separate source storage and overlapping
`source = environment - 74h`; the latter includes forward write/read propagation
across copied groups and scalars. Eight x87 control words cover all four rounding
modes at 24-bit and 64-bit precision, with infinities, signed zero, quiet/signaling
NaNs and subnormals. All 16 comparisons match, including x87 exception flags and
stack status. The fixture also checks identical mutable/read-only field addresses,
unchanged private underwater bytes/reference count, and host error timing. It
does not simulate an exception handler changing B4 between scalar accesses.

The focused build compiles this new translation unit with the integrated owner
and allocator sources under MSVC 19.51 Win32 `/O2 /W4`; it passes without compiler
warnings. `./scripts/build.ps1` also passes both existing CTest checks on base
`b8e27e6`. This packet owns no CMake file, so the new source still needs integrator
registration in the main target. It is source-reconstructed, directly compiled
and native-fixture-tested, with no native ABI replacement or game/render proof.

Local reproduction artifacts remain in the worker worktree:
`local/build_authored_fog_check.ps1`, `local/authored_fog_native_check.cpp`, and
`local/authored_fog_native_check.log`. Exact hashes are in the audit report.
