# Native game ownership and fixed-step binding (R155)

`GameNativeGameRuntime` composes the existing native game constructor,
destructor, contact receiver and physics world into one stable source owner.
`GameFixedStepHost` can now run physics for that completed actual game.
Ordinary application startup does not yet construct or attach this owner.

## Native evidence

Live Ghidra `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr` matched
the original PE for 3,371 bytes: existing constructor 004DDB90 (1,751 bytes),
destructor 004DCF90 (1,559), scalar deletion 004DE270 (30), the 23-byte caller
fragment 00875DFA..00875E10, and two four-byte tables. No new native function
body is claimed by this packet.

| Native location | Source binding |
|---|---|
| CE7CB8, slot 0 = 004DE270 | Runtime's complete scalar-deletion table |
| CE78C0, slot 0 = 004D4CE0 | Canonical process's complete contact-report table |
| 004DDBB2, game+0 store | Primary source table, at original construction point |
| 004DDBC6, game+1Ch store | Contact table, before world callback publication |
| 004DCFB0, game+0 store | Same primary source table during destruction |
| 00875DFA..00875E0C | Reload game publication, read game+18h, simulate world |

Both tables have exactly one entry; strings immediately follow their native
entries. The primary slot's source thunk uses ECX for the game and one stack
flags DWORD, and returns the original pointer through the existing 004DE270
normal implementation. Flags 0x101 exercise the native low-byte bit-zero rule.

## Ownership contract

The runtime borrows externally supplied 71A0h storage and actual construction
and lifetime services. It owns stable table storage and persistent operation
frames. Private context copies supply the source table identities without
modifying the caller's contexts. Existing raw diagnostics retain original image
identities when their optional source-table bindings are null.

Construction and destruction must share game and grid publication cells and
the canonical Dyn process's engine/world contexts. The physics lifetime context
must also use the parent destructor's call-service object, as the existing
native cleanup adapter requires. These identities are checked before construction.

Construction and deletion are explicit one-shot operations. Source failures
retain partial native state and operation frames; destroying an unresolved
runtime terminates rather than guessing cleanup. Diagnostic retirement requires
the caller to resolve retained resources first and does not permit replay.

The fixed-step host borrows the runtime. Its physics method requires a completed
game whose actual publication still names that owner, then reads the current
world pointer at game+18h. It does not allocate a world or accept projected
game storage. Successful calls are counted separately. An unattached host keeps
the existing unimplemented-method record.

## Validation and limits

- Strict MSVC Win32 build and all three existing CTests passed.
- Existing constructor-family differential fixture: 33 paired cases, including
  four complete game-parent cases plus nested dependencies, with identical saved
  native/source observations. Its native image table identities remain intact.
- Existing destructor differential fixture: 52 paired cases, 3,537 boundary
  snapshots, **141,402,076 identical bytes**, plus four failure/replay checks.
- New focused composition fixture constructs an actual 71A0h game through
  004DDB90 and checks both source tables during constructor callbacks. The
  canonical engine/world are created by that constructor, and world+24h points
  to the actual game's report subobject. The fixed-step host runs four actual
  simulations through both solvers, with four observed initialized contact
  points after nonempty simulations and six contact-listener calls.
- Deletion through the primary virtual slot runs the real game destructor
  parent and native physics teardown. It releases the report vector once,
  clears its fields, and requests the game allocation's release once. Real
  process pool trimming and atexit end with zero tracked **physics** allocations;
  the fixture closes 101 handles not closed by the native lifetime path.
- The fixture controls 109 nonphysics construction boundaries and 56 lifetime
  boundaries, including mapped-arena allocation and the final game-free request.
  It does not prove complete game cleanup. A service-identity mismatch found
  during fixture composition now has an explicit pre-construction rejection
  check. Pre-construction simulation and foreign game publication are rejected.
- The ordinary application launch attempt was declined because another harness
  held the game slot. No process was stopped or replaced.

Remaining work includes composing the ordinary application's actual game
services, allocating/constructing the game at its native startup point, attaching
this owner to the running loop, and validating rendering and gameplay. Native
FH3/failure unwind, RTTI and full original entry ABI remain separate. R155 does
not reconstruct the rest of 00875BB0 or manufacture application admission.

Evidence, call checks, prior Ghidra annotations and artifact receipts are in
`reports/native_game_runtime_r155.json`.
