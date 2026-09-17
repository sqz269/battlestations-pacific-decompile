# Native game storage defaults (R112)

Addresses: 004c2700, 004c2750, 004c27a0, 004c2830, 004c1950, 004c26b0, 004c1a40, 007ff9d0, 008882d0, 00884830, 004ddb90, 005826b0, 007f8540, 00907ca0, 004c3180, 004c1970.

Nine `NativeGameConstructionCalls` methods now have real defaults: seven container allocation leaves, the raw race initializer and the mission Lua owner. Ten service providers remain required. The raw game parent is still not application-owned or reached by startup.

## Recovered behavior

| Address | Allocation / normal stores |
| --- | --- |
| 004C2700 | 14h tree; zero0/4/8, color10=1, nil11=0 |
| 004C2750 | 28h tree; zero0/4/8, color24=1, nil25=0 |
| 004C27A0, 004C2830, 004C26B0 | 18h tree; zero0/4/8, color14=1, nil15=0 |
| 004C1950, 004C1A40, 00884830 | 24h,0Ch,2Ch list heads; next/previous self, opaque payload preserved |
| 007FF9D0 | vptrD08D20, zero8/C; preserve index4 and colors10..1C |
| 008882D0 | vptrD0E7A8, allocate2Ch head, publishC, clear10 then4; preserve8 |

Allocation leaves consume no incoming arguments, return the pointer in EAX and use RET. Each independent wrapped-address guard is retained; a returning null allocator does not create a successful empty container. Parent4DDB90 performs sentinel nil/self-link finalization after tree allocation. No payload or padding initialization was invented.

The shared detail header factors the existing profile blank-node and game-array list-head rules. Production allocation still delegates to existing BF681B. The existing naked4C26B0 remains unchanged. These are bounded library storage contracts, not new STL/CRT implementations.

The race adapter reproduces the already recovered typed constructor's three stores without treating raw game bytes as a live nontrivial RaceRecord. ECX is the native receiver; EAX returns it and native ECX is cleared. No additional unique game-function credit is taken.

### Correction to earlier Lua-owner extent

Current live listing and PE bytes show **008882D0..008882F3 inclusive,36 bytes**, ending in RET, rather than the older882FF/48-byte claim. The native ECX receiver is14h bytes; EAX returns it. The8h allocator word remains opaque. Sentinel allocation occurs after the vtable store and before head/count/Lua-pointer stores. If allocation throws, the existing parent operation retains current allocation and4DE093/state40; no game+1A08 publication occurs. Source diagnostic no-replay does not emulate native FH3 cleanup.

## Validation

- Strict Win32 build and all three existing CTests passed.
- 16659 live Ghidra/PE bytes agree;418 direct CALL rows include36 read-only score-layout producer calls. Existing Catch_All4D0DB5 owns the three nested exception CALLs.
- 82 copied envelopes/14643B,382 relocated CALLs and4 external JMPs;29 original/source comparisons matched **1669 ordered observations and 113893452 bytes**.
- Prior26 composed cases now use actual game storage leaves/race/Lua owner. Added raw race,Lua opaque-word,and allocator-callback-mutation comparisons. Source Lua allocation failure retains vptr-only receiver/stage and rejects replay.
- Nine additional real CRT allocations preserve all opaque bytes and were manually released for diagnostics. No reconstructed destruction claim follows from this check.
- Application executable differs only at timestamp fields: True; changed application objects: []. No application runtime rerun.

Accounting: one new unique game function,36B;15 reuse/library/composition fragments. No function creation or flow repair was needed. Names remain descriptive hypotheses; prior comments are preserved and appended evidence is saved/read back/exported.

## Follow-up packets

Remaining service providers are `call_008d9150, call_00432650, call_0087d7b0, call_00717e80, call_0070bd70, call_00727bd0, call_00be4800, call_00c55f50, call_00c420e0, call_00c31a40`. Lease and inspect their actual bodies before naming host behavior. Native ABI/FH3, parent application ownership, teardown and gameplay remain open. A runnable game has not been established by these fixtures.

Evidence: `reports/native_game_storage_defaults_r112.json` and `reports/native_game_storage_defaults_flow_r112.json`; ignored fixture/toolchain/native-byte artifacts are sealed before integration.
