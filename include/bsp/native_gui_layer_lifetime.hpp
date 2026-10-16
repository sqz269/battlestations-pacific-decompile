#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiWidgetLifetimeContext;
struct NativeStringRawPoolContext;

using NativeGuiLayerReferenceDecrement = long (__stdcall*)(volatile long*);

// Required calls over the SAME actual manager/store/scene identities. No
// typed GuiLayerImage, GuiLayoutPage, SceneResource or default owner is used.
class NativeGuiLayerLifetimeBindings {
public:
    virtual ~NativeGuiLayerLifetimeBindings() = default;
    virtual void* manager_004c12b0() = 0;
    virtual void remove_camera_store_00aa4b30(void* actual_manager,
        void* actual_store) = 0;
    // Side-effect-free metadata lookup, not an Original call. Resolve the
    // captured numeric CURRENT profile to its SAME live table (>=1 DWORD).
    // The returned table and these bindings must survive terminal retirement.
    virtual const volatile std::uint32_t* current_scene_table(void* actual_scene,
        std::uint32_t captured_profile) = 0;
    virtual void call_scene_zero_00(void* actual_scene,
        std::uint32_t captured_target) = 0;
};

struct NativeGuiLayerLifetimeContext {
    NativeGuiWidgetLifetimeContext& widgets;
    NativeStringRawPoolContext& strings;
    NativeGuiLayerLifetimeBindings& bindings;
    // Borrow the actual stdcall LONG decrement target cell; read it only at
    // the reached native CE2220 call. No host import/default is supplied.
    NativeGuiLayerReferenceDecrement const volatile& actual_decrement_00ce2220;
};

// Whole B72250[28B]: ECX nonnull actual scene, no stack args; current CE2220
// decrement at aligned live LONG+04, nonzero RET or CURRENT profile/table0
// tail-dispatch at zero. The SAME raw prefix/table/terminal and context-lifetime
// contracts below apply. No typed NativeGuiSceneOwner conversion or extra
// count is used. No scene storage is read after terminal retirement. Source
// C++ return/call/exception ABI does not promise the Original tail-jump ABI.
// This leaf adds no cleanup; failure propagates to its caller's cleanup state.
void release_native_gui_scene_storage_00b72250(void* actual_scene,
    NativeGuiLayerLifetimeContext&);

// Whole AC5480[213B]: ECX actual124h page, no stack args, RET. Requires its
// genuinely constructed raw base/containers and initialized reached derived
// fields, not an unfinished AC6600 prefix or a cast from a typed companion.
// Scene profile/count and tables are aligned live Win32 DWORD/LONG storage;
// every reached provider/current decrement target must be genuinely supplied.
// Contexts/metadata cannot alias retired scene/name/pool backing or private
// Original stack scratch. Serialized callbacks may change live page fields;
// the reads and post-callback EC clear retain the Original schedule. No scene
// storage is read after its zero callback. Active page storage survives calls.
//
// The owned arm uses the whole raw B72250 above; shared decrement stays
// inline at its native site. Normal name return uses raw419CC0/BD1510, no header
// clearing. Ordinary Source C++ unwind: state1 ->41DD20(page+100) ->AA9730;
// state0 ->AA9730; state-1 ->no repeated cleanup. Cleanup failure terminates.
// An entered failure is not restartable; caller retains raw slot/unresolved
// resources for explicit disposition. Native FH3/SEH, corrupt storage, private
// frame aliases, binary ABI and complete populated-subtree proof are excluded.
void destroy_native_gui_layer_00ac5480(void* actual_page,
    NativeGuiLayerLifetimeContext&);

// Whole AA38F0[28B]: ECX page, one flags DWORD, RET4/EAX original identity.
// This Source API borrows an initialized caller DWORD and reads its LOW BYTE
// only AFTER AC5480 returns. On bit0 use the existing AC4C40 Source pool
// return; the page must belong to that SAME initialized layer pool. On throw
// no slot return occurs. Return is identity only after free; no count gate,
// layout retention, manager-vector removal or implicit ownership is added.
void* scalar_delete_native_gui_layer_00aa38f0(void* actual_page,
    const volatile std::uint32_t& current_flags, NativeGuiLayerLifetimeContext&);
} // namespace bsp
