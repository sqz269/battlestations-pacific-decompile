# Actual scene-record map head allocation and construction

This packet adds complete ordinary Source for the actual raw head allocation
and map initialization below. The legacy `Seg6Host` projection remains separate;
the new code uses the existing concrete current-CRT allocator directly.

| Routine | Native range | Coverage | Original ABI | New C++ symbol |
| --- | --- | --- | --- | --- |
| `00468260` | `[00468260,00468297)` | complete | No arguments; EAX raw head; RET | `allocate_native_scene_record_map_head_00468260` |
| `00468E70` | `[00468E70,00468E9D)` | complete | ECX actual tree; two unread stacked words; RET8; EAX original tree; ESI preserved | `construct_native_scene_record_map_head_00468e70` |

The allocation helper requests exactly1Ch bytes through `00BF681B`, then tests
the returned address before zeroing link0, tests wrapping address+4 before
zeroing that DWORD, and tests wrapping address+8 before zeroing that DWORD.
It unconditionally writes color18=1 and nil19=0. Source preserves the complete
conditional-store footprint using DWORD address arithmetic; it does not
value-initialize a node or touch payloadC..17 or padding1A/1B. These are inline
placement-style null-address tests, with no placement function call or separate
cleanup scope in this routine.

The genuine existing `singleton_lifetime_allocate` service implements the
observed `00BF681B` schedule through current CRT malloc, `_callnewh(size)` retry,
and `std::bad_alloc` when the new handler declines. This is nonnull-or-throw:
there is no null-head recovery policy or stubbed allocation. Current CRT handler
identity and exception representation remain a host boundary, so private
original CRT exception/handler parity is not claimed. Native decompilation's
`void` helper return is corrected by the complete assembly's EAX data flow.

The map constructor keeps the caller's actual tree address across allocation,
publishes the captured head at tree+4 and writes captured head.nil19=1. It then
reloads current tree+4 separately before each store, in parent+4, left+0,
right+8 order, and writes count+8=0 last. No allocation word at tree+0, old-head
cleanup, header snapshot, extra validation or rollback is introduced. The two
native stack words consumed by `RET8` are never read; the new C++ interface
takes only the actual tree pointer.

The node layout agrees with the previously established actual1Ch scene-record
map node contracts: links0/4/8, stringC/10, mapped14 and color/nil18/19.
This helper does not initialize or give nominal meaning to those payload fields.
All current reads and writes use raw volatile storage, preserving the observed
reload behavior rather than caching the head across link stores.

| Containing function | Site | Native callee | Established contract |
| --- | --- | --- | --- |
| `00468260` | `00468262` | `00BF681B` | Stack size1Ch; EAX raw allocation; caller ADD ESP,4 at `00468267`; nonnull-or-throw. |
| `00468E70` | `00468E73` | `00468260` | No native arguments read; EAX raw1Ch head; RET. |

The four live allocator-helper callers were inspected: `00468903`, `00468E73`,
`0046F133` and `0046F19C` in their containing constructors. They use EAX as the
head and set nil19 before the self-link writes; ECX at the helper entry is not
consumed. Live xrefs found no references to `00468E70` itself. The source gap is
therefore closed without asserting that this constructor is game-reachable.

Live batches verify configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Complete original bytes are compared with the
installed PE. The report preserves prior function names and empty comments;
new evidence is added under the Ghidra write lock before save and export.
Strict MSVC Win32 `scripts/build.ps1` passed for Source revision
`4e7a7b6f8e44ece9a35066ef744a26cf804889ef`, including both configured existing
CTests (`reconstructed_math` and `tool_tests`). Native math differential tests
were not enabled in this worktree.
No new tests, candidate-specific original-byte differential, original ABI or
FH3/SEH compatibility, startup/game execution or gameplay proof are claimed.
