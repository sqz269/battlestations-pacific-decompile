# Raw95 producer and current-dispatch closure

Frozen Source: `eb63c7c17ae1e20f0ad728e2d410a360e1ebc4c1`. This packet changes only this document and [its report](../reports/cc16_frontend_raw95_producer_closure.json). It admits no Source function, adapter, activation or Native/ABI/runtime credit. **The whole frontend Source migration is not ready.**

The bounded next recovery is the complete actual MenuPrompt constructor `00533120..0053378D` together with its genuine singleton getter `00425D10..00425DD9`. Existing raw `005311C0`, `005321F0` and ID leaf `00532310` must be reused. Recovering another Frame constructor, another bank, or another semantic screen would not close the actual call chain.

## Actual receiver contract

The bank is exactly 95 DWORDs at `E18B60..E18CDB` (380 bytes); `E18CDC` is a separate sentinel byte. Capacity does not imply 95 constructed classes. The report pins the selected 7 main-menu, 42 HUD, press-start, Frame, prompt and separate command-owner profiles, sizes and current primary slots. It does not enumerate unrelated remaining class families.

Installed PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. All 53 selected primary profiles were read as ten DWORDs from that PE. Every selected current slot-0 target has `B8 imm32 C3`, with the recorded native ID. Profile words and IDs are evidence identities; they are not host function pointers or arguments to base registration.

The existing authentic `004F7180` writes primary `CEAE54`, then separately clears byte `+4` and byte `+5`. Base `CEAE54/+0` is `BF698E` purecall. The base has no successful default ID. The semantic two-bool `FrontEndScreen`, `MenuScreen::owned_flags`, `PressStartScreen` and `MainMenuRegistrationBindings` cannot be cast into these allocations.

Whole base registration `004F71D0` captures ECX, loads its CURRENT primary slot 0 and calls it at `4F71D7`; `4F71D9` publishes the captured raw pointer at `E18B60 + returnedDWORD*4`. There is no ID parameter, clamp or range guard. The store does not change EAX, so the native return retains the current-0 result. A proposed ordinary Source signature is:

```cpp
std::uint32_t register_raw_front_end_screen_004f71d0(
    void* captured_receiver, const GameNativeFrontEndDataView&,
    RawFrontEndScreenDispatch&);
void destruct_raw_front_end_screen_004f71a0(
    void* captured_receiver, const GameNativeFrontEndDataView&);
```

These are contracts for coupled implementation, not APIs implemented here. Destruction first writes `CEAE54` at actual `+0`, then clears **every** equality match across all 95 cells. It does not touch the sentinel. The scan leaves native EAX at `E18CDC`; a void Source destructor does not claim that incidental register result.

`RawFrontEndScreenDispatch` belongs to the SAME retained `bsp::game::GameNativeFrontEndApplication`, proposed in `include/bsp/game_native_frontend_application.hpp` and `src/game_native_frontend_application.cpp`. Every reached current-0/04/08/0C/10/14/18/1C/20/24 operation resolves the actual current profile again. Unknown profiles and purecall are unresolved boundaries, not successful defaults. Numeric vtable DATA is never cast to a callable.

The actual `+4/+5` bytes carry wanted/active state. Pump exit, enter and update are three separate bank sweeps. In the exit arm, retain the captured receiver across current `+1C`, then reread that receiver's byte `+5`; a callback may replace the publication or bank entry. Enter sets `+5`, commits, then calls current `+18`. Update uses current `+20` and the original float argument. Recompute calls current `+04` at its native phases; per-class `+04/+08` overrides in the report must not become universal false predicates. Commit invokes current `+24` and carries actual Page-slot addresses to the child visibility calls. Slot-number lookup cannot substitute a newer receiver after a callback.

## Producers and registration callers

Paged `bsp.py show --asm` established 52 current-primary-`+10` calls. The report retains each native slot load/call and nearby receiver context. These calls have ECX equal to the actual stored/captured screen and no explicit stack argument:

| Caller | Actual producer and custody | Current registration calls |
| --- | --- | --- |
| `00686380` MainMenu Init | `operator_new`, genuine constructor, manager `+58..70` raw fields; 7 objects | `686708`, `68674A`, `68678F`, `6867D4`, `686816`, `68685B`, `6868A0` |
| `0068CC70` HUD Init | Exact sizes and construction order; manager raw fields `+40..E8`, with hole `+D0`; 42 objects | All 42 sites in report, `68CD3F..68D733` |
| `0068D8D0` title activation | Allocate `18h`, actual `0067C840`, publish title `+40` before its null test | `0068D966` |
| `004DD5B0` InitOnce, first call | Captured genuine `00425D10` MenuPrompt, publication `E18E6C` | `004DD756` -> current `+10`, initially `005311C0` |
| `004DD5B0` InitOnce, second call | Captured existing genuine `004C1AC0` Frame, publication `E18D80` | `004DD764` -> current `+10`, initially `00517ED0` |

The main-menu allocations are already described by real raw constructor bodies:

| Manager field | Size | Constructor | Primary | Current 0 / ID | Current +10 |
| --- | --- | --- | --- | --- | --- |
| `+58` | `578` | `5902E0` | `CEFC5C` | `590570 / 01` | `582F30` |
| `+5C` | `034` | `5CA880` | `CF170C` | `5CA8E0 / 02` | `5CAAF0` |
| `+60` | `260` | `626630` | `CF50CC` | `626710 / 04` | `629D10` |
| `+64` | `138` | `51E4D0` | `CEC948` | `51E250 / 03` | `51E280` |
| `+68` | `05C` | `52FCE0` | `CED200` | `52FD60 / 08` | `52FFC0` |
| `+6C` | `16C` | `563370` | `CEE9CC` | `5633F0 / 0A` | `564220` |
| `+70` | `5D0` | `5098B0` | `CEB9B4` | `509D60 / 0B` | `50FDB0` |

The JSON pins their Source signatures/locations and secondary profiles, rather than relying on old host comments saying none are reconstructed. The HUD table likewise pins every size, field, primary, current ID/flag/control target and applicable secondary/embedded profile store. Thirty-seven HUD constructor entries have named or shape Source bodies. The five Init-inline entries correspond to existing `small_hud_screen_construct` shapes for `68A570`, `68A3B0`, `68A5A0`, `68A2D0` and `68A4A0`; a zero table constructor address does not authorize skipping construction. `0068CC70`'s semantic boolean-array projection still supplies no authentic manager fields or allocation custody. The native allocation-null path still reaches unguarded current dispatch.

HUD teardown `0068B630`/`0068BC60` collects the actual receivers, performs the hide/clear/commit pass, then the scalar-delete pass. Each raw receiver remains retained through that complete sequence. Main-menu destruction likewise separates hiding from deletion. Raw allocations, Page/list/vector members and callback subobjects cannot be owned by diagnostic `MenuScreen` records.

Press-start's current primary is `CF6D90`: `67C860` returns `5C`, `67CA60` is scalar deletion, `67CA80` registers, `67CB40` enters, `67C870` exits, `67CFB0` updates and `67D860` collects. The existing `press_start_screen.cpp` bodies use a typed semantic receiver and supplied ID. They describe behavior but supply no actual `18h` raw prefix/allocation/current-dispatch/retirement composition. Preserve the native untouched Page/widget bytes until their real writers execute.

Frame's current primary is `CEC39C`, current secondary at `+8` is `CEC398`; construction briefly writes `CEC388`. Its admitted `517D30` returns 12. Current `+10` is whole `517ED0`, not `+14`. Current secondary `+0` is `517D50` adjust-minus-8 to the scalar/destructor chain. `517D60` is a destructor, never a creator. The [Frame selection closure](CC16_FRONTEND_FRAME_SELECTION_CLOSURE.md) owns complete Frame/Page registration, selection and retirement composition.

`00425D10` is the **MenuPrompt** singleton, not the `E1930C` main-menu command owner. Prompt allocation is `290h`; constructor `533120` first calls the admitted raw base and installs primary `CED3C4`, with `+8=CED3B0`, `+C=CED3AC`, `+10=CED38C`. Current `532310` returns `55`; current `+10=5311C0`, scalar `532A00`, destructor `5321F0`. Its lifetime manager receives actual `instance+0C`. The separate OnInit `004E3AA0` allocates the `ACh` command owner with `54A8D0`/`CEDFDC`, ID `58`; its constructor remains analyzed-only. Do not route either object's calls to the other.

## Exact remaining packet

There is no complete bounded Root Source adoption packet yet. Recover the following connected producer/publication packet at **max** effort, with native assembly/register/EH evidence:

1. Whole `00533120..0053378D`: 513 instructions, 57 blocks, 74 calls. Recover genuine `290h` storage, constructor register inputs, base/profile store order, two native string-vector members at `+40`, seven `48h` records at `+60`, vector `+268`, pending list `+278`, actual strings/keys and native cleanup order. The live `undefined(void)` prototype is not a reliable signature. The existing `frontend_prompts.cpp:66` is explicitly a host projection.
2. Whole `00425D10..00425DD9`: 56 instructions. Preserve the double check, SAME manager/guard, genuine allocation, raw constructor, `E18E6C` publication **before** singleton registration, actual `instance+0C`, allocation-null behavior, registration exceptions and guard release. `storage_backend.cpp` merely forwards to a semantic UI object.
3. Compose the existing raw `5311C0` registration, `5321F0` destruction, ID `532310`, current scalar/secondary adjustment and Page/list/record/string providers in the SAME application. `5311C0` already calls base registration and loads/hides `_DialogueBox` and `_DialogueFarBlackPlane`; duplicating it is unnecessary.

Proposed disjoint implementation files are `include/bsp/native_menu_prompt_construction.hpp`, `src/native_menu_prompt_construction.cpp`, `include/bsp/native_menu_prompt_publication.hpp` and `src/native_menu_prompt_publication.cpp`. Concrete ordinary Source signatures are `void* construct_native_menu_prompt_screen_00533120(void*, NativeMenuPromptConstructionContext&)` and `void* native_menu_prompt_get_singleton_00425d10(NativeMenuPromptPublicationContext&)`. Contexts borrow authentic storage/providers and retain reached operation frames; they do not manufacture native classes or successful placeholder callbacks.

Primary integration owns edits to the shared application, view, InitOnce boundary, singleton dispatch, CMake and sharded ledgers. Existing actual String/pool/`450540` append bodies should be reused. Exact native contracts for `530E30`, `4322C0`, `532190` and the EH vector iterators still require honest composition; a host iterator substitution provides no Native FH3/SEH proof. This packet is not permission to activate a successful unused adapter or to reconstruct vendor-library bodies for game-function credit.

The [canonical authority closure](CC16_FRONTEND_CANONICAL_AUTHORITY_CLOSURE.md) fixes one owner-created `GameNativeFrontEndDataView`, with the existing process manager and existing `GameNativeSettingsProcess::game_00e188a8()` reference passed separately. Startup must retain the SAME `GameNativeFrontEndApplication` before the first raw creator, through menu/frontend teardown and the last raw unregister/scalar call. Real GUI/Page/String services and receiver metadata belong to that retained graph. Old live Frames can differ from the current `E18D80` publication. Do not copy/reseed bank/name/selector cells or create another Game/manager authority.

Remaining whole-route gates are genuine main/HUD manager allocation and retirement, raw press-start, the separate `54A8D0` owner, full raw current dispatch, authentic nonempty Page lifetime and the Frame sibling's reached Text/widget providers. The 13 bank/sentinel consumer entries retained in the report are `4F71A0`, `4F71D0`, `4F75E0`, `4F8830`, `4F7620`, `4F8390`, `4F8530`, `4F85D0`, `4F8670`, `4F8710`, `4F87B0`, `4E4A40` and `4D64E0`. Prior 41 static xref rows are qualified evidence, not dynamic index/receiver coverage.

Validation here is limited to Source inspection, configured `bsp.gpr`/`battlestationspacific.exe` read-only count/proto queries with autostart disabled, installed-PE table/leaf checks, paged native callsite inspection and report consistency. No build, test, probe, game process, Ghidra write, ABI comparison or activation occurred.
