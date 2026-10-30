# Actual Frame selection and retirement closure

Frozen Source: `eb63c7c17ae1e20f0ad728e2d410a360e1ebc4c1` (`origin/main`, verified before worktree creation). This evidence-only packet supersedes the Frame/Page portion of the older `CC16_NATIVE_FRONTEND_GLOBAL_AUTHORITY_DESIGN.md`; it does not implement its proposed interfaces. The connected Type2 Page/Model batch is now on Main: 22 production files, its recorded common Win32 build and 2/2 suites passed, and the recorded default Host frame/drain passed. Those are prior receipts, not execution by this packet. `kNativeGuiPageChainCc15Bound` remains false.

**The Frame composition is a finite Source repair; whole frontend caller adoption is still not Source-ready.** The precise remaining genuine provider gap is changed raw `ABAED0` -> nonempty raw `ABA8D0`/current+50 and the reached actual Text/Icon/FrameBox factories and methods. A second semantic layout owner, an unused Page accessor or an empty Frame bank cannot close it.

## Closed bodies and the exact repair boundary

| Operation | Existing evidence and next action |
| --- | --- |
| Raw construction/getter/ID | Actual4F7180,517D10,4C1AC0 and ID517D30 are already Source admitted. Reuse them; no new body credit. Getter allocates164h, constructors leave+C..160 untouched, and getter does not run517ED0. |
| Register517ED0 | `frontend_seg12_17_cc13.cpp:2997` captures each outer current name and rereads own+160 for inner rows, but delegates string construction/getter/load to a `string_view` host. Replace that projection with the real NativeString/pool/getter/AA5840 schedule. Header+14 is stale; actual CEC39C uses+10. |
| Set518250 | `title_init.cpp:99` is a qualified value/set-range/static-name projection. Implement the whole body against the actual receiver and borrowed request-DWORD/commit-BYTE homes; preserve both retirement arms below. Do not merely replace `kLayoutNames`. |
| Destruct517D90 | Existing body at `frontend_seg12_17_cc13.cpp:1544` has the right table traversal but its host receives the Page before the GUI getter. Repair to call the real getter then reload the actual slot. |
| Scalar/adjustor | 517EB0 and517D50 have known complete saved listings (derived scalar, and subtract8/tail scalar). Existing ledger classes them compiler generated/out of scope. Supply actual Source lifetime dispatch with zero CG/duplicate-body credit. 517D60 is the distinct base scalar, not the derived destructor. |
| Apply/flag/title/position/collector | Existing projected bodies are in `menu_screens_2.cpp:1488..1550` and `frontend_seg12_17_cc13.cpp:1530..1587`. Repair their actual receiver/current/global/getter and arithmetic schedules rather than adopting their typed state owners. Collector currently caches own and all three offsets and must change. |
| Actual Page graph | `GameNativeGuiPageApplication` now supplies whole Type2 Page/Group/Model/child/node/timed services and private manager current+20/+04 retirement. Existing raw AA31F0,AA38F0,AC5480,AA9730,AA8320 and BD30E0 are reusable; do not reconstruct them again. |

## One authority and one retained application

Use the sibling canonical contract's nondefault borrowed `bsp::game::GameNativeFrontEndDataView`, supplied by the EXISTING permanent `GameNativeCanonicalDataOwner`. Its raw bank95 E18B60..CDB, separate sentinelCDC, publicationD80, selectorsD84/88/8C, live writable name-DWORD tables E08500/E08604/E0862C, and separate E08654 flag remain process data. No per-Frame selector copy/reset or table reseed. Borrow the SAME manager Source reference from `GameSingletonHost::manager_publication_01090aa0()`. Borrow the SAME existing `GameNativeSettingsProcess::game_00e188a8()` reference separately for Game/apply/title; mapped E188A8 bytes do not create another Game publication.

Select this proposed, actually consumed application seam in `include/bsp/game_native_frontend_application.hpp` and `src/game_native_frontend_application.cpp`:

```cpp
namespace bsp::game {
class GameNativeFrontEndApplication final {
public:
    GameNativeFrontEndApplication(GameHostLog&, GameSingletonHost&,
        const GameNativeFrontEndDataView&, void* volatile& actual_game_00e188a8,
        GameNativeGuiManagerApplication&, GameNativeGuiPageApplication&,
        NativeStringRawPoolContext&, GameNativeReadOnlyData&);
    void* frame_004c1ac0();
    void register_current_primary_10(void* captured_receiver);
    void select_layout_set_00518250(void* captured_frame,
        const volatile std::uint32_t& requested_dword_home,
        const volatile std::uint8_t& commit_byte_home);
    void apply_00518c80(void* captured_frame);
    void select_00518d60(std::uint32_t backdrop, std::uint32_t panel,
        const volatile std::uint32_t& title_dword_home,
        const void* const volatile& text_pointer_home);
    void select_backdrop_00518da0(std::uint32_t backdrop);
    void flag_00518b90(std::uint32_t flag);
    void title_005189b0(const void* captured_native_header8);
    void set_title_position_005186b0(const void* box_xyz, const void* title_xyz);
    void get_title_position_00518820(void* box_xyz, void* title_xyz);
    void collect_00518e00(void* captured_frame, void* actual_output_vector);
    void scalar_current_primary_0c(void* captured_frame,
        const volatile std::uint32_t& flags_dword_home);
    void scalar_current_secondary_00(void* captured_plus8,
        const volatile std::uint32_t& flags_dword_home);
};
}
```

These signatures are a proposed Source ABI. Native raw argument homes belong to the retained caller invocation, not to a provider's cached `int,bool,string_view` copy. Each current dispatch captures the receiver's actual current profile and requested slot codeword, resolves only established real Source bodies, and transports the actual argument words. No numeric codeword is cast to a host function pointer. The same app supplies the sibling `RawFrontEndScreenDispatch` service, whole current-ID4F71D0 and raw4F71A0; it owns no second bank.

The exact sibling raw-base seam is `uint32_t register_raw_front_end_screen_004f71d0(void* captured_receiver, const GameNativeFrontEndDataView&, RawFrontEndScreenDispatch&)` and `void destruct_raw_front_end_screen_004f71a0(void* captured_receiver, const GameNativeFrontEndDataView&)`. Existing `Fe8FrameRegisterHost::front_end_screen_register_004f71d0` returns void; replace the adopted route with the genuine current00/raw-ID operation and preserve its authentic normal return. Whole517ED0 ignores that returned EAX after the complete base operation; it must not supply ID12 or a semantic receiver to make registration succeed.

Add `GameStartupHost::native_frontend_` as a `unique_ptr<GameNativeFrontEndApplication>` adjacent to `native_gui_pages_`. Construct it after the actual Page/manager/providers and before the first raw Game/screen/Frame creator. Menu/HUD/loading/InitOnce borrow it. Its per-allocation receiver records and per-invocation string/native-call frames retain older live Frames even when `receiver != E18D80`.

The admitted two-reference getter alone cannot publish all custody metadata: it publishes and registers CURRENT+8 before return, and registration callbacks can replace publication or throw. A wrapper recording only the returned/catch-time publication loses the captured old allocation. The coupled Source packet must add a USED metadata-only overload of `get_native_front_end_frame_004c1ac0(manager_ref, frame_ref, NativeFrontEndFramePublicationCustody&)`. Preprepare stable invocation/receiver records before entering it; nonthrowing notifications at allocation, construction return and captured-allocation free record those exact identities BEFORE manager registration. Keep captured allocation, constructed return and later current+8 distinct. Notifications perform no native call, retain, initialization, extra catch, rollback or reset. The original native body/schedule is reused with zero duplicate credit. Partial registration failure preserves publication, raw bytes and the entered operation; it does not make all85 slots initialized.

The proposed overload and pure custody interface belong in the existing `native_front_end_frame_publication.hpp/.cpp`; the app supplies the preprepared implementation:
```cpp
struct NativeFrontEndFramePublicationCustody {
    virtual ~NativeFrontEndFramePublicationCustody() = default;
    virtual void allocation_returned(void* captured_allocation) noexcept = 0;
    virtual void construction_returned(void* captured_allocation,
        void* constructed_return) noexcept = 0;
    virtual void lifetime_argument_captured(void* captured_current_plus8) noexcept = 0;
    virtual void allocation_freed(void* captured_allocation) noexcept = 0;
};
void* get_native_front_end_frame_004c1ac0(void* volatile& manager,
    void* volatile& frame, NativeFrontEndFramePublicationCustody&);
```
The lifetime-argument notification records the existing local AFTER current+8 capture and BEFORE the second manager getter. On the fast path the app authenticates the returned existing live record; it does not create a new allocation epoch. Actual creators outside this app must supply this same custody domain before their receivers can be adopted.

## Whole517ED0 and518250

517ED0 first calls whole raw4F71D0 on its captured receiver. It then writes own+160=4. For each group (13 backdrop columns,2 panel,2 title), each outer column captures its CURRENT name pointer using CURRENT own+160; each of five inner rows rereads own+160. On row match/non-null name, measure that captured C string, initialize the actual8-byte NativeString, real41DD40(length,1), copy length+1 using the CURRENT resulting header, then real4C12B0 and AA5840(header,1,0). Publish the returned Page/null into the actual slot before the reached pool/header cleanup. Continue slot-major/set-minor; callbacks can affect later own/name reads. Row4's initial null names are data, not a policy or omitted producer.

518250 reads request home for the entry comparison, then commit BYTE for the retirement decision. Traverse backdrop13x5, panel2x5 and title2x5 slot-major/set-minor. Each row comparison rereads request home. Each nonnull slot reads actual Page+4:
- **Count exactly1:** call the actual GUI getter, THEN reload the CURRENT actual slot and call whole AA31F0(manager, reloaded Page). AA31F0 removes the first same registry pointer, calls current+20, and freshly resolves current+04(flags1). It does not decrement Page+4.
- **Other count:** capture that CURRENT Page; call the SAME retained CE2220 target on its aligned+4. At returned zero, capture that Page's CURRENT profile/current+00 and call its genuine target. For D5BE38 this is BD30E0; it freshly resolves current+04(flags1). No GUI getter, registry removal or current+20 is inserted in this arm.
- Clear the original Frame SLOT only after the selected retirement call returns, even if a callback replaced it. On failure preserve effects and the uncleared slot; no automatic recovery action.

Each acquisition group independently rereads request home to form its row. Each slot reads its CURRENT writable name cell BEFORE checking whether the actual Page slot is already populated. For empty/non-null name use the actual NativeString/getter/AA5840(header,1,1) sequence; publish before normal temporary cleanup. At the end reread commit BYTE, and if nonzero reread request DWORD before own+160 publication. Do not clamp/refuse indices, snapshot names, clear before calls, force row4, or replace native pointer arithmetic with a successful invalid-input fallback. Ordinary Source invalid-domain/fault semantics remain unproved.

The Page additions below are only admitted with this USED startup-owned consumer, over the SAME existing `Impl::Page`, lifetime contexts and call frames:
```cpp
// GameNativeGuiPageApplication: all may throw; no noexcept zero terminal.
void remove_page_00aa31f0(void* actual_manager, void* actual_page);
long decrement_reference_00ce2220(void* captured_actual_page);
void call_current_page_virtual_00(void* captured_actual_page);
```
The first delegates existing registry/manager retirement. The decrement reads `layer_lifetime.actual_decrement_00ce2220` only at the reached call; it supplies no new import/default/count. The zero method captures current00 AFTER decrement returned zero, then uses existing `invoke_native_ref_counted_delete_00bd30e0`; its `NativeRefCountedDeleteCalls::delete_vslot04` validates SAME identity/flags1 and the freshly captured current04, then uses SAME `Impl::delete_page`/AA38F0/AC5480 services. Do not insert a scene-released precondition.

There is no `NativeGuiLayerReference` counted companion class at this baseline; only the decrement typedef has that prefix. Page itself has raw allocation/custody records. Existing Group/Text `RenderCommandReference::release_zero_references() noexcept` terminals catch/terminate and remain unchanged. Using them for the throwing whole518250 call would change exception transport. No new Page counter/companion/native retain is needed. Installed PE static bytes prove D5BE38+00=BD30E0 and+04=AA38F0; that is not live receiver state.

## Apply, title, position and caller graph

Apply518C80 keeps its captured Frame. Each native phase reloads own+160 and current Page/selector/profile/+34 target; it must not reacquire a replacement Frame or cache a set across callbacks. Panel/title visibility carriers preserve the computed index's upper24 bits with SETZ in the low byte; backdrop uses its separately computed index carrier. A generic `bool shown` dispatch loses this word. Only a captured genuine AA8530 target may consume its documented byte semantics. Read the SAME current Game+5D4 after the two small groups: mode0D skips backdrop AND flag tail. Otherwise tail flag receives CURRENT E08654.

518D60 captures title DWORD first, writes84/88/8C, calls real getter/apply, THEN reads CURRENT text-pointer home and CURRENT header length for conditional title. 518DA0 writes84 then getter/tail apply. Flag518B90 compares signed panel selection before storing E08654 DWORD, follows its own repeated getter schedule, and reloads E08654 LOW WORD after child lookup for current+88(word,0,1.0f).

Title5189B0 and position5186B0/518820 use their real repeated getter pairs: first return supplies the Page-array receiver, second return supplies own+160, with current title selector read at each original point. They can refer to different live Frame generations. Build/return each real child-name NativeString and preserve child lookup, cleanup, submit and widget target epochs. Title supplies actual ABAED0(header,1), not a copied string view. Its width path is exact x87: FLD extent, FDIV double960, FSTP float, reload double pad and float extent, FCOMI/FSTP/JBE (unordered takes the minimum arm), then the actual size getter and FADD double pad/FSTP float on the other arm. Keep the native memory spill across the getter callback and ambient x87 control/status; do not substitute `std::max`, double-only arithmetic or alter the FP environment. Position getter performs its ordered component FLD/FSTP stores, not a bulk memcpy. Source repair alone does not establish unmasked-FP faults, Original FH3/SEH or private-frame aliases.

| Actual caller and current Source hook | Required same-object sequence |
| --- | --- |
| InitOnce004DD5B0, `oct18_tail_a.cpp:619..620` | Real menu-command00425D10 getter/current primary+10 registration at4DD756 FIRST; real Frame getter then captured returned receiver CURRENT primary+10 at4DD764 -> whole517ED0. Abstract `InitOnceServices` currently has no Root production implementation. |
| Game title004C9A70; sites4C9B7E/85; `MenuTitleInitHost` | Caller-owned homes set2/commit1; getter then whole518250 on captured return. |
| Game interface004C9CA0 site4C9E06; `GameHudHost::apply_in_game_interface` | set3/commit1 through the same getter and whole service. |
| Game mission004DFB70 site4E04EE; `GameHudHost::select_front_end_layout_00518250` | set3/commit0, no semantic `acquire_hidden` visibility policy. |
| Loading57CB60 sites57CD1E/7B; `MenuLoadingScreenHost::select_menu_layout` | set2/commit1 and set1/commit1 respectively, same homes/getter/Page graph. |
| Menu/state/screen-set/park/hint/pause families | Use sibling actual raw95/current dispatch and this same writer/apply/title/position service; no `FrontEndWorld::screens` or private `frame_layouts` authority on the adopted route. |

Collector518E00 retains its captured Frame and separately reloads own+160 atE03/E2A/E4B and selectors atE0C/E30/E51. Pass each actual Page-slot ADDRESS to4D6790 before starting the next group. Park4D64E0/current primary+24 must not reacquire CURRENT publication or copy Page values. These are repairs to existing bodies, not new-body credit.

## Two Frame retirement entrances and Root order

517D90 stamps CEC39C and secondaryCEC398; for all85 nonnull slots in native order, call real4C12B0 THEN reload that slot before AA31F0. It does not clear each Page slot. After traversal unconditionally clear the SAME E18D80, stamp secondaryCE3818, and raw4F71A0 stamps primaryCEAE54 then clears ALL95 exact receiver matches. Derived517EB0 calls full517D90 before reading the CURRENT flags LOW BYTE; free captured allocation only if bit0 and return identity only. Secondary517D50 adjusts the captured+8 receiver by-8 then takes that route. Base517D60 samples flags bit0 BEFORE clearing E18D80/stamping its receiverCE3818; clear517CF0 also writes this SAME publication unconditionally. Do not bind the real Frame's CEC398 profile to existing CE3818 base deletion.

Game destruction004DCF90 (also used by scalar004DE270) reaches4C0CE0 at4DD0E6 via existing `NativeGameLifetimeContext::singletons`. Its context must borrow the SAME manager and E18D80 refs, with actual CURRENT primary+0C scalar dispatch.4C0CE0 captures section, enters/increments, captures publication+8 BEFORE its second manager getter, unregisters that captured pointer, reloads CURRENT publication for scalar(flags1), then clears publication AFTER scalar returns, decrements/leaves original section. A callback-created replacement can be cleared; add no compare-and-clear or replay. Shared/fallback manager drain independently needs CEC398/current+00 ->517D50/-8/517EB0 in `NativeSingletonDeletionBindings`, with full flags transport, installed BEFORE getter registration can occur.

The actual Root currently deletes Menu and GameFrontend at `game_hosts.cpp:2313..2315` before shared drain2321; Page and manager are retained past it2335..2348. Move the new Frame/raw95 services out of `GameMenuHost::Impl`, retain `native_frontend_` through real Game OnDestroy and normal/fallback shared drain, and retain actual Page/renderer/Model/VFS/Lua/string/scene/node/child/timed providers beneath it. Bind its Frame scalar resolver before any registered+8 identity exists. Release/acknowledge Frame operations only after their genuine completion and shared drain; then drop Frame bindings/application before Page and manager. Other Page clients/renderer callbacks keep their existing drain acknowledgements. Permanent canonical cells, settings Game cell and manager publication survive these object epochs and later CRT callbacks. A failed partial getter/register/selection/destruction retains its real graph and operation; diagnostic retirement is not graph disposal. This plan adds no new terminal-process policy.

## Smallest next work and verdict

1. **Finite Source composition/repair packet:** actual Frame operation/header homes, used Page retirement methods, getter custody notifications, whole517ED0/518250/517D90 repairs, known517EB0/517D50 lifetime dispatch, apply/flag/title/position/collector schedule, Startup retention and exact caller migration above. Use the sibling canonical and raw95 contracts in the SAME adopted graph. No new Native body is missing for the two Page retirement primitives or known Frame wrappers. Do not land an unused Page accessor or claim full title readiness from it.
2. **Genuine raw title/widget provider packet:** `native_gui_text_source_storage.cpp:242..246` rejects the changed arm BEFORE ABAF07. Existing raw ABA8D0 admits only empty/unchanged; typed `cc13_gui1/gui2` and `gui_text_runtime_*` counterparts do not supply the same raw Text object. Recover/admit raw changed ABAED0 -> localization A9FAD0 -> nonempty ABA8D0/current+50 on the SAME Text/Model owners, and compose only reached Text/Icon/FrameBox factories/current methods into the same Page client. Current AA6560 binding rejects every non-Type2 at `game_native_gui_page_application.cpp:424`. Raw AA7E00 for those actual child lists also needs its actual-storage variant; the existing owner overload uses `GuiLayoutWidget`/typed children. AA6740 and AA67F0 are established trivial address leaves, not missing OS calls. Reuse existing body evidence/lifetimes; do not credit a duplicate Native reconstruction. This is the exact provider cut that prevents declaring the whole caller packet ready.
3. Independently close the sibling's real raw95 menu-command/MenuPrompt/current dispatch providers and canonical capability/view. No raw Frame-only ID12 bank suffices.

Whole adoption `source_ready=false`, `caller_admission=false`, `activation=false`. This packet changes only this document and its report. No Source/header/CMake/ledger/shared Ghidra edits, build/link/probe/executable/body execution or game/process action occurred. One ignored local lookup index was generated; original PE and saved exports were read only. Static Source/assembly/PE inspection is the only new validation. Original/runtime/vendor/duplicate-body/CG/ABI/gameplay/activation credit is zero. Ordinary Source C++ exceptions do not establish Original ABI/FH3/SEH/private-frame/fault/unmasked-FP behavior.
