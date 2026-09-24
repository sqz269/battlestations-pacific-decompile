# Raw scene registry storage

Addresses: `B83600`, `B83220`, `B81B90`, `B829D0`, `B82B40`, `B82E90`,
`B82C60`; compiler boundaries `B832B6`, `CC2350`, `CC2370`, `CC237B`.

These routines construct and destroy actual `28h` registry storage. They reuse
the existing raw `B82390` sentinel and `B82570` pair-fill leaves. They do not
use the logical `SceneNodeRegistry`, create owner arrays, or replace its
allocator with a separate implementation. Names are descriptive hypotheses.

| Routine | Complete extent / last instruction | Coverage |
| --- | --- | --- |
| `B83600` construct | 120B; `B83675 RET8`, length3 | Raw constructor and source exception cleanup |
| `B83220` vector construct | 150B; `B832B3 RET8`, length3 | Raw caller-cell driver; required library throw binding |
| `B81B90` allocate pairs | 85B; `B81BE0 CALL BF6885`, length5 | Exact allocation/overflow decision; source bad_alloc transport |
| `B829D0` destroy list | 72B; `B82A17 RET`, length1 | Full raw body including both missing free continuations |
| `B82B40` list alias | 5B; `B82B40 JMP B829D0`, length5 | Exact tail alias |
| `B82E90` destroy registry | 49B; `B82EBC JMP B829D0`, length5 | Full vector free/reset then list cleanup |
| `B82C60` vector cleanup | 42B; `B82C89 RET`, length1 | Full free/reset body |
| `B832B6` catch | 17B; `B832C2 CALL BF6885`, length5 | Native compiler-frame boundary; source catch/rethrow projection |
| `CC2350` handler | 10B; `CC2355 JMP BF6B43`, length5 | Missing Ghidra function; native FH3 boundary |
| `CC2370` unwind | 11B; `CC2376 JMP B82B40`, length5 | Existing native compiler-frame boundary |
| `CC237B` handler | 10B; `CC2380 JMP BF6B43`, length5 | Missing Ghidra function; native FH3 boundary |

The registry holds allocator byte0, untouched bytes1..3, embedded list at4,
embedded vector at10h, mask20h and active24h. The list is `{opaque0,head4,
count8}`. Its `Ch` nodes contain next0, previous4 and borrowed key8. The vector
is `{opaque0,begin4,end8,capacityC}`; its elements are eight-byte `{list,node}`
pairs. Opaque list/vector words and sentinel key8 retain their preimages.

`B83600` copies only the first allocator byte before allocating the sentinel.
It writes head8 then countC=0, captures that head for the actual local pair,
and constructs nine pairs. Mask20 and active24 become1 only after success.
The second allocator pointer is unused. A failed sentinel allocation propagates
without list cleanup; a later vector-construction exception destroys the
current embedded list before rethrowing. No borrowed-key release is invented.

`B83220` captures the incoming count before clearing vector4/8/C. Zero skips
allocation and leaves the caller count cell unchanged. Nonzero count above
`1FFFFFFF` invokes the required `B82DD0` length-failure entry. After allocation,
it zeroes only the low byte of the **current** caller count DWORD, reads that
cell twice for the unused fill slots, publishes capacity, then captures the
current caller pair pointer. Begin and end are written afterward. Existing
`B82570` copies the current pair's two words in native order, including overlap;
the driver finally publishes end. `B81B90` also allocates on count0 and rejects
counts above `1FFFFFFF` before multiplication or allocator invocation.

The caller supplies initialized, stable `NativeSceneRegistryVectorArguments`
cells and constructor pair/argument scratch. Scalar count is captured before
callbacks; the late pair pointer and overwritten count cell remain live.
Constructor scratch is a new source layout. Native saved-register, return,
EBP and private-stack aliases are not claimed. In particular `CC2370`'s native
`[EBP-18]` owner recovery is represented by the explicit source registry input.

List destruction captures the old first node, self-links the head's next,
reloads the current head for the previous self-link, compares the captured
first node with current head, then zeroes count. Before each free it captures
next; afterward it compares that captured pointer with the **current** head.
It finally reloads/frees the sentinel and clears head only after free returns.
Registry destruction first frees current vector begin14, clears14/18/1C,
then destroys the list. `B82C60` likewise frees current begin4 and clears4/8/C.

Live bytes equal the installed executable across571 owned code bytes and128
EH metadata bytes. Current listings omit `B829F8..B82A02`, `B82A0C..B82A17`,
`B82EA0..B82EA2`, and `B82C70..B82C72`; these continuations are recovered from
the pinned bytes, with exact instructions and required repair ranges in the
report. No worker Ghidra mutation was made.

Bindings require the genuine current `BF681B` allocator and matching `BF65AC`
free domain. The required, nonreturning `B82DD0` provider constructs the native
`vector<T> too long` length failure; it has no successful/no-op default. This
is a library exception-transport boundary. Native `B81B90` builds an exception
through `BF6340`, selects vtable `D6923C`, and throws with `E03CC0`; the source
uses `std::bad_alloc` with explicitly different CRT exception identity.

`CC2350` uses FuncInfo `DFB6A0`, two unwind states with no actions, and a
catch-all for state0 whose `B832B6` calls `B82C60` then rethrows. `CC237B` uses
FuncInfo `DFB724`, one state whose action `CC2370` destroys the embedded list.
Source catch/rethrow expresses C++ cleanup only. The reused fill leaf is
`noexcept`; native access-fault/SEH/FH3 handling is neither implemented nor
exercised by this source catch. No native CRT exception ABI is claimed.

Strict MSVC Win32 build and all three existing CTests pass. One ignored probe
compares copied original bodies with source using real allocator/free calls:
constructor storage and padding, allocation-time caller count/pair changes,
overlapping pair fill, free-time head mutation, vector reset order, and zero
allocation. It compares actual normalized storage and call-order observations.
Separate source-only checks cover allocation-failure sentinel cleanup and
overflow rejection; they are not original FH3 evidence. The probe uses the
existing raw singleton length-error transport as its required library binding.
No game process, whole scene-resource lifetime, general STL behavior or native
private-frame ABI is validated.
