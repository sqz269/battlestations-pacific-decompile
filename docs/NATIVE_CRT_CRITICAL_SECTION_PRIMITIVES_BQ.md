# Native CRT critical-section primitives

This packet reconstructs the complete `00C17643` fallback and `00C17639`
encoded-word store. It provides no part of the `00C17653` wrapper. The source is
`src/native_crt_critical_section_primitives.cpp`, with the public borrowed-state
interfaces in `include/bsp/native_crt_critical_section_primitives.hpp`.

Base: `0b3bbf7e21a202d2a31dcaba3e6c2631737b415b`. Prior discovery:
`01fc5c40538266cd29f1ab33e479401c70603be1`,
`docs/NATIVE_CRT_LOCK_INIT_FRONTIER_BP.md`. All 80 prior local artifacts were
checked against their committed size, SHA256, SHA512 and exact path inventory
before reuse. That worktree is unchanged. Fresh complete live spans from the
verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` match both the
retained discovery and the installed PE. The machine-readable evidence and
whole packet-local inventory are in
`reports/native_crt_critical_section_primitives_bq.json`.

## Complete native contracts

| Entry | Complete body | Recovered native ABI | Source ABI |
| --- | --- | --- | --- |
| `00C17643` | `00C17643..00C17652`, 16 bytes | stdcall, actual critical-section pointer at entry ESP+4, ignored spin DWORD at ESP+8; RET 8 | Same argument slots and cleanup, descriptive C++ symbol |
| `00C17639` | `00C17639..00C17642`, 10 bytes | cdecl encoded DWORD at entry ESP+4; plain RET; EAX retains supplied bits | cdecl encoded DWORD plus reference to the actual writable `0109E454` DWORD |

The live database prototypes still say `undefined ... (void)`; argument and
return contracts come from the complete instructions, native caller evidence
and real import, not those provisional prototypes. Existing library names are
preserved; no Ghidra annotation or listing changes were made.

`00C17643` pushes the original pointer and calls through original IAT
`00CE220C`, which the PE imports as `KERNEL32.dll!InitializeCriticalSection`.
The Windows callee consumes its one argument. The fallback then executes
`XOR EAX,EAX; INC EAX; RET 8`. The spin argument is never read. On normal API
return EAX is exactly one, and the XOR/INC arithmetic-flag schedule is retained
(CF, PF, AF, ZF, SF and OF are zero after the increment).
An API exception or memory fault prevents that normal-return sequence; this
source has no catch, error conversion, retry, or invented failure policy. It
borrows the caller's real 24-byte Win32 critical-section storage and does not
allocate or publish a private object.

`00C17639` loads the input into EAX before its single DWORD store to native
`0109E454`, then returns. The supplied encoded bits are unchanged; this is not
an encoder, decoder, initialized-cache owner, or pointer validator. EAX retains
the input although the native caller does not consume a semantic return value.
The native MOV/MOV/RET sequence preserves arithmetic flags and other registers.

The source setter adds a borrowed reference to the stable actual destination.
Its exact sequence is `PUSH ECX; MOV EAX,[ESP+8]; MOV ECX,[ESP+0Ch];
MOV [ECX],EAX; POP ECX; RET`. This preserves EAX/input, ECX, nonvolatile
registers and arithmetic flags while making exactly one destination store.
The actual destination must not alias this entry's stack frame or argument
storage. The extra argument, stack save and destination-address load change
the native stack and fault domain; the context interface is not the original
one-argument binary entry. No private static cache is present.

## Ownership boundary remains open

Native `__init_pointers` at `00BFBDFB` receives encoded null from `00C04FD5`
and calls the setter at `00BFBE0A`; `__mtinit` calls that initializer before
`00C11A93`. Initial PE/database zero at `0109E454` does not substitute for that
real encoded-null publication. The retained discovery records the complete
76-byte initializer and its direct calls, along with the external `__mtinit`
excerpt. Those owning functions are not implemented by this packet.

The complete 197-byte `00C17653` wrapper remains open. It decodes the actual
cache word; on decoded null it obtains the actual OS platform, conditionally
chooses this fallback or dynamically resolves
`InitializeCriticalSectionAndSpinCount`, encodes the selected pointer and
publishes it before activating its native SEH4 scope. The current invocation
uses the retained raw pointer. Its platform-getter error path calls `00BF65BB`
with five zeros; retained bytes include the omitted `00C1768E ADD ESP,14h`
continuation if that call returns.

The wrapper's omitted filter/handler at `00C176D9..00C17707` is retained with
the complete body and scope table at `00E037B8`. It handles exception
`C0000017` by its native conditional `SetLastError(8)` path and zero result;
other codes continue search. This scope excludes earlier decode, platform,
Watson, dynamic lookup, encode and cache-publication operations. The source
primitives add no approximation of that scope or Watson policy.

Existing `00BFBAB2`, `00C04F67` and `00C04FDE` source providers expose qualified
borrowed-state interfaces; their canonical errno/TLS/PTD/module-gate ownership
is not supplied here. Native `00C07C00`/`00C07C45` frame providers and
`00BF65BB` Watson ownership remain separate. `00C11A93` still needs the actual
36-entry lock table, 14-object static pool and full wrapper behavior; these
two primitives do not establish an initialized lock domain or close PTD/heap
ownership.

## Static and build validation

The strict MSVC Win32 build uses `/W4 /WX /fp:strict /MD /std:c++17`. Eight
native seed comparisons and the two existing CTests passed. No new tests or
runtime experiments were added. Compiler command/read/write tlog records,
compiler metadata, source inputs, object, full archive and extracted unique
archive member are retained locally.

The fallback emits 16 bytes, identical to native except its one four-byte
COFF DIR32 relocation at offset 6 to
`__imp__InitializeCriticalSection@4`. The call remains an actual import call.
The setter emits the exact 13-byte sequence documented above, with no
relocations. Both functions have one archive symbol definition in the one
matching object member, byte-identical to the current build object. The object
contains no writable sections. Complete native/compiled disassembly and the
explicit argument-binding difference are recorded in the report.

There are zero direct calls in these two native bodies; the standard direct
call checker therefore checks zero rows. The single indirect import call is
independently qualified by the original PE import table, complete call bytes
and current compiler relocation. Build and artifact checks do not establish
native-frame, drop-in binary, or game/runtime equivalence. The next owning
boundary remains the full `00C17653` wrapper with its real state and SEH4/Watson
dependencies, not a wrapper assembled from invented callbacks.
