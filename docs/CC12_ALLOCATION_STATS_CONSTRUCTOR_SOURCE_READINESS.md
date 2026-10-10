# Allocation-stats constructor Source readiness

The complete retained base/derived constructors are ready for standalone ordinary
Source using the existing genuine raw singleton-manager getter, registration,
guard destructor and base-profile reset. This packet adds no implementation.

The proposed context borrows the actual live manager publication `01090aa0`
and stats publication `0109cefc`. The caller supplies the original writable
12-byte receiver and owns its lifetime. Source must arm base cleanup before
the `00d685e0` store, capture the first manager's +10 section, enter/increment
it if present, arm guard cleanup, publish the receiver, call the manager again,
then reload CURRENT stats for registration. Normal decrement/leave uses the
first captured section; return preserves the original receiver. State-one C++
cleanup calls `00411ee0` before `00412430`; state-zero calls only `00412430`.
Publication remains as written on failure: no rollback, retry or allocation/free.

Derived `00be2900` calls that base before writing profile `00d685f4`, raw DWORD
`40000000` at +4 and zero at +8. The profiles retain original word identities;
they do not create callable rebuilt virtual tables. Genuine existing lower
functions close the ordinary provider contract. Private Source RAII follows
the already-reviewed raw render-service constructor model; complete emitted
cleanup/code and normal Win32 checks remain required for an implementation.

The current `platform_window` helper instead stores a null vtable and the two
scalar fields, without calling the base, publishing or registering. `GameHost`
uses it on local stack storage. That existing projection cannot establish native
startup allocation, process publication/lifetime or actual virtual consumers.
Those changes are excluded from the proposed standalone packet.

Root froze the full quoted Source closure of nine implementation roots and
replayed all 750 Source746 input/artifact pins. The retained complete base
145-byte/40-instruction and derived32-byte/9-instruction evidence comes from
the accepted constructor primary review; no new Native window/query was used.
The report retains the complete Source/edge/pin inventories and ordered contract.
Original register ABI, FH3/SEH/hardware faults, private spills, OS registration,
startup/gameplay and executable application wiring remain unproved.
