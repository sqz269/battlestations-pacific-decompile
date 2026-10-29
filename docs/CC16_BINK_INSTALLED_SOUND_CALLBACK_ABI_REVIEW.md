# Installed Bink sound callback machine-shape review

Independent qualified pass of `a9f6830184934f3ac9f06b976cf97f46c7f7be08`.
The claimed Win32 stack widths, callee cleanup and callback return dataflow are
supported by the installed file. No blocking discrepancy was found. This is
static machine-shape evidence, with no full ABI or runtime approval.

## Inputs and method

The reviewed input is `I:/SteamLibrary/steamapps/common/Battlestations Pacific/binkw32.dll`,
178440 bytes, SHA-256 `7f22055e8f62e228eb90f09846f9b668e0a0b1d29057af6e3b7bff1e08c0d724`.
Independent PE parsing confirms x86 machine `014C`, PE32 magic `010B`, preferred
image base `18000000`, and the three named, non-forwarded exports. All addresses
below are preferred addresses in this DLL, rather than BSP addresses or callable
host pointers. The two exploratory export windows remain windows, not established
function boundaries.

The source declaration is `include/bsp/bink_runtime_library.hpp`, SHA-256
`510edc8d0f710009872357e976d6fb62bfb9dca50430eb8100155d9713e66007`,
Git blob `73f927f786ef979edb0199a197e92164078a104b`. Its system opener takes one
`uint32_t` and returns `BinkSoundOpen`. Its `__stdcall` sound callback takes one
opaque state pointer, four 32-bit scalar arguments and one `BinkHandle*`, returning
`int32_t`. The setter takes that system-opener pointer and one 32-bit argument.

A separate read-only verifier parsed the actual PE and followed instruction
successors with a Capstone x86 worklist. It followed both conditional edges and
direct jumps, stopped at returns/traps, rejected overlapping instruction starts
and outgoing or indirect jumps, and treated calls solely as normal-return
fallthrough. It did not descend into or translate called vendor helpers. The
declared graph bounds, instruction counts and return sites reproduce independently;
this reachability model is not an execution or semantic-feasibility claim.
The verifier also rehashed all twelve primary artifacts, both exploratory code
windows and the source header, preserving the primary input records in the report.

## Machine evidence

| Entry RVA | Reachable instructions | All reachable returns |
| --- | ---: | --- |
| `CEA0` setter | 24 | `1800CEAA`, `1800CEE5`: `RET 8` |
| `11FC0` system opener | 66 | `18012007`, `180120B0`: `RET 4` |
| `11CD0` returned callback | 212 | `18011D07`, `18011EC5`, `18011FB2`: `RET 18h` |
| `D690` BinkOpen | 988 | `1800E5B3`, `1800E5F0`: `RET 8` |

At `1800CECA`, the setter reads its second DWORD argument, pushes it at
`1800CECE`, then calls the first argument through `EAX` at `1800CECF`.
The system opener's failure return has the unique predecessor `18012005`
(`XOR EAX,EAX`); its other return has the unique predecessor `180120AB`
(`MOV EAX,18011CD0`). That literal target lies in an executable PE section.
This supports the observed null-or-code-pointer result and one-DWORD system
callback footprint without establishing the meaning of the supplied argument.

The setter stores nonzero callback `EAX` into `1802E59C` at `1800CED5`.
A zero result branches over that store. Its following status calculation tests
the existing saved word and returns a normalized zero/nonzero result; it does
not prove that the latest system-opener call produced a fresh callback. This
qualifier is already present in the candidate. The source factory's ignored
ordinary sound result must not become a new success gate on this evidence.

The actual consumer at `1800E251` is `CALL DWORD PTR [1802E59C]`.
Six DWORD pushes occur at `1800E243`, `1800E244`, `1800E24B`, `1800E24C`,
`1800E24D` and `1800E250`; the continuation tests callback `EAX` without a caller
stack adjustment before its post-call stack accesses. Together with all three
`RET 18h` sites, this corroborates six DWORD stack arguments and callee cleanup.
It supports the declared scalar return width, not signedness, semantic parameter
roles or the layouts of either pointed-to object.

## Proof scope and retained artifacts

The separate verifier and result are retained under this review worktree's
`local/bink-installed-sound-callback-abi-review/`:

- `verify_static_contract.py`: SHA-256 `fd4eccd69d67f757706c49e634bc4733d365e920728fa4f451db7c61ccebb65b`.
- `independent_contract.json`: SHA-256 `7ffe3c7a30d85a78cf643c1c9521034d8b51e02e7be260c8b33fe73a98d0360c`.

The report preserves the primary module/header/artifact records and independently
derived instruction-byte fingerprints. No vendor SDK identity, opaque layout,
all-register convention, helper semantics, original BSP ABI/FH/SEH, fault,
nonlocal-exit or concurrency equivalence is established. Actual loaded-module
identity, decoder/audio success, callback completion, retention and unload
quiescence remain separate requirements. No DLL was loaded or called; no source,
guard, CMake, Ghidra or shared metadata changed, and no game, probe or test ran.
The source binding guard remains OFF. Native body, vendor body and activation
credit are all zero.
