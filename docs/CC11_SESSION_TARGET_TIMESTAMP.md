# Complete native session target timestamp refresh

`007839A0..007839CA` is a complete 42-byte, 16-instruction consumer. Its native
interface takes the actual target in ECX, no stack arguments, and returns with
plain RET. `BSP_SessionTarget_RefreshTimestamp` is a descriptive hypothesis.
Current live Ghidra has no references to this entry; this packet establishes
the function body, not a reachable caller or a publication producer.

`refresh_native_session_target_timestamp_007839a0` reloads the same borrowed
actual `01090AB0` publication through the existing concrete clock adapter. That
adapter admits only the actual `D68D50` identity and current slot `+20 == BEE080`.
It then calls the complete real raw sampler. Null/unsupported publications or
profiles produce the adapter's Source `logic_error`; this is not the original
native fault or private exception ABI.

The caller explicitly supplies the actual 16-byte sample-stack preimage, as in
the existing transport stamper. The returned sampler pointer supplies both
signed qwords. The native `FILD/FILD/FDIVP/FSTP` schedule retains ambient PC/RC
and stores once to the actual target's `D68` float. Entry does not normalize
the x87 stack; masked overflow retains the original status and stack effects.
There is no default clock, zero preimage, alternate provider callback, C++
floating expression, or intermediate floating spill.

## Evidence and original composition

Live project/program identity was checked through `bsp.py`: `bsp.gpr`,
`/battlestationspacific.exe`. All 42 bytes matched the installed PE. The genuine
sampler is `00BEE080..00BEE0DD`, 93 bytes and 32 instructions. It copies all
16 bytes from actual clock `+20` in fixed mode. Otherwise it calls real
`QueryPerformanceCounter` and copies actual frequency `+60`, ignoring the API
Boolean. The supplied caller preimage does not define the sampler's separate
internal stack contents if QPC fails.

The ignored probe at `local/cc11_target_timestamp_20261007_a` copies both full
original bodies (135 bytes total). Its only code changes are two four-byte
absolute operands:

| Native operand | Original cell | Executable-copy offset | Fixture binding |
| --- | --- | --- | --- |
| `007839A8` | `01090AB0` | `08` | Native clock's actual publication cell |
| `00BEE0B5` | `00CE2270` | `135` | Pointer to real Win32 QPC |

Every other byte, both original RET schedules, and all original branches remain
unchanged. Copies transition from writable to executable/read-only storage.
There is no Source bridge inside this original numerical composition.

The fixture separately relocates the native dispatch data: it copies the ten
original `D68D50` words, replaces only slot eight with the copied sampler entry,
and points the already constructed native-side clock at that table. Its original
identity is restored before Source destruction. The Source clock retains the
raw `D68D50` identity and unchanged borrowed PE method words. These are explicit
fixture address relocations, not production profiles or a fabricated provider.

Both actual raw clocks are constructed and published through complete
`BEDFB0`/manager services, and enter fixed mode through the real existing
method. Both actual targets use complete Source constructors, owned locks,
cursors, and histories; complete clock/target/manager destructors run afterward.
A nonzero explicit preimage is copied by Source and seeded into the exact
original caller-stack region before the copied native entry runs.

## Validation

The full original-body comparison passed **71 cases**, with 391 checks and no
failures. Check counts include PE range and fixture setup checks.

- 60 cases cover PC 24/53/64, all four rounding modes, and 0/3/6/7/8 occupied
  x87 registers. All exceptions are masked; MXCSR is `1F80`, with DAZ/FTZ off.
  Occupancies seven and eight produce 24 observed invalid/stack-fault cases.
- Eleven signed-qword edge pairs include signs, zero numerator, zero divisor,
  and signed 64-bit extrema. These are arithmetic storage fixtures; zero or
  negative frequencies do not establish normal clock-producer reachability.
- Comparisons include raw `D68`, all other target bytes, both owned history
  headers and all 53 samples, unchanged actual clocks and supplied preimage,
  CW/SW/FTW, all eight raw 80-bit register fields, and MXCSR. FP code/data
  pointers differ by address and are excluded. The new Source ABI permits
  caller-saved XMM clobbers; those registers are excluded.
- One null-current-publication check confirms the existing Source admission
  failure before a target write. One real QPC sequential smoke confirms
  nonnegative/nondecreasing results; it makes no equal-sample or failure claim.

Fourteen current repository translation units plus the probe were compiled
fresh with MSVC 19.51.36244 Win32, `/O2 /W4 /WX /fp:strict`, and an embedded
`asInvoker` manifest. The final immutable snapshot contains 55 source/header/
fixture inputs. All snapshot and workspace hashes, three read-only support
library copies, and a frozen original PE copy matched before and after the
final run. The original installed PE also remained unchanged.

Support libraries were frozen from the parent's completed `e0dfeae01` build;
the observed metadata head at freeze was `50892f7c9`. Main's `bsp_core.lib`
subsequently changed during the authorized parallel build. That does not alter
the linked frozen copies; the report records both observations separately.

The actual Source COFF section is 64 bytes. Its sole REL32 operand at `18`
belongs to the call at `17` into `sample_published_native_frame_clock`. The
returned EAX is saved at `1F` and reloaded at `2B`; the only x87 instructions are
`33:FILD`, `35:FILD`, `38:FDIVP`, and `3A:FSTP`. The 16-byte preimage copy uses
`MOVUPS`, which can clobber caller-saved XMM0 but performs no floating arithmetic.
No float spill precedes the target store.

The native call row at `007839B6` is **indirect**. Current table evidence resolves
it to `BEE080`, but the call-row verifier does not prove a virtual runtime target.
The sampler's QPC call is an indirect import, also not a direct CALL row.

## Boundary

This is complete ordinary Source behavior and a focused original-body fixture,
using a new Source ABI. It is not a native binary replacement or game validation.
Unmasked exceptions, QPC-failure private-stack contents, derived target/session
lifetimes, socket behavior, publication producers, and caller reachability are
not established. No tracked tests, shared metadata, CMake, or Ghidra state were
edited by this worker. Primary integration and the full main build remain the
integrator's responsibility. Machine-readable evidence is in
`reports/cc11_session_target_timestamp.json`.
