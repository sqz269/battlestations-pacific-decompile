# Dyn SAP flagged update and pending-list dispatch

This packet reconstructs complete `00C4CBB0..00C4CBE5` (54 bytes, 23
instructions) and `00C4CBF0..00C4CC82` (147 bytes, 48 instructions). Names
are descriptive hypotheses. New C++ interfaces use the actual raw Win32
manager/endpoint objects and the caller's genuine `DynLiveCrt`; no class,
vtable, object, global, default provider or returning stub is created.

Every live batch used the verifying `bsp.py ghidra` client for project
`bsp`, `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE
32-bit, image base `00400000`. Both exports were refreshed, their complete
listings reviewed, and their full contiguous byte spans hashed in the report.

## Original ABI and exact dispatch

`00C4CBB0` is RTTI `Dyn::SAPBroadPhaseManager` vtable `00D7A13C` slot 2;
live DATA reference `00D7A144` confirms the slot location. Its original
contract is ECX manager, stack endpoint, `RET 4`, with EBX/EDI preserved
and ESI saved/restored on the taken branch. It reads endpoint+2Ch once.
When nonzero, it directly calls the recovered `00C4C700` for axes 0,1,2
with that same manager and endpoint. It does not recheck the flag between
calls or directly write it. A zero flag makes no axis call.

`00C4CBF0` receives manager in EDI, has no stack arguments, and uses plain
RET. It saves/restores EBP/EBX/ESI and leaves EDI unchanged. Its native
prologue aligns ESP down to eight bytes. Manager+194h is the pending
endpoint-pointer data word; manager+198h is the unsigned live count.
For every captured endpoint, the exact sequence is:

1. `00C40900` append axes 0,1,2, with ECX=manager and endpoint/axis on stack.
2. `00C40B30` key publication and left movement for axes 0,1, with
   EAX=axis, ECX=endpoint, EDX=manager.
3. `00C4C700` full update for axis 2, with ESI=2 and manager/endpoint on stack.
4. Increment the uint32 index, store endpoint+2Ch=1, then reload count and
   compare the index unsigned. Reload the data pointer for each next entry.

The source directly composes `cc16_dyn_sap_update::append_axis_endpoints_00c40900`,
`cc16_dyn_sap_endpoints::update_keys_move_left_00c40b30`, and
`cc16_dyn_sap_update::update_axis_endpoints_00c4c700` with those same actual
objects. The same CRT reaches append/update and their real pair dependencies.
The two left-only calls receive their original object/manager argument order;
they are not replaced by full updates on axes 0 and 1.

The initial count check accepts every nonzero 32-bit count. Count is not
snapshotted, and endpoint identity is retained across all six leaf calls.
Pending entries are not gated on their existing endpoint flag.
Count is explicitly written zero after normal completion, including the
initially empty path. It is not cleared before dispatch, per entry, or on a
C++ allocation exception. An exception can expose prior vector/pair changes
and prior endpoint flags while skipping the current final flag store. No
rollback, guard or exception translation is introduced. Original FH/SEH
transport and exact fault-register state are not claimed.

## Verification and remaining boundary

Strict MSVC Win32 C++17 object compilation passed with `/permissive- /EHsc
/fp:strict /O2 /W4 /WX`. Generated assembly was reviewed for the nine direct
calls, one flag read, leaf axis/argument order, post-call flag store, and live
unsigned count/data reloads. All nine original call rows passed the live
`verify_report_calls.py` checker. No tests, probe executables, native/game
launches, shared CMake/ledger edits or Ghidra mutations were made.

Both public functions return void; incidental native EAX values are not
presented as semantic returns. The interfaces do not reproduce original
thiscall/register argument transport or the aligned native stack frame.
The integrator owns shared registration, annotations/save/export refresh,
the common Win32 build/checks, genuine caller and CRT binding, ABI adapters,
and any fixture/native/game validation before activation. The live incoming
call to `00C4CBF0` is `00C5A864` inside `00C5A850`; that parent and actual
host/vtable wiring are outside this packet. Vendor CRT bodies receive no
reconstruction credit.

## Primary integration

The primary reviewed the complete source/assembly bodies and registered the
module in `bsp_core`. The combined Win32 Release build over accepted
`c458c4d24` with source candidates `57f644d5`/`a5b6836e` and the dormant
accuracy correction passed both existing suites (10.69 seconds). Native
call rows passed; final-object inspection confirms the actual x87 copy epochs.
Receipt: `local/cc16_sap_accuracy_off_build.log`. Metadata-only placeholders
now name compiled/unwired bodies. Locked annotations were saved and exports
refreshed; previous values are in
`local/ghidra-annotations-20261029T052526Z.json`. Genuine game/root, caller
and CRT custody, original register/fault ABI and runtime gates remain open.
