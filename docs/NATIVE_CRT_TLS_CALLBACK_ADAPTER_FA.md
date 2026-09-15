# Native CRT TLS allocation callback adapter (FA)

Addresses: 00C0504C

FA reconstructs the complete nine-byte adapter at `00C0504C..00C05054` with
its original MSVC Win32 ABI. It calls the real `KERNEL32.dll!TlsAlloc`
import and discards the incoming FLS callback word without invoking it.
There is no source context, replacement provider, TLS emulation or owned
CRT global. The descriptive name is a hypothesis, not a recovered symbol.

| Entry | Coverage | Original bytes | Original ABI |
| --- | --- | --- | --- |
| `00C0504C` | complete, 9 bytes / 2 instructions | `FF 15 B8 20 CE 00 C2 04 00` | `DWORD __stdcall(PFLS_CALLBACK_FUNCTION)`; argument at entry `ESP+4`; `RET 4` |

The original `CALL [00CE20B8]` is six bytes; `RET 4` begins at `00C05052`
and ends at `00C05054`. Fresh Ghidra reads match the installed PE for this
entire body, all 388 bytes of its producer, the exact Fls* strings and the
four TLS import words. The import table names `TlsAlloc` at `00CE20B8`.
These are analysis-image and disk observations, not loaded-game IAT reads.

At entry ESP=S, the adapter's CALL pushes a return address for the actual
zero-argument provider; its ordinary return restores ESP=S. The adapter's
`RET 4` returns with ESP=S+8. Its argument is never explicitly read. It
returns the provider's complete EAX value, including `FFFFFFFF` on failure,
without translation or LastError restoration. CALL/RET themselves do not
change flags or general registers; the provider's effects remain visible.
Nonvolatile preservation depends on the real Win32 provider ABI. Callback
cleanup, index publication, process/thread teardown and CRT initialization
remain the responsibility of their actual owners.

## Actual producer and call contract

The full live body of `00C053DC __mtinit` ends at `00C0555F`; all sites below
belong to that body. The complete 109-instruction listing is retained.

| Site(s) | Native operation | Evidence and consequence |
| --- | --- | --- |
| `C053E2` | `CALL [CE215C]`, GetModuleHandleA | exact `KERNEL32.DLL` at `D69FD0`; null module branches to existing failure cleanup |
| `C05404`, `C05411`, `C0541E`, `C0542B` | `CALL ESI`, actual GetProcAddress captured from `CE20F4` | exact FlsAlloc / FlsGetValue / FlsSetValue / FlsFree names; results stored at `109DE34/38/3C/40` |
| `C0542D..C05455` | test all four results | any zero result enters `C05457`; all four nonzero results retain FLS providers |
| `C05457..C05476` | replace the entire quartet | getter from `CE20BC`, allocator=`C0504C` at `C05466`, setter captured from `CE20B4`, free from `CE20B0` |
| `C0547B`, `C05496` | actual TlsAlloc then TlsSetValue | creates getter-cache index `E15B00` and stores the raw getter; failure retains existing branches |
| `C054AB/BB/CB/DB` | four calls to original `C04F67` | helper results replace the four provider cells; `ADD ESP,10h` at `C054E0` removes the four outgoing words |
| `C054F1`, `C054F6`, `C054FC`, `C05501` | push real callback `C05246`, push allocator cell, call original decoder `C04FDE`, pop ECX | decoder uses one cdecl word and returns selected pointer; POP removes only the decoder argument |
| `C05502` | `CALL EAX`, selected actual allocation provider | the callback word remains stacked: FLS uses it, FA ignores it and consumes it with `RET 4`; result becomes `E15AFC` at `C05507` |

The adapter and allocator-cell xrefs are retained in full. They show the
fallback publication and producer read/use sites; they do not prove that
arbitrary runtime indirect references cannot exist. The decoder and encoder
listings were inspected read-only to verify their original one-word cdecl
interfaces. Neither owner is reconstructed by FA. In particular, the
existing reconstructed decoder's extra source context is not an original
ABI bridge and is not needed by this leaf. FA does not change DD's reserved
global page or provide __mtinit/termination closure.

## Ghidra integration plan

Ghidra remains unchanged. `C04FDE` ends at `C0504B`; the next existing
function starts at `C05055`. `C0504C` has no Ghidra function, so the report
lists inclusive end `C05054` under `no_ghidra_function`. The primary may
define `[00C0504C,00C05055)` using the normal write lock only if a permitted
typed mechanism preserves existing instruction definitions, then review/apply
`BSP_CRT_TlsAllocIgnoringFlsCallback`. Do not expand the decoder body or
use `disassemble_first`. Preserve current library names and comments.
The failed plate-comment read for this unassigned address and successful
neighbor/producer comment archive are retained. The worker did not rename,
comment, define, alter a prototype, export over saved evidence or save Ghidra.

## Validation boundary

The packet's report records the strict `scripts/build.ps1` result, original
seed verification and existing math tests. No new test is introduced. The
existing tests do not invoke this adapter. Full compiler command/read/write
records, consumed headers, object, archive, extracted member and normal game
image/map are retained with the exact tools and import library used.

Static comparison requires the complete nine-byte section, one COFF DIR32
relocation at byte 2 to `__imp__TlsAlloc@0`, and unchanged `RET 4`. A forced
DLL with `/NOENTRY` extracts this member from the actual built archive and
resolves it through the real kernel32 import library; its two instructions
match the original after the single IAT operand relocation. This DLL is
never loaded or executed. Its body is present; the ordinary game's complete
map and image omit this adapter, and no source caller has been added.

This establishes complete original-ABI source and static instruction/import
proof. It does not establish original-address installation, startup adoption,
TLS callback teardown, loaded OS/provider state, fault behavior, CRT closure
or gameplay validation. Every attempted command, failed capture and method
is retained in `local/native_crt_tls_callback_adapter_fa`. Two SHA256/SHA512
inventories cover the complete local payload and each declared external raw
path spelling. Only explicitly listed seal paths are excluded from self-hash
recursion; nested seal-like basenames remain covered.
