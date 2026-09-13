# Native scene lifecycle notification

Addresses: `00926390`, `009263C0`.

The wrappers borrow `NativeUnitObserverAlias`, the same unit's volatile byte
lvalues `5C/5D/5E/5F/60`, `ObserverEventDeliveryContext`, the actual controlled
listener publication `E188DC`, and required producer-specific virtual providers.
There is no new endpoint, flag snapshot, semantic-unit cast, or runtime binding.
Native byte fields are established by the constructor stores `925E08..925E14`;
the existing `SceneNodeFlags` projection covers fewer fields and operations.

| Entry | Original ABI | Coverage |
| --- | --- | --- |
| `926390..9263B1` (34 bytes) | ECX unit; no stack inputs; RET or tail JMP slot7C | complete source behavior |
| `9263C0..926414` (85 bytes) | ECX unit; no stack inputs; RET or tail JMP slot80 | complete source behavior |

`926390` returns immediately if current byte5D is nonzero. Otherwise it stores
5D=1 then60=1, invokes the existing conditional slot08 observer notifier, and
reads the current profile before invoking slot7C. Observer callbacks may change
both flags and profile; their changes survive.

`9263C0` calls the current slot18 getter even when already destroyed. If its
result is nonnull, it reloads the table, captures `E188DC` at `9263D4`, then
calls the second getter at `9263DC`. Equality with that **captured** listener
calls `4BCA80(0)`. Only afterward does it load byte5E at `9263EA`. A zero byte
permits stores5D/5E/5F=1,5C=0, conditional slot04 notification, and current-table
slot80 dispatch. This preserves callback-mutated flags and listener publication.

| Site / containing entry | Provider contract |
| --- | --- |
| `9263A1 / 926390` | existing complete `925C90` conditional slot08 notification |
| `9263C8,9263DC / 9263C0` | required captured-table slot18; two independently selected calls |
| `9263E5 / 9263C0` | external complete `4BCA80`: publish supplied value, read currentF8D39C, callB0D7B0 |
| `926401 / 9263C0` | existing complete `925C40` conditional slot04 notification |
| `9263AE / 926390`, `926411 / 9263C0` | required current-table slot7C/80; no universal implementation |

The conditional notifiers sample count under the actual observer lock, release
it, then acquire a separate dispatch lock when the saved predicate is true.
No second sample is added. The renderer setter's B0D7B0 callee writes+1C0,
then callsB4EC90 on current+30 when nonnull; this renderer behavior remains an
explicit external contract. Native virtual providers consume ECX only here.

Current xrefs still report **none** for `9263C0`; no reachability claim follows
from reconstructing it. MDestroyer tableCFC3D0 has slot74=`926390`.
Its slot18 points to `6D1E80`: seven bytes read `[ECX+4A4]`, but live Ghidra
reports `no_ghidra_function`. That dependency is recorded, not defined or renamed.

Both owned bodies have complete stored membership and no gaps. Seven spans
(190 bytes) match saved Ghidra and disk. The report retains build, existing
CTest, call-check and native/source fixture results and exact artifacts.
Strict Win32, both existing CTests, five direct call checks, and six paired
native/source scenarios passed. The fixture shares actual observer providers; virtual/renderer boundaries are
explicit test implementations. Original tail-call ABI, hardware faults,
renderer/base-tail completeness, and gameplay behavior remain unvalidated.
Ghidra access is strictly read-only; the integrator applies ledger annotations.
