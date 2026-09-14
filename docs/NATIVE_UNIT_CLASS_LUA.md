# Native entity ClassID, Name and Class binding

Addresses: `009292B0`, `00B67460`, `00B66790`, `00B675D0`.

| Routine | Inclusive body | Coverage | Original ABI |
|---|---|---|---|
| `bind_native_unit_class_lua_009292b0` | `009292B0..00929454`, 421 bytes | complete | ECX actual entity; stack signed class ID, C-string name; RET8 |
| `native_lua_set_integer_00b67460` | `00B67460..00B674B5`, 86 bytes | complete | ECX actual Lua object; stack NativeString key, signed int32; RET8 |
| `native_lua_set_cstring_00b66790` | `00B66790..00B667CE`, 63 bytes | complete | ECX actual Lua object; stack C-string key, C-string value; RET8 |
| `native_lua_set_object_00b675d0` | `00B675D0..00B67622`, 83 bytes | complete | ECX actual Lua object; stack NativeString key, actual Lua object value; RET8 |

These are descriptive source names, not recovered symbols. The explicit source
interfaces borrow existing actual objects and the application's mutable publication
cells. They add no Lua state, semantic host table, registry reference, fallback
world or provider stub. `B675D0` already had a reviewed Ghidra name but no source
body; that name is retained. The Lua5.1.1 C API is the existing library dependency.

The formal caller at `00955498` pushes the virtual name result from `00955489`,
then the signed DWORD from the current unit-class descriptor at `+70`, and passes
the original unit in ECX. The raw call at `007F218E` has no containing Ghidra
function in this snapshot. The retained census has 65 incoming references: two
to `9292B0`, 40 to `B67460`, 13 to `B66790`, ten to `B675D0`. `007C9805` and
`0081BF81` also lack containing functions. No reader treats these as evidence
for an invented function boundary. Every formal site has its actual body and
preceding argument setup retained in `local/unit_class_all_call_sites.json`.

The caller first invokes canonical `00927B40` to construct self. Its source API
receives the same `NativeString` lvalue at entity `+178`: the producer `00928A00`
formats entity `+174` and resizes/copies at `00928A36/54/6A`. The actual world
constructor produces the embedded Lua owner with `004DDE8A` LEA `+1A0C` and
`004DDE9B` call `B66BD0`. These agree with the existing canonical layouts in
`native_mission_entity_lua_self.hpp` and `native_lua_objects.hpp`.

`9292B0` constructs a zeroed native string header, resizes to seven preserving
bytes, and copies length+1 bytes from `ClassID` only when the returned data is
nonnull. It calls `B67460`, lowers cleanup state, and returns the allocated
length+1 block through the actual raw pool getter and return operation. It then
calls `B66790` with `Name` and the original C-string pointer. The same eight-byte
header is explicitly zeroed again before constructing `Class` at length five.
Only after that construction does `0092938E` reload `E188A8`, add `1A0C`, get
globals, find `VehicleClass`, and look up the captured class ID. `B675D0` assigns
the row using self as its target. Cleanup runs row, VehicleClass, globals,
Class string, self. Every Lua temporary keeps a stable actual address so the
existing stack tracking can move indices and remove exact pointer identities.

Each typed setter ignores the result of checkstack(2). `B67460` reads current
NativeString length/data after checkstack; null data uses the original empty
literal without changing length. It pushes the exact length, including embedded
NULs, and converts the signed input with `00B67491` FILD DWORD followed by
`00B6749D` FSTP binary64. All int32 values are exact in binary64. `B66790` uses
two C-string pushes; a null value pushes nil and a null key follows Lua's error
behavior. These setters have no kind gate. Owner is reloaded before each Lua
operation and target index is read after the pushes.
Actual field reads use volatile storage and explicit statements. The final
production object preserves length before data, and FILD before owner/state
loads before FSTP. Initial nonvolatile code passed value fixtures but allowed
load reordering; that object is retained separately and the corrected code was
rebuilt and tested again.

`B675D0` reads the target owner at `00B67605`, value index at `00B67607`, and
that target owner's state at `00B6760A`. It does not read the value's owner,
normalize a relative index, or check that both objects belong to one state.
This matters when a Name metamethod changes the published world: VehicleClass
and its row may be read from the new world, while SetObject still interprets
the row's numeric index in self's captured state. The retained native/source
fixture preserves this behavior: with empty initial stacks, new-world row
index 2 resolves to the just-pushed `Class` key in the old state, so self.Class
becomes the string `Class`. No cross-state repair is invented.

The report enumerates every direct call in the four owned bodies, all six EH
cleanup jumps, and the FH3 selector's tail jump with inclusive endpoints. The
root integrator formally defined and saved the selector after its raw-byte audit.
The selector `00CA71A0..00CA71A9` loads metadata `DDA610` and jumps to the original
CRT frame handler `BF6B43`. Its six-state map at `DDA634` is:

| State | Next | Cleanup thunk | Object |
|---|---|---|---|
| 0 | -1 | `CA7170` | self at EBP-5C |
| 1 | 0 | `CA7178` | ClassID at EBP-64 |
| 2 | 0 | `CA7180` | Class at EBP-64 |
| 3 | 2 | `CA7188` | globals at EBP-20 |
| 4 | 3 | `CA7190` | VehicleClass at EBP-34 |
| 5 | 4 | `CA7198` | row at EBP-48 |

Source explicitly changes state before each normal destructor call. Construction
failures before an object is armed do not introduce cleanup for that object.
Source C++ unwinding expresses completed-object effects; original CRT FH3/SEH,
Lua errors/longjmp, simultaneous cleanup exceptions and arbitrary aliases into
the native frame are separate unproved domains. Neither native FH3 nor source
exceptional cleanup was executed by the retained normal-path fixture.

Validation and exact artifact hashes are recorded in the accompanying report and
`local/unit_class_manifest.json`. The paired fixture executes all 653 original
owned instruction bytes with ABI adapters to existing actual Lua/string/pool
providers, then the corresponding source over real Lua5.1.1. Original handler
registration is deliberately unbound for this normal-path fixture. It checks
signed extremes, string/nil Name, normal row identity, publication mutation,
cross-owner index semantics, embedded NUL and null-empty NativeString keys,
kind-zero setters, stack balance, tracked reference cleanup and retained high
water. This evidence does not establish binary replacement or gameplay behavior.
