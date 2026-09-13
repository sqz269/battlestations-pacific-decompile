# Native deadline map subscript source adapter

`subscript_input_deadline_map_storage(actual_tree, key)` supplies the real storage
operation required by `NativeInputActionDeadlineCalls::call_004d6900`. It borrows
the actual 12h header and returns the actual node's float address at +10h. The
application provider can forward directly; this packet adds no owner, context,
shadow map, clock, or callback success substitute.

This is an equivalent source adapter over a consistent initialized native tree,
not the original STL ABI. General `4D3CD0` arbitrary-hint insertion remains
unimplemented. Original library names and Ghidra metadata are unchanged.

## Storage and reuse

The producer/lifetime and leaf evidence is in `NATIVE_INPUT_ACTION_DEADLINE_MAP.md`,
`NATIVE_INPUT_DEADLINE_MAP_LOOKUP.md`, and `NATIVE_INPUT_DEADLINE_MAP_STORAGE.md`.
The header retains its leading DWORD, head at +4, and count at +8. Nodes remain
18h allocations: left/parent/right at 0/4/8, signed key at Ch, raw mapped float
bits at 10h, color/nil at 14h/15h, and untouched padding at 16h/17h.

The existing B23020 lower-bound and B2F540 unique-insert C++ drivers were extracted
into `detail/native_tree_insert_storage.hpp`. Hardware wrappers preserve their
28h node layout, multiword comparator calls, iterator operations, and result
store order. The deadline specialization uses signed scalar comparisons, lazy
one-time search-key capture, and the existing 18h allocator/predecessor/link
providers. It does not copy a new STL implementation or reinterpret 28h nodes.

## Why the internally generated hint can use the unique driver

`4D6900` computes a signed lower bound before calling `4D3CD0`. A found equal key
returns its existing mapped address without inserting or changing its value. On
a miss, its copied key is strictly between the predecessor and candidate, where
those exist. Thus only the following five native insertion sites are reachable:

| Native hint path | Native link CALL | Same unique-search vacant site |
| --- | --- | --- |
| Empty tree | 4D3CEF | Left of the head |
| New minimum | 4D3D38 | Left of the minimum |
| New maximum/end hint | 4D3D7D | Right of the maximum |
| Interior, predecessor right child nil | 4D3DC5 | Right of that predecessor |
| Interior, predecessor right child non-nil | 4D3DDC | Left of the candidate |

In the last row, the candidate is the successor of that predecessor and has no
left child. These are exactly the leaf parent and side selected by the existing
unique driver. Native hint owner checks pass because the internally supplied
owner equals the nonnull tree. Interior predecessor checks pass because the
candidate is neither the minimum nor the sentinel. The extra unique traversal
therefore invokes no external service before reaching the same storage call.
The increment/equality/fallback paths in general hinted insertion cannot be
reached by this generated valid lower-bound hint.

The source API requires consistent count/head/link/nil/key ordering on entry and
no unsynchronized concurrent mutation. It accepts key pointers into live header
or node storage. It retains allocation/new-handler mutations after the common
parent/side capture, provided the callback keeps subsequently accessed storage
alive. It does not extend support to arbitrary malformed entry headers, native
private stack/iterator spill aliases, ABI register clobbers, or original EH/RTTI
identity. These limits do not remove any valid-entry `4D6900` hint path.

## Observable loads, results, and errors

`4D6916` loads the search key only when the root is non-nil; that value stays
captured through traversal. `4D692C` reloads the current head. `4D6935` reloads the
key before comparing the candidate; `4D693C` reloads it again into a private 8h
pair with mapped DWORD zero before allocation. The adapter makes those load
orders explicit. It returns the same mapped pointer and preserves every mapped
bit, including NaN payloads, for duplicates.

The shared link contract retains the unsigned `1FFFFFFE` count limit, real CRT
malloc/new-handler/free domain, and owning D69260 length-error transport with
19 payload bytes (`map/set<T> too long`, followed by its separate terminator).
The link operation captures its inputs before allocation, then reloads the head
and count when native code does. Allocation exceptions precede count/result
publication. The adapter then captures the returned owner/node, invokes the real
returning-capable CRT diagnostics corresponding to `4D6965` and `4D696F`, reloads
the owner's current head after the first diagnostic, and finally returns the
captured node plus 10h. It adds no successful fallback after an invalid result.

## Original ABI and callers

The read-only evidence body for `4D6900` is 128 bytes, through RET 4 at 4D697D
(inclusive end 4D697F, exclusive end 4D6980): ECX actual header, stack pointer to
signed int32 key, EAX mapped float pointer, no consumed incoming EDX. Original
B23020 uses ECX header and one stack key pointer, RET 4; B2F540 uses ECX header,
stack result pointer and pair pointer, RET 8. Source policies are not native args.

All four direct caller bodies were inspected: `4D8CD0` (16 calls) and `4D92B0`
(two calls) use E18A7C; `4D9420` and `4D9480` (two calls each) use game+5C8. They
pass pointers to actual local/argument DWORD keys and consume writable float
addresses. Game clock arithmetic stays in those callers. The report records all
22 caller CALL rows and the complete driver CALL rows with verified membership.

## Validation and remaining integration

Strict MSVC Win32 build passed with /W4 /WX, both existing CTests passed, and all
eight native seeds matched. The unchanged hardware fixture passed 8,582 behavior
words, 56 native and 56 source output stores, and five real CRT throw events each.

The one ignored deadline fixture executes pinned original `4D6900`, `4D3CD0`,
and reached `4B7520` instructions. Its `4CF010` boundary forwards to the real
already verified source storage layer; this is a driver differential, not an
independent native-link/EH comparison. The prior 492-byte link equivalence proof
and existing hardware native differential cover that reused kernel separately.

It passed 15,055 compared words over 120 permutations including INT_MIN, -1, 0,
1, and INT_MAX; all five reachable hint paths; 605 insertions, 600 duplicate
address/value checks, 602 key aliases, two actual new-handler calls, one returning
CRT diagnostic, and two throwing error cases per run. Raw node/sentinel allocation
and free counts were 730/730 in each run. Fault injection changes the head to the
new allocation to reach the post-insert diagnostic; the real returning handler
repairs head/link storage before ordinary cleanup. This intentionally tests a
callback-created invalid intermediate state and does not claim malformed entry
support. A separate new-handler mutation verifies copied-key retention and current
count reload; another throws before publication. The owning length error is
checked through the real source transport.

Ignored runners accept `-CoreWorktree` and link only that tree's archive/headers:
`local/run_deadline_lookup_differential.ps1` and
`local/run_hardware_insert_storage_regression.ps1`. Both probe executables embed
a manifest. Reports retain archive, fixture, and log hashes. No permanent tests,
Ghidra writes, SDK operations, game launch, or gameplay validation occurred.
Primary application composition still must forward its required deadline call
to this adapter and bind the already recovered static map lifetime and callback.
