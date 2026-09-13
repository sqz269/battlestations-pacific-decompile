# Native mission entity Lua self object

Address: `00927B40` (`BSP_MissionEntity_LuaSelfObject`, existing correct name).

The source reconstructs the complete `00927B40..00927BE1` sequence, 162 bytes.
Native ABI: ECX is the actual canonical entity, one fresh `14h` output pointer
is passed on the stack, EAX returns that captured pointer, and `00927BDF RET4`
consumes the argument. The C++ interface instead borrows the same entity's actual
`NativeString` field at `+178`, the fresh output, and the actual mutable `E188A8`
publication cell. It does not reinterpret a semantic unit or construct a world.

## Captured owner and current key

`00927B63` reads `E188A8` once. The existing `NativeLuaStateStorage` is embedded at
that captured world's `+1A0C`; it is not a pointer field. The producer is the world
constructor at `004DDE8A` (`LEA ECX,[ESI+1A0C]`) and `004DDE9B` (`CALL B66BD0`).
This agrees with `kMissionLuaStateOwnerField` in `mission_lua_bindings.hpp` and the
existing mission-load offset. A valid constructed owner and live Lua state are
required; the getter has no fallback for a null world or closed state.

`B67980` constructs a globals object. `B67800` then reads the literal `thisTable`
at `CE7494`. Only after that lookup returns does `00927B92` form the entity's
`+178` key address for `B68100`. A Lua metamethod during the first lookup can change
both publication and key: the getter retains the first Lua owner, then reads the
changed key. There is no world reload, lock acquisition or entity-ID formatting.

The key producer `00928A00` reads u16 `entity+174` at `00928A23`, formats it through
`004260B0`, then resizes/copies into `entity+178` at `00928A36/54/6A`. The eight-byte
`NativeString` contains length at `+178` and data at `+17C`. `B68100` captures data
only, substitutes the existing empty C-string identity `0108FF2C` for null, and
uses `B67800`'s NUL-terminated lookup. Neither the network ID nor stored key length
is read by this getter. The source borrows the real key and does not snapshot it.

## Complete calls and cleanup

| Caller/site | Native target | Contract established from the complete callee |
|---|---|---|
| `927B74` | `B67980` | ECX embedded Lua owner, fresh output on stack, EAX output, RET4; globals kind1, untracked |
| `927B8D` | `B67800` | ECX returned globals; fresh output and C-string key; RET8; actual tracked stack object |
| `927BA5` | `B68100` | ECX returned thisTable; fresh output and actual NativeString pointer; RET8 |
| `927BBB` | `B67700` | Destroy thisTable temporary at `[ESP+08]` |
| `927BC9` | `B67700` | Destroy globals temporary at `[ESP+1C]` |
| `CA6ED3`, caller `CA6ED0` | tail `B67700` | Globals cleanup at `[EBP-20]` |
| `CA6EDB`, caller `CA6ED8` | tail `B67700` | thisTable cleanup at `[EBP-34]` |
| `CA6EF3`, caller `CA6EE0` | tail `B67700` | Completed output from `[EBP+04]`, only if bit0 at `[EBP-38]` is set; clear bit first |
| `CA6EFE`, caller `CA6EF9` | tail `BF6B43` | Original FH3 selector loads `DDA2C4`; required external runtime |

The actual `B67800` provider calls Lua checkstack, pushes the C-string, reads the
current object index, performs gettable, then captures the current owner and top
to register the output's actual address. Kind3 selects the globals pseudo-index;
other kinds use their current index. Lua stack capacity and fewer than 50 tracked
slots/five references per slot are original caller contracts. No universal defaults
or registry-reference objects are substituted.

`B67700` skips kind0. Otherwise it calls the existing `B66DE0` tracked release with
captured owner/index and remove-stack=1, then clears only the current kind. Releasing
the intermediate thisTable at stack index1 moves the surviving output from index2
to index1 and updates its actual tracked-reference record. The owner high-water mark
and stale pointer cells are retained. Globals cleanup is untracked and does not pop
the surviving value. The caller owns the final output and destroys it separately.

| FuncInfo `DDA2C4`, map `DDA2AC` | Cleanup | Coverage |
|---|---|---|
| state0 → -1 | `CA6EE0[25]`: conditional completed-output cleanup | Complete effects; original funclet/FH3 ABI excluded |
| state1 → 0 | `CA6ED0[8]`: globals | Complete effects; original funclet/FH3 ABI excluded |
| state2 → 1 | `CA6ED8[8]`: thisTable | Complete effects; original funclet/FH3 ABI excluded |

The native output-completed flag starts at zero. Globals success arms state1;
thisTable success arms state2. GetByNativeString must return before the output flag
is set. State changes occur before each normal temporary destructor. If temporary
destruction throws a C++ exception, source guards destroy remaining temporaries and
the completed output in the native map order. Partial construction belongs to the
existing callee's contract. Native FH3 execution, native stack aliases, cleanup
failures, Lua error/longjmp and hardware-fault unwinding are explicitly unproved.

## Validation and integration

The complete body has no stored membership gap. The initial `CA6EF9[10]` selector
query reported `no_ghidra_function`. Root subsequently defined/saved it from verified
bytes and named it `EH_MissionEntity_LuaSelfObject`; all ten direct/tail rows now
verify. The worker preserves the existing getter name/comments and proposes only
appended evidence in its owned name ledger. Before/after documentation differs only
in the synthesized stack-8 local name/type inferred after the handler definition;
exact old/new records are retained. All other fields and stored hashes are unchanged.

The retained incoming census contains 39 call-site references, rather than a new
claim of 39 distinct caller functions. Thirty-five have stored containing functions
and consistent entity/output argument setup. `0074B29B`, `0077B101`, `007C97E3` and
`0081BF6B` currently have no containing function; their raw direct-call evidence is
retained without attributing an invented caller. Older docs' 25-caller count is a
historical snapshot. `00928CDC` confirms the W base-killed caller's exact output ABI.

The minimal paired fixture executes all 162 original bytes with five explicit ABI
adapters to the actual existing Lua-object providers, real Lua5.1.1, real pooled
NativeString storage, and constructed borrowed Lua owners at the proven offset.
It checks current worldA/worldB with a null key, then actual Lua metamethod mutations
of key and world publication. It verifies EAX output identity, result value, retained
padding, moved stack index, tracked-reference membership, high-water behavior and
caller cleanup. The original FH3 handler is deliberately unbound in that fixture.
DoFile is a required external callback outside the exercised path and fails explicitly
if reached; no successful placeholder is used.

This provider can satisfy W's `construct_self_00927b40` by resolving the supplied
existing canonical alias to its same actual key lvalue and passing the application's
actual `E188A8` cell. No runtime/host binding or other worker file is changed here.
The report and `local/lua_self_manifest.json` retain exact source/compiler/library,
PE/live bytes, runtime relocations, inputs, outputs, failures and validation hashes.
Fixture and build evidence do not establish gameplay or binary replacement.
