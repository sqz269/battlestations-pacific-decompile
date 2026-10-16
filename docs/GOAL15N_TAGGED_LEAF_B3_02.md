# B3 tagged leaf packet 02

Nine previously tagged, unreconstructed whole entries now have genuine C++
Source in `src/native_tagged_leaf_b3_02.cpp`. The strict MSVC Win32 `/MD /O2
/W4 /WX` object matches every live Ghidra and installed PE body byte and
instruction operand: **9 routines, 44 Native body bytes**. CMake registration
and Ghidra annotation remain with the primary integrator. Packet 01 files are
unchanged from `8898e52544bc0ff0f5f8a5a63486b5a534f4c5a6`.

## Qualification and dispositions

This continuation reused the same worktree and build cache, started with
`bsp.py brief`, and merged Main as `8447626c1a4fb2215791837ce2e62c3004ddec5f`
without conflicts before taking a new lease. No new shared CMake, packet, or
Ghidra changes were authored. The first packet's eight addresses were excluded.

Sixteen additional whole pseudocode and assembly bodies were inspected from
the previously uninspected 68 candidates. Thirteen received complete Source
coverage checks against the refreshed index and typed/raw `src`/`include`
coverage; none had an existing complete Source entry. Nine were completed.
Seven were deferred:

- `00B343E0`: ECX plus `[ESP+4]` input, `RET 4`, and a store/result at +84h;
  a separate exact Source calling boundary needs review.
- `00B343F0`, `00B34490`: absolute reads of actual globals `00E13078` and
  `0108FE94`; actual-storage binding work is outside this closed-leaf packet.
- `00B344C0`, `00B344D0`, `00B344E0`, `00B344F0`: eleven-byte zero-three-word
  bodies explicitly leave EAX=0, despite void pseudocode. Straightforward
  genuine three-`memcpy` Source emitted 23 bytes with immediate stores;
  `memset` emitted 17 bytes with an XMM operation. Those bodies are not claimed
  as exact Source. The rejected Source and actual COFF remain in ignored
  `zero_preflight.cpp`, `zero_preflight.obj`, and `zero_preflight_dump.txt`.
  No assembly workaround, new runtime fixture, or new test was introduced.

Across both packets, the original 79-candidate inventory now has 27 actual
bodies inspected, 17 completed entries, 10 deferred entries, and 52 uninspected
candidates. Candidate/index holes are screening hints; only individually
qualified complete bodies count as reconstruction.

## Complete Original contracts

All nine entries are two instructions ending with plain `RET`, with no stack
argument/cleanup, call, global access, branch, x87, or hidden dependency. Readers
consume ECX alone; constant entries consume no input register. No EDX argument
or receiver class is invented. Original C++ symbols and language-level calling
conventions remain unknown; chosen Source boundaries are one-argument
`__fastcall` readers and argument-free `__cdecl` constants.

| Original | Complete bytes | Machine operation |
| --- | --- | --- |
| `00B343D0` | `8b8184000000c3` | `EAX = dword [ECX+84h]` |
| `00B34450` | `8b4104c3` | `EAX = dword [ECX+04h]` |
| `00B34460` | `8b4104c3` | `EAX = dword [ECX+04h]` |
| `00B34470` | `8b4104c3` | `EAX = dword [ECX+04h]` |
| `00B34480` | `8b4104c3` | `EAX = dword [ECX+04h]` |
| `00B344A0` | `8b01c3` | `EAX = dword [ECX]` |
| `00B34500` | `b80a000000c3` | `EAX = 0Ah` |
| `00B34510` | `b806000000c3` | `EAX = 06h` |
| `00B34520` | `b810000000c3` | `EAX = 10h` |

Exact declarations are in `include/bsp/native_tagged_leaf_b3_02.hpp`. Readers
use `const void*` with exactly four readable backing bytes at the observed
offset; `memcpy` introduces no class, alignment, or field-meaning claim. The
constant meanings are unknown. Names are operation hypotheses, not recovered
symbols. No validation, guard, default, or global substitute was added.

Live xrefs and installed-PE address-literal counts are zero for all nine
accepted entries. No caller setups were identified to inspect. These are
observations, not proof of unused code or closure over indirect/generated
callers. Existing library names and interfaces were preserved.

## Verification and throughput trial

`bsp.py` live queries verify project `bsp`, configured saved project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language,
and image base using the existing verified client. The installed PE hash and
actual COFF/Source/header hashes are in the report. All nine complete bodies
match live/PE/COFF bytes and normalized instruction operands; body relocations
and undefined external symbols are zero. The 342-byte live capture span
contains 298 bytes outside the accepted bodies; only 44 bytes are counted.
No executable, runtime fixture, tracked test, or CRT call was added.

`./scripts/build.ps1` passed with both existing CTest suites (2/2). This Source
is unregistered; compiled-core inclusion is not established by that build.
Warm-cache build duration and full qualification-to-handoff preparation time
are recorded separately in the report, alongside the first packet's cold-build
duration. The measured start is the completed Main merge; the preceding brief
and merge command durations are separately included as a small start estimate.

Ignored artifacts remain under `local/goal15n_tagged_leaf_b3_02/`: `compile.cmd`,
`audit.py`, `leaf.obj`, `coff_dump.txt`, `whole_body_proof.json`, fresh live
proto/bytes/xrefs, rejected zeroing evidence, and `build.log`. Reproduce with
`cmd.exe /d /c local\goal15n_tagged_leaf_b3_02\compile.cmd`, then `python
local/goal15n_tagged_leaf_b3_02/audit.py`.

This proves complete Source and fixed-compiler machine emission, including the
observed register/result/stack contract. It does not establish runtime use,
class identity, game validation, installation, or binary deployment.
