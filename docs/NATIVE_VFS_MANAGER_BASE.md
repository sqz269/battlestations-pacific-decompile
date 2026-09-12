# Actual VFS manager base lifetime

Addresses: `00BDA6F0`, `00BDA790`, `00BDA8E0`.

This packet reconstructs the complete logical raw-storage constructor,
destructor and scalar deleting wrapper in `native_vfs_manager_base.hpp/.cpp`.
The source explicitly borrows the real `0109CEEC` publication cell and the
existing `SoundLifetimeAccess` bridge, which resolves either the actual raw
`01090AA0` manager or the canonical semantic fixture domain. It creates no
private singleton, pool, registration policy or CRT handler. Descriptive names
are hypotheses. The containing `BE1DC0/BE1F60` manager remains a separate packet.

| Entry | Coverage | Original ABI | Logical range |
| --- | --- | --- | --- |
| BDA6F0 | complete logical body | ECX owner, no consumed EDX/stack input; EAX original owner; RET | BDA6F0..BDA781, 145 bytes |
| BDA790 | complete logical body | ECX owner, no consumed EDX/stack input; no semantic result; RET | BDA790..BDA829, 153 bytes |
| BDA8E0 | complete logical body | ECX owner, DWORD stack flags; EAX original owner; RET4 | BDA8E0..BDA8FE, 30 bytes |

Ranges above are end-exclusive. Live stored bodies cover those same bounding
ranges, but the deleting wrapper's listing lacks `BDA8F5..BDA8F7`. Installed
and live bytes there are `83 C4 04` (`ADD ESP,4`), balancing the single cdecl
free argument before `MOV EAX,ESI; POP ESI; RET4`. This is a disassembly repair
request, not a new function or extension past the stored body. The worker
made no Ghidra mutations. Target-verified reads used `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Installed PE is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Storage and call schedule

Both base routines write `D683E4` only to `owner+0`; the destructor finally
writes `CE3818`. No remaining object byte is initialized, cleared or freed by
these routines. Constructor producer `BDA718`, destructor producers `BDA7B0`
and `BDA816`, and root cleanup body `412430` establish these profile DWORDs.
They are native table identities, not callable C++ virtual tables.

The complete listings preserve `EDI=ECX` at `BDA70A/BDA7AA` and capture the
first getter's `manager+10` section in ESI at `BDA723/BDA7C3`. No later EDI/ESI
write changes those values before register restoration. For nonnull sections,
`EnterCriticalSection` precedes a wrapping DWORD increment at section+18;
normal release decrements the same captured section then leaves it.

Constructor publishes the captured owner at `BDA746`, then calls the getter
again at `BDA74C`, reloads current `0109CEEC` at `BDA751`, and registers that
value at `BDA75A`. It returns the original owner, including when callbacks
change publication. The destructor calls its second getter at `BDA7E6`,
reloads current publication at `BDA7EB`, and removes that value at `BDA7F4`.
Removal is not based on captured `this`. Only after successful removal does
`BDA7F9` clear the publication. Neither path re-resolves the section for unlock.

| Callee | Reviewed body and source contract | Packet call sites |
| --- | --- | --- |
| 415350 | Actual existing manager getter; captured fast read, otherwise raw14h construction/publication; no added lock | BDA71E, BDA74C, BDA7BE, BDA7E6 |
| BD0C30 | Bounds validation may return; null ignored after validation; nonnull passed via its actual stack slot to append; RET4 | BDA75A |
| BCFCA0 | Null ignored, scan current vector and zero first matching slot, retain length/holes; RET4 | BDA7F4 |
| CE2218 import | Win32 EnterCriticalSection, one stdcall pointer | BDA737, BDA7D7 |
| CE2210 import | Win32 LeaveCriticalSection, one stdcall pointer | BDA768, BDA80C |
| BDA790 | Complete destructor above | BDA8E3 |
| BF65AC | Live five-byte JMP BF9DC8; existing actual matching source CRT free service | BDA8F0, followed by ADD ESP,4 |

The deleting wrapper always calls destruction before testing byte flags bit0
at `BDA8E8`. It frees the original captured ESI only when that bit is set,
returns that original address after free, and ignores all other flag bits.
Destruction failure never reaches the flag test/free. There is no null-owner
guard or array-delete branch. Xrefs show the base constructor called by
`BE1DDF`, the destructor consumed by the wrapper and full-manager cleanup
paths, and the wrapper referenced by table `D683E4`.

## Cleanup maps

Raw handler bytes were decoded separately; the four existing `Unwind@` helper
bodies were then verified live as eight-byte functions. No new function
boundaries are inferred for gaps or helper thunks.

| Routine | FuncInfo / unwind map | State0 to -1 | State1 to 0 |
| --- | --- | --- | --- |
| BDA6F0 | E002FC / E002EC, two entries | CC5FD0 loads saved owner `[EBP-18]`, jumps 412430 | CC5FD8 takes saved guard `[EBP-14]`, jumps 411EE0 |
| BDA790 | E00330 / E00320, two entries | CC5FF0 loads saved owner `[EBP-18]`, jumps 412430 | CC5FF8 takes saved guard `[EBP-14]`, jumps 411EE0 |

`412430` writes root profile `CE3818` and returns. `411EE0` writes guard profile
`CE37FC`, loads the captured section from guard+4, decrements its +18 word,
and leaves the section if nonnull. Base state0 is armed before the first getter;
guard state1 becomes armed only after Enter and its depth increment. State1
remains armed across publication, the second getter, registration/removal and
normal Leave. Source nested cleanup consequently restores the root profile
after a first-getter failure, or releases the captured section then restores
the profile after a second-getter/registration/removal failure. A constructor
failure retains the last publication value; a removal failure skips the clear.

The borrowed bridge and C++ catch/guard structure preserve source provider
exceptions. Original FH3/SEH dispatch, mutable EH-spill aliases, exact provider
register/throw identities and hardware faults during Enter/Leave are not
reproduced. In particular a native fault during normal Leave can re-enter the
armed guard cleanup; source RAII does not establish that SEH behavior.

## Verification

`scripts/build.ps1` passed under MSVC Win32 `/W4 /WX /fp:strict`; both existing
CTests passed after all eight seed ranges matched PE/live memory. The report
checker validated 12 direct calls/tail jumps with zero failures. The one retained
probe passed 93 assertions; all 37 files pinned before execution remained
unchanged afterwards. `reports/native_vfs_manager_base.json` records the native
byte hashes, build result and retained fixture pins.
The ignored probe compares all three normal native bodies with the source using
actual raw singleton-manager storage, both absent and present captured sections,
current-publication removal, untouched bytes and flags `80000000/80000001`.
Native getter/register/unregister/free calls are explicitly redirected to the
existing canonical source services; imported section operations use Win32.
These are bounded native/source comparisons, not unmodified game execution.

The same probe exercises returning and throwing registration validation in the
existing semantic lifetime domain. It checks callback publication writes,
captured registration argument, profile restoration and guard depth. It does
not execute original FH3 exceptions, force a getter allocation failure, or
establish arbitrary removal-fault behavior. Source/header/object/library/probe
inputs are physically retained before first execution and hashed afterwards;
failed compile attempts, if any, remain in the same local evidence directory.
No permanent tests were added. Build/fixture results do not establish binary
ABI replacement, original CRT identity or gameplay validation.
