# Resource-support access through the actual lifetime manager

`resource_support_singleton_00b3e730` now accepts a borrowed
`SoundLifetimeAccess`, so raw owners can register through the caller's actual
`01090AA0` publication cell. The existing `SingletonLifetimeDomain&` overload
delegates to the same body. Neither route introduces an owner, manager, allocator
or publication domain. The role name remains provisional.

The four lifetime fields in `NativePhysicalBufferOwnerContext`,
`NativeSurfaceOwnerContext`, `NativeHardwareLayoutOwnerContext` and
`NativeD3d9ShaderLifetimeContext` are `SoundLifetimeAccess` values. Existing
aggregate initialization from a domain reference still converts implicitly;
raw callers pass their stable `void* volatile&` AA0 cell. The same support cell,
manager domain and owner services must remain alive through use and teardown.

`SoundLifetimeAccess::borrows_same_domain(SoundLifetimeAccess) const noexcept`
compares the stored semantic-domain and raw-cell pointers without resolving a
manager or reading either publication. Copies of a view share identity; different
cells with equal current values do not. Raw and projected views never compare
equal. `native_stream_clone.cpp` now uses this comparison instead of comparing
wrapper addresses, preserving legacy same-domain admission after the field type
change. Its physical-lock context remains projected, so mixed raw/projected
clone contexts fail the existing admission check before acquisition.

## Native ordering and exception ownership

The full `B3E730..B3E7EC` body is 189 bytes and 51 instructions, including its
plain `RET`. Its native entry has no arguments and returns the support pointer
in EAX. The new source overloads have explicit arguments and are not binary
replacement entry points.

| Native site | Required action |
| --- | --- |
| `B3E745` | Capture current `0108FEDC`; nonnull returns without manager access. |
| `B3E756`, `B3E75B` | First `415350` getter, then capture returned manager's section at `+10`. |
| `B3E76F`, `B3E775` | Enter captured section through IAT `CE2218`, then increment its `+18`. |
| `B3E779`, `B3E781` | Arm state 0, then recheck current support publication. |
| `B3E78C`, `B3E794` | Allocate eight bytes through `BF681B`, retaining that allocation. |
| `B3E79A`, `B3E7A3` | Arm state 1, then call `B61D50` on nonnull allocation. |
| `B3E7AC`, `B3E7B1` | Disarm allocation cleanup to state 0, then publish. |
| `B3E7B6`, `B3E7BB` | Resolve the second manager **before** reading the current registration argument. |
| `B3E7C2`, `B3E7C4` | Use the second returned manager as ECX and call actual `BD0C30`. |
| `B3E7CD`, `B3E7D2` | Decrement and leave the first captured section. |
| `B3E7D8` | Reload current support publication **after** unlocking. |

The source uses an inner `CapturedSoundLifetimeSection` scope. It has no
`release()` member; returning inside its scope would evaluate the volatile
publication too early. The built object confirms second-getter call at local
offset `8B` before argument load `90`, and guard destructor call `9E` before
final publication load `A3`.

Handler `CBEDB3` loads FuncInfo `DF75D0` and tail-jumps to `BF6B43`. The complete
FH3 record has magic `19930522`, two states and unwind map `DF75C0`:

| State | Next | Funclet and exact captured receiver | Action |
| --- | --- | --- | --- |
| 1 | 0 | `CBEDA8`; allocation loaded from adjusted EH EBP minus `18h`, pushed at `CBEDAB` | `CBEDAC` calls `BF65AC`; `CBEDB1` pops ECX and `CBEDB2` returns. |
| 0 | -1 | `CBEDA0`; ECX is adjusted EH EBP minus `14h`, whose `+4` retains the first section | `CBEDA3` tail-jumps to `411EE0`; rewrite guard profile `CE37FC`, decrement captured section `+18` and leave it when nonnull. |

A registration exception therefore retains the published owner and allocation;
only the captured lock is released. A subsequent warm call returns that owner
without retrying registration. Allocation failure occurs while state 0 is armed.
The source retains the constructor-failure free structure, but its recovered
`B61D50` leaf is `noexcept`: the compiled C++ path does not reproduce native
hardware-fault cleanup or mutable FH3 frame aliases. `/EHsc` C++ exception
cleanup is distinct from executing the original FH3 handler.

All 493 bytes across the body, cleanup leaves/handler, map, getter, registration,
constructor, deleter and profile were captured through identity-checked wrappers
and matched against installed PE bytes. Seven numeric direct/tail-call rows pass
`verify_report_calls`; two IAT calls are explicitly unproved by that gate. The
handler's `CBEDB8 -> BF6B43` jump is byte-verified separately because the captured
Ghidra snapshot has no function at the handler entry. Its `CBEDA8` funclet also
stops after the returning free call; root must include the saved `POP ECX/RET`
tail. This worker made no Ghidra changes.

## Validation and remaining integration

The strict Win32 build passed with eight native seeds verified before configure
and both CTests enabled and passing. One separately manifested `/MD` fixture
links solely against the complete rebuilt libraries and exercises:

- Actual raw manager creation, registration into its real pointer vector,
  balanced Win32 section/depth, warm getter with an empty AA0 cell, and explicit
  raw unregister plus the existing support scalar deleter.
- A throwing actual CRT validation during raw registration and a corresponding
  legacy callback failure: each observes the owner already published and depth
  one; afterward the owner stays published, the section is unlocked, no append
  occurred, and a warm call does not replay registration.
- The old domain overload's normal registration and side-effect-free copied,
  distinct and mixed borrowed-domain identity comparisons.

The fixture executes reconstructed source and real CRT/Win32 services; it does
not execute original FH3 or an active game renderer. Its raw-manager teardown
occurs after explicit owner removal/deletion and does not establish canonical
resource-support draining.

The captured base's canonical `BD0400` dispatcher lacks profile `D62B64`, whose
slot zero is `B61D60`. The follow-up needs a borrowed
`NativeResourceSupportStorage* volatile*` pointing to the same `0108FEDC` cell
in `NativeSingletonDeletionBindings`, then dispatch to the existing
`delete_native_resource_support_00b61d60(owner, flags, publication)`. That provider
unconditionally clears the current publication, writes `CE3818`, frees for flag
bit zero and returns the captured identity; it has no publication-identity gate.
This packet does not edit the dispatcher. Physical-lock diagnostics and
`NativeD3d9ShaderConstructionContext` also remain projected, separate frontiers.

`reports/native_resource_support_actual.json` records source and native hashes,
call sites and validation closure. Ignored `local/output/resource_support_inputs`
contains immutable native/base-source inputs. The final validated archive holds
the compiler-read source closure, libraries, fixture, tools and separately
identified modules actually loaded by that fixture. Build/hash agreement does
not establish original ABI, SEH, game or visual parity.

## Integrated validation at 7cbd532e

Shared legacy and actual-AA0 resource-support source checks passed, including borrowed identity and postpublication validation failure retaining the owner. The native publication operands and captured-section exit before final reload were independently checked. Original getter/FH3 execution remains unproved. The combined strict Win32 build, eight seed checks and both CTests passed.
The four final-library probes,134 direct/tail rows, twelve saved/read-back
annotations and58 live/PE spans are retained in `local/checkpoints/7cbd532e/native-renderer-destructor-wave/validation.json`
(SHA256 `559269e15cbe10ee773e9bc8dcd8372695dc890a33b9254553319ff8e6a39fb2`). Full parent execution and application/gameplay
validation remain open.
