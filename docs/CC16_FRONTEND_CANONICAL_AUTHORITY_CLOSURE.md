# Frontend canonical authority closure

Pinned Source: `origin/main` / checkout `eb63c7c17ae1e20f0ad728e2d410a360e1ebc4c1`. This packet changes only this document and `reports/cc16_frontend_canonical_authority_closure.json`. It selects an implementable storage/capability/view and Root lifetime contract. It implements no production owner, adapter or caller, and grants no activation permission.

**The storage contract is closed: extend the existing permanent `GameNativeCanonicalDataOwner` with matched `CanonicalFrontEndV3`, two authenticated writable pages and one mandatory borrowed view. The production migration is still not ready until the genuine raw95 and Frame consumer contracts below are supplied together. An unused zero bank is not an admissible implementation batch.**

The current connected Type2 Page/Model Source batch is accepted, build checked and default-Host-regression checked by the pinned `CC16_POPULATED_GUI_COMBINED_SOURCE_ADMISSION` receipts. Those checks were not rerun here. The Page guard is false. The earlier global-authority design's Page mapped-key/child-lifetime blockers describe its older baseline; they are superseded by that connected Source work. Actual populated execution, full raw95/current profile/Frame/Game ownership, other factories and genuine HUD timers remain open.

## Exact capability and allocation contract

Current Source admits v2 RO masks `038F` / `039F`, with writable pages `E15000`, `E16000`, `0109E000`. Its 21 capability slots already include E00000 as RO slot18, E10000 as slot19 and1090000 as slot20, but production does not select E00000. E10000 reservation does not commit E18000. `GameNativeReadOnlyData::data_at` remains bounded to `CE2000..E07B23` and selected bands.

Select enum `GameNativeDataPlan::CanonicalFrontEndV3`, wire version3. Add genuine RO span `{0x00e00000u,1}` before the optional D2 span in `game_main`'s selected spans. The existing mapper commits the selected band's real `.rdata` intersection `E00000..E07B23`, with page protection through E07FFF. This span selects an existing valid RO band; it does not put writable data in the RO API. The resulting exact RO masks are `0004038F` / `0004039F`; full reservation masks, including the same two CRT allocation roles, are `001C038F` / `001C039F`. Keep v1/v2 plans and their exact masks unchanged.

Preserve the existing40-byte handoff prefix and append explicit frontend fields. A single shared validator must match the exact version, roles/pages, PID pair and full mask in parent and child; v2 can never satisfy a frontend request.

| Wire field | Offset | Exact v3 value |
| --- | --- | --- |
| magic/version | 00/04 | existing magic /3 |
| parent PID / child PID | 08/0C | actual controlled parent/child |
| band mask / state | 10/14 | selected `1C038F` or `1C039F` / existing state protocol |
| `mutable_page[3]` | 18..23 | E15000, E16000,0109E000 |
| `mutable_roles` | 24 |7, preserving three CRT roles |
| appended `frontend_page[2]` | 28..2F | E08000,E18000 |
| appended `frontend_roles` | 30 |3: bit0 initialized names/flag page; bit1 loader-zero frontend page |

Proposed record size is52 bytes; assert size/offsets. Appended fields are zero for v1/v2. Parent constructor, pre-resume checks, child acceptance and both normal/fast-exit ACK acceptance must validate the same selected contract. In particular, `wait_for_mapping` currently accepts `state_mapped` without rechecking identity/plan; v3 must verify its saved version/PIDs/mask/page roles before accepting either mapped branch. Cancellation still wins against a late child `claimed -> mapped` CAS.

`transfer_ro_subset` accepts v3 only for its exact frontend RO mask and transfers E00000 into `readonly_->reservations_[18]`; its residual mask is exactly `00180000`. `transfer_mutable_subset` accepts that v3 residual and transfers the existing E10000/1090000 bases into `mutable_`. No new capability slot or second E10000 allocation is needed. Both subsets transfer before any page commit.

| New committed page | Allocation custody | Cold preimage and protection |
| --- | --- | --- |
| E08000,4096 bytes | existing `readonly_` child owns E00000 allocation | full initialized PE page at file A08000; MEM_PRIVATE/COMMIT/PAGE_READWRITE |
| E18000,4096 bytes | existing `mutable_` child owns E10000 allocation | complete loader-zero page beyond `.data` raw end E18000; same RW protection |

The existing canonical coordinator commits/verifies these subpages after both transfers. The frontend view owns neither allocation and cannot free a band. Supported PE identity is12,223,752 bytes, SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. E08000 page hash is `45b9d99ece79ccff905e2a9b8b9fb5060a370c5e3e32bffb1ce29164d255102c`; E18000 zero-page hash is `ad7facb2586fc6e966c004d7d1d16b024f5805ff7cb47c7a85dabd8b48892ca7`. These preimage facts come from the pinned earlier PE/loader audit, not a new live query. File A18B60 is unrelated file data, not the runtime bank.

Authenticate the complete new pages and allocation-base/state/protection relationships before ACK, alongside existing RO/D6/CRT and genuine cookie/complement checks. Construct all borrowed-view metadata before ACK; accepted `finish(true)` precedes permanent pointer publication. Nothing throwing may follow ACK before publication. Pre-ACK failure releases each transferred allocation once through its existing child custody and only untransferred bands through the capability; failure does not leak/double-free E00000 or E10000. After ACK the owner remains intentionally non-destructed through process/CRT exit. No Frame replacement resets, seeds or unmaps these pages.

## Selected borrowed view and existing publication authorities

Freeze `bsp::game::GameNativeFrontEndDataView` in proposed `include/bsp/game_native_front_end_data.hpp`. It is private-constructed only by the existing canonical owner, nondefault, noncopyable/nonmovable and nonowning. `const GameNativeFrontEndDataView& GameNativeCanonicalDataOwner::front_end_data() const` exposes it only after successful v3 initialization; a v1/v2 owner throws instead of supplying an empty/nullable/default view. There is no reset/initialize/release API.

| Exact accessor | Return type | Actual cells |
| --- | --- | --- |
| `screen_words_00e18b60()` | `volatile uint32_t*` | exactly95 DWORDs,380 bytes,E18B60..E18CDB |
| `pump_sentinel_00e18cdc()` | `volatile uint8_t&` | separate byte E18CDC |
| `frame_publication_00e18d80()` | `void* volatile&` | actual Frame pointer cell |
| `prompt_publication_00e18e6c()` | `void* volatile&` | actual MenuPrompt singleton cell |
| `backdrop_selector_00e18d84()` | `volatile int32_t&` | process-global signed DWORD |
| `panel_selector_00e18d88()` | `volatile int32_t&` | process-global signed DWORD |
| `title_selector_00e18d8c()` | `volatile int32_t&` | process-global signed DWORD |
| `backdrop_name_words_00e08500()` | `volatile uint32_t*` | CURRENT writable5x13 pointer DWORD cells |
| `panel_name_words_00e08604()` | `volatile uint32_t*` | CURRENT writable5x2 pointer DWORD cells |
| `title_name_words_00e0862c()` | `volatile uint32_t*` | CURRENT writable5x2 pointer DWORD cells |
| `flag_00e08654()` | `volatile uint32_t&` | separate DWORD, authenticated cold value2 |

Use `std::` on integer types in implementation. E08654 is neither a byte nor a fourth contiguous selector; its later low-WORD widget input remains a separate epoch. Current names and global selectors survive all Frame generations. Cold row4 name nulls are loader values, not permission for an enduring empty-Page implementation. Frame+160 and85 Page words belong to each genuine receiver; the prefix constructor deliberately leaves them untouched. Data accessors do not authorize adjacent globals as initialized services, arbitrary out-of-domain accesses or the original fault ABI. Numeric original codewords never become host callable pointers. Unknown profiles/slots remain unclosed providers; no successful default or metadata-only return substitute is admitted.

Primary cross-contract correction after the original seal: the mandatory InitOnce MenuPrompt getter00425D10 also needs the SAME actual E18E6C publication reference. The explicit accessor above adds that omitted consumer contract, bringing the view to11 accessors. E18E6C..E18E6F is inside the already authenticated loader-zero E18000 page; it requires no additional page, allocation, publication owner or initialization policy. Its genuine getter publishes and registers actual+0C, and its constructor/destructor contexts borrow this same cell. The historical43-pin/10-accessor seal validation below is retained as the original check, not a claim that the initial list included this correction. No standalone accessor implementation is authorized by this evidence-only correction.

The view deliberately contains no manager/Game copies. Manager authority stays `game_native_string_process().manager_01090aa0()`, borrowed by `GameSingletonHost::manager_publication_01090aa0()`, not a numeric01090AA0 mapping. Getter, Game singleton lifetime and adjusted singleton drain must use that same reference.

Source additionally establishes a permanent Game publication already: `GameNativeSettingsProcess::Impl::game`, returned by `settings_process_->game_00e188a8()`. Its profile settings context and `GameNativeEnumLuaDeclarationBindings` already borrow that cell. Native Game construction/lifetime and Frame apply/title/position/GameOnMove must borrow the SAME reference. `GameNativeGameRuntime::Impl` already compares construction/lifetime reference addresses. Loader-zero bytes at numeric E188A8 are not a second Game authority. No current Root adoption of full Native Game is asserted.

## Concrete Root placement and lifetime seam

The adopted route changes `game_main` to retain the initialized canonical-owner pointer and give `GameStartupHost` a mandatory owner reference. The host derives its RO-data and frontend-view references from that owner. The parent and child select v3 plus E0 in this same coupled migration. No optional fallback view or dormant owner-only route is accepted.

Both sibling contracts select `std::unique_ptr<GameNativeFrontEndApplication> GameStartupHost::native_frontend_`, beside `native_gui_pages_`. Proposed files are `include/bsp/game_native_frontend_application.hpp` and `src/game_native_frontend_application.cpp`. Freeze its constructor:

```cpp
GameNativeFrontEndApplication(GameHostLog&, GameSingletonHost&,
    const GameNativeFrontEndDataView&, void* volatile& actual_game_00e188a8,
    GameNativeGuiManagerApplication&, GameNativeGuiPageApplication&,
    NativeStringRawPoolContext&, GameNativeReadOnlyData&);
```

Create this actual client and its stable receiver/operation/lifetime records with genuine Page/GUI/renderer/model/VFS/Lua/string/scene/import services before the FIRST actual raw Game, screen or Frame creator. Current placement bounds are real Page setup at `game_hosts.cpp:3181` and the still-semantic Menu constructor at3274; those landmarks do not establish existing raw Game/InitOnce binding. All context metadata and actual primary/secondary deletion bindings must be ready before creator entry, including partial startup.

Selected used methods are `frame_004c1ac0`, `register_current_primary_10(captured_receiver)`, `select_layout_set_00518250(captured_frame,request_DWORD_home,commit_BYTE_home)`, `apply_00518c80(captured_frame)`, selector/flag/title/position operations and primary/secondary current scalar operations. The Frame closure specifies exact request/title/text/flags-home reference types. Use the same application in InitOnce/Menu/title/HUD/loading/captured collector/park services; active semantic `world.screens`, `frame_layouts` and the four-field selection object cannot remain parallel owners.

There is a mandatory used Source seam in the admitted getter:

```cpp
get_native_front_end_frame_004c1ac0(manager_ref, frame_ref,
    NativeFrontEndFramePublicationCustody&);
```

The two-reference body currently allocates, constructs, publishes and registers CURRENT+8 before returning. A wrapper that records only the returned value or rereads publication in a catch loses allocation/registration provenance. Prepare a stable invocation FrameRecord before the whole getter begins. The overload reuses its actual schedule and makes metadata-only `noexcept` custody notifications at actual allocation, constructor-return and captured-free sites before manager registration. Keep captured allocation, constructed result, CURRENT publication and adjusted registered receiver distinct. No extra native call, byte initialization, refcount/retain, catch, reset, replay or compare-and-clear belongs to this seam. Record old live generations independently of CURRENT publication; keep borrowed context addresses stable.

Current teardown deletes Menu/Frontend at `game_hosts.cpp:2314..2316` before shared drain2321. Consequently neither `GameMenuHost::Impl`, `MenuFrameLayoutHost` nor a deleted `GameFrontendHost` can own the raw Frame contexts. Keep `native_frontend_`, actual Page/GUI-manager services, renderer/model/scene, VFS/Lua/string/counter/import providers and genuine Game backing alive through Game OnDestroy4C0CE0 and normal/fallback shared singleton drain. Only after real Frame/Page retirement and shared-drain acknowledgments may frontend bindings retire, followed by Page and GUI-manager bindings. The permanent canonical pages and manager/Game Source cells outlive that sequence.

Before any provider teardown, Root checks incomplete frontend operation retention alongside its existing retained Page/manager checks. Entered native failure preserves partial publications, graph and diagnostics; the existing process-retention/exit boundary keeps those providers alive. Diagnostic acknowledgment is allowed only after actual resolution and supplies no cleanup. There is no replay/rollback or synthetic retirement on failure.

## Actual consumer cut and next bounded work

The exact13 bank/sentinel entry families are `4F71A0`, `4F71D0`, `4F75E0`, `4F8830`, `4F7620`, `4F8390`, `4F8530`, `4F85D0`, `4F8670`, `4F8710`, `4F87B0`, `4E4A40`, `4D64E0`. This retains the earlier qualified41-row saved-xref inventory; it is not dynamic receiver/index coverage. Set-level/restore/pump/GameOnMove sentinel stores use the same separate byte, including4F8597/8637/86D7/8777/87FB,4F839A,4F8881,4E544C/5481. Screen-set list/header provenance is a separate real provider contract.

Raw4F71D0 captures its receiver, calls its CURRENT primary+00 through the genuine Source resolver, publishes at the unchecked returned ID and preserves authentic EAX. It accepts neither supplied ID nor a range policy. Raw4F71A0 stamps CEAE54 and clears every exact receiver match across95 cells. Actual creators must precede all uses of their +4/+5 fields and current slot bodies; semantic two-bool screens cannot be reinterpreted. CEC39C primary and CEC398 adjusted+8 scalar dispatch use current profile/current captured codeword and real operations. Admit the adjusted Frame lifetime binding before getter registration; the existing CE3818 base drain arm is insufficient.

Frame publication writers/getter4C1AC0,517CF0/517D60/517D90; register517ED0; set518250; selectors518D60/518DA0; apply518C80; flag518B90; title5189B0; position5186B0/518820; collector518E00 and park4D64E0 all join the same view and retained graph. Retain callback/reload and argument-home epochs. Complete scalar517EB0/secondary517D50 and Game4C0CE0 must preserve unconditional publication clears, including callback-created replacement effects.

Real caller adoption includes InitOnce4DD5B0's getter/current+10, title4C9A70 at4C9B7E/85, HUD4C9E06 set3/commit1 and4E04EE set3/commit0, and loading57CD1E/7B. The31 saved writer518D60 and2 writer518DA0 caller families must join before those routes are admitted. The report records exact address/file groups and current hashes. These are required bindings, not current admission claims.

The minimal storage delta is five existing files: native-data-bootstrap header/CPP, mutable-CRT/canonical-owner header/CPP and `game_main.cpp`; add only the borrowed-view header. No RO mapper behavior change is required. The mandatory coupled Root/lifetime delta additionally includes `game_hosts` header/CPP, Menu and Singleton host bindings, singleton destruction header/CPP, the admitted getter header/CPP and the actual frontend application files. Reached semantic consumer sources must move to genuine operations under the sibling contracts; editing only parameter types does not close them.

Next bounded actual producer recovery is **MenuPrompt00425D10 /00533120**, in proposed `native_menu_prompt_construction.hpp/.cpp` and `native_menu_prompt_publication.hpp/.cpp`; raw registration5311C0 and destructor5321F0 already exist. This is distinct from analyzed-only command owner54A8D0/E1930C. The [raw95 closure](CC16_FRONTEND_RAW95_PRODUCER_CLOSURE.md) pins the full producer/current-slot cut. The [Frame closure](CC16_FRONTEND_FRAME_SELECTION_CLOSURE.md) pins whole Frame operation/lifetime/caller recovery and the remaining raw Text ABAED0/ABA8D0 and reached Text/Icon/FrameBox method/factory requirements. The accepted Page Source work does not supply those unrelated missing contracts.

The integrator may implement the selected storage and Root seam only as a coupled batch with genuine reached creators/operations and both lifetime routes after reviewing those contracts. Preserve actual same-Page direct decrement/current0 retirement separately from manager AA31F0 retirement, using the existing Page graph rather than a private Frame deletion owner. Keep the Page guard and other activation gates unchanged until their actual execution/qualification requirements pass.

No production Source/header/CMake/ledger edits, live Ghidra queries/mutations, executable/build/probe/game execution or process/service changes occurred here. New Original/reconstructed/vendor/duplicate-body, ABI, runtime, activation and gameplay credit are zero. The report contains43 immutable Source/evidence pins and the exact proposed wire/API/ownership contract.
