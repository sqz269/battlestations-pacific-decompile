#pragma once
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
struct NativeSingletonDeletionBindings;

// Actual40h state, not GuiIconState or a second owning representation. Name
// is the native8h header; texture is its one owned actual +04 reference.
// Native AB3A70 leaves3E/3F unwritten and copies2C/30/34/38 via x87.
struct NativeGuiIconStateStorage {
    void* texture_00;
    std::uint32_t name_length_04;
    char* name_characters_08;
    std::uint32_t resolved_uv_0c[4];
    std::uint32_t authored_uv_1c[4];
    float pivot_x_2c, pivot_y_30, width_34, height_38;
    std::uint8_t pending_3c, flag_3d, unwritten_3e[2];
};
// Actual F0..FF header. F0 proxy is constructor-unwritten, not a host proxy.
struct NativeGuiIconStateVectorStorage {
    std::uint32_t proxy_f0;
    NativeGuiIconStateStorage* first_f4;
    NativeGuiIconStateStorage* last_f8;
    NativeGuiIconStateStorage* end_fc;
};
// Borrowed EC..137 view only. All padding/proxy bytes stay native preimages.
// The pool's slab-index DWORD138 is separate and never part of this payload.
struct NativeGuiIconTailStorage {
    std::int16_t current_ec;
    std::uint8_t unwritten_ee[2];
    NativeGuiIconStateVectorStorage states_f0;
    std::uint8_t has_texture_100, delayed_101, unwritten_102[2];
    std::uint32_t shader_length_104;
    char* shader_characters_108;
    std::uint8_t flag_10c, unwritten_10d[3];
    std::int32_t mode_110;
    float ratio_114;
    std::uint32_t crop_118[4];
    float timer_128;
    std::uint16_t temporary_12c;
    std::uint8_t unwritten_12e[2];
    float duration_130;
    std::uint8_t filter_134, unwritten_135[3];
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuiIconStateStorage) == 0x40);
static_assert(offsetof(NativeGuiIconStateStorage, pivot_x_2c) == 0x2c);
static_assert(sizeof(NativeGuiIconStateVectorStorage) == 0x10);
static_assert(sizeof(NativeGuiIconTailStorage) == 0x4c);
static_assert(offsetof(NativeGuiIconTailStorage, states_f0) == 4);
static_assert(offsetof(NativeGuiIconTailStorage, shader_length_104) == 0x18);
static_assert(offsetof(NativeGuiIconTailStorage, filter_134) == 0x48);
NativeGuiIconTailStorage& native_gui_icon_tail(void* actual_icon);

// Explicit SAME-domain services; metadata only, no singleton/count/registry
// construction. Delayed and strings must use these same actual process cells.
// The appended shared singleton deletion arm must already point to delayed;
// every Icon destructor checks that equality BEFORE AB5700 is reached.
struct NativeGuiIconStorageContext {
    NativeGuiWidgetLifetimeContext& lifetime;
    NativeRenderActualOwnerRegistry& actual_owners;
    NativeStringRawPoolContext& strings;
    NativeGuiIconDelayedLoads& delayed;
    const NativeSingletonDeletionBindings& singleton_deletion;
    const volatile std::uint32_t* icon_profile_00d5c4c0; // at least33 DWORDs for copy CURRENT80
};

// Mandatory genuine CURRENT80 over the SAME full raw Icon, Model and geometry.
// AB5D30 captures its native table at AB5E12, then reloads that table's slot80
// at AB5E48; argument is the zero-extended current EC word from AB5E28.
// No typed GuiIconRuntime state mirror, no-op or projected rebuild is supplied.
class NativeGuiIconCurrentCalls {
public:
    virtual ~NativeGuiIconCurrentCalls() = default;
    virtual void call_rebuild_current80(void* actual_icon,
        std::uint32_t selected_target, std::uint32_t current_state_word) = 0;
};
struct NativeGuiIconCopyContext {
    NativeGuiWidgetCopyContext& base;
    NativeGuiIconStorageContext& storage;
    NativeGuiIconCurrentCalls& current;
};
struct NativeGuiIconCopyAcquired {
    NativeGuiWidgetCopyAcquired base;
    bool started{}, base_returned{}, vector_entered{}, states_returned{};
    bool current80_entered{}, cleanup_entered{}, complete{}, failed{};
    std::int32_t native_eh_state{-1};
    std::exception_ptr failure;
};
struct NativeGuiIconDestroyAcquired {
    enum class Phase { fresh, delayed_getter, delayed_cancel, shader, states,
        base, pool_return, complete, failed };
    Phase phase{Phase::fresh};
    std::int32_t native_eh_state{-1};
    bool started{}, cleanup_entered{}, complete{}, failed{};
    std::exception_ptr failure;
};

// AB3A70 ECX destination40h, source stack, RET4/EAX same. Real raw string
// resize/copy then native field order and a single actual texture+04 increment.
// AB3280 ECX actual state, RET: release CURRENT texture, clear only after its
// terminal returns, then release CURRENT name. Name unwind also runs if texture
// release throws. Headers are not silently repaired or cleared on name release.
// Textures are actual live canonical owners in this SAME registry; no host ptr.
void* copy_native_gui_icon_state_00ab3a70(void* actual_destination,
    const void* actual_source, NativeGuiIconStorageContext&);
void destroy_native_gui_icon_state_00ab3280(void* actual_state,
    NativeGuiIconStorageContext&);

// Qualified stock vector substitution, not reconstructed vendor-library credit.
// AB58F0: ECX destination header, stacked source, EAX destination, RET4;
// AB5850: ECX header, no stacked arguments, RET. These C++ APIs are not ABI shims.
// Actual F0 header/40h records and compatible CRT backing only. Exact-size copy,
// native completed-prefix cleanup and raw AB3A70/AB3280 ownership. F0 preserved.
// Serialized valid native ranges/nonoverlapping copy; malformed storage, debug
// iterator/proxy ABI, allocation-null faults and racing mutation are excluded.
// Tidy captures first/end once, releases in forward order, frees CURRENT first,
// then clears F4/F8/FC. No std::vector, texture/name mirror or alternate heap.
void copy_native_gui_icon_states_00ab58f0(void* actual_destination_f0,
    const void* actual_source_f0, NativeGuiIconStorageContext&);
void destroy_native_gui_icon_states_00ab5850(void* actual_vector_f0,
    NativeGuiIconStorageContext&);

// Complete normal AB5C60 over actual138h, AA9390(type6) first. Preserve every
// native constructor-unwritten byte and the pool+138 slab index. No registry
// admission; the caller owns the fully constructed allocation until bound.
void* construct_native_gui_icon_default_00ab5c60(void* actual_icon,
    const volatile std::uint32_t& actual_one_00d7a24c);
// Complete AB5D30 source path with mandatory genuine current80. Real AA9520
// clone must support the SAME source Model; source actual+04 is not retained.
// ShaderName is RESET, not copied. x87 stores and native unwind states0/2 are
// preserved. Failed clone effects remain in the caller's retained base frame.
void* construct_native_gui_icon_copy_00ab5d30(void* actual_destination,
    const void* actual_source, NativeGuiIconCopyContext&, NativeGuiIconCopyAcquired&);
// AA1410 ECX optional source / EAX actual slot or0 / RET; split reached arms.
// Require initialized actual F8BDA0 pool. Null allocation returns0; constructor
// failure returns only its captured failed slot after constructor unwind.
void* create_native_gui_icon_default_00aa1410(
    const volatile std::uint32_t& actual_one_00d7a24c);
void* create_native_gui_icon_copy_00aa1410(const void* actual_source,
    NativeGuiIconCopyContext&, NativeGuiIconCopyAcquired&);

// AB5E70 ECX actual138h / RET: stamp Icon; SAME AB5700 -> AB50E0 cancellation;
// shader string; state vector; AA9730. One-shot frame records native effects and
// failure; no retry after entry. Full raw child/class/timed/parent dispatch,
// backing and all providers are mandatory transitive caller custody.
void destroy_native_gui_icon_00ab5e70(void* actual_icon,
    NativeGuiIconStorageContext&, NativeGuiIconDestroyAcquired&);
// AB6100 ECX this, stacked flags DWORD low byte AFTER dtor, EAX same/RET4.
// No widget count test/decrement. Flags1 returns SAME F8BDA0 slot; flags0 leaves
// the destroyed physical allocation with caller. Identity only after return.
void* delete_native_gui_icon_00ab6100(void* actual_icon,
    const volatile std::uint32_t& actual_flags_slot, NativeGuiIconStorageContext&,
    NativeGuiIconDestroyAcquired&);
} // namespace bsp
