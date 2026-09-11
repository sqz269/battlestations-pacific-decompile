# Sound system destruction

Addresses: `00A883B0`, `00A882C0`, `00A817D0`, `00A816B0`, `00A7B9C0`,
`00A7E240`, `00A85AE0`. Names are hypotheses, not recovered symbols.

The reconstructed shutdown now composes configured-effect release, retained
pointer cleanup, active-entry release, the existing listener updates, resource
destruction, EventSystem release, member storage destruction and singleton
unregistration. `src/sound_shutdown.cpp` uses the existing canonical owners;
these are new C++ interfaces, not the original object layouts or binary ABI.

| Native routine | Coverage | Original ABI |
|---|---|---|
| A883B0 derived scalar destructor | Complete wrapper | ECX owner, flags byte in stack word, EAX old pointer, RET4 |
| A882C0 derived destructor | Complete ordered sequence and base unwind in documented domain | ECX owner, RET |
| A817D0 base scalar destructor | Complete wrapper | ECX owner, flags byte in stack word, EAX old pointer, RET4 |
| A816B0 base destructor | Complete member sequence and remaining-member unwind in documented domain | ECX owner, RET; **not** a vector-deleting destructor |
| A7B9C0 retained pointer cleanup | Complete sequence over explicit reference/virtual operations | ECX owner, RET |
| A7E240 sound close | Complete sequence for D5B000/D5B44C manager and D5B210 resource profiles | ECX owner, RET |
| A85AE0 resource scalar destructor | Complete wrapper over an ownership-slot projection | ECX resource owner, flags byte in stack word, EAX old pointer, RET4 |

## Ordering and ownership

A882C0 installs D5B44C and gets the lifetime manager **before** reading F8BBCC
for unregistration. It rereads that global for the deleting virtual call, and
clears it only after that call returns. A7F560 then releases configured groups
and the additional handles at 144..154. A7E240 closes playback and resources.
Finally A882C0 frees nonnull field174, clears it, and calls A816B0. The missing
instructions at A88338..A88344 contain the field174 clear.

A7B9C0 visits indices 0, 1, 2. For each nonnull pointer74 it calls virtual8 with
zero, reloads the pointer, releases that current reference and clears the slot
after the release callback. It then releases pointer80 and clears that slot.
The `NEG/SBB/TEST E19311` sequence is a nonnull test, not a pointee type mask.
The identified producer is A7F2F0: A7F50F assigns its derived channel into74,
and A7F519 assigns the source record's sample into80. Inside 004ED910,
004EDDCA acquires that sample through A83FD0, 004EDDE4 publishes it in the
scene record, and 004EDE43 passes the record and index to A7F2F0. Indices0/1/2
construct D5AD58/D5ADA0/D5ADE8 via A7DA40/A7DB80/A7DCD0. Their additional
14h records at5C own pooled names and sample references, released through
4C7FA0 before base channel cleanup. Base-only deletion would omit them.
These variants remain unreconstructed; the required virtual/reference host
must supply their real lifetime behavior. This producer proof does not claim
an exhaustive absence of other writers.

A7E240 drains the current active-entry array using release/clear **before**
the count decrement. Its virtual4 calls at A7E332 and A7E3C2 each pass a fresh
identity matrix and a by-value zero velocity. There is no input delta time;
the existing update implementation samples the clock. The second call reloads
the current manager profile. A7E3D1 deletes resource54 with flag1. EventSystem48
is read afterwards, and A7E3DC calls its release virtual with explicit self on
the stack. Only result2B triggers two-output memory statistics at A7E3ED.

The native close leaves 44, 48 and 54 unchanged. `SoundResourceOwnerSlot` keeps
the pointer word separate from allocation ownership: resource cleanup can still
find the current cache through 54, then the allocation is freed while the old
word remains visible. That dead word may be compared but never dereferenced.
The canonical slot must not be reset or reassigned during its own resource
destructor; supporting arbitrary native field replacement requires an actual
owner/allocation interface. Repeated destruction of the same owner is invalid.
The base scalar destructor A817D0 does not touch resource54 at all. When freeing
the C++ manager it detaches that convenience ownership first, leaving any live
resource allocation and its references untouched, as the native base path does.

A816B0 installs D5B000 and destroys members in this order:

1. System DSP pointer buffer134 (no individual DSP release).
2. Configured group buffer128 and each record's storage.
3. Listener ownerA4, including retained listener references.
4. Class table98 and active-entry table8C.
5. Pointer80 array in reverse order through 004C3810 semantics.
6. Pointer74 array in reverse order through 00524180 semantics.
7. Type buffer38 and each record's storage.
8. Singleton base A7B230, which unregisters the **current global** owner.

Container storage is standard C++ storage except the listener worker's actual
pointer table and listener records. Logical native capacities survive teardown;
dead native array pointer/header bytes are not all represented by std::vector
or std::string. This does not claim allocator ABI or freed-buffer observability.
Configured DSPs remaining in the system buffer belong to the released FMOD
system. Clearing that pointer buffer does not independently release those DSPs.

## Exceptions and evidence boundaries

FuncInfo DEC3DC has one unwind state, via CB6240 to A816B0. DEBB18 has eight
states at DEBB3C; CB5B00..CB5B78 resolve to singleton base, types, pointer74,
pointer80, entries, classes, listener and groups in reverse construction order.
The base sequence skips the member whose destructor has already begun and
destroys the remaining members. Existing destructors own their inner cleanup.
A second exception during C++ unwinding terminates. This is a C++ exception
projection, not proof of the original MSVC SEH ABI or asynchronous exception
behavior. Required reference-release callbacks are nonthrowing.

The false no-return overrides after CRT frees were repaired without changing
callee no-return flags. A816B0's decoded tail A816F3..A817CF still lies outside
its stored Ghidra function body ending A816F2. Its instructions and calls are
recorded separately; the call-site verifier must not count them as body-owned
calls. Raw/live byte equality and the final RET establish the decoding evidence.

## Validation and follow-up

See `reports/sound_shutdown.json` for exact build, fixture, byte hashes and
annotation receipts. No permanent tests were added. The installed-library
fixture uses a no-sound device; it cannot establish audible playback.

Dependencies are documented in `SOUND_CONFIGURATION_SHUTDOWN.md`,
`SOUND_LISTENER_OWNERSHIP.md`, `SOUND_SAMPLE_CACHE_SHUTDOWN.md`,
`SOUND_SYSTEM_UPDATE.md` and `SOUND_RESOURCE_CLEANUP.md`.

The executable's current phase5 host still reports A88770 sound initialization
as unimplemented. Production startup must bind shared globals, lifetime,
clock, VFS, FMOD and deletion callbacks together, including the actual alternate
engine destructor and the derived pointer74 channel profiles. Full game and gameplay
validation remain open; a successful local sound fixture does not prove them.
