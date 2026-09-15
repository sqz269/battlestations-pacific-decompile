# Container and SoldierClass listing repairs

Addresses: 0087B950, 0087C380, 004B1210, 004B1120, 004B1280, 004B1310,
and new definition 004AF520.

Complete live bytes matched the installed image before mutation. Every change
used the owning worktree's lease and the shared Ghidra write lock in the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; no game bytes changed.
Old metadata and comments were captured, the project was saved and exports were
refreshed. Reports retain tool events and whole-span SHA256 preflight evidence.

| Entry | Repair | Final listing |
| --- | --- | --- |
| 0087B950 | Clear erroneous free CALL_RETURN overrides and decode BADE..BAE0, BB18..BB23. | 753 bytes, 286 instructions, no gaps. |
| 0087C380 | Clear erroneous free CALL_RETURN overrides and decode C4FA..C4FC, C52E..C539. | 692 bytes, 264 instructions, no gaps. |
| 004B1210 | Restore destructor tail through 004B127E and recreate body preserving comments. | 111 bytes, 39 instructions, no gaps. |
| 004B1120 | Restore destructor tail through 004B119F and recreate body preserving comments. | 128 bytes, 38 instructions, no gaps. |
| 004B1280 / 004B1310 | Restore each three-byte ADD ESP after free. | 30 bytes, 11 instructions each, no gaps. |
| 004AF520 | Define previously missing MOV AL,1; RET after byte preflight. | 3 bytes, 2 instructions. |

The restored registry destructor clears its head/count, clears E187F0 and writes
the singleton-base profile after node release. The restored SoldierClass
destructor clears its map header then calls real base cleanup 00489E80 at
004B118A. Both scalar deleters return the captured owner, not an undefined
decompiler temporary. These repaired destructors remain source-absent in this
packet; recovered listings are evidence for their next dependency closure.

Records: `reports/native_container_flow_repairs_orch4_f6.json`,
`reports/native_soldier_body_repairs_orch4_f6.json` and
`reports/native_soldier_definition_repair_orch4_f6.json`.
Related source evidence: `docs/NATIVE_SOLDIER_CLASS_RESOLUTION_ORCH4.md`,
`docs/NATIVE_DAMAGEABLE_SECTION_VECTOR_ORCH4.md` and
`docs/NATIVE_DAMAGEABLE_FAKE_EFFECT_VECTOR_ORCH4.md`.
