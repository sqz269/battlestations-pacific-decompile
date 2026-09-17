# Restored native compiler stream and pass providers (R87)

Addresses: 00BE40D0, 00BE4460, 00B1FF50, 00B44B10.

The four reviewed providers existed on `agent/orch5-20260911` at `769bdc3e3`
but were absent from main at `f881951f7`. R87 restores their exact source and
header into the current build, after rereading each complete native listing.
The source originated with `0097f52b`; this is integration of existing work.
The old reports and immutable archives remain historical evidence.

| Routine | Coverage and native ABI | Current validation |
| --- | --- | --- |
| BE40D0 | Complete normal physical DWORD writer; ECX stream, two stack args, RET8 | Original/source real WriteFile comparisons |
| BE4460 | Complete normal physical counted-string writer; ECX stream, two stack args, RET8 | Original/source real WriteFile comparisons |
| B1FF50 | Complete seven-byte capability address leaf; ECX renderer, EAX address, RET | Three original/source pointer arithmetic cases |
| B44B10 | Complete normal numeric-profile pass constructor; ECX88h owner, EAX same, RET | Strict compilation and current-library linkage; runtime open |

## Recovered behavior

BE4460 captures the length, writes it through current stream+54/BE40D0,
reloads name data, then writes that payload through current+28/BF4F50. Both
writes use the same optional count output: payload count replaces prefix
count. Null name data uses the borrowed original empty literal. BE40D0
forwards its actual value cell, four bytes and count output. The existing
physical writer performs real WriteFile and updates the low/high position
words with carry, without an added success check or cached-size update.

B44B10 calls the existing actual B5F720 base constructor, publishes D61BE8,
and initializes fields6C/70/74/78/7C/80/84 in recovered order. It registers
the same three state owners, allocates `white.tga` through the actual string
pool, then reads current renderer D5F0A8+64 and invokes actual B319B0 with
flags0. The persistent cache child exists before loading. The returned
owner transfers to84 without another retain; normal string return leaves
the raw header stale. The legacy callable-table constructor remains a
separate interface. Four binding slots and padding bytes remain unwritten.

The CBF3D0 FH3 descriptor has state0 base cleanup and state1 temporary-string
cleanup followed by state0. R87 verifies that metadata but does not claim
native exception delivery: the restored source retains failed child state.
It does not silently destroy an unresolved cache operation.

## Current verification and limits

Strict MSVC Win32 build and all three existing CTests pass. Thirteen spans
totaling463 bytes match live Ghidra and the installed PE; eight seeds match.
All guarded queries verify `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. No installation file is modified.

The retained prior fixture was adapted only to link these providers from
the current CMake library. Two original/source stream cases cover empty
and nonempty names, output-count replacement, null optional output and
64-bit position carry. Three capability cases cover zero, ordinary and
wrapping addresses. All pass. The link map resolves all four functions to
`bsp_core:native_material_compiler_providers.obj`; there is no source override.

The private original code relocates the empty-literal operand and real
WriteFile import-cell operand. This is bounded leaf comparison, not full
compiler or original whole-pass execution. The pass constructor is linked
but not executed in R87. Full compiler, cold texture/VFS loading, raw platform
message integration, original ABI/FH3/SEH, teardown, drawing and gameplay
remain open. The application is not rerun because it does not yet bind this
compiler path. No new repository test cases are added.

## Follow-up

The prior complete compiler requires missing actual shader construction,
sampler/cache entry and online/input message providers. Review and integrate
those dependencies in bounded packets, then bind and execute the full
compiler/post-effect graph. The R87 local dependency inventory records34
missing header/source files from the prior branch; it is a planning aid,
not proof that those components are independently ready.

Evidence and final integration receipts: `reports/native_material_compiler_providers_r87.json`.
