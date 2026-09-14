# Actual effect-registry destruction

This packet binds the complete actual effect-registry clear/destruction chain to
the existing material-effect owner, record, string-pool and array-resize providers.
It creates no second cache, owner map, reference count, allocator or callback
fallback. Names are descriptive hypotheses, not recovered symbols.

| Entry | Full original span | Native contract |
| --- | --- | --- |
| B31750 | B31750..B317B9, 106 bytes | ECX registry; reverse clear; RET |
| B317C0 | B317C0..B317D6, 23 bytes | ECX actual0Ch array header; resize0/free; RET |
| B32010 | B32010..B3202E, 31 bytes | ECX unused; stack nonnull effect; RET4 |
| B320F0 | B320F0..B3214E, 95 bytes | ECX registry; object destruction; RET |
| B32200 | B32200..B32204, 5 bytes | JMP B320F0, same receiver and stack |

These are new C++ interfaces. They do not promise original private-stack/FH3,
register-return garbage or drop-in native caller ABI. No additional scalar-delete
entry is claimed, and none of these bodies frees the registry allocation itself.

## Borrowed graph and dependency ancestry

`NativeEffectRegistryDestructionContext` borrows the existing
`NativeMaterialEffectCacheContext` plus the original numeric D5F04C base table.
The existing context supplies the D5F074 derived table, D5E534/D61A00 effect
tables, actual string pool, real array allocation/free domain and loader's
canonical actual owners. All of them must describe the same application graph
and survive the full operation, including its cleanup. Unsupported current
profiles or table words are explicit source-domain errors.

The native registry normally occupies renderer+1A98. Its +04 data pointer,
+08 count and +0C capacity form the actual0Ch record-array header; records have
stride2C and their unretained effect identity is +28. Registry+10 is DWORD
accounting. No C++ container replaces this storage.

Substantive B30410 resize is reused from `native_renderer_cache_cleanup`,
dependency commit `8e1bdea084a82e0cb764a67757c83128dc4b8ff6`. That original commit
was merged into the isolated worktree, preserving ancestry, as `203f311f`.
Only the independent CMake additions conflicted; all were retained. Dependency
sources and metadata were not edited. B30410 retains its complete growth/shrink,
signed capacity, current count/base and partial-state behavior. B2FA10 remains
the existing complete actual effect-record destructor, including its returning
free tail and actual string-pool release.

## Clear and reference release

B31750 loops while the current DWORD count is nonzero. It loads current count
then current base to find the last effect, reads that effect's current profile
and +0C accounting slot, and subtracts the returned DWORD from current +10.
Both observed D5E534 and D61A00 tables resolve that slot to the existing
substantive A82250 zero-accounting leaf. The call is retained in source.

After accounting it reloads current count/base and captures the current last
effect. Only then does it load the registry's current profile and +10 release
slot. Both D5F04C and D5F074 resolve to B32010. Numeric original table words are
validated as data and never executed as addresses in the rebuilt image.

B32010 decrements the captured actual effect+04 first. The existing
`release_native_render_actual_owner` resolves an owner only when the decrement
reaches zero and validates that its companion borrows the exact same atomic.
The existing material-effect canonical terminal checks current virtual0
BD30E0 and the appropriate current scalar-delete slot, then executes real base
or derived destruction/deallocation. This packet does not perform another
decrement, lookup before zero, speculative retain, null substitution or owner
creation. Nonnull aligned live atomic storage is the original/source precondition.
Existing canonical terminal profile and nonthrowing disposal requirements remain
in force; arbitrary virtual profiles are not supported.

After release, B31750 reloads count. If nonzero, it loads current base, destroys
the current last record with B2FA10, and only then decrements the current count.
The count used for the decrement is reloaded after record destruction. If a
release changed count to zero, that record destruction/decrement is skipped.
The next iteration again checks current count; this is not a captured range.
When empty, it still calls B30410 on registry+04 with zero.

B317C0 calls the same substantive B30410 resize0, then reloads the current data
pointer and passes it to the cache context's actual BF6989 free binding. It does
not clear the freed pointer or capacity. No allocator family is silently changed.
The required free binding is the same one used by B2FFE0 record-array reserve.

## Destructor state and failure

B320F0 saves its actual owner, publishes D5F04C, and arms state0 before calling
B31750. On normal return it disarms before B30410 resize0 and current-base free.
The source shares that normal tail with B317C0 rather than cloning resize logic.
B32200 calls the same full destructor through the new interface; its native
body is only a tail jump.

The native handler at CBDCCB loads FuncInfo DF6660. That record has maxState1;
its sole unwind-map row at DF6658 is state-1/action CBDCC0. The action reads the
saved registry at [EBP-10], adds4 and jumps to B317C0. Thus an exception during
clear destroys the *current* array state, while a failure after disarming does
not run the array cleanup again. Source preserves this schedule with a single
armed cleanup object. A second C++ exception during cleanup terminates.

Prior accounting, terminal effects, name releases and count mutations are not
rolled back. The array cleanup destroys record storage, not another copy of each
effect reference. A failure before a remaining effect release can therefore
leave that actual ownership unresolved even when its record storage is freed.
The existing loader's retained failed-operation and terminal lifetime restrictions
are not relaxed by this destructor. Full renderer-parent EH composition remains
an integrator responsibility; this packet does not infer it from a saved parent
body or release any retained operation owner.

## Evidence, annotation repair and validation

All reads target the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through the repository wrappers. Eleven complete
live/disk spans cover the five entries, four observed tables, the FH3 handler
and its map/FuncInfo. The installed PE, saved exports, live bytes and disk
disassembly are retained independently. This worker makes no Ghidra mutations.

The initial live/saved B320F0 body again ended at B3213B, after B32137's call
to returning BF6989; B317C0 ended at B317D1 after B317CD's same call. Fresh full
bytes prove the remaining stack/FS restoration and RET at B3214E, and ADD ESP,4 /
POP ESI / RET through B317D6. These complete spans establish source coverage;
the truncated decompilation is not treated as the complete function. B32200's
decompiler expands the target body, whereas assembly shows its sole JMP.
Evidence was sent to the integrator for annotation repair. No x87 instructions
occur in the five bodies; ECX/stack argument provenance comes from assembly.

The report records the strict Win32 build, eight verified seeds, both configured
CTests, direct/tail call-site verification and immutable source/build/tool/runtime
evidence. No new test was added. The dependency's focused actual-storage fixture
covers adjacent B30340/B316C0/B31730 paths; B30410 has build and static evidence,
not original differential execution. This packet's new call schedule is checked
against complete assembly and existing actual providers. The dependency fixture
is not a runtime test of these new destructors. Original FH3 dispatch, active
renderer-parent destruction, unrestricted callbacks/SEH/concurrency and gameplay
remain unproved.

## Integrated validation at 33ac99a2

The integrated source at `33ac99a202f5630d2fe4e706609cb84fc7ffb7f9` passed the strict MSVC Win32 build and both CTests. Strict build and native static/call evidence only; no dedicated runtime fixture.

The packet passed 8 numeric direct-call checks. Reviewed names and evidence comments were applied, saved and read back in Ghidra while preserving previous comments. All 23 affected exports were forced after the snapshot refresh; eight repaired returning tails were checked in the live bodies and refreshed assembly. The combined checkpoint covers 22 new entry bodies including thunks and one overload of an existing body, with distinct per-entry validation scopes in the report. It retains 3390 immutable source, build, compiler, fixture and measured runtime artifacts at `local/checkpoints/33ac99a2/native-renderer-cleanup-refreshed/validation.json` (SHA-256 `9e496724999d084f197d606cf7351cc861f75a14050c0e5ad85c1ca145bef4c0`). Worker manifests and intermediate flow-repair reports remain historical inputs. Full renderer lifetime, active-frame execution, application adoption, native exception ABI and gameplay remain unvalidated.
