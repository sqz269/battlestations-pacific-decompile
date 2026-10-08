# CC12 physical WriteFile raw entry

`raw_write_native_physical_stream_00bf4f50` adds the complete `00BF4F50..00BF4F86` body to the existing compiled physical-stream module. Whole Original, normal-production COFF and linked proof passed, followed by three fresh real-OS Source/Original pairs. This is a raw ABI fragment for an already counted Original function: zero new Original functions. Ordinary ports and previous raw entries are unchanged.

## Complete body and ABI

The entry has 55 bytes and 21 instructions. ECX supplies actual stable 20h backing, with HANDLE at `+8h`; incoming EDX is unused. The caller stacks data, requested count and an optional count pointer. The raw function returns the actual count in EAX, the optional pointer in ECX, and executes `RET0Ch`. EDX is unqualified volatile state.

```asm
mov edx, [esp+4]
push esi
push 0
mov esi, ecx
mov ecx, [esp+10h]
lea eax, [esp+10h]
push eax
mov eax, [esi+8]
push ecx
push edx
push eax
call dword ptr [00CE2290] ; KERNEL32.dll!WriteFile
mov eax, [esp+0Ch]
add [esi+10h], eax
mov ecx, [esp+10h]
adc dword ptr [esi+14h], 0
test ecx, ecx
pop esi
jz done
mov [ecx], eax
done:
ret 0Ch
```

`WriteFile` receives the address of the caller's requested-count DWORD (`entry ESP+8`) as its count output. That requested value has already been copied into the API's argument list. The raw entry does not initialize the scratch, inspect the API BOOL, retry, or call a failure service. Its EAX comes from the overwritten count slot. It adds that count to cached position `+10h/+14h`; the intervening MOV preserves carry for ADC. The optional count store follows the entire position update, including when the valid output pointer aliases the position field. Cached size is not refreshed.

Microsoft documents that [WriteFile clears its count destination before work/error checking](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile). The raw optional pointer is a separate post-call output; even when it is null, the actual API receives the nonnull stack-count destination and null OVERLAPPED.

## Whole-body and real-OS identity

The family is `local/cc12_physical_write_raw_entry_20261008a/run01`. All 55 live Ghidra bytes agree with the installed Original image. The normal-production COFF body is complete and has exactly one `DIR32` relocation, `__imp__WriteFile@20`, at byte 26. Only that four-byte operand is normalized. The unique raw linker symbol/start and unique complete linked 55-byte body are checked, together with exact whole-object membership in the real current core archive. The Source IAT is `2001F024`, backed by the actual PE import.

The current Source raw constructor `00BF50D0` is independently proved whole 43-byte equal across Original/current COFF/linked code without normalization. Before any fixture production entry executes, loaded entry addresses and all body bytes are checked, and the Source IAT is verified against the actual OS export. The Original 55 bytes remain unchanged in private RX memory; only the actual `00CE2290` import cell is privately mapped read-only and bound to the same real `WriteFile` export. No callback or import surrogate replaces Windows. The actual import/export implementation in this process resolves to `C:/Windows/SysWOW64/kernel32.dll`.

## Three fresh real-file pairs

Each Source/Original pair uses distinct, simultaneously live `CreateFileW` handles and separate fresh files with identical 32-byte initial contents. Each raw call gets actual guarded backing established by the current Source raw 43 constructor. Its whole 64-byte constructor output is checked before installing that call's HANDLE and test cache words. The constructor retains literal Original profile `00D691B0`; no Source class/table is fabricated. No Original constructor runs.

| Pair | Requested / EAX / overwritten slot | Final cached position | Actual file cursor | Optional output and LastError |
|---|---|---|---|---|
| Null output, 64-bit wrap | 8 / 8 / 8 | `00000000:00000004` | 5 to 13 | Null ECX; both errors observed `5A5A` |
| Output aliases position low | 5 / 5 / 5 | `12345679:00000005` | 3 to 8 | ECX actual `backing+10h`; both errors observed `5A5A` |
| Invalid HANDLE, separate output | 7 / 0 / 0 | `89ABCDEF:76543210`, unchanged | 11 to 11 on valid control files | Guarded separate count becomes 0; both errors 6 |

For the invalid case, both backing HANDLE fields are `FFFFFFFF`. The log's `Source_handle`/`Original_handle` columns identify the separate live control files used to observe unchanged bytes and cursors.

For the alias case, the initial cache is `12345678:FFFFFFFE`; adding 5 produces `12345679:00000003`, and the subsequent optional store makes the observed low word 5. The intermediate arithmetic value follows from the complete body; it is not a separately instrumented observation.

Every call checks all 64 backing/guard bytes against its exact expected position effects and its own actual HANDLE. No cross-pair HANDLE normalization is used. Guarded payload, separate-count guards, cached size, profile, references and `+Ch` remain correct. The complete actual 32-byte files, file lengths and cursors match. The caller's popped requested slot, EAX, actual ECX output pointer, ESP/RET0Ch and EBX/ESI/EDI/EBP are checked. Successful EDX values actually differ between Source and Original; EDX is recorded and never compared or canonicalized. EFLAGS are unqualified. LastError is captured immediately; success preservation of seeded `5A5A` is only an observation for this run. All six temporary files are deleted on close.

There are six instrumented write calls and six current Source constructor input calls. Executed production bodies total 98 Source bytes (55+43) and 55 Original bytes. Harness file initialization/inspection uses ordinary Windows APIs. No previous fixture is replayed, and Source read `BF5030` is neither changed nor invoked: its unresolved failure-owner path is outside this packet.

## Build and physically retained inputs

`./scripts/build.ps1` passed all three existing CTests: `reconstructed_math`, `native_math_differential`, and `tool_tests`. There are no new permanent tests. The two-object/zero-BSP-archive link failed with 10 unresolved externals (exit 1120); its exact recipe/log is retained. The genuine current core, Lua511 and zlib121 archives then linked successfully, with no stubs/adapters and no admission of their unexecuted ordinary providers. The probe embeds an `asInvoker` manifest. The whole-body gate and first runtime attempt both passed; there is no hidden discarded static run.

The manifest physically freezes the whole production object/archive, both explicit objects, the executable, Original image/body inputs, selected actual headers/libraries, compiler backends, OS DLLs and recipes, with equal pre/post/copy hashes. It includes 202 normal-module inputs, 164 consumed probe headers, 19 searched libraries, 16 recipe/tool records, 46 compiler/backend files, two configuration tools, two normal-build recipes and nine generated recipes. The generated normal MSBuild project is brought current before freezing; its actual compiler command and project remain unchanged through the build. No tracked CMake file is edited.

Candidate scanning is distinct from retained copies: this run hashes 550 library candidates, freezes the 19 searched libraries, and leaves 531 unused candidates uncopied. Of 6030 header candidates, 164 are frozen by the probe-header selector; normal-module inputs are independently retained. Candidate hashes do not establish copied artifacts. The executable also has an explicit frozen copy, not only a hash of its runnable output path.

See [the report](../reports/cc12_physical_write_raw_entry.json), `run01/whole_write_proof.json`, `run01/inputs/manifest.json` and `run01/execution.log`. Scope remains actual stable backing and the demonstrated synchronous file/HANDLE cases. Other alias patterns, async/pipe/device/partial writes, Source class/table/lifetime, raw substream dispatch, universal CPU/OS behavior, startup and gameplay remain unqualified. Ghidra and shared-ledger registration remain primary-owned.

Primary acceptance verified the current integration build against the complete retained worker providers and whole archive members. Details: `local/cc12_physical_write_primary/review.json`. The combined MSVC Win32 build passed all three existing checks. No new runtime execution was performed during primary review. Retained historical build recipes remain authoritative for that build; subsequent unrelated recipe additions do not change the frozen provider evidence.
