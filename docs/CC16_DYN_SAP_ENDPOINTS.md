# Dyn endpoint removal and leftward update

`src/cc16_dyn_sap_endpoints.cpp` recovers two complete original bodies:
`00C40800..00C408FB` (252 bytes) and `00C40B30..00C40CB9` (394 bytes).
Names are behavioral hypotheses. Full original assembly and full live byte
spans were inspected in the existing BSP project/program. Neither native
body calls another function or requires an invented process global/provider.
Their actual raw object, axis-array and tagged object storage remain required.

## Original ABI and records

Removal receives object in EAX, axis arrays in ECX and axis in EDX, with
RET0. EBX/ESI/EDI are saved; EBP is saved only on one compaction path and
otherwise untouched. The leftward update receives axis in EAX, object in
ECX and axis arrays in EDX, with RET0 and EBX/EBP/ESI/EDI saved. Callers do
not consume a semantic EAX return. The new explicit C++ interfaces return
void; they do not supply either original register ABI or exact fault state.

Each native 12-byte axis record has its packed 8-byte entry buffer at +4
and count at +8. Object lower WORD indices occupy +20/+22/+24, and upper
indices +26/+28/+2A. Each entry contains an unsigned sortable key and a
tagged actual object pointer. Tag bit one selects the object's lower index,
zero selects its upper index; the other bits are the object address. The
source uses raw proven offsets rather than a fabricated class or vtable.

## Removal

The first pass decrements intervening objects' selected WORD indices by
one, between the object's lower and upper entries. The second decrements
those after the upper entry by two. WORD arithmetic wraps at 16 bits.
The source then rereads the actual upper index and removes that entry,
decrements count, rereads the lower index, removes it and decrements count
again. Both compactions copy key then tag forward, preserving repeated
base reads and per-iteration count/base reloads. Endpoint loops terminate
by pointer equality, without added count, axis, null or ordering guards.

## Key publication and stable movement

The update publishes both new keys before either leftward pass. The original
lower `FLD` precedes its array-base and index reads. Its `FSTP` writes the
entry's float word, which is then transformed with the observed sign-bit
mask. The upper index is read before its `FLD`, followed by a fresh base
read and `FSTP`. This is real ambient x87 transport, not a raw integer copy:
exception masks and signaling NaN/denormal behavior remain hardware concerns.
One free x87 slot is required; no FP control or MXCSR setting is forced.

Each pass captures the entry's tag and key, shifts strictly larger unsigned
keys right and increments the shifted object's selected WORD index. Equal
keys stop the walk. Only a moved entry publishes its new own index, then
its original tag and key. Each final DWORD store gets a fresh base read.
The lower and upper passes retain their separate own-index field offsets.
This routine moves entries left; the other direction remains a distinct
original path, rather than an invented general sorting operation.

The six bytes omitted by the saved listing at `00C40BBA..BF` were checked
live as `8D 9B 00 00 00 00` (`LEA EBX,[EBX]`), an unreachable alignment
instruction following the unconditional jump. No branch or body was inferred
from that listing gap, and no neighboring analysis was altered.

## Caller and compilation evidence

| Original call site | Native callee |
|---|---|
| `00C4CCA2`, `00C4CCB0`, `00C4CCBE` | `00C40800`, axes 0/1/2, actual object/manager storage |
| `00C4CC3D`, `00C4CC4B` | `00C40B30`, axes 0/1, actual object/manager storage |

Complete caller setup windows confirm the register contracts and ignored
EAX returns. The report verifies all five call opcodes. Those callers still
have missing bodies/providers; call evidence is not original root reachability.

Both sources are registered in `bsp_core`. The common MSVC Win32 Release
build on accepted `88a2c8c9f` and both existing suites passed (10.87 seconds).
Receipt: `local/cc16_dyn_sap_endpoints_build.log`. Independent final-object
inspection shows the key publisher has precisely `FLD/FSTP/FLD/FSTP`, the
reviewed intervening memory/integer instructions, no SSE or calls, and no
FP-control changes. Its compiler prologue differs from the native body;
instruction/fault-state identity is not claimed.

No new test, fixture or game run is added. These bodies replace metadata-only
placeholders as compiled/unwired source. Actual game-side custody, native ABI
adapters and root activation or qualified dead-chain closure remain open.

All five native caller rows verified with zero failures. Locked Ghidra
annotations were saved and both exports refreshed; previous values remain
in `local/ghidra-annotations-20261029T043333Z.json` and git history.
