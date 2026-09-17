# Raw player-profile construction and reset, R106

Addresses: `007FEE20`, `007FDB20`, `00436710`; composition at `004DDB90`.

The game constructor now calls a concrete raw player-profile constructor and
reset on game+`650h`. Its former `call_007fee20` external boundary is removed.
The new code operates on the observed `F8h` slot and its actual string/container
headers. The earlier `ProfileResetState` projection remains available to its
existing callers; it is not used by this raw construction path.

This supplies the normal parent bodies, not a complete application profile
owner. Mission-progress, remaining collection services and settings binding,
native exception cleanup, whole-game lifetime and application startup remain
open. There is no default successful substitute for those services.

Files: `include/bsp/native_player_profile_owner.hpp`,
`src/native_player_profile_owner.cpp`, the native game construction pair, and
`reports/native_player_profile_owner_r106.json`.

## Evidence and coverage

The existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was verified.
All four native bodies match the installed PE: game parent 1,751 bytes, profile
constructor 409, reset 986, and empty-name helper 27. Nine DWORD constants and
three literal spans bring the comparison to 3,233 bytes. All 83 direct CALL rows
are checked against their containing functions and live instructions.

The reset listing skips seven bytes at `007FDE59..007FDE5F`. They decode as an
unreachable `LEA ESP,[ESP]` alignment instruction after an unconditional jump.
They are preserved and classified in the report, not mistaken for missing logic.
No listing or function definitions are modified in this packet.

`007FEE20` and `00436710` previously had reviewed names but no reconstruction
records: two new unique function addresses, 436 bytes. `007FDB20` already had
the projected reset implementation, so its new raw body adds no unique native
address. The game parent receives composition evidence only. Existing Ghidra
names/comments are retained, evidence appended under the write lock, the project
saved, names/comments read back and exports refreshed.

## Preserved construction and reset behavior

Both profile entries use ECX for the actual profile and plain `RET`; the
constructor returns the same pointer in EAX. `00436710` similarly constructs and
returns an actual 8-byte empty-name header through the existing `0041E870`
primitive. Source interfaces carry explicit providers and are not binary entry
replacements.

The constructor sets its profile table, initializes two string lists, one
lobby-filter list, five tree sentinels, four string headers and two checked
vectors, then calls the full raw reset. Unlisted bytes survive. It does **not**
initialize the progress pointer at +`64h` before reset inspects it. The caller
must supply a valid prior pointer or null; ordinary native startup supplies a
zeroed game allocation. XUID, version, byte +`58h`, opaque headers and unlisted
padding are not invented or zeroed.

Reset preserves the native order:

1. Construct/copy/release the temporary empty save name; write the player and
   display strings, voice and difficulty values.
2. Capture and destroy the old progress owner, free the captured pointer,
   clear +`64h`, then allocate/construct its replacement. Null allocation remains
   null; no successful owner is manufactured.
3. Clear the three string-set roots, counter root, checked string vector and
   transient root with the original head reloads. Both returning vector
   validation calls preserve the original captured endpoints.
4. Set nose-art defaults, clear the counter tree again, construct `RANK`, obtain
   its mapped value and write one. Release the originally captured key buffer
   with the current temporary length even if the map service changes the header.
5. Clear the first string list, reset lobby-filter scalars, detach/free old lobby
   nodes using current head comparisons, then allocate nine nodes: five gate
   bytes set, four clear. Link each allocated node only after the checked count
   increment, using the head captured before the allocation call.
6. Reset score/drop-rate values, clear the content list/mask, then call control
   and gameplay settings resets on the same borrowed canonical settings address.

The existing raw string primitives are used directly. Known allocation,
string-subtree, checked-vector, gate-node, checked-count and CRT services have
concrete bindings to their existing source implementations. Other library and
subsystem services remain required methods with raw receivers. Literal and
settings pointer bindings cannot be reassigned through the source context.

The operation retains constructor and reset call sites, native unwind states,
temporary string storage, captures and pending allocation. The game operation
owns this nested operation. A source exception leaves the partial graph
available for diagnosis and rejects replay; failed operations require explicit
diagnostic resolution before retirement. Native FH3 member destruction and
production recovery are not replaced by this mechanism. The existing
`NativeStringStorage::release` noexcept boundary and its exception limitations
remain explicit.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. No permanent test
suite is added. A focused manifested `/MD /W4 /WX /fp:strict` probe runs the four
copied original bodies together against the new source composition.

Six cases match **407 ordered observations and 18,601,896 bytes** across six
binary pairs. The four game-construction cases cover zeroed storage, patterned
storage with self-name alias and a controlled prior progress owner, null game
and profile allocations, and changed game publication/rank header. Two repeated
resets cover freeing the nine existing lobby nodes, rank-buffer capture across
header changes, null progress allocation, and two returning validators that
change vector fields while the parent forwards its earlier captures.

Observations include the full `71A0h` game, supplied publication cells, `4000h`
allocation arena, `C0h` settings image and semantic call arguments; the existing
world boundary also captures its full descriptor. Private-stack addresses are
normalized to values/content. Input/Lua/string primitives and checked list
growth are genuine shared source helpers. The remaining external bodies are
controlled boundaries, not independently validated native callees. In
particular, the malformed-vector case proves parent capture behavior only.

Source-only failures verify both nested and late retention. A progress
constructor failure leaves game site `004DDD4A`/state 10, profile constructor
site `007FEF9F`/state 13 and reset site `007FDC29`/state 1. Replay is rejected and
diagnostic retirement cascades to the failed child. A later Dyn failure retains
the already completed profile and partial published game. Original native
exception handlers are not executed by the fixture.

The ordinary application does not yet reach this constructor. All 53 application
objects and executable bytes outside two timestamp fields match R105. Runtime
is not rerun for this unreachable path; the last R105 smoke failed in the existing
renderer null-device path while an independent D3D probe returned `8876086A`.
There is no new application, gameplay, visual or native ABI proof here.

Tested artifacts are sealed before integration. The report separately records
the combined strict build, source/byte parity and annotation receipts.

## Follow-up work

Recover/bind the actual mission-progress owner and remaining profile collection
services, then compose raw settings and profile destruction. Finish the remaining
game member/array constructors and actual game allocation/lifetime. Use that
same owner for online profile callbacks and input actions; do not fabricate a
profile or game clock to bypass startup dependencies.
