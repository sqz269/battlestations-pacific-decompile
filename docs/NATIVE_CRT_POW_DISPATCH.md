# Complete CRT pow branch selection

The original BFEB10..BFEB63 entry is 84 bytes and 23 instructions. It consumes
two x87 values, checks the current109EEA0 dispatch word and the original MXCSR
and x87 mask conditions, then selects the complete SSE2 wrapper or x87 fallback.
The following BFEB64 alternate entry is outside this function.

The source adds one stable context word. It preserves each original mask test,
the LEA cleanup that carries either comparison's flags, and the fallback's
ordered FXCH, FSTP x, FST y and stored-y high-word input to BFEB6D. It reads only
the selected provider context. The fallback saves/restores the four nonvolatile
registers used by its complete source binding before allocating its original
20-byte argument/scratch frame. The SSE2 transfer uses a fixed context-forwarding
entry and the complete C19260 source wrapper. No host power function, forced
dispatch or new floating-point environment policy is introduced.

The original absolute dispatch cell is borrowed current storage. Its saved
zero fill is not evidence that the runtime flag is zero. Existing source
provider contracts, including their real owning CRT state, literal regions,
callback identities and current TLS/PTD storage, apply to reached paths. Added
private source stack layout and code addresses are qualified; no native SEH,
nonlocal-exit restoration or original caller compatibility is claimed. Incidental
final EFLAGS equality is also excluded: the final fallback ADD ESP uses a shifted
stack address and can produce a different parity flag. Branch decisions retain
their original flag producers.

Complete source, the strict main build and the complete emitted dispatcher
proof have passed. The actual SSE2/libm/decoder chain has also passed full
static replay against the same frozen main archive. No new routine has run.


Primary integration passed against the frozen main library. Complete 84-byte/23-instruction dispatcher maps to 112 bytes/39 instructions plus the exact 17-byte/five-instruction context helper. All original operands, branches, current-control tests, context offsets and three actual bindings checked. The strict Win32 build passed both existing CTests and eight reference seeds with 56 unchanged source/header/build inputs. Fourteen actual archive members, 14 compiler commands and 266 read dependencies are frozen under `local/pow_chain_build_frozen/`; reviewed proof replays are under `local/pow_chain_primary/`. Ghidra names/comments are saved, prior values retained, reconstruction records registered and all affected exports refreshed. These static checks do not establish runtime, original caller ABI, native SEH or game behavior.


The complete existing fallback dependency set was also revalidated: all nine
actual objects are byte-identical to the prior reviewed build, including all
121 COFF sections and 51 CODE sections (5,028 bytes). All 32 compiler-read
repository source/header inputs are LF-equal. The unchanged verifier passed
for 13 raw functions totaling 1,537 original bytes. That fallback relocation
proof uses synthetic addresses; the dispatcher image separately establishes
actual linked calls and archive membership. The earlier source ABI and CRT
startup/FP/SEH boundaries remain in effect.
