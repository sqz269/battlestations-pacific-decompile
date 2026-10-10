# Raw game-state enqueue: primary acceptance

Root independently accepted worker `5d31f649bfcf8b0bf18a7e8ba0c9ed73d04d3d82`. It copied all 449 retained artifacts, replayed the guarded portable verifier from the copied location, checked 4,224 actual worker Git objects/current files separately, and matched all 4,206 current Root Source files under stated newline normalization. Root decoded the complete Original `004D3ED0` body independently against the original PE: 117 bytes and 46 instructions. The accepted startup epoch is unchanged; pending particle-model headers are outside this review.

The leaf captures the incoming embedded queue, conditionally grows the map, rereads capacity/head/count, and subtracts capacity at most once from its index. A missing slot requests a 16-byte block; the map is reloaded before publishing it. Only after final destination selection does the nonzero arm read the caller's pointer and one DWORD value. A zero computed destination skips both reads and the value store but still increments the then-current count. The value width is four bytes; 16 bytes is block allocation size. No allocation-null fallback, eager value snapshot or cached-map equivalence is established.

The leaf's `RET 4` closes the preceding wrapper's cleanup assumption. Growth cleanup, allocator behavior, nonvolatile preservation and exceptional paths remain external. Portable audit enumerates 24 syntactic normal-return paths under its declared provider assumptions; those counts do not establish feasibility for valid queue states. EAX retains the computed destination on normal return; no semantic return API is inferred.

No C++ activation, build, test, Native execution or Ghidra mutation is credited. Production raw queue ownership, provider behavior and deferred menu/profile/settings lifetime remain open. Next bounded frontier: `004D20D0`.

Root receipt: `local/cc12_game_state_enqueue_primary/Root_review.json`, 201,625 bytes, SHA-256 `d0fdb853af2519a0320e62be4f0b5ef1aa8f1faa3f57316c8ae31f395f595b5c`. Primary report: `reports/cc12_game_state_enqueue_owner_primary_review.json`.
