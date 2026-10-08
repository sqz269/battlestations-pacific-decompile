Type8 fresh prelaunch code peer review

Independent raw parsing and whole-Main/control/frame review found **no blocking
static mismatch** in the selected fresh t8p3 artifacts. Source credit is **0**.
This report is neither approval nor evidence that a target process ran.

The worker family `J:\PROG\battlestations-pacific-decompile-cc12_type8_fresh_prelaunch_code_audit\local\t8preaudit20261008a` contains **210 sealed recursive files**, including
all utilities, initial analyzer stops, intermediate results and complete listings.
Seal SHA256: `a99d36e5818112ba9790c2c880760facf8e006987425e1eff6cf2120d0a4b5c2` (34121 bytes).
Ten stable readouts were sent to Root before final packaging. Their full pins
are in the [report](../reports/cc12_type8_fresh_prelaunch_code_audit.json).

The input selection follows the actual recipe's 22-file current_review_bindings
list, matched symbolically from its AST without importing it. Three actual frozen
GAME/UCRT/VCR copies came from frozen_inputs.json, plus seven current readouts and
metadata files. All selected 32 files and current Source4 matched before/after.
The Root family was unlaunched and unsealed when selected; no final inventory was
inferred and no runtime file was read. Installed providers and Native were not queried.

The independent decoder reproduced 32 logical / 30 physical TU bodies,
287 logical / 281 physical relocation operands plus six alias rechecks, and
all 48 whole gated spans: 30 TU bodies and 18 helpers. All mapped executable
definitions, full weak mode1/3 chains, complete relocations and aliases are
accounted. No mapped EH handler was present. Chkstk43, cookie14, delete16->5->free6,
all ten normal FF25 import thunks and cold std exception helpers remain covered.
Internal unreachable NOP alignment is proven separately from trailing padding;
the reachable CxxThrow INT3 remains an actual bound guard. Both nonabsolute
call sites have explicit formal/dispatcher bindings; none remain unresolved.

All 5715 bytes / 1456 Main instructions were read. The independent CFG confirms
28 dominance ordering pairs and 78 guard edges that cannot reach the successful
zero return. The one dispatcher call at49002A23 uses the Source/RX/Source/RX
array at ESP+60/+64/+68/+6C. The successful path contains exactly four entries,
counts7/11/11/7 and flagsD6B23900/7C48E100/A53D6C80/2E917F02, all DF0. Failure
paths may exit earlier; no dynamic entry count is claimed.

Frozen GAME Original112 equals its selected bytes. Source keeps all104 literal
positions and all34 instructions except the two CALL operands. The allocator
bridge's private suffix71adab9d is recovered from the actual Source relocation;
its61-byte/17-instruction body routes the real requested size into canonical
allocation. Raw capture191/68 and ordinary wrappers26/7 and25/7 preserve the
DATA/COUNT/FLAG stack contract at T+4/+8/+C with target RET12. Capture140 has
108 register bytes at offset16 and post guards at124. Dead argument words are
captured before PUSHFD. Ordinary wrappers remain dynamically uncalled.

Four56-byte roots and both live owned children11/7 are checked with complete
guarded input48/capture140/root56 snapshots. All live snapshots precede the two
child frees, then four root frees, then RX release. Code/provider gates precede
root allocation and follow frees. Retained input pointers never enter the owned
child free list. This is a reviewed success-path contract, not an observed heap trace.

Type8's8D5 flag mask includes AF; copy flags come from ADD(T-24,16). ES is captured
but machine_ok has no ES equality branch. No blanket ES/FPU/MXCSR preservation
through providers is admitted. Provider verification before roots includes
MEM_IMAGE, mapped/physical NT paths, held-handle identity/hash, PE/export/IAT and
prefix comparison. Post-free checks are inlined into Main and repeat IAT,
held-handle FileID/size, physical hash and prefix comparison only. They do not
repeat MEM_IMAGE/path discovery/GetProcAddress/live PE headers. Serialized
normalized_live_prefix is expected relocated raw bytes compared live, not an
independent live dump.

No Root recipe/helper/reader/compiler/target ran in this review. Only the worker's
independent static tools and metadata utilities executed. Cold EH/OOM/GS,
private CRT internals, owning-class lifetime and game behavior remain unadmitted.
