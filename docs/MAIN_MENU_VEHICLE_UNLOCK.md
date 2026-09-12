# Main-menu vehicle-class unlock provider

Addresses: 00584750, 00B67690, 00B65FB0, 00B66250.

`main_menu_vehicle_class_unlocked_00584750` implements the complete normal
00584750 caller using the existing actual LuaObject/NativeString owners and
the existing profile unlock predicate. The only required storage associations
resolve the current global game's Lua owner and profile. They do not implement
Lua, perform unlock decisions, or manufacture another profile.

| Routine | Inclusive native body | Original ABI | Coverage |
| --- | --- | --- | --- |
| 00584750 | 00584750..00584A21 | Two stack DWORDs; AL Boolean; RET8 at584A1F; incoming ECX unread | Complete normal caller |
| 00B67690 | 00B67690..00B676FD | ECX destination, stack source; EAX destination; RET4 atB676FB | Complete actual-storage assignment |
| 00B65FB0 | 00B65FB0..00B65FF0 | ECX LuaObject; AL Boolean; RET | Complete actual-storage nil predicate |
| 00B66250 | 00B66250..00B66264 | ECX LuaObject; AL Boolean; RET atB66264 | Complete actual-storage Boolean getter |

The main routine has an SEH frame with handler C6F9BC. The reconstruction uses
C++ scope cleanup for its normal stack objects; native SEH/longjmp behavior is
not claimed. There are no missing Ghidra function definitions or fall-through
gaps in these four bodies. Ghidra was verified as C:/Users/sqz269/bsp.gpr,
program /battlestationspacific.exe by the repository's live wrappers. The
worker made no Ghidra writes; annotation proposals are in the report.

## Decisions and access order

The second operand is read at58476D from ESP+AC after the frame and saved
registers. The first operand's ESP+A8 slot is never read; other accesses near
these offsets occur after pushes and address the SEH state instead. Incoming
ECX is overwritten at584783. Both callers pass a screen in ECX: 599039 inside
598B60 and59951A inside5993A0. Both push a class id and a derived Boolean;
the callee's RET8 establishes the two operands. The first is deliberately
retained and ignored by the new API.

Class -1 returns true before loading either owner. Otherwise584783 loads
[E188A8], adds1A0C and calls B67980. The existing globals object supplies
`VehicleClass`; its temporary is destroyed before constructing two default
objects. One default object is unused but retains its native lifetime. The
indexed result is assigned to the other, and its original temporary is then
destroyed. The assigned object's actual address is registered, so this cleanup
retains the same Lua stack value rather than creating another reference model.

| Current Lua entry | Result |
| --- | --- |
| VehicleClass[class] is not a table | false |
| Unlock is not an exact Boolean, or is false | false |
| Unlock is true and UnlockID is nil | true |
| Unlock is true and UnlockID is another non-string type | false |
| UnlockID is a string whose NativeString equals empty | true |
| Nonempty string | Existing 7FC4C0 predicate result |

The Boolean getter calls lua_toboolean without another kind query. IsString
requires exact LUA_TSTRING, so numeric UnlockID is not converted. B662B0 and
41E870 construct a NativeString for425850's actual-header equality. For a
nonempty value the code repeats B662B0 and constructs a separate NativeString;
it does not reuse the comparison buffer. Only then5848EA reloads [E188A8],
adds650 and calls7FC4C0. An allocation callback changing the current global
profile therefore affects this decision. The expression string is released
before the comparison string, then UnlockID, Unlock, the class entry, the
unused default object and VehicleClass. The empty literal at CE3A0C is reused
semantically as an empty C string.

## Shared Lua helpers

The three additions extend `native_lua_objects.hpp/.cpp`. They use the existing
14h LuaObject and4C8h owner definitions; no duplicate Lua interpreter, registry
reference model or storage layout was introduced.

B67690 first releases a bound destination through B66DE0 with remove-stack1
and then clears destination.kind. Only after that does it capture source
tracked10, kind04, index08 and owner00, in that order. Removing the destination
can shift a separately tracked source's index. It copies exactly those fields,
leaving opaque0C and padding untouched. A tracked source registers the actual
destination address in the existing owner slot, using wrapped DWORD
stack_offset+index and the current high-water/count fields. Native unchecked
50-slot/five-reference capacity remains a valid-input contract.

There is no self-assignment guard. With a sole tracked reference, self-copy
removes its Lua stack value, clears kind to0, then re-registers its unchanged
tracked/index metadata. This surprising native result is preserved rather
than described as a supported live reference. B65FB0 returns false for kind0
and other non-reference kinds; kind2 compares lua_type to LUA_TNIL. B66250
unconditionally calls lua_toboolean at the current owner/index and normalizes
the result. Lua itself is the existing linked Lua5.1.1 library.

## Profile ownership and remaining limits

The provider borrows `ProfileResetState::unlock_state` by const reference and
calls `is_unlock_expression_satisfied_007fc4c0`; it never copies or repopulates
any unlock container. The established predicate tries mission completion,
saved unlocks, pending unlocks, positive named counters and content IDs, in
that order, for each space/comma-delimited token; the first matching source
and token win. Empty-token expressions are false. Its evidence, producer
offsets and native ABI are in `PROFILE_UNLOCK_PREDICATE.md`.

That existing predicate is a semantic projection: its C++ maps, sets and vector
are not native profile-tree storage. `ProfileResetState` owns this same
projection today. Mission completion is synchronized from MissionProgress by
the existing persistence path, so this provider does not establish coherence
with later changes to a separate native progress owner. The old predicate's
token allocations/comparator implementation and suppression of the native
256-byte buffer overflow are inherited; its internal pool/CRT effects are
not reconstructed anew. A storage provider must bind the actual chosen game
profile and Lua owner, with the current profile reload at the documented point.
Global game layout, native profile ABI, executable reachability and gameplay
validation remain separate integration work.

## Validation

Strict MSVC Win32 compilation of both changed/new source units passed with
/W4 /WX /fp:strict. One ignored local fixture linked the parent's existing
bsp_core and real Lua5.1.1 libraries, with /MANIFEST:EMBED. It passed the Lua
type branches, class-1 bypass, pending-unlock expression, false/empty-token
expressions, current profile switching on the second allocation, reverse
string cleanup while all four Lua references remain live, final stack/refcount
cleanup, assignment with source-index shift, unchanged opaque/padding bytes,
and native self-assignment behavior. It is not an original-instruction or
installed-game comparison. No permanent tests were added.

`reports/main_menu_vehicle_unlock.json` enumerates every native CALL in the
four implemented bodies, including all cleanup arms, and the two caller sites.
All37 rows passed `verify_report_calls.py`. The complete `scripts/build.ps1`
build passed and its one enabled existing CTest (`reconstructed_math`) passed.
This includes the shared Lua helpers. The new provider was compiled and linked
in the focused fixture; its CMake registration belongs to the integrator.
