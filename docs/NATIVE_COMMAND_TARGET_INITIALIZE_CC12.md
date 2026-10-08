# Raw native command-target initializer, CC12

`initialize_native_command_target_004f1830` reconstructs the whole native
`[004F1830,004F1870)` body: 64 bytes, 15 instructions, zero calls and zero code
relocations. Its name is a hypothesis. It exposes caller-owned raw storage;
neither native class identity nor a running command/world owner is established.

The original PE and saved program `/battlestationspacific.exe` in
`C:/Users/sqz269/bsp.gpr` agree with the emitted Source COFF and unique linked
body. Whole-body SHA-256:
`d63a15a125ffd391d4da60d40c27c914a8b8cd8c571f6fe0abdae1a582c723f0`.
The preceding sealed `cc11_adjacent_4f18_leaf_audit20261008a/audit.json`
(`2623f4f1be4c908c1ac4f978b338997e9a70fabd6a866bbf67aebcdd476fa989`)
provides the caller-storage and native-data investigation. Supported live
Ghidra queries rechecked the complete body and data before and after this family.

## Physical contract

The new declaration is `void* __fastcall(void* actual_ecx, void* unused_edx)
noexcept`. The second formal places a caller-provided value in EDX; the original
entry consumes ECX, consumes no stacked operands and preserves EDX. Genuine
native callers use `LEA ECX` into their own stack storage. The admitted receiver
is stable, aligned, writable caller-owned storage covering all `18h` bytes.
There is no null/size guard, temporary vector, allocation, class adapter,
`hostmake_command_target` delegation or unresolved-call stub.

| Native address | Ordered effect |
| --- | --- |
| `004F1830` | Read the actual dword at `00F87574` into XMM0. |
| `004F1838` | Copy entry ECX into EAX. |
| `004F183A` | Store XMM0 low dword to receiver `+08h`. |
| `004F183F` | Fresh read of the actual dword at `00F87578`. |
| `004F1847` | Store XMM0 low dword to receiver `+0Ch`. |
| `004F184C` | Fresh read of the actual dword at `00F8757C`. |
| `004F1854` | Store XMM0 low dword to receiver `+10h`. |
| `004F1859` | XORPS clears all 128 XMM0 bits. |
| `004F185C` | XOR clears ECX and defines the final arithmetic flags. |
| `004F185E` | Write zero WORD at receiver `+00h`. |
| `004F1861` | Write zero DWORD at receiver `+04h`. |
| `004F1864` | Write zero BYTE at receiver `+00h`, retaining the overlap. |
| `004F1866` | Write zero WORD at receiver `+02h`. |
| `004F186A` | Store zero XMM0 low dword at receiver `+14h`. |
| `004F186F` | RET with no stack-argument cleanup. |

EAX returns the identical entry receiver; ECX is zero. EDX, EBX, ESI, EDI, EBP,
ESP, XMM1 through XMM7, x87 state, MXCSR and DF are preserved. Final defined
XOR flags are CF=0, PF=1, ZF=1, SF=0, OF=0; AF is undefined and is not asserted.
Numeric MOVSS encodings in Source avoid an extra DS prefix from the MSVC inline
assembler. All remaining instructions use normal inline assembly.

## Actual native global prerequisite

Production requires genuinely readable absolute addresses `00F87574/78/7C`.
It creates no mapping, fallback, replacement provider or default vector. These
cells belong to the writable `.data` virtual zero-fill tail, with section flags
`C0000040`; there are **zero on-disk file bytes** for these cells. Their current
saved-program 12 zero bytes match PE loader initialization, SHA-256
`15ec7bf0b50732b49f8228e07d24365338f9e3ab994b00af08e5a3bffe55fd8b`.
This does not establish immutable constants or running-process values. Capped
READ xrefs and escaping MOV/PUSH addresses do not prove absence of writers.

The three global loads remain interleaved with output stores. If receiver
storage aliases native data, it must be writable, and later loads observe prior
writes in the literal native order. Whole identical code establishes that order.
This family does not execute mutable-global alias cases or invent nonzero global
values. Native runtime data provision and a native world remain unbound.

## Fresh worker evidence

The unique ignored family is
`local/cc12_command_target_initialize_worker20261008a` in the worker worktree.
It consumes exactly two accepted TUs: the new Source CPP and the ignored
`probe_v2.cpp`; it consumes zero BSP archives. MSVC 18 Community
14.51.36231 Hostx64/x86 and SDK 10.0.26100.0 compile with `/MD /O2 /W4 /WX
/fp:strict /permissive-`. The accepted executable embeds the default asInvoker
manifest, whose XML and exact requested execution attributes pass preflight.

Before any successful process entry, `static_gate_v4.json` verifies all original,
Source COFF and linked 64 bytes, 15 complete instructions, zero calls, no code
relocations and one linked occurrence. The ordinary generated caller loads EDX
from `[ESP+8]`, ECX from `[ESP+4]` and jumps directly to Source. The complete
229-byte raw caller is also gated: it records the actual argument registers,
flags, ESP and full FXSAVE state around its actual call, then restores the
surrounding C++ caller's floating state and DF.

The standalone fixture reserves `00F80000` for 64 KiB before opening artifacts,
commits `00F87000`, copies the actual PE loader-derived 4096-byte page and makes
it readonly. All 4096 bytes are qualified virtual zero-fill. Original's
unmodified 64 bytes execute from a separate RX allocation. The production
function performs no such setup. The receiver is a real stack-owned 24-byte
member with 16 preceding and 24 following guard bytes inside the same declared
caller allocation; no heap object or class facade is used.

One accepted process launch executes Original once, Source once through the
same raw caller and Source once through the ordinary generated caller. Original
and Source use receiver `001AE4E0`, EDX `D5C4B3A2` and identical captured entry
GPR operands. All three full outputs are 24 zero bytes for this admitted loader
snapshot; both real adjacent guards remain intact. EAX identity, ECX zero,
EDX/nonvolatiles/ESP, XMM0 all-zero, XMM1-7, nonempty seeded x87 state and MXCSR
`3F80` pass. Flags change from `0603` to `0646`, preserving seeded DF=1 and the
defined XOR results. The full page and both complete live code bodies remain
unchanged.

All consumed Source/project files, 108 compiler-reported headers (including
extensionless standard headers), selected compiler/resource tools and five
actually searched system libraries have before/after hashes. Whole original
PE, saved-program code/data and 119 prior context/artifact files are bookended.
Prior receipts are preserved context; no old fixture or accepted native launch
is replayed. The family records one failed probe compilation (C4324 alignment
padding), one failed link (missing `rc.exe` on PATH), and one malformed manifest
rejected by CreateProcess with WinError 14001 before program entry. Corrected
artifacts retain the accepted Source/probe COFF without recompiling them; all
failed files and attempts remain sealed. There is one accepted native launch.

| Artifact | SHA-256 |
| --- | --- |
| `recipe_v4.py` | `91d540638f49439e364659ecced84913fed41874c2354dc304b914cc6a96ee1d` |
| `static_gate_v4.json` | `f143c1e1b98c62c41434ff306f9fa1ff8992b43dc30b7e4ea7ccb3f5198e5df1` |
| `runtime_v4.json` | `22571a6b4f4d41ff4f4cc0b7ad16598dabeaad8377965e21a85301e40246a9dd` |
| `seal.json` | `f2233c5d55629b8ac976b9b7f678933778056c98ece60bc957a902a44af3b310` |

This is whole-body raw-memory reconstruction, standalone strict-build proof and
one frozen-loader-snapshot fixture. Root integration owns CMake registration,
the full Win32 build, existing tests, independent fresh fixture and Ghidra
annotation/export. Class integration, runtime global state, mutable-global alias
execution, startup and gameplay are not validated here.

## Primary integration

Whole raw64B15 native-memory initializer, literal Original=SourceCOFF=unique linked body, zero CALLs/relocations. Actual ECX receiver, unused EDX padding formal, RET0/EAX receiver/ECX0, EDX and nonvolatiles/ESP/DF preserved; full XMM0 zero with other SSE/x87/MXCSR preserved. Literal interleaved MOVSS DATA reads and sized output writes retained. Production requires actual readable F87574/78/7C; writable alias destinations are caller prerequisites. PE .data cells/page are mutable loader-zero virtual fill, not immutable file constants. Independent fresh2TU main fixture uses complementary receiver/guard/EDX/XMM/MXCSR poison, actual same stack receiver and caller, one Original raw/one Source raw/one ordinary Source call; whole-body and both caller ABI gates before sole execution, complete output/guards and readonly qualified exact-VA loader-snapshot page bookends. All prior artifacts unchanged; worker objects/BSP archives not linked; no replay/new tracked tests. Mutable-global alias execution, live native DATA provider, class/world/startup/game admission remain unbound.

Exact main Source build `2a2cf9277b43596887685c69e1919cd13f478709` passed all three existing CTests. Independent fresh two-TU family `local/cc12_command_target_initialize_primary20261008a` passed once, with seal `45c381e0c6098cfbda52c5c4e2c58e732dd1aa1dc568040fb60970bf59979fe8`. Full-build core `f6672d6f8a55c7caa1e2bac51e2f1555715f9bda72411c222991990479545636` is context only; the fixture consumes no BSP archives. Saved analysis receipts follow in the report.
