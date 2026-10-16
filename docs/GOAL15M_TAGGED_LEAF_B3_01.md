# B3 tagged leaf packet 01

Eight previously tagged, unreconstructed whole entries now have genuine C++
Source in `src/native_tagged_leaf_b3_01.cpp`. Each fixed MSVC Win32 `/MD /O2 /W4
/WX` emitted body is byte-identical to both live Ghidra and the installed game
PE: **8 routines, 31 bytes**, excluding alignment padding. CMake registration and
Ghidra annotation are deliberately left to the primary integrator.

## Discovery and scope

The supplied `00b30000_leaf_candidates.json` contained 79 screening candidates.
The worker inspected 11 actual whole exported bodies: the eight below plus
`00B33E50`, `00B343B0`, and `00B3FF30`. The latter three were deferred to keep
this packet a contiguous read/address family; 68 candidates remain uninspected.
An inventory hole alone was not counted as a reconstruction. All eight accepted
entries were checked against the refreshed index, typed/raw `src`/`include`
address coverage, neighboring `render_fx_cc13.cpp`, and
`docs/CC13_SHADER_VTABLES.md`. The last document already records `00B34320` as
class-unidentified. No existing complete body or library identity was found.
No existing interface or correct library name was changed.

## Complete Original contracts

All entries consume **ECX only** and finish with plain `RET`: no consumed EDX,
stack argument, stack cleanup, global, call, hidden dependency, branch, x87, or
no-return behavior. The Source uses one-argument `__fastcall`; this describes
the chosen C++ boundary, not a recovered Original language-level convention.

| Original | Complete bytes | Operation/result | Source suffix |
| --- | --- | --- | --- |
| `00B34300` | `8b01c3` | `EAX = dword [ECX]` | `read_dword_00b34300` |
| `00B34310` | `8b4104c3` | `EAX = dword [ECX+04h]` | `read_dword_04_00b34310` |
| `00B34320` | `8a410cc3` | `AL = byte [ECX+0Ch]`; upper EAX retained | `read_byte_0c_00b34320` |
| `00B34330` | `8b4110c3` | `EAX = dword [ECX+10h]` | `read_dword_10_00b34330` |
| `00B34340` | `8d4104c3` | `EAX = ECX+04h` modulo 32 bits | `address_04_00b34340` |
| `00B34350` | `8b4120c3` | `EAX = dword [ECX+20h]` | `read_dword_20_00b34350` |
| `00B34360` | `8b4114c3` | `EAX = dword [ECX+14h]` | `read_dword_14_00b34360` |
| `00B34370` | `8d4118c3` | `EAX = ECX+18h` modulo 32 bits | `address_18_00b34370` |

Names have namespace `bsp` and prefix `native_tagged_b3_`; declarations are in
`include/bsp/native_tagged_leaf_b3_01.hpp`. Readers take `const void*`, with
readable backing required for exactly the observed width/offset. Their result
is `std::uint32_t`, except the byte reader's `std::uint8_t`. Both address entries
take and return `std::uint32_t` address bits, retaining wraparound without
imposing C++ pointer-arithmetic preconditions. No receiver class, field meaning,
validation, default, or guard is introduced. `memcpy` expresses word reads
without imposing alignment or a speculative class layout; the actual optimized
object has no CRT or other undefined external symbols.

All eight live xref queries reported no references. The installed PE also has
zero literal occurrences of each four-byte entry address. Thus there are no
identified caller setups to inspect; neither observation proves the entries
unused or excludes indirect/generated callers. No common owner-class identity
is claimed. Names remain operation hypotheses.

## Validation and reproduction

Every `bsp.py ghidra` command verifies configured project `bsp` (saved project
`C:/Users/sqz269/bsp.gpr`), program `/battlestationspacific.exe`, x86 language,
and image base through the existing verified client. The retained ignored
directory `local/goal15m_tagged_leaf_b3_01/` holds:

- `compile.cmd`, `compile.log`, `leaf.obj`, `coff_dump.txt`: actual MSVC 14.51
  Win32 strict object and symbol/relocation/disassembly output.
- `audit.py`, `whole_body_proof.json`: independently parse installed PE and
  actual COFF, disassemble complete bodies with Capstone, compare live bytes,
  instruction operands, and relocations, and record hashes.
- `live_count.txt`, `live_proto.txt`, `live_bytes.txt`, `xrefs_*.txt`: fresh live
  evidence. The 116-byte live span includes 85 bytes of alignment; only 31 bytes
  are counted as complete Original body coverage.
- `build.log`: required `./scripts/build.ps1` result, separately recorded in
  the report: build passed and both existing CTest suites passed (2/2).
  This unregistered Source was validated by its actual strict COFF;
  the repository build does not establish its compiled-core inclusion.

From this worktree, run `cmd.exe /d /c
local\goal15m_tagged_leaf_b3_01\compile.cmd`, then `python
local/goal15m_tagged_leaf_b3_01/audit.py`. All eight Original/live/COFF bodies
match exactly; body relocations and undefined externals are zero. The byte
getter emits `MOV AL`, retaining Original upper-EAX bits, with no added
zero-extension. No runtime fixture or new tracked test was necessary for this
whole-body identity proof. No ad hoc executable was produced.

The report records elapsed qualification and implementation from worktree
creation, plus the explicitly unmeasured short orientation before creation.
This is Source and fixed-compiler emission evidence, including the consumed
register/result/stack contract. It is not game validation, installation, dynamic
caller closure, recovered class semantics, or a drop-in binary deployment claim.
