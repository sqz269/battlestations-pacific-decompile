# Canonical CRT unlock entry

Addresses: `00C11B31..00C11B45` inclusive, 21 bytes.

`bsp::unlock_native_crt_canonical_00c11b31` supplies the complete original
one-word cdecl unlock entry. It reads the actual fixed descriptor table at
`00E16478` and invokes the real `KERNEL32.dll!LeaveCriticalSection` import.
It requires caller-owned initialized lock state; it does not complete native
lock initialization, acquisition, PTD, SEH, startup or gameplay closure.

| Routine | Coverage | Source | Original ABI |
|---|---|---|---|
| C11B31, desired `__unlock` library-name hypothesis | complete, 21 bytes / seven instructions | `src/native_crt_canonical_unlock.cpp` | One 32-bit word, cdecl; original EBP frame; plain RET, caller removes argument |

The frozen source pin is `15cc82222fe06a3081ccf5c5baad78e8a47c0fba`.
The worktree helper initially selected another main commit; the clean branch
was explicitly pinned before taking evidence. The local packet is
`local/canonical_crt_unlock_fn/`, with summary
`reports/native_crt_canonical_unlock_fn.json`.

## Original entry and import

Every live query used the existing `bsp.py ghidra` read interface with autostart
disabled. Its client verifies project `bsp`, program `/battlestationspacific.exe`,
x86 language and `00400000` image base before each query. The configured project
is `C:/Users/sqz269/bsp.gpr`. No Ghidra mutation, restart, export refresh,
flow/prototype/comment repair or bridge script gate change was made.

The installed original executable is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
Its SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Fresh analysis bytes agree with this PE for the whole entry, all 36 eight-byte
descriptor rows and the four-byte IAT slot. PE import ownership at `00CE2210`
is exactly `KERNEL32.dll!LeaveCriticalSection`.

The seven original instructions are:

```asm
00C11B31 PUSH EBP
00C11B32 MOV EBP,ESP
00C11B34 MOV EAX,[EBP+8]
00C11B37 PUSH DWORD PTR [EAX*8+00E16478h]
00C11B3E CALL DWORD PTR [00CE2210h]
00C11B44 POP EBP
00C11B45 RET
```

The signed source type describes one raw 32-bit stack word. The assembly performs
the original 32-bit scaled address calculation with wraparound, reads only the
descriptor's first DWORD, and passes that pointer to the real stdcall import.
It performs no signed C++ multiplication, index/null check, initialization,
extra argument, private state, provider substitution or error conversion.
EBP is saved/restored; the import retains its actual volatile-register and flag
effects. PUSH/MOV/POP/RET add no arithmetic-flag changes. The return type is void;
no synthetic success value is written over the import's resulting EAX.
Invalid descriptor storage may fault before the API call; invalid lock state
retains the actual API behavior. The source/archive entry does not reproduce
original code placement or fault/return PCs. Its presence in the archive does
not establish admission or reachability in the game executable.

## Storage and initialization boundary

All 36 pointer DWORDs in the installed initial table are zero. Lock 12 is row
`00E164D8`: initial pointer zero, second DWORD one. These are initial image values,
not observed runtime values. Existing `GameNativeMutableCrtData` commits and
copies the exact initialized `E15000` and `E16000` PE pages and zero-fills
`109E000`. This supplies table/storage admission only.

The actual producer `C11A93 __mtinitlocks` iterates 36 rows; for a second DWORD
equal to one it publishes a pointer from `109E1C0`, advances by 24 bytes, and
calls the actual `C17653` initializer with spin count 4000. Failure clears that
row pointer and returns zero. `C11C21 __lock` lazily establishes missing entries
through `C11B5E`, handles the original failure path, then calls the real
`EnterCriticalSection`. Their bodies were read for ownership evidence only.
The frozen source has no implementations of these native owners; the existing
`C17643` no-spin primitive is a different entry. Neither source presence nor
page admission authorizes invoking this unlock with an initial null pointer.

FH's `NATIVE_CRT_THREAD_INIT_CLOSURE_FH.md` and FK's
`NATIVE_CRT_CANONICAL_LOCALE_REFERENCE_FK.md` describe the corresponding PTD,
locale and lock-12 boundary. This leaf supplies one previously missing unlock
entry; successful actual initialization, acquisition, lifetime and matching
native frame/scope ownership still belong to the callers and original producers.

## Bounded caller evidence

The fresh xref listing requested up to 100 rows and contains 36 call sites. The following four complete
cleanup bodies were inspected; coverage is a bounded sample, not an exhaustive
contract for all callers or indices. The implementation itself retains every
32-bit index without a policy change.

| Containing function | Call site | Setup | Caller cleanup | Body end |
|---|---|---|---|---|
| C051AE, PTD cleanup | C051B0 | C051AE PUSH 12 | C051B5 POP ECX | C051B6 RET |
| C01AE9, locale updater cleanup | C01AEB | C01AE9 PUSH 12 | C01AF0 POP ECX | C01AF4 RET |
| C028DA, setlocale cleanup | C028DC | C028DA PUSH 12 | C028E1 POP ECX | C028E2 RET |
| C028E6, setlocale cleanup | C028E8 | C028E6 PUSH 12 | C028ED POP ECX | C028EE RET |

Live proto/body boundaries and exact CALL instructions support these rows.
The existing `tools/verify_report_calls.py` check on the final report verifies
all four direct caller rows with zero failures. Its single indirect import row
is explicitly skipped; the separate PE/IAT and whole-body byte evidence resolves it.
The stale local snapshot lists 44 callers and a `___crtExitProcess` callee;
neither is used as current contract evidence. The live callee query returns
only `LeaveCriticalSection @ EXTERNAL:00000159`, consistent with the complete
seven-instruction body and PE import.

The first xref read used the CLI's default 25-row limit. Its output and initial
local count audit are preserved, but that count was superseded after inspecting
the CLI limit handling and requesting the complete bounded listing. It is not
reported as the total caller count.

## Validation and retained artifacts

The required eight original seed spans are verified before the single
`scripts/build.ps1` invocation. That script builds MSVC Win32 Release with
the existing strict options and runs the two existing math CTests. No test is
added and those tests do not exercise unlock.

Before the owned object exists, the precompile guard records and retains the
exact source/header, build registration and relevant owner/verifier inputs,
plus the installed toolset and both x86 compiler-host candidates. The postbuild
guard requires unchanged hashes and retains the exact owned CL command/read
blocks, including the exact source and header paths. Grouped write tracking
retains the selected source/output positions and full original batch hashes
without copying unrelated compiler inputs or the build tree.

The object audit requires all 21 body bytes to match after substituting only
the genuine four-byte DIR32 operand at offset `0Fh` for
`__imp__LeaveCriticalSection@4`. The fixed seven-byte indexed PUSH at offset
`06h` remains exact. The actual object, identical unique `bsp_core.lib` member,
archive member header, linker-symbol uniqueness and archive digests are retained.
The machine-readable report records the observed outcomes and hashes.

The first object audit failed its exact symbol lookup because the adapted
template still matched a malformed locale-derived name. The original audit
method remains unchanged under its pre/post guard, with its failed output.
The separately retained second method corrects only that matcher to the actual
decorated unlock symbol and passes the complete byte/relocation/member checks.
No compiled input changed and no second build was run.

`__unlock` is a descriptive CRT library-name hypothesis based on body/import
and neighboring lock ownership. The previous inventory name/comment is retained;
the desired annotation is for the primary to apply later. No live name or
comment was changed by this packet.

This is static complete-body and caller-ABI evidence with a build and existing
math fixtures. The relocatable source entry/import operand is not original placement.
No native unlock, lock/init, probe, PTD/SEH path, original body or game was
executed. Runtime fault/unwind behavior and gameplay remain unvalidated.
