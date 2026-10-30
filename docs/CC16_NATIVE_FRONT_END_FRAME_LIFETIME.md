# Native front-end Frame lifetime

Baseline is `071439bfb5e0d82c956253d21b8cc73f2651c174`, including the sealed GlyphQuad backing correction. This packet adds four new files only. It supplies ordinary Source composition for the actual Frame destruction path and its scalar/secondary wrappers; all older semantic/typed bodies, shared application/view/singleton bindings, CMake, ledgers and saved Ghidra analysis remain unchanged. It supplies zero additional Original, vendor, helper, compiler-generated or duplicate-body credit. Whole front-end readiness remains false.

## Whole boundaries and evidence

| Native entry | Inclusive end | Physical bytes | Decoded instructions | Source operation |
| --- | --- | ---: | ---: | --- |
| `00517D90` | `00517EAB` | 284 | 83 | `destroy_native_front_end_frame_00517d90` |
| `00517EB0` | `00517ECD` | 30 | 11 | `delete_native_front_end_frame_00517eb0` |
| `00517D50` | `00517D57` | 8 | 2 | `delete_native_front_end_frame_secondary_00517d50` |
| `004F71A0` | `004F71C4` | 37 | 10 | Private exact base composition |

The three public operation bodies account for 322 physical bytes/96 decoded instructions. The private existing base schedule adds 37 bytes/10 instructions without extra body credit. The destructor's jumped-over six-byte `LEA EBX,[EBX]` padding at `517E4A` and the base's three-byte `LEA ECX,[ECX]` padding at `4F71AD` were read from both installed PE and live Ghidra bytes; they do not add decoded body instructions or Source effects. No x87 arithmetic occurs.

The full saved assembly for all four addresses was read, including every register input and scalar flag access. The installed PE is 12,223,752 bytes, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Whole PE body hashes, eight saved assembly/pseudocode export pins, nine direct E8 call targets, the secondary JMP, two actual current-profile words, and the FH3 handler/map/actions are recorded in [the report](../reports/cc16_native_front_end_frame_lifetime.json). Bad saved `undefined(void)` prototypes do not replace the observed ECX/stack ABI. Before live batches, `bsp.py ghidra count` verified project `bsp`, `/battlestationspacific.exe`, x86:LE:32:default and image base `00400000`; live/snapshot counts were both 77,632. The configured project is `C:/Users/sqz269/bsp.gpr`. No analysis was mutated or new export created.

## Captured receiver and all 85 slots

`517D90` captures the actual 164h Frame and stamps primary `CEC39C`, then secondary `Frame+8 = CEC398`. It does not choose the current publication, consult own `+160`, decrement Page counts, or infer initialized slots.

The exact traversal is column first, then five sets:

| Group | First slot | Columns | Set stride | Visits |
| --- | --- | ---: | ---: | ---: |
| Backdrop | `Frame+0C` | 13 | `34` | 65 |
| Panel | `Frame+110` | 2 | `08` | 10 |
| Title | `Frame+138` | 2 | `08` | 10 |

For example, the first backdrop column visits `0C,40,74,A8,DC`; the last visits `3C,70,A4,D8,10C`. Panel visits `110,118,120,128,130`, then `114,11C,124,12C,134`; title similarly visits `138,140,148,150,158`, then `13C,144,14C,154,15C`. All 85 DWORD homes from `+0C` through `+15C` are tested exactly once in this order. `+160` is not read or written.

Each nonzero probe reaches the whole genuine GUI getter `get_native_gui_manager_004c12b0` with the same existing `NativePlatformFocusOwnersContext`. Only after that getter returns does Source reload the SAME actual slot, then call the existing whole `remove_and_destroy_native_gui_page_00aa31f0` with that returned manager, the newly loaded Page word, and the same retained `NativeGuiPageLifetimeHost`. This corrects the old projected host interface's pre-getter Page capture without editing it. A getter-mutated replacement or null is passed unchanged; there is no post-getter null test, cached Page, count/initialized-slot gate or fallback.

The existing raw `AA31F0` removes the first matching Page pointer from the actual manager vector before calling its current virtual `+20`, then current `+04` with flag one. If absent it returns; this Frame body does not replace that behavior with unconditional Page deletion. It performs no per-slot clear and no native retain/decrement. Already retired pointer identities may therefore remain in slots exactly as Native permits; they are not a liveness claim. Future callbacks may alter later slot words, so their volatile probes/reloads remain at their reached epochs.

## Publication, secondary base and raw95 base

After all traversals return normally, Native stores zero to the SAME `E18D80` publication unconditionally, including when callbacks published a different Frame. It then writes the captured Frame's `+8` profile to `CE3818`. The base call is disarmed at `517E8C` before `4F71A0` at `517E94`; Source keeps the normal base tail outside the traversal catch.

Private whole `4F71A0` composition first stamps the captured Frame primary to `CEAE54`, then scans exactly the existing 95 volatile DWORD cells `E18B60..E18CDB`. It clears EVERY current word equal to the captured Frame's 32-bit identity. It does not stop at one match, use a returned/default ID, compare against current publication, reseed the bank, or touch the separate sentinel at `E18CDC`. `CEAE54` remains a profile identity, not a default receiver or callable table.

`NativeFrontEndFrameLifetimeContext` contains four pure borrows: the same actual GUI owners context, the existing Page lifetime provider, the same volatile Frame publication reference, and a constant pointer to those same 95 writable cells. It allocates or copies no manager, Page, Frame, owner, bank, descriptor or global cell. It uses the real raw getter and raw registry body directly; no new abstract GUI-getter/free provider hides an already recovered helper.

## Exception map and partial effects

Handler `C6A85F` names FuncInfo `D93CE8`, whose two-state map is at `D93CD8`:

| State | Parent | Action | Native effect |
| ---: | ---: | --- | --- |
| 1 | 0 | `C6A838` | From saved captured Frame at EBP-10, select its `+8` secondary; tail `517CF0`: clear SAME publication, restore secondary `CE3818` |
| 0 | -1 | `C6A830` | Reload saved captured Frame; tail whole `4F71A0` |

State 1 is armed after both derived profile writes and covers every reached GUI getter and Page call in all three traversals. On an ordinary C++ exception, Source performs this same two-step base cleanup once and rethrows if it returns. It does not replay the destructor traversal, clear Page slots, free the Frame, read scalar flags, or roll back Page/registry effects. The existing `517CF0` helper is only this publication/reset schedule; its semantic body and helper credit are unchanged. Its native null adjustment arm is documented in the report; a valid nonnull Frame must already survive the earlier derived prefix stores before this cleanup interval is reached.

The new module introduces no `noexcept`, termination, retry or rollback policy. Existing reused suppliers retain their own previously documented exception contracts. Accessible backing is required for the raw base writes; original access faults, secondary FH3/SEH transport and exact private stack aliases are not proved by this ordinary C++ catch.

## Scalar and secondary route

`517EB0` captures the allocation identity before calling the whole destructor. Only after normal return does it read BYTE zero of the SAME live full DWORD flags home (`TEST BYTE[ESP+8],1` at `517EB8`). Bit zero calls existing `singleton_lifetime_free`, the real `BF65AC` Source composition, on that captured allocation. Upper bits are not read, normalized, converted into a bool or written. No receiver byte is read after possible free; the returned pointer is captured address identity only. A throwing destructor skips the flag read, free and return.

`517D50` is the complete `SUB ECX,8; JMP517EB0` secondary adjustor. Source uses wrapping DWORD subtraction on the captured secondary identity and forwards the SAME flags home and context. It does not reload publication or select a new receiver. PE current-profile words confirm `CEC398+00 -> 517D50` and `CEC39C+0C -> 517EB0`; primary `CEC39C+00` remains the real ID getter, not a destructor. The separate base-construction-profile scalar `517D60` is not implemented or credited by this packet. Both requested CG wrappers remain `compiler_generated_out_of_scope` in shared records.

## Remaining real binding and proof cuts

The retained actual startup consumer must bind these operations only for the captured genuine profile/target/receiver epoch and borrow its existing canonical Frame publication/raw95 cells and same GUI/Page domain. The two existing Page host operations must perform genuine current `+20` and `+04` dispatch over each actually reached Page; the same provider must also serve the real GUI manager. No fake default provider or semantic Page/Frame cast supplies this obligation.

Caller proof must cover the actual Frame's entire operation, all 85 readable slot words, actual manager/vector/Page backing at their reached reads, borrowed canonical cells, Page/current-target/provider contexts, and the live flag home until its late byte test. Raw constructor `517D10` initializes only the prefix and does not prove these slot/lifetime obligations. Do not add blanket zero initialization, count/range gates or native retains to make a partial object appear ready. The captured Frame may be an older live generation than publication. After a partial entered failure, retain the unresolved graph/provider domain for explicit disposition; an automatic C++ destructor must not replay this raw body.

No CMake, shared application/view/singleton-map, ledger or activation binding is changed here. No build, tests, probe, game/native execution, process control or Ghidra write was performed. Source/static hashes and reviewed assembly establish the bounded schedule; original physical register/RET ABI, invalid-storage behavior, exact private aliases/FH3/SEH, compiled instructions, runtime/gameplay and visual parity remain unproved. Minimal next work is Root's shared integration/build plus genuine retained current Frame/GUI/Page bindings, not another helper recovery or standalone owner.
