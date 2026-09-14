# Native resource-container lifetime (CE)

`native_resource_container_lifetime.cpp` reconstructs complete default and game
resource destruction and scalar deletion. It closes the lifetime of the actual
44h/74h container populated by the established raw manager, parser and hierarchy
paths. The accompanying reference adapter supplies current-slot deletion for
these two container profiles and the existing fallback item.

| Entry | Complete bytes | Native contract |
| --- | ---: | --- |
| `00B88430` | 636 | ECX resource; base destructor; RET |
| `00718810` | 96 | ECX game resource; derived destructor; RET |
| `00B88760` | 30 | ECX resource, stacked flags; EAX captured address; RET4 |
| `00718C20` | 30 | ECX game resource, stacked flags; EAX captured address; RET4 |

These are new explicit-context C++ interfaces, not native ABI replacements.
Names remain descriptive hypotheses. The detailed byte, call, ownership and
fixture receipts are in `reports/native_resource_container_lifetime_ce.json`.

## Destruction order and retained state

The base destructor stamps `D63228`, then walks the current signed primary count
at +14, reloading the current backing pointer at +10 each iteration. It captures
one item and decrements item+4 atomically. At zero it captures that item's current
table and slot0 and dispatches through the explicit reference binding. It does
not guard a null primary element or clear the element slot.

It captures the resource's +8 name-header address before calling the actual
`4C1400` manager getter. `B801C0` removes that name from the cache. The mapped
pointer need not equal this resource; the mapped value is borrowed and is not
released. The manager's current-resource field is not cleared.

Next it walks the current signed hierarchy count at +20, reloading +1C each time.
Each captured nonnull record has its fields destroyed by `B88180`, then its 88h
slot is returned through the actual hierarchy pool also used by the parser.
The return takes the real critical section before reading the record's current
slab ID at +84. Null hierarchy elements are skipped.

The normal destructor then frees hierarchy and primary pointer backings, returns
the captured current name data through the current raw string pool, and stamps
the `BD30F0` base profile `CEB130`. Pointer/capacity/name headers, resource refcount
and metric remain stale. Normal pointer-array cleanup is inlined in the binary:
the source preserves its signed capacity branches, current header reloads,
wrapping byte-offset `ADD/JS` growth loop and data capture before the final count
zero. Calling the existing resize helper would lose that extreme-count schedule.

The game destructor first stamps `CFD8CC`, frees/zeros the borrowed checked-vector
triplets at +68, +58 and +48 in that order, and preserves the proxy words at +64,
+54 and +44. These classification arrays own no item references. It then runs
the complete base destructor. Both scalars finish destruction before testing
flags bit0, optionally free the allocation, and return its captured address.

The reference adapter accepts captured `BD30E0` for `D63228`, `CFD8CC` and fallback
`D631C0`, then rereads the owner's current slot4. Its finite source targets are
the base, game and fallback scalar deletions. Other profiles and stream operations
are forwarded. Its contained-item dispatch returns through the same adapter and
the supplied Camera/GroupParams/other complete item chain. Numeric native tables
are readable identity/data; they are never invoked as executable source callbacks.

## EH and saved analysis

The 68-byte region at `DFBA44` contains the four-action map and `DFBA64` FuncInfo:

| State | Next | Action |
| --- | --- | --- |
| 3 | 2 | `CC25EE`: hierarchy backing array, `B87B40` |
| 2 | 1 | `CC25E3`: primary backing array, `B87B20` |
| 1 | 0 | `CC25D8`: name, `41DD20` |
| 0 | -1 | `CC25D0`: base, `BD30F0` |

State2 is published before normal hierarchy backing cleanup, state1 before
primary backing cleanup, state0 after capturing the name-data pointer and before
returning it, and state-1 before base destruction. A throwing item callback does
not retry unprocessed item or hierarchy pointees. Only still-owned arrays, name
and base unwind. A second cleanup exception terminates in the source boundary.
The derived destructor has no independent EH region.

Five returning-free call overrides hid continuations in `718810`, `B88760` and
`718C20`. The repair cleared only those call overrides, recreated the three
functions without clearing bytes or changing callee no-return flags, restored
names/comments, saved the project and refreshed exports. The final live audit
after Ghidra restarted verifies all 272 executable instruction owners: 259
ordinary and 13 support. One unreachable three-byte alignment LEA at `B885BD`
is excluded; executed alignment instructions elsewhere remain included. The
ordinary spans total 792 bytes; five support bodies total 51 bytes. All bytes
match the installed PE, with 25 direct transfers and four indirect calls audited.

## Validation and limits

`scripts/build.ps1` compiled both pending CC and CE modules into `bsp_core` and
passed both existing CTests. This build used an ignored worktree-only
`CMAKE_PROJECT_INCLUDE_BEFORE` hook with deferred `target_sources`, because `cc7`
still owns `cmake/startup.cmake`. The tracked registry does not yet contain either
module. Remove this cached hook when tracked registration becomes available.

One controlled-child fixture copies the four complete original ordinary bodies,
relocates 20 direct and 11 absolute operands, and compares four source/original
pairs (eight executions): default/game containers with scalar flags2 and flags1.
Each resource is populated through actual raw manager/root parser dispatch with
GroupParams, Camera and fallback items, a duplicate owned GroupParams reference,
two parsed hierarchy records, a null hierarchy element, and an owned cache key
whose borrowed mapped value deliberately points elsewhere. It checks callback
order, name-only erase, stale owner headers, derived borrowed-vector clearing and
return of all slots to the actual hierarchy pool. Two additional source cases
release full containers through the actual resource-slot reference path.

One source failure case throws at the camera zero-reference callback. It verifies
the exact retained item/hierarchy/cache obligations and EH-owned cleanup before
explicitly cleaning those retained obligations. The original EH path is guarded
out. The fixture initially expected the pool destructor to reset its slab count;
`B18470` proves that field remains stale after freeing/unlinking. Correcting this
fixture expectation made the full run pass without changing CE source.

The existing CC controlled-child fixture also passes against this CMake-built
library, with no separately compiled CC object. Original paths share established
source dependencies and item terminals; fixture item tables restore the native
profile before invoking each shared terminal. Original pool return uses the same
actual storage with real Windows critical-section and atomic functions.

These checks do not prove original ABI/FH3/SEH/fault behavior, private stack
aliasing, all callback mutations, allocation failure, lazy pool recreation,
extreme malformed counts, native token startup or the remaining item terminals.
Tracked registration, incoming main corrections/review, full executable admission
and gameplay validation remain open.
