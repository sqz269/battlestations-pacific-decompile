# Registry and constructor-cleanup listing repairs

The primary coordinator repaired these bodies while the owning workers kept
Ghidra read-only. Commands ran from each owning worktree under its existing
address lease and the tools' write lock; the resulting records are held in
the primary worktree. Project: `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`.

| Entry | Repair | Final listed instructions |
| --- | --- | ---: |
| `00441360` | Extend stored end from `00441399` through `004413BD` | 27 |
| `00441840` | Restore returning-delete stack cleanup at `00441855..00441857` | 11 |
| `0081B0A0` | Extend stored end from `0081B0D7` through `0081B0DC` | 20 |
| `0087C260` | Restore returning-delete stack cleanup at `0087C282..0087C284` | 21 |
| `00879240` | Restore stack cleanup and POP EDI at `0087926D..00879270` | 29 |

Before recreation, the complete 94-byte registry teardown and 61-byte point
array teardown matched the installed PE and live Ghidra bytes. Raw instruction
graphs end at their final RETs. Full function documentation was archived
before and after. The two recreated entries had ordinary FUN names and no
recovered signature to replace; existing plate comments were preserved.

The registry tail is semantically significant: after free returns it clears
publication `00E17BF4`, writes the surviving owner's profile/vtable word to
`00CE3818`, then restores the exception registration and returns. The previous
truncated pseudocode omitted these operations. The point-array tail contains
stack cleanup, saved-register restoration and RET. No original bytes or global
callee no-return flags were changed.

All five live readbacks now have zero flow gaps. The project was saved and
exports refreshed. The first three commands reached their post-save display
step and raised a path-formatting ValueError because the record was outside
the worker root; successful save events and independent live readback establish
the actual result. Later commands used an equivalent relative record path.
Those formatting errors were not treated as failed mutations or grounds to
repeat the repair.

Records: `reports/native_lifetime_flow_repairs_orch4_d4.json` and
`reports/native_lifetime_body_repairs_orch4_d4.json`; private before/after
documentation and byte preflight are under `local/`. This packet repairs
analysis evidence; source and runtime claims belong to the separate registry
and damageable-construction packets.
