# Unit order record and HUD publication fragments

Packet `orch4_unit_order_record`, worker `agent/orch4-orders-20260910`.
Evidence acquired 2026-09-11 UTC in project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. All live queries used the verified `bsp.py ghidra`
interface. Ghidra was read-only: names below are repository hypotheses pending
integrator annotation, not recovered symbols.

## Corrections to the earlier producer packet

* `00815440` performs actual floating-point comparisons and clamps. Both leading
  fields are floats, not merely ambiguous words copied through x87.
* `00816A40` is `void __thiscall(unit*, float, float, byte)`, ECX is the unit,
  and `00816AEF` is `RET 0Ch`. The prior cdecl/RET0 description and earlier call
  offsets in `UNIT_COMMAND_PRODUCERS.md` and the old name evidence are superseded.
* `00651760` does **not** call `00816A40`. It ends at `006517FD: RET 4` (last byte
  `006517FF`). The third HUD caller begins at the currently undefined function
  `00651800`; its direct call is `00651AA3`. The candidate partition's enclosing
  function association is not a Ghidra function boundary.

## Complete builder: `00815440`

Native ABI: record pointer in ECX; stack arguments float A, float B and one byte
in a four-byte slot; returns the original record pointer in EAX and pops 12
bytes. Start `00815440`, last instruction `0081550E: RET 0Ch` (3 bytes), last
body byte `00815510`. There are 62 listed instructions and no listing gaps.

| Record offset | Proven builder action | Evidence |
| --- | --- | --- |
| `+00h` | A clamped to `[-2,+2]` | compare `00815472/8A`, store `0081549D` |
| `+04h` | B clamped to `[-2,+2]` | compare `008154B0/DB`, stores `008154C3/EE`, `00815506` |
| `+08h..0Bh` | untouched | no store in complete body; downstream publisher separately clears slot byte `+08h` |
| `+0Ch`, `+14h` | A upper bound, B upper bound: float `+2` | `0081544E/49`; `00CE3958` bytes `00 00 00 40` |
| `+10h`, `+18h` | A lower bound, B lower bound: float `-2` | `00815460/5B`; `00CE7D7C` bytes `00 00 00 C0` |
| `+1Ch` | low byte of third argument, unchanged | `008154C0/EB`, `00815503` |
| `+1Dh..1Fh` | untouched | no store in complete body |

Store order is B-upper, A-upper, B-lower, A-lower, clamped A, kind, clamped B.
Both clamp comparisons use `FCOMIP` followed by `JBE`. Unordered comparisons
take the branches leading to `MOVSS` of the original argument. Thus a NaN is
retained, not replaced by a bound; signed zero is also retained. This is a
material difference from a literal translation of the decompiler's ordered
`<=` conditions. Floating-point exception/status state is outside the C++ API.

The native builder does not initialize all 32 bytes. The reconstruction takes
caller-supplied `UnitOrderRecordStorage` and preserves untouched bytes rather
than silently zeroing them. It returns the existing compact `UnitOrderRecord`
projection for the queue API. This return is a semantic interface, not native
EAX. The storage is the full record; `sizeof(UnitOrderRecord)` is not 20h.

## Complete bounded publication: `00816A40`

The native body is `00816A40..00816AF1`, with last instruction `00816AEF: RET
0Ch`. It captures ECX in EBX, builds the stack record at call `00816A76`, then
passes that record to `0080DAD0` at `00816A82`. It reads game session mode
`DAT_00E188A8 + 1FE4h` only after publication (`00816A8D`). Mode 2 takes this
sequence:

1. Call `0075B430` with ID `8Eh` at `00816AA1` to construct a base message.
2. Set message word `+18h`, byte `+1Ah` and dword `+04h` to zero at
   `00816AB5/BA/BE`; set vtable identity `+00h` to `00D02DA8` at `00816AC2`.
3. Copy eight dwords of the **original builder record** to message `+1Ch` at
   `00816ACA`. The queue's cleared active byte does not replace these bytes.
4. Send through `0077C2A0`, ECX=unit, stack `(message,0,0)`, at `00816AD9`.

`issue_unit_order_record_00816a40` models this sequence. The host has separate
constructor and send methods and supplies the base message bytes. It carries
the full 3Ch-byte message with 20h payload, preserving base fields not written
here. The vtable value is an observed identity, never dereferenced by C++.
The existing `publish_unit_order_0080dad0` semantic queue is reused, including
its documented artificial capacity and range guard. The old compact-record
issue host in `unit_orders.hpp` cannot represent this full payload and should
be retired by integration. SEH and native machine ABI are outside this model.

## HUD commands and exact publication boundaries

Three native paths publish the ordered thrust/turn pair. The first path's
control-transfer seed explicitly loads unit `+980h` into HUD `+24h` and unit
`+984h` into HUD `+28h` (`0064B9AE..0064B9BD`), supporting those field meanings.
The UI screen class names and physical key bindings remain unproved.

Let `F` denote the external CRT helper at `00BF85B0`; its existing library name
is retained. The inspected pseudocode is floor-like, but its exceptional-value
and x87-control behavior are not reconstructed. Every call stays a distinct
host boundary. All expressions below spill to float before promotion to double
for F, and its result spills to float before scaling:

* Thrust quantization: `float(F(double(float(4*A + bias)))) * 0.25`.
* Turn quantization: `float(F(double(float(B / step + bias)))) * step`.

The final multiplication is evaluated with the native double constant and
spills to float. `bias` is exactly `0.4900000095367431640625` (double bytes
`00 00 00 00 29 5C DF 3F` at `00CF5E58`), and `step` is exactly
`0.16666667163372039794921875` (double bytes `00 00 00 60 55 55 C5 3F` at
`00CF5E60`). They must not be replaced with 0.5 or exact one-sixth. Double 4
and 0.25 are verified at `00D7A328` and `00D7A348`.

| Native path | Issue arguments and callers | Reconstructed fragment |
| --- | --- | --- |
| `0064B870` | Quantized HUD `+24h` thrust, quantized `+28h` turn, kind 0. F calls `0064BAB5`, `0064BAEE`; issue `0064BB12`. | `0064BA97..0064BB16` |
| `00651800` | Quantized HUD `+54h` thrust, quantized `+58h` turn, kind 0. F calls `00651A32`, `00651A5C`; issue `00651AA3`. Both F calls finish before either scaling operation. | `00651A15..00651AA7` |
| `0067C4F0` | Quantized HUD `+1Ch` thrust, `-0.0f - steering_input`, byte HUD `+20h`. F call `0067C7A7`; issue `0067C7CB`. | `0067C772..0067C7CF` |

The first two HUD paths integrate axes over delta time, then clamp thrust to
`[-0.5,+1]` and turn to `[-1,+1]` before the fragments. The first also has a
device-dependent direct thrust path. The third clamps both raw axes to
`[-1,+1]`, integrates or directly maps thrust, then clamps thrust to
`[-0.5,+1]`; turn is immediate and sign-inverted, without quantization.
The subtraction starts from negative zero: parent integrator verified
`00D7A208` bytes `00 00 00 80`, loaded at `0067C77E`. A positive-zero steering
input therefore normally yields negative zero; ordinary unary-negation
intuition must not replace the actual SUBSS operation for exceptional inputs.
Its command byte is exactly 1 when the input record at input-object `+3090h`
has a nonzero byte `+28h` and value `+24h` is strictly greater than the threshold
at `00D7A218`; otherwise it is 0 (`0067C675..0067C6B6`). The semantic name of
that action is open. The fragment accepts the byte, not an invented action enum.

All three issuance paths require the relevant unit `+6C8h` gate and successful
`00927F30(unit,1)` query. The missing function also first requires query 0.
The meanings of those queries and `0077C470(unit,2,1)` control transfer remain
external. The APIs start after these gates, input updates and clamps; they
do not claim to implement entire HUD ticks. Their host methods map one-to-one
to native call sites and their inputs are explicit values at those boundaries.

## Missing function and annotation handoff

* `00651800`: live proto and disasm report no Ghidra function. Local PE decode
  shows a fresh SEH prologue immediately after `006517FD: RET 4`. It ends at
  instruction `00652814: RET 4` (3 bytes), inclusive byte end `00652816`, body
  length `0x1017` (4119). The following bytes are INT3 padding before `00652820`.
  A live data xref at `00CF63BC` points at `00651800`. Live bytes verified the
  entry and epilogue; the issue fragment was decoded from the disk image.
* `00651760`: actual body is `00651760..006517FF`; it binds a unit pointer at
  HUD `+24h` and resets view state. Analyzed only, not reconstructed here.
* `00815440`, `0064B870`, `0067C4F0`: verified live flow reports 0 listing gaps.
  The missing `00651800` has no body to audit with that tool. No repairs applied.

Suggested names and old values are recorded in `reports/unit_order_record.json`.
The integrator must create the missing function under the Ghidra write lock,
annotate evidence, save, refresh exports and callers metadata. No worker Ghidra
mutation or save was performed.

## Validation and limits

The constructor and issue path are complete bounded semantic reconstructions;
the three HUD paths are explicitly partial fragments. Win32 Release build and
existing CTest results are recorded in `reports/unit_order_record.json` after
completion. The parent integrator reported 121 successful full 32-byte native
constructor comparisons: paired bounds, ordinary values, signed zeros,
infinities and quiet NaNs, with A5-prefilled untouched bytes and compact return
bytes checked. The original 209-byte body and two constants (217 bytes total)
matched live Ghidra and disk; only two constant addresses were relocated. That
audit is parent-owned and excludes the HUD fragments. No new test target or
shared test was added by this worker. No native HUD,
network-session or game validation has been claimed. Double intermediates
with explicit float spills do not promise arbitrary x87 precision-control or
floating-point status equivalence.
