# Native game embedded-state destruction (R123)

Addresses: `0076A760`, `0076DB20`, `0076F000`, `007849C0`; parent binding `004DCF90`.

R123 reconstructs four complete normal bodies (713 bytes) and binds the game
destructor's embedded-state calls to them. The same parent now uses the existing
concrete tracked-lock, Lua-state and input-configuration cleanup implementations.
This closes source dependencies; it does not admit the raw game owner into the
ordinary application or establish native exception unwinding or gameplay.

## Recovered bodies and ABI

All four original entries take the actual borrowed owner in ECX, no stack
arguments, and return with RET. The new C++ functions use explicit context and
progress arguments and are not binary ABI replacements. Descriptive names are
hypotheses, not recovered symbols.

| Entry | Bytes | Established normal behavior |
| --- | ---: | --- |
| `76A760` | 41 | Clear shared byte `E0AF14`; clear byte `94` in current nonnull owner fields `188`, then `18C`. |
| `76DB20` | 117 | Prefer current peer `188`, otherwise `18C`; flush it; scalar-delete nonnull vector entries with flags 1; reread begin/count after each callback; finally zero count `250`. |
| `76F000` | 223 | Stamp `D039CC`; clear entries; scalar-delete current `18C`, then `188`, clearing each field after its callback; release actual lock slot `298`; free/clear buffers `25C`, then `24C`; conditionally free large-string storage `224`; reset length `234`, capacity `238`, and only byte `224`. |
| `7849C0` | 332 | Enter actual tracked lock `44`; snapshot up to eight first-list keys; dispatch queued records with a matching nonzero key; unlink/free queued nodes; release the currently published lock `44`. |

The vector traversal retains its cursor but recomputes the end from current
`24C + 4*250` after callbacks. It does not clear cells or free the buffer.
The embedded destructor compares string capacity as unsigned against 16; its
normal small-string reset preserves the other bytes of the old storage union.
The peer virtual calls use slot 0 with one flags argument; dispatch uses the
current peer `4` receiver and current vtable slot `4` with one record argument.
Their payload classes and game-specific destructor/dispatch bodies remain contracts.

## Peer queue details

The first checked-list header is at peer `8`, with sentinel at `C`; node `8`
contains the key. The second header is at `38`, sentinel `3C`, count `40`;
node `8` points to a record whose `8` is the key. The original private array
has eight entries. Valid source use requires at most eight first-list nodes;
the original overflows its private frame for a longer list. No clamp is added.

After filling missing keys with zero, the original caches key 7 in EBP.
Matching skips key zero. Each dispatch reloads the receiver and record pointer,
then reloads the node's next link after the callback. Returning CRT validation
is retained, including the post-callback current-sentinel check. The diagnostic
at `784A4F` is unreachable behind the original self-comparison.

Cleanup captures the first queued node, resets current sentinel links and count,
then captures each next pointer before free and compares it against the current
sentinel afterward. It retains the sentinel and does not delete record payloads.
Entry increments the captured section's depth at `18`; exit reloads peer `44`,
decrements that section's depth, and leaves that current section. Callback-driven
replacement can therefore leave the entered section held. Source progress records
that outstanding section; no automatic unlock changes the native behavior.

## Concrete parent bindings and failure ownership

Parent `4DCF90` sites `4DCFDE` and `4DD268` pass the actual embedded context and a
retained child destruction operation. Missing context or a different call service
fails at the reached binding. The context carries the actual shared flag byte and
required virtual services; no successful placeholder virtual implementation exists.

The existing parent address methods also now invoke:

- `41CC80`: actual tracked-section slot release, including positive recursion drain.
- `B669A0`: close the actual owned Lua state and clear its slot.
- `4DCEB0`: release the actual input configuration's flat/nested vectors and Lua owner.

The full embedded source operation is one-shot. Exceptions retain the partial graph,
native site, unwind state and entered lock. Diagnostic acknowledgement is permitted
only after the caller resolves those resources; acknowledgement itself frees and
unlocks nothing. The parent refuses acknowledgement while this child is failed or
running. These controls do not implement original FH3/SEH unwinding.

## Ghidra evidence

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was checked.
All 713 live bytes match the installed original PE. Four returning-free gaps totaling
31 bytes were repaired under the shared write lock: three in `76F000` (21 bytes)
and one in `7849C0` (10 bytes). These restore stack cleanup, field clears and the
node-loop back edge. No callee-wide no-return flag or body recreation was used.
Final flow audits report no remaining gaps in these two bodies.

Names and evidence comments are saved with prior annotations preserved, read back,
and exports refreshed. The report records every direct CALL and indirect boundary.

## Validation and limits

- Strict MSVC Win32 build and all three existing CTests pass.
- Four copied original bodies compare with the source in **86 paired cases**:
  **1,000 observations**, **5,789,856 matching normalized bytes**, **175 dispatches**
  and **two returning diagnostic calls**.
- Cases cover absent/either/both peers; 0/1/7/8 keys; capacities 15/16/FFFFFFFF;
  replaced receivers; appended queued nodes; replaced sentinels with returning
  validation; replaced exit locks; changed vector counts/begin; peer publications;
  and buffer callbacks changing later buffer/string state.
- The fixture uses the existing real embedded constructor, actual Win32 critical
  sections and actual CRT allocations/frees. Virtual objects and record handlers
  are explicit fixture implementations, not validated game payload profiles.
- The concrete parent defaults additionally drain a real twice-entered lock, close
  owned Lua 5.1.1 with a finalizer, preserve a borrowed state's lifetime, and free
  real flat/nested input buffers while closing that configuration's actual Lua state.
- One source failure during dispatch retains its entered lock/partial graph and
  rejects replay; caller diagnostic cleanup drains resources before acknowledgement.
- The parent comparison passes **52 paired cases**, **3,537 snapshots** and
  **141,402,076 matching bytes**, plus four source failure/replay cases. Embedded
  and array calls remain controlled in that separate parent-schedule comparison.

Known allocation pointers are normalized; freed storage, OS lock internals and the
original private stack are not compared. Native exception handlers, hardware faults,
arbitrary private-stack aliasing, concurrent mutation and invalid oversized key lists
remain outside the tested domain. Existing constructor/lock/Lua/input implementations
are shared dependencies, not newly differential-tested bodies. No ordinary application
run is attributed to this unreachable raw-game cleanup packet. Local probes remain
under ignored `local/game_embedded_lifetime_r123`; no permanent test suite was added.

## Follow-up packets

Recover and bind the remaining required parent cleanup services, including the peer
payload virtual implementations, resource-manager/DYN disposal and checked containers.
Then compose the real allocation/construction and teardown owners with retained
exception ownership before admitting the raw `71A0` game object to the application.
