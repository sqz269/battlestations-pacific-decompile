# Action binding polling and rebind walk

Addresses: 00a92370, 00a922a0, 00a92090, 00a91d60

Packet `orch3_input_binding_poll`, 2026-09-10. Evidence was read from existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, through the
target-verifying `bsp.py ghidra` commands. No Ghidra mutation was performed.
Names are hypotheses. Implementation is `src/input_binding_poll.cpp`, with the
shared binding types extended in `include/bsp/input_action_classifier.hpp`.

## Native contracts

| Address | ABI | Last instruction |
| --- | --- | --- |
| 00a92370 | ECX=30h action; no stack arguments; RET | 00a926e0, 1 byte |
| 00a922a0 | ECX=input singleton; no stack arguments; RET | 00a922d2, 1 byte |
| 00a92090 | ECX=30h action; AL boolean result; RET | 00a920d4, 1 byte |
| 00a91d60 | two stack floats, primary then paired; ST0 result; RET8 | 00a91e1b, 3 bytes |

The current pseudocode drops 00a91d60's return and misrepresents its x87 CRT
argument. Assembly shows the complete result, including the two returns and
the caller's immediate float store at 00a9255b. No missing functions or flow
repairs were found. The existing poll comment's sole-caller claim is obsolete:
current callers are 00a92a20, 00a92c40 and 00a92d40.

## State and device boundary

`InputActionRecord::bindings` owns the already reconstructed binding records;
there is no second binding table. The existing `previous_hold` and `current_hold`
names are retained for compatibility, but these are signed input values, not
elapsed hold times. Listener clocks remain separate. Native binding fields
newly modelled by this packet are:

| Native offset | C++ member | Evidence |
| --- | --- | --- |
| binding+01h | `response_curve` | 00a9255f gates the piecewise curve |
| binding+10h | `input_code` | passed to device slots +1Ch/+20h/+24h |
| binding+14h | `force_unit_scale` | 00a923b4 action-wide scan; 00a925fe selects scale |
| binding+30h | `scale` | 00a9260f load, 00a9261e multiply |
| modifier+0Ch | `input_code` | required 00a92405; forbidden 00a9244d |

These descriptive flag names specify observed behavior, not recovered setting
names. In particular no reverse-axis flag is inferred. Modifier+10h and
binding+2Ch remain unmodelled. The adapter's member order/defaults do not claim
to reproduce the native structure layout or initialization.

`InputBindingPollHost` exposes the actual three device slots and CRT sqrt call;
it does not duplicate `InputDevice`. Device calls take ECX=device, one code on
the stack, and callee cleanup. Slot +1Ch and +20h return an AL byte; slot +24h
returns ST0, immediately spilled to float. The methods intentionally preserve
byte returns because forbidden modifiers compare exactly with 1.

## Poll ordering and arithmetic

00a92370 first invokes the existing state shift exactly once: current value/down
become previous value/down, then current value becomes +0 and current down
becomes false. It does not test the record's enabled flag; 00a92c40 owns that
guard.

The next operations are two separate scans. Helper 00a92090 visits each primary
binding with a nonnull cached device and asks slot +1Ch about its code, stopping
at the first nonzero result. This scan ignores the binding's resolved byte and
modifiers. A second scan checks +14h on every binding; any nonzero flag forces
unit scale for all subsequently accepted bindings, including when the flagged
binding itself is unresolved or fails its modifiers.

The evaluation walk skips unresolved bindings. It asks required modifiers'
slot +20h in order and stops on zero. Only after every required modifier passes,
it visits forbidden modifiers and stops on exactly 1. Thus a return byte of 2
passes both checks. A rejected binding makes no primary value/down queries.

An accepted binding reads its primary value via slot +24h. If that value is
nonzero (including unordered), the initial action-wide slot +1Ch scan was false,
the device class is 2, and the unsigned code is 3Ch..3Fh, the poll asks slot +1Ch
about its paired code: 3C/3D or 3E/3F. Only a zero paired query leads to the paired
value read and 00a91d60. `FUCOMIP; LAHF; TEST AH,44h; JNP` at 00a92497..49f
skips equality but retains unordered; directly translating it as an ordered
nonzero comparison would lose that case.

00a91d60 selects `max(abs(primary), abs(paired))`, selecting the paired magnitude
on equality or unordered. It spills the reciprocal and both normalized
components to float, computes their sum of squares with x87 intermediates, and
spills that sum to float. It calls actual CRT sqrt 00bf7030, spills the returned
root to float, multiplies the original primary by that root and spills again.
The final result is capped only when greater than +1. Negative values have no
-1 cap; zero denominators, infinity and NaN are not sanitized. The implementation
retains the x87 schedule around an explicit `crt_sqrt_00bf7030` host boundary.
The CRT itself is neither ported nor replaced silently by modern `std::sqrt`.

When +01h is set, values whose absolute magnitude is below 0.5 are multiplied by
0.25. Otherwise the magnitude is transformed by `(abs(value)-0.5)*0.875*2+0.125`
with the original x87 schedule and one final float store. An ordered negative
input negates that stored result using `-0.0f-result`. The outer branch also
handles unordered, as dictated by `COMISS/JBE` at 00a9258a..592.

The poll then multiplies by the binding's scale, or 1 when the action-wide +14h
scan succeeded. It combines same-sign values by strongest magnitude (maximum
nonnegative, minimum nonpositive), and opposite signs by addition. Equal values
retain the accumulator, preserving its signed zero. An unordered operand takes
the addition path. The accumulator uses the native x87/SSE branch sequence:
the optimized differential probe caught a signed-zero mismatch in an equivalent
scalar comparison/select under MSVC `/O2 /fp:strict`. Finally it ORs the down flag
with primary slot +20h, skipping
that virtual call entirely once current down is already true. Value combination
and down querying remain separate: a zero value can still set the down flag.

Image constants verified by live bytes were 00ce3800 float 0.5; 00d7a280 double
0.5; 00ce42e0 double 0.875; 00d04380 double 0.125; 00d7a348 double 0.25;
00d7a208 float negative zero; and 00d7a24c float 1. No invented mutable globals
are needed for these immutable arithmetic constants.

## Rebind walk and adapter preconditions

00a922a0 walks singleton+4h/count+8h with a 30h action stride and invokes
existing 00a91e80 on every record, including disabled records. The reconstruction
reuses `rebind_input_action_00a91e80(record.bindings, groups)` directly.
Its class-index and stable-vector preconditions remain those documented in
`INPUT_ACTION_CLASSIFIER.md`.

All device and binding/modifier storage must remain valid and stable during a
poll. Every visited resolved cached pointer must be nonnull. Native code does
not repair corrupt resolved state or protect vectors from mutation by device
callbacks; this adapter does not invent such recovery. The scan helper alone
checks for null cached primaries. These are new typed C++ interfaces, not native
layouts or drop-in ABI replacements.

## Validation

The local probe executes verified copies of the three native poll/query/pair
functions in a separate Win32 process. Bytes match the disk image; only the two
direct poll calls, the paired helper's CRT call, and seven constant addresses
are relocated. All branch bytes remain unchanged. One fixture supplies device
virtuals and a float-spilled x87 sqrt service to both sides and records call order.

All 6,000 full poll and 400 direct paired-helper comparisons passed with MSVC
`/O2 /W4 /WX /fp:strict`. Input values
include zeros, positive/negative values, adjacent 0.5 values, subnormals, minimum
and maximum finite floats, infinities and quiet NaNs. Deterministic combinations
exercise empty/multiple bindings, unresolved/null caches, class/code gates,
modifiers returning 0/1/2, response curves, scale override and down-query
short-circuiting. All four record outputs and service call order match; float
bits match except NaN payloads. No tracked tests were added.

This establishes behavior under the probe's default masked floating-point
environment and explicit services. It does not establish native CRT error/
errno/SEH behavior, signaling-NaN payloads, floating-point exception flags,
non-default control modes, concrete device semantics, or gameplay behavior.
Build/CTest results are recorded in `reports/input_binding_poll.json`. The shared
source registry was leased by another harness, so the worker build includes the
new source through an ignored local CMake top-level include; integration must
register `src/input_binding_poll.cpp` in `cmake/startup.cmake`.
