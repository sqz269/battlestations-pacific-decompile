# Raw game-state queue growth: primary acceptance

Root accepted worker `2d7031ba3609c8cba61cc6c81e94f5d1897cdfc1` as provider/owner readiness. The complete native `004D20D0` body is 341 bytes and 129 instructions. It takes the raw queue in ECX and an incoming increment DWORD on the stack, can promote that actual argument only when the preferred growth fits remaining capacity, and returns with `RET 4`. This closes the enqueue review's growth argument-cleanup assumption.

Allocation receives a register count at `004B8C90`, and the body reloads current map/capacity after allocation and around copy calls. It also reloads the incoming increment and saved new pointer. After copies/fills it frees the currently published old map, adds to the current capacity, then publishes the late-loaded replacement. Provider, alias, reentry and failure effects remain explicit; no rollback or safe allocation behavior is invented.

The verified bytes contain `004D220E: ADD ESP,4`, absent from Ghidra's 128-row listing. They establish continuation after free through the capacity and map writes. Root retained additional typed flow and instruction-context reads. No Ghidra flow, listing, body or signature repair was performed, and the listing gap alone does not identify its cause.

Root copied all 447 retained artifacts, ran the copied guarded portable verifier, independently decoded the complete body and checked it against the untouched original PE. Separately it checked 4,224 actual Git objects, the frozen Source tree at both base/growth commits, and all 4,206 current Source files in each worktree. The two later model-header changes were qualified against the exact accepted typed-view epoch. It does not claim the worker original-worktree verifier passed unchanged after that later sync.

The retained audit reproduces seven memory-write sites, 17 reads and 768 syntactic normal-return paths under stated provider/stack capsules. Those counts do not prove path feasibility or unwinding. Existing semantic C++ explains the stable valid-state rotation, but a production raw queue mutation binding still needs allocation, failure, copy/free, ownership and lifetime contracts. The next bounded body is `004B8C90`; no C++ activation, new build/test/runtime, Original ABI or gameplay proof is included.

Root receipt: `local/cc12_game_state_growth_primary/Root_review.json`, 217,519 bytes, SHA-256 `5d166764ab4a7291f91d40818603db5c393ce5e55a2bf1a7a55c705a45d9ac3a`. Primary report: `reports/cc12_game_state_queue_growth_owner_primary_review.json`.
