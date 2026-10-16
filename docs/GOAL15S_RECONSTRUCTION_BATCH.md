# GOAL15S complete x87 leaf integration

The tagged-range sweep closes whole `00B40040..00B40046`: seven Native
bytes, two instructions, `FLD m32real [ECX+1F8h]; RET`. The Source uses
readable MSVC Win32 assembly because ordinary float-return forms introduced
extra stack accesses. The owning class and Original C++ declaration remain
unknown. The chosen Source interface has one actual ECX backing pointer and
an ST0 floating result, with no fabricated EDX or stack input.

Worker pin `e85d913c829fe1c13b850c052cec683415aa52a2` was independently
reviewed at max effort, with no blocking findings. The primary checked both
entire strict and registered executable COFF sections against fresh live
Ghidra and the installed PE: seven bytes, public symbol offset zero, no
executable relocations or undefined providers, and no extra executable
sections. The entire registered object equals its actual `bsp_core.lib`
member. `scripts/build.ps1` passed all three existing checks at Source
integration `796c9bda6`; no new tests or x87 runtime probe were added.

The original name, prototype and comments were captured before mutation.
The saved project now has provisional name
`BSP_NativeTaggedB4_LoadFloat01F8_00b40040` and verified
`float __fastcall(void * backing)`. Its affected export and snapshot were
refreshed and the index rebuilt. Existing tags and earlier evidence remain.

Identical instructions retain the addressed load and inherited x87 behavior
for equal incoming machine state. Absolute saved x87 instruction pointers
and fault addresses depend on actual code placement. Source placement,
Original caller adoption, binary replacement and gameplay remain unproved.

Worker full setup-through-handoff time was 1839.948 seconds; its full build
took 1310.656 seconds. Those costs, independent review and primary
integration must be included when assessing sweep throughput. A two-instruction
success alone does not establish a throughput improvement.

Machine-readable primary proof: `reports/goal15s_reconstruction_batch.json`.
Preserved author evidence: `local/goal15s_x87_author/`; independent review:
`local/goal15s_x87_review/`. Worker details remain in
`docs/GOAL15Q_TAGGED_X87_B4_01.md` and its corresponding report.
