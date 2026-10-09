# Application ordinary-retirement unwind boundary

This bounded read-only audit follows the installed `CC7260` handler referenced
by the complete `BEA8B0` ordinary retirement body. Its encoded unwind map has
two actions: state1 targets the captured guard cleanup and continues to state0;
state0 targets the captured receiver's generic base reset and continues to -1.
The corresponding concrete Source services already exist. No new exception
wrapper, Application owner, callable profile or Native EH compatibility is
admitted by this audit.

The [report](../reports/cc12_application_retirement_EH_readiness.json) retains
78 fresh code/data bytes plus six alignment bytes, independent instruction decodes, current PE checks,
parent provenance, physical frame offsets, SDK layout reference and Source pins.
No runtime handler, direct cleanup target body or other function was expanded.

## Handler and data records

The parent pushes `CC7260` before installing its exception registration.
At that address, ten actual code bytes execute `MOV EAX,E01CF8` followed by
`JMP BF6B43`. Six subsequently captured bytes are alignment `CC`, not part
of the two-instruction handler. Current Ghidra has no function starting at
`CC7260`; this packet neither creates one nor alters its enclosing listing.
The shared handler's runtime protocol remains an external contract.

The file-backed 36-byte record at `E01CF8` contains magic `19930522`,
maxState2, absolute unwind-map pointer `E01CE8`, zero try-block/map fields,
zero IP-map fields, zero exception-type-list pointer, and encoded EHFlags1.
The absolute x86 interpretation is consistent with the current installed
MSVC `ehdata.h` layout. That header is a layout reference, not proof of the
original compiler/runtime implementation or hardware-fault policy.

| State index | Encoded next state | Action entry | Complete action |
| --- | --- | --- | --- |
| 0 | -1 | `CC7250` | `MOV ECX,[EBP-18h]`; tail `JMP 412430` |
| 1 | 0 | `CC7258` | `LEA ECX,[EBP-14h]`; tail `JMP 411EE0` |

Both actions are eight bytes/two instructions and are listed completely in
Ghidra. Each tail jump preserves the helper-provided return address and
caller stack. Neither contains a publication read/clear, manager lookup,
outer or vector-data free, local catch, replay or rollback instruction.
Their callees' effects are separate, qualified contracts.

## Parent frame and state timing

The already captured complete parent uses these addresses relative to incoming
ESP, derived from its actual pushes and 12-byte local allocation:

| Parent slot | Incoming ESP offset | Stored value |
| --- | --- | --- |
| State | -4 | Full zero at `BEA8D6`, low byte1 at `BEA901` |
| Registration previous/handler | -12/-8 | Old exception chain / `CC7260` |
| Receiver spill | -24 | Actual captured EDI receiver at `BEA8CC` |
| Guard profile | -20 | `CE37FC` at `BEA8E8` |
| Guard section | -16 | Captured first-manager section at `BEA8F0` |

The action offsets align with that captured receiver and local guard when
the runtime supplies EBP equal to the parent's incoming ESP. This alignment
does not independently establish the unexpanded `BF6B43` frame-helper ABI.
The actions consume their current frame slots; this evidence does not replace
them with a newly fetched manager, publication or detached guard.

State0 is recorded before the first manager getter and optional Enter/depth
increment. State1 is recorded only after those ordinary operations, including
on the null-section path, and before the second getter/current-publication
removal. The ordinary parent never writes a lower state after publication
clear or before its decrement/Leave/final-profile sequence. An invented early
disarm, automatic publication restoration, allocation free or cleanup-success
assumption would exceed these recorded transitions.

## Concrete existing Source and limits

`destroy_native_singleton_guard_00411ee0` reads the actual raw8 guard's current
section at +4, resets the guard profile to `CE37FC`, and, when nonnull,
decrements the actual unsigned depth DWORD then calls real LeaveCriticalSection.
`destroy_native_generic_singleton_base_00412430` writes `CE3818` to the actual
receiver's first DWORD. Current Source definitions are pinned and reviewed;
their native bodies and historical emitted proofs were not re-queried here.

These services provide the observed cleanup actions for later explicit Source
composition. They do not supply the original shared handler's recognition,
frame setup, action invocation, nested-cleanup-failure or hardware-fault behavior.
There is no local catch table in the encoded record; that is not a general
runtime exception-policy assertion. Full Source failure retention must respect
the recorded parent state schedule and remain labelled separately from Native
FH3/SEH compatibility.

All 84 captured file-backed bytes match the unchanged installed PE. The complete
153-byte parent was replayed from the published ordinary-retirement audit.
No Source, CMake, ledger or Ghidra edit, build, test, probe, Original credit,
Application production wiring or runtime/gameplay proof occurs in this packet.
