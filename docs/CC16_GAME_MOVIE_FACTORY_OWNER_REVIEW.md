# Independent game movie factory owner source review

No blocking static source-admission finding was found in candidate `6d4c288834c36f5ddc83da629336077389732da7`. All 101 header and 178 source lines were inspected. This verdict covers dormant source composition; actual publication, deletion, Startup and custody integration remain unbound.

Review base is `2c25911ef30940f93ef3a56c6202198013b59b45`. Candidate parent `86e4bfc10e3fa92865f88061cce5f32e0bca40fa` adds only the four Bink-owner files from `68ab882bbe7709822936c15b21f6e9804076ffb2`. The original owner contract `8f14bfd2`, caller EH recovery `1214cca8` and independent Bink review `6d3d02` were pinned read-only evidence inputs. Candidate working blobs and baseline helper dependencies match the reported hashes.

## Ownership, publication and call order

The C++ owner is separate metadata. It acquires exactly four raw bytes through a required matched-heap service and starts the trivial `MovieFactoryPrefix` lifetime without zeroing its word. Required real manager/publication cells, retained library and validated original-data owner have no default substitutes. Its destructor is metadata only: no native deletion, free, drain, retry or termination.

Registration writes incoming `CFEA60`, captures the first actual manager's `+10h` section, enters/increments that section, and publishes incoming storage. A separate completed statement resolves the second manager before the volatile current publication is read as the registration argument. `CapturedSoundLifetimeSection` releases the originally captured section. This matches saved assembly at `007366D8..00736728`; it does not snapshot a manager in place of a section or substitute incoming for the current publication.

The existing `construct_movie_factory_00737710` is reused unchanged: register, stamp incoming `CFEA78`, then `set_sound_system(open_direct_sound,0)` through the same retained library's factory table, ignoring the result. All thirteen imports are established before that owner exposes its stable table storage. No direct callback invocation or status fallback is added. The Bink callback typedefs retain the older-header qualification from `6d3d02`; this is not installed callback ABI proof.

`GameNativeReadOnlyData::data_at` checks the requested bands. The constructor copies and validates numeric words at `CE3818`, `CFEA60`, `CFEA78/+4`; it never installs the returned data pointer as a C++ vptr or calls native code. Fresh installed-PE reads agree with the expected `{00412440}`, `{00737030}`, `{00737950,00737730}` words.

## Construction and retirement exceptions

Fresh installed original PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Saved caller assembly confirms allocation 4 at `0073DF9B/9D`, capture before state `21h` at `0073DFAD`, register at `0073DFBC`, incoming `CFEA78` at `0073DFC7`, ignored-result sound call at `0073DFCD`, and demotion at `0073DFD4`. Allocation exceptions precede acquired cleanup; the original null branch skips constructor effects and is not a replacement returning-null provider.

Fresh `DB609C/DB61C8/C86BEC` bytes confirm state `21h -> -1` and only `free([EBP-138h])`. Source construction failure frees that captured raw allocation and rethrows. It adds no unregister, publication clear, full destructor or sound reset. Registration helper maps `DB53F0/DB53E0` release the captured guard and then reset incoming `CE3818`; sound failure after helper return does not reenter that reset. Reached publication/registry effects can therefore dangle after raw free.

Both deletion entries preserve captured incoming identity independently of current publication. They stamp `CFEA60`, capture the first section, resolve the second manager before rereading/unregistering current `F8AEFC`, clear publication, release the captured section and stamp incoming `CE3818`. Only normal completion with `flags&1` frees captured incoming storage; the returned address is identity even after free. The original `00737950/00737030` `ECX/ESI`, flags, `EAX` and `RET4` contracts agree. Fresh `DB5424/DB5414/C85FA0` metadata confirms guard release then base reset on retirement unwind, with no invented extra clear or free.

## Qualifications and root gates

The source API requires serialized, once-only, nonreentrant lifecycle calls. Invalid replay/foreign/overlap entries are rejected before new native effects. Those checks are not locking or arbitrary native reentrancy proof: if a callback suppresses an invalid nested-call exception, the already-running outer operation can later assign `ready` or `retired`. Root must enforce the stated precondition; the review grants no irreversible terminal-latch guarantee outside it.

`requires_process_retention()` returning false while `ready` does not authorize destruction or skipping explicit flags-1 retirement. Failure/interruption and flags-0 storage require whole-aggregate retention. After raw free, terminal retention must suppress ordinary shared drain of dangling publication/registry residue. This is deliberate host custody policy, not FH3 equivalence, native recovery or repair of the dangling state. Keep publication/context, manager/section, module/Impl/tables, providers and every decoder/callback/renderer/native borrower together until OS exit when that policy applies.

Baseline singleton/deletion sources have no actual `F8AEFC` cell or `CFEA60/CFEA78` routes. Root must supply/admit that real cell, verify canonical manager/domain and matched heap identity, install both deletion routes before registration, add actual build/Startup composition after terrain and before `010909B0`, and bind explicit shutdown at `0073804F/0073805F`. A current replacement publication may belong to another genuine owner; this class supplies no foreign-current shutdown assumption. Normal handle/callback closure, complete shared drain and final module release remain root obligations.

Independent MSVC Win32 object compilation passed `/std:c++17 /permissive- /W4 /WX /fp:strict /EHsc /O2 /MD /c`; COFF machine is `014Ch`. Exact object/source/dependency receipts and static bytes are in the report. No full build, tests, probe, DLL load/import call, game or live Ghidra mutation was performed. There is no new native/vendor-body, binary ABI, activation, playback or gameplay credit. Arbitrary SEH, foreign exceptions and cleanup-failure continuation remain unqualified. Shared source/CMake/Startup/singleton/deletion bindings, ledgers, subtitle guard and private accuracy candidate `2c182` were untouched.

[Machine-readable review](../reports/cc16_game_movie_factory_owner_review.json).
