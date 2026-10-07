# Complete session and transport countdown pair

Packet `cc11_health_transport_countdown` reconstructs two complete bodies:

| Entry | Exclusive end | Original bytes / instructions | Native ABI |
|---|---|---|---|
| `0076C4A0` | `0076C4F3` | 83 / 25 | ECX session, one stack float, RET4 |
| `00782840` | `00782862` | 34 / 10 | ECX transport, one stack float, RET4 |

The countdown names are descriptive hypotheses, not recovered symbols. New
Source functions are cdecl with actual backing, raw DWORD step and borrowed
constant bindings; they are not native ABI replacements. No new virtual
provider, callback, CRT helper, allocation, clock or queue is introduced.

## Complete ordinary behavior

The session function loads the actual double D7A280, then the original step
as a float, executes FCOMIP ST0/ST1 and FSTP ST0, and takes JBE for below,
equal **or unordered** input. Only ordered-above input loads the actual float
CE3800 with MOVSS and replaces the local step word. It then reads secondary
`session+18C`, calls the complete leaf if non-null, and only afterward reads
primary `session+188`. Each reached call has its own FLD/FSTP float argument
stage, even when the cap did not change the step. These conversions are not
replaced with raw copies: they have native NaN/denormal/status effects.

The leaf captures actual `transport+48` with MOVSS, executes **float COMISS**
against D7A260, then MOVSSes the captured word to its local stack slot before
JBE. Below/equal/unordered values are untouched. Otherwise FLD loads the
captured word, FSUB consumes the step, and FSTP writes the actual `+48` cell.
There is no floor at zero or -1. SSE comparison and x87 subtraction retain
their respective ambient controls and exception flags. No C++ floating-point
condition, min, NaN shortcut or host math helper implements either operation.

Primary and secondary may reference the same actual transport. Its second
call sees the first result, including a result crossing below -1 that makes
the second leaf skip. Session pointer fields are never cached as a pair.

| Borrowed actual cell | Width | Verified bits | Value |
|---|---:|---|---:|
| D7A280 | double | `3FE0000000000000` | 0.5 |
| CE3800 | float | `3F000000` | 0.5 |
| D7A260 | float | `BF800000` | -1 |

## Native evidence and connected original fixture

Read-only Ghidra queries verified the existing `C:/Users/sqz269/bsp.gpr`
project and `/battlestationspacific.exe` program. Both full bodies and all
three constant cells match the installed PE bytes. Original SHA-256 values:

- Parent: `11f71db6c6fd0bf2910e46d86c7041c4c4951db9c77bfef5ed25428da48a08d9`.
- Leaf: `cf3c6aff9485356dd6b51e78c7dab2543b13035aecbd6eaaa8938d52b4839b36`.

The fixture executes the complete connected **original 83+34-byte pair**.
Both original parent CALLs target the original leaf copy. Three absolute
constant operands point to the actual cells in a PAGE_READONLY mapping of
the frozen PE; Source borrows those same cells. Only five four-byte operands
are relocated: parent offsets 02/17/34/4C and leaf offset 09. All other 97
bytes are asserted unchanged. Copies are sealed executable after setup.
There is no Source forwarding thunk or downstream provider substitution.

The sole observed incoming edge is CALL `0077007E -> 0076C4A0` inside
`0076FFC0`. Its preceding FLD/FSTP already stages the caller's step argument.
The parent has two direct CALLs, at `0076C4D3` and `0076C4EB`, to `00782840`.
The leaf has no calls. All three call rows pass live static verification.
No indirect target, tail transfer or imported service is involved.

## Build, floating-point state and backing checks

Fresh MSVC 19.51.36244 Win32 compilation uses the new Source TU and probe TU,
with three immutable repository-source/header/probe inputs. Flags include
`/O2 /Gy /W4 /WX /fp:strict /MD /EHsc /FAs` and `/MANIFEST:EMBED /OPT:REF`.
The executable manifest requests asInvoker. Three current support libraries
were frozen read-only with source-before/source-after/copy hash equality
before permitting another main build. They are link inputs; the new functions
have no imported or reconstructed-provider dependency. Final source, snapshot,
probe, frozen-library and original-PE hashes remain unchanged.

Build and probe exit zero: **1,696 checks, zero failures**, comprising:

- 15 ordinary connected cases covering null combinations, distinct/aliased
  pointers, ordered cap, thresholds, infinities, signed zeros, NaNs and denormals.
- 36 connected arithmetic cases across PC24/53/64 and all four x87 RC modes,
  with independent MXCSR and pre-existing x87 invalid status.
- 24 connected SIMD/NaN/denormal cases across all four MXCSR rounding modes,
  DAZ/FTZ off or both on, fixed independent x87 control and sticky flags.
- One connected case with an empty incoming x87 stack.
- Eight direct original-leaf cases, including uncapped step and direct sNaN
  input, using one occupied x87 value and independent x87/SSE rounding controls.

Actual session backing exposes `188/18C`, and two 0xA60 transport backings
expose `48`. Entire transport results compare bit-for-bit, and complete
session/pointer/backing preimages remain unchanged except exact countdown
cells. These are explicit actual layouts, not full session/transport factories.

Inspected integer-only drivers FXRSTOR the same hardware seed immediately
before each call and FXSAVE immediately afterward. Checks compare full x87
control/status words, TOP, exception/condition/stack flags, tag occupancy,
all eight raw 80-bit x87 values, full MXCSR and its mask, and all eight 128-bit
XMM results. Incoming occupied x87 values and controls remain unchanged;
the tested scratch-register remnants also match. FXSAVE instruction/data
pointer and last-opcode/address metadata differ with relocated code/stack
addressing and are qualified separately, as are reserved/padding bytes.
No native CPU EFLAGS or general-register ABI compatibility is claimed.

The Source parent COFF is 117 bytes/43 instructions with exactly two REL32
relocations to the complete Source leaf. Its cap is at 1A/1C/1F/21/23;
secondary read/stage/call at 34/41/45/49; fresh primary read/stage/call at
54/61/65/69. The leaf is 49 bytes/18 instructions with no relocations:
capture/float-COMISS/stack-MOVSS/JBE at 15/1A/1D/22 and x87 subtraction/store
at 24/27/2A. Full emitted bodies and fixture drivers were reviewed.

Verification uses masked exceptions and sufficient free x87 slots, including
occupied incoming stacks. Unmasked traps, stack overflow, concurrent field
mutation, private faults and gameplay remain unproved. Exceptional helper
inputs do not assert observed caller reachability. Whole `0076FFC0` remains
open at its other producers and `00782870` transport+1C dispatch to unbound
host A42280/client A41130 providers. This packet performs no network delivery.

Machine receipt: `reports/cc11_session_transport_countdown.json`. Ignored
fixture: `local/cc11_transport_countdown_20261007_a/`. The worker changed no
shared metadata, CMake, ledgers, tracked tests or Ghidra state. Main integration
and the full main build belong to the primary integrator.

## Primary integration

Main `ea9896f474b29e787cc7fb05c2d386a532e7c177` passed the full Win32 build and all three existing CTests. Root independently reviewed the whole Source/native/299-line fixture and rebuilt two actual TUs with three current Source/header/fixture inputs (one compiler include), three current libraries and the original PE pinned before/after. The manifested independent run reproduced 1,696 checks across 76 connected original-pair cases and eight direct original-leaf cases. All 117 native bytes and three actual constant cells matched disk/live; fresh Source COFF bodies and integer-only fixture drivers were independently checked. All three direct calls passed. Floating-point, backing, caller, ABI and game qualifications above remain. No tracked tests were added.
