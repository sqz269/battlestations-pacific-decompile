# Source145 current-host CRT services admission

Root imports author `7c99122d270ed8523dc26d0bc66c772d592a8087` as
`5a36f048c` on published Main `cda8c653d1c894486437a6b8aa02c86270d13721`,
preserves its exact four new files and registers one Game source row.
Whole Root and independent Source review pass with zero findings. Separate
independent actual registered Code review and Root transfer also pass.

The stateless provider uses real SDK-declared current Win32 Release /MD APIs:
`_errno()` on every call; returning `_invalid_parameter_noinfo()` only after
checking five zero words; `_stricmp_l(left,right,nullptr)` only after checking
a null opaque locale. Both strings pass unchanged, including null. No errno
cell, locale, handler, ASCII fallback or Native owner is fabricated or cached.
Real CRT validation/handler outcomes propagate. Canonical game locale state
remains separate from UCRT private state.

All three registered endpoint sections transfer exactly to actual Root Code:
137 bytes, 45 instruction starts and 11 relocations. Root read all45 starts,
including guard ordering and the three genuine import cells. The actual Game
project compiles the TU once and its retained real link command includes it.
Build head `849516b5ee9f7fef856dc2263542aaf976c720a4` passed the official
Win32 build in25.3490634 seconds and existing checks in0.21/13.93 seconds;
3,896 frozen inputs and221 artifacts are retained.

Actual Root consumer binding, lifetime/caller thread and production CRT import
retention/resolution remain open. The independent worker map strips this unused
provider and its PE lacks `_stricmp_l`; object import proof is not retained PE
body or execution proof. No dummy consumer or retention anchor was added.
All nine GUI gates remain OFF. This grants zero Native/vendor credit and no
Original static-CRT/TLS/errno/handler/locale/ABI/Origin/fault/SEH/FP/runtime
equivalence. No tests, probes, Game run or Ghidra mutation were added.

Exact seals, current-host input fingerprints and actual Code/build provenance
are indexed in `reports/cc16_host_crt_stricmp_source145_admission.json` and
ignored `local/function_source_batch145/`.
