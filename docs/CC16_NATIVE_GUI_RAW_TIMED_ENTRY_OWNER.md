# Raw GUI timed-entry storage and Page lifetime contract

Frozen Source is Main `a7a1a46f7d77db931534d81fee8985d8809b1f13`. This packet specifies an implementable raw allocation/producer/deletion contract; it implements no provider and admits no new Page, update or runtime path. The existing semantic `GuiTimedEntryStorage`/`GuiTimedEntryOwner` cannot supply it: their `widget_08` is a `GuiLayoutWidget*` and their header belongs to that layout. A raw Page or child must keep its actual widget allocation, its own `+88/+8C/+90`, and actual entry `+08` pointer. No cast between those domains is valid. Supporting receipts and exact Native findings are in `reports/cc16_native_gui_raw_timed_entry_owner.json`.

## Actual reach and existing whole bodies

The CURRENT Page path has **no timed producer**. `game_native_gui_page_application.cpp:630–650` reaches scene acquisition, root propagation, position, AAA710 prefix/interaction and the childless key suffix. The prefix writes properties directly; interaction reads Visible/MouseBlock/MouseHit and recomposes the transform. The full library AAACD4 body creates widgets/Models, parents them and invokes current `74/18/78`; it contains no AA8B00/AD3A80 call. The current application's suffix instead rejects every mapped widget at `705–708`. Its timed deletion callback rejects at `374`, its table lookup first requires a live Page at `336–339`, and `delete_page` still requires all three timed-header words zero at `787–790`. These are current boundaries, not genuine timed providers.

AA9390 initializes `88/8C/90` to zero (`native_gui_widget_base_storage.cpp:65–66`); AA9520 copy also initializes those destination words to zero (`native_gui_widget_copy_storage.cpp:77–78`). This proves those constructor effects, not an invariant over all later callbacks, widgets, scripts or future HUD bindings. Do not replace a populated lifetime supplier with an empty-header test or inject an invented timer call into Page properties.

Live verified Native callers of AA8B00 are HUD `0064DD30`, `00648060` and `00648C20`. Existing Source examples are `ingame_gui_cc13.cpp:3057–3070` (award ticker: index0, AC2F20, then entry+4 countdown) and `oct18_tail_a.cpp:1715/1782–1785` (unit row: index0, AC2F20, then +4). The current `game_hosts_hud.cpp:2940–2949` circle-progress bridge records the operation and creates no entry; it does not connect the raw producer. These HUD call sites, not a newly imagined Page property, are the real future raw-widget producer consumers. Their update/setter integration is a separate packet. No reached current Page operation requires AC2F40, ABE7B0 or AD39A0, and none is added here.

The complete existing raw normal producer is `cc13_gui1.cpp:267–279` AA8B00 -> `319–335` AD3A80. AA8B00 preserves all three signed count checks/resizes; it captures the ORIGINAL null slot address before AD3A80, stores the returned identity there after normal return, and finally reloads CURRENT header/data. A callback may select another retained backing; do not redirect the publication to that replacement or cache the returned lookup from the old array. Negative/wrapped indexes or a callback that frees the captured destination are outside valid physical storage; reject/retain at that Source boundary, never defer a genuine free or fabricate a replacement destination. An allocation/constructor throw performs no slot publication and no rollback of earlier resize or callback effects.

## Actual storage, construction and current scalars

Add a **separate** physical raw type, with no C++ virtual functions or ownership in its fields:

```cpp
struct NativeGuiTimedEntryStorage {
    std::uint32_t profile_00;
    float remaining_04;
    void* actual_widget_08;
    float target_0c;
    float rate_10;
}; // MSVC Win32: size14h, offsets0/4/8/C/10; assert them.
```

Allocate exactly `14h` raw bytes with the existing `singleton_lifetime_allocate({object,0x14,0x14})`; that is the current Source malloc/CRT-new-handler/retry domain. Begin only the trivial raw type's lifetime without value initialization. Its matching BF65AC site calls `singleton_lifetime_free` on that SAME allocation. No GUI pool, widget `+04` credit, `NativeRenderActualOwnerRegistry` entry or extra canonical companion belongs to a timed entry. `+04` is a remaining-time float, never a reference count. The entry borrows its actual widget; deletion does not read or freeze `+08`.

| Native body / original ABI | Required raw effect |
|---|---|
| AD3970, ECX block; widget stack; EAX block; RET4 | Write D5D204, then +4 positive zero, then +8 actual widget. Leave +C/+10 preimages untouched. |
| AC2ED0, same ABI | Call the raw AD3970 above, read live D7A24C ONCE with the same single-precision bit transport, write D5CA74, then that value to +C/+10. No semantic widget dispatch. |
| AD3A80, ECX signed index, EDX actual widget; EAX entry/null; RET | Indices0/1 allocate14h then AC2ED0. Index2 allocates14h then writes +8 widget, +4 zero, D5D210, +C zero, +10 zero IN ORDER. Other indexes and null block return null. |
| AC2FE0, ECX entry; flags stack; EAX original identity; RET4 | Call AD3990 to reset D5D204; test low flags bit0; if set BF65AC(entry). |
| AD3A40, same scalar ABI | Test low bit0, stamp D5D204, conditional BF65AC(entry), return identity. |
| AD3A60, same scalar ABI | The actual BASE D5D204 current0: same reset/conditional free schedule as AD3A40. Required after an earlier flags0 derived teardown. |

Map genuine immutable tables from the SAME `GameNativeReadOnlyData`, by the CAPTURED profile: D5CA74 -> current0 AC2FE0, D5D210 -> AD3A40, D5D204 -> AD3A60. These numeric cells identify compiled Source functions; never call them as addresses or synthesize a replacement C++ table. Pure lookup first authenticates independent actual allocation custody, never a widget registry/count. Dispatch uses the captured current0 target and accepts only its matching genuine family/phase. Flags0 keeps the allocation with base profile; flags1 consumes that allocation's provenance immediately before the actual free. Return the original pointer value without reading freed bytes. Unknown/foreign/failed identities reject before entry; a destructor does not clear a widget slot itself.

### Constructor EH omitted by current Source

`cc13_gui1.cpp:321–323` presently has no captured-block cleanup if `timed_entry_construct_00ac2ed0` throws. Native AD3A80 stores the raw allocation in its local home BEFORE selecting EH state0 for index0 or state1 for index1, then calls AC2ED0 at AD3B1A. Handler CB9376 loads FuncInfo DF0958: `maxState=2`, unwind map DF0948. State0 -> CB936B and state1 -> CB9360; both load `[EBP-10h]`, call BF65AC on that captured block and return. Thus the raw whole-body Source route must use `try { return construct(block, widget); } catch (...) { free_captured(block); throw; }` after nonnull allocation. A new-handler/allocation throw has no allocated block to free. Index2 has no constructor EH action. Preserve zero-slot/no publication and all preceding header effects. This is the one concrete omitted Source body arm found here; there is no unresolved Native leaf for these constructors or scalars. Ordinary C++ cleanup does not establish native FH3/SEH/private-home or fault transport.

## Coupled Source interface and custody

Proposed names below are new Source interfaces. Put the raw type, raw ctor/scalar declarations and allocation interface in the existing `native_gui_widget_lifetime.hpp/.cpp`, avoiding a new CMake target. Extract the existing producer's three hooks into a narrow `cc13_gui1::GuiTimedEntryHooks` and add the mandatory constructor-failure free hook:

```cpp
struct GuiTimedEntryHooks {
    virtual ~GuiTimedEntryHooks() = default;
    virtual void timed_entry_array_resize_00aa77b0(void* actual_header, std::int32_t) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void* timed_entry_construct_00ac2ed0(void* actual_block, void* actual_widget) = 0;
    virtual void free_failed_constructor_00bf65ac(void* captured_block) noexcept = 0;
    virtual void timed_entry_construction_returned(void* actual_entry) noexcept = 0; // Source metadata only
};
void* gui_timed_entry_get_or_create_00aa8b00(GuiTimedEntryHooks&, void*, std::int32_t);
void* gui_timed_entry_create_by_index_00ad3a80(GuiTimedEntryHooks&, std::int32_t, void*);

struct NativeGuiTimedStorageCalls {
    virtual ~NativeGuiTimedStorageCalls() = default;
    virtual void* allocate_timed_pointer_slots_00bf681b(void* actual_header, std::uint32_t bytes) = 0;
    virtual void free_timed_pointer_slots_00bf65ac(void* actual_header, void* actual_backing) = 0;
};
void reserve_native_gui_timed_pointers_00aa6f30(void*, std::int32_t, NativeGuiTimedStorageCalls&);
void resize_native_gui_timed_pointers_00aa77b0(void*, std::int32_t, NativeGuiTimedStorageCalls&);
void destroy_native_gui_timed_pointers_00aa7f50(void*, NativeGuiTimedStorageCalls&);

void* construct_native_gui_timed_entry_base_00ad3970(void* block, void* actual_widget) noexcept;
void* construct_native_gui_timed_entry_alpha_00ac2ed0(
    void* block, void* actual_widget, const volatile float& actual_one_00d7a24c) noexcept;
void reset_native_gui_timed_entry_base_00ad3990(void* actual_entry) noexcept;
using NativeGuiTimedEntryFree = void (*)(void* context, void* actual_entry) noexcept;
void* scalar_delete_native_gui_timed_entry_alpha_00ac2fe0(
    void*, std::uint32_t flags, void* free_context, NativeGuiTimedEntryFree);
void* scalar_delete_native_gui_timed_entry_section_00ad3a40(
    void*, std::uint32_t flags, void* free_context, NativeGuiTimedEntryFree);
void* scalar_delete_native_gui_timed_entry_base_00ad3a60(
    void*, std::uint32_t flags, void* free_context, NativeGuiTimedEntryFree);
```

`GuiWidgetHooks` inherits the narrow hook base, retaining its existing unrelated methods. Keep its two old public producer signatures as forwarding overloads; move the SAME complete producer body into the narrow overloads and fix the EH arm there. There is no full `GuiWidgetHooks` implementation full of rejecting/no-op unrelated operations. Existing real providers must implement the genuine matching constructor-failure free; no default is supplied. Raw constructors/scalars use actual `void*` widget storage and owned raw entry bytes, not `gui_timed_entry_types.hpp` casts.

For the Page graph, use one retained `RawTimedEntries` service within `GameNativeGuiPageApplication::Impl`, with records associated with the SAME retained Page/WidgetRecords. It implements the narrow producer hooks. Its backing methods are exposed by the actual composite `Widgets` lifetime binding through the pure `NativeGuiTimedStorageCalls` interface. At creation of an AA9730 schedule, discover this cross-interface once by a pure metadata `dynamic_cast` of `c.bindings`; a qualified raw Page admission requires its exact service before effects. This preserves existing context/acquisition layouts and automatically follows the SAME context through Layer/Group/ClipBox/Text/Icon base calls; no alternate companion or derived-scalar replacement is needed. Legacy callers without that interface retain their existing genuine Source CRT allocation/free route. Its absence is never accepted as a raw Page custody provider.

The service's `void* get_or_create(void* actual_widget, std::int32_t index)` authenticates that SAME live Page/WidgetRecord and calls the narrow existing AA8B00 body, using a distinct retained invocation adapter for its hooks. Each invocation retains its owning actual widget and prepared allocation records; allocation/new-handler reentry cannot overwrite another invocation's receiver or pending records in a global `current_widget`. The alpha hook directly calls the raw AC2ED0 function with the SAME mapped D7A24C reference. The scalar service authenticates identity/target/custody before entering the raw body and passes an already prepared no-throw receipt-consumption/free thunk; it does not add a no-throw boundary to Page/Model dispatch. `timed_entry_construction_returned` runs once after the constructor/inline producer returns and before publication, including index2; its receipt transition is allocation-free and nonthrowing. It is not a Native virtual call. Backing callbacks receive the actual header for authentication against its same Page/WidgetRecord; native free(null) remains a real free(null), without inventing a receipt.

Share each raw array body between the old genuine-CRT entry and explicit-custody overload. Reserve clamps requested capacity to at least1, signed-compares CURRENT capacity, computes wrapping byte count, allocates, then copies with CURRENT count/data reloads, frees CURRENT old backing, and only then publishes fresh data/capacity. Resize preserves its native zero-fill and decrement/count-store order; shrinking never deletes pointed entries. Do not call a typed vector implementation or snapshot a header around new-handler callbacks. The admitted owned-storage domain excludes negative counts, invalid physical spans and wrapped allocation sizes; retain the whole Source schedule without silently normalizing malformed storage. AA9730's distinct inline finish, including its negative-capacity reserve and negative-count branch, must remain the existing body rather than being replaced with generic resize0.

Every allocation receipt is prepared in stable host storage BEFORE its native allocation callback, then filled/linked without further throwing allocation after return. Backing and entry receipts are independent; their sizes and actual addresses prove physical custody but mirror no native count. An actual captured slot and every backing needed by a pending read/store must remain allocated; a replacement current header does not transfer or erase the old receipt. A newly completed entry not ultimately returned from CURRENT backing still stays retained. At a native backing-free site, consume only the matching actual backing receipt immediately before `singleton_lifetime_free`. Metadata rejection can throw before that free; the already acquired replacement and partially changed graph remain retained. Do not roll back header changes made by the allocator's new handler, free a completed orphan to make bookkeeping tidy, or defer an actual native free to preserve an otherwise invalid captured slot.

## Actual Page deletion binding and next packet

Extend the existing Page `Widgets::current_table` to authenticate actual timed receipts BEFORE the Page/widget-only lookup and return the genuine captured-profile table. Its `call_timed_delete_00(actual_entry,target,flags)` delegates the exact raw scalar/free service above. If using `NativeGuiWidgetLifetimeDispatch` elsewhere, its existing required calls already have the correct current-table/deletion signatures (`native_gui_widget_lifetime_dispatch.hpp:14–29`); the raw service supplies those methods rather than manufacturing a counted owner.

AA9730 must retain its existing current count loop, captured ORIGINAL slot, actual current0(flags1), and post-return original-slot zero. Route ALL its timed backing sites through the same prepared storage service: loop resize, inline final negative-capacity reserve/final free, and state2 AA7F50 unwind cleanup. At state2 cleanup it frees BACKING ONLY, never undeleted pointed entries. A callback failure leaves the original slot uncleared; library cleanup may then free its backing while the independent entry receipt survives. A Page-specific outer raw-failure transport that explicitly retains the graph must preserve that different policy and not invent native exception-origin classification. Neither policy may retry, replay deletion, clear replacement slots, drain receipts or erase a failed widget binding. The same frames, actual widget/Model/scene owners, timed entries and remaining backing survive the chosen entered-failure disposition. Do not introduce a new `noexcept` boundary for normal Page/Model callbacks.

The bounded implementation lower bound is **five existing production files**: `include/bsp/cc13_gui1.hpp`, `src/cc13_gui1.cpp`, `include/bsp/native_gui_widget_lifetime.hpp`, `src/native_gui_widget_lifetime.cpp`, and `src/game_native_gui_page_application.cpp`. It must couple the shared real producer body, raw ctor/scalar/EH arm, backing callback sites, actual Page table/deletion binding, and retained Page/WidgetRecord custody. This is not a claim that those five files alone complete populated Page: the separate recursive reader, raw parenting and canonical Model terminal packets remain required. A real HUD-to-raw-widget producer bridge also changes its actual HUD host/consumer files; do not count an unused accessor or artificial property call as that integration. If an implementation instead preserves a closed no-timer Page phase, prove that complete phase's reached producer/callback graph explicitly and keep later timer/HUD activation gated.

Next packet: implement the coupled five-file raw storage/producer/lifetime changes under one lease, integrate them with the actual Page graph supplier, then connect the selected genuine HUD consumer when its raw widget domain is ready. Use the existing focused checks and required Win32 build; one targeted constructor-failure/captured-slot case is justified by the concrete missing EH arm. This evidence packet ran no build, test, executable, gameplay or process-control operation, made no Ghidra mutation, and grants zero new reconstruction/ABI/game-validation credit.
