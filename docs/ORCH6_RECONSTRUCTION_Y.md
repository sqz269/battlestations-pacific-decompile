# Orchestrator 6 reconstruction batch Y

Addresses: 00955420, 008673B0, 00927B40, 0042E950, 00CA6EF9.

This closes the three already-assigned Y packets. No new reconstruction packets were dispatched after the request to finish and clean up the current set.

| Change | Evidence and limits |
| --- | --- |
| [Unit scene initialization](NATIVE_UNIT_SCENE_INITIALIZATION.md) | Complete 383-byte normal caller; four native/source cases preserve current-owner reloads, property lookups and SSE conversion. Health, Lua, allocation, property, flag and material providers remain required. |
| [Per-effect cleanup](NATIVE_WRECK_EFFECT_ITEM_CLEANUP.md) | Complete 238-byte normal body; eight native/source pairs, X-to-Y composition and a source failure check. Existing lock, reference and exact raw-array providers are reused. Getter/dispatch fixture bridges and untested native unwinding are explicit. |
| [Mission Lua self](NATIVE_MISSION_ENTITY_LUA_SELF.md) | Complete 162-byte getter using actual Lua 5.1.1, embedded Lua owner, pooled key and existing object tracking. Current-world/key callback and cleanup evidence is retained; original FH3 and Lua-error transport remain unproved. |

The older property-holder header comment now identifies +04 as a kind tag, matching the native producers and copy dispatch. The historical constant and stored value are unchanged. The supporting 16-byte name getter and ten-byte Lua-self EH selector were defined from verified bytes. Ghidra names/comments were saved with prior values preserved and affected exports refreshed.

MSVC Win32 Release and both existing CTests passed at `ab297da426c15402763569b6e242112a79810497`. Executable SHA256: `4d44239546c2b3aef1ef6f0a4c7e51ac6797df5e01dade76ad710b5896e005de`. The 120-frame USN01 compatibility run produced 18,557 finite trajectory rows, 241 unchanged Airfield2 samples, 2,400 avoidance queries, 1,080 cruise reads, 10,080 generic ticks and 420 valid world nodes. All 77 actual unit observer prefixes were torn down before manager drain while the owner remained live.

The mission queues remained empty through 240 passes. This run checks existing application behavior; it does not establish new scene/lifecycle gameplay execution. Native/source fixtures provide the narrower evidence described above. Full game reconstruction, host bindings, native ABI/exception/concurrency behavior and original visual/gameplay parity remain incomplete.

The archive retains 1345 worker artifacts and 6 metadata artifacts. Completed worker worktrees can be retired only after verifying their commits are merged, their tracked/untracked work is clean, and source/archive hashes agree. The orchestrator worktree retains the evidence archive.
