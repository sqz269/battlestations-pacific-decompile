#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// SAME native counted arrays: pointer, signed element count, signed capacity.
// These are not the Line's proxy/begin/end/capacity STL header.
struct NativeGuiCurveCountedArrayStorage {
    void* data;
    std::int32_t count;
    std::int32_t capacity;
};
struct NativeGuiCurveTailStorage {
    NativeGuiCurveCountedArrayStorage control_ec;
    NativeGuiCurveCountedArrayStorage widths_f8;
    std::uint32_t texture_length_104;
    char* texture_data_108;
    NativeGuiCurveCountedArrayStorage points_10c;
    std::uint32_t word_118;
    float uv_11c[4];
    void* material_12c;
    std::uint32_t authored_proxy_130; // UNWRITTEN by both constructors
    void* authored_first_134;
    void* authored_last_138;
    void* authored_end_13c;
};
NativeGuiCurveTailStorage& native_gui_curve_tail(void* actual_curve);

using NativeGuiCurveInterlocked = long (__stdcall*)(volatile long*);
struct NativeGuiCurveStorageContext {
    NativeGuiWidgetLifetimeContext& lifetime;
    NativeStringRawPoolContext& strings;
    NativeRenderActualOwnerRegistry& actual_owners; // SAME Curve/material +04 companions
    NativeGuiCurveInterlocked const volatile& decrement_00ce2220;
};
// Required genuine AA7170: current+60(0), then current raw bounds, on the SAME
// actual widget/tree. Existing load-hook source can be qualified by its real
// table provider; numeric native profiles are never called as host pointers.
// No empty callback or projected GuiWidgetOwner is an acceptable binding.
class NativeGuiCurveLoadedCalls {
public:
    virtual ~NativeGuiCurveLoadedCalls() = default;
    virtual void loaded_base_00aa7170(void* actual_curve) = 0;
};

// External diagnostics only, not another allocation/reference/vector owner.
// Fresh per entry. An interrupted operation retains reached effects, SAME raw
// storage/providers and this record; never retry or return its slot. Destroying
// entered/failed diagnostics terminates. Native FH3/SEH/fault ABI is unclaimed.
struct NativeGuiCurveStorageAcquired {
    enum class Phase { fresh, entered, complete, failed };
    enum class Entry { clear, append, loaded, destroy, scalar };
    Phase phase{Phase::fresh};
    Entry entry{Entry::clear};
    void* actual{};
    std::uint32_t active_call_site{};
    std::uint32_t failed_call_site{};
    std::int32_t native_eh_state{-1};
    std::int32_t failed_eh_state{-1};
    void* captured_material{};
    NativeGuiCurveInterlocked captured_decrement{};
    long material_release_result{};
    bool material_decremented{}, material_zero_entered{}, material_zero_returned{};
    bool cleanup_entered[6]{}; // state0 base .. state5 authored vector
    bool slot_returned{};
    NativeGuiCurveStorageAcquired() = default;
    ~NativeGuiCurveStorageAcquired() noexcept;
    NativeGuiCurveStorageAcquired(const NativeGuiCurveStorageAcquired&) = delete;
    NativeGuiCurveStorageAcquired& operator=(const NativeGuiCurveStorageAcquired&) = delete;
};

// ABE030, ECX actual140h/EAX same/RET: genuine AA9390(type13), D5C7F8,
// exact derived defaults. Preserve130 and pool-owned trailing140 DWORD.
void* construct_native_gui_curve_default_00abe030(void* actual_curve,
    const volatile std::uint32_t& actual_one_00d7a24c);
// ABE100, ECX destination/stack source/EAX same/RET4. Real AA9520 first,
// then EXACT default derived fields: no source points, widths, UV or material
// copy. No derived acquisition can throw after the base returns normally.
void* construct_native_gui_curve_copy_00abe100(void* actual_destination,
    const void* actual_source, NativeGuiWidgetCopyContext&, NativeGuiWidgetCopyAcquired&,
    const volatile std::uint32_t& actual_one_00d7a24c);
// Genuine source AA1800 arms only. SAME initialized F8BE5C process pool,
// ABC790/ABC650 allocation and ABC410 return; no retained-factory admission.
void* create_native_gui_curve_default_00aa1800(const volatile std::uint32_t& actual_one);
void* create_native_gui_curve_copy_00aa1800(const void* actual_source,
    NativeGuiWidgetCopyContext&, NativeGuiWidgetCopyAcquired&,
    const volatile std::uint32_t& actual_one);

// ABC920 empties counted points then widths; preserves backing and capacities.
void clear_native_gui_curve_points_00abc920(void* actual_curve,
    NativeGuiCurveStorageAcquired&);
// ABC970, ECX raw/stack actual pair pointer and float-bits slot/RET8. Pair
// is read AFTER pair reserve via x87 FLD/FSTP; width is read AFTER width reserve
// by its unchanged DWORD bits. No early snapshot or alias repair. Caller keeps
// source/stack storage valid through the exact reached reserve operations.
// Existing86A350 body + canonical CRT binding and existing8153E0 are reused.
void append_native_gui_curve_point_00abc970(void* actual_curve,
    const void* actual_pair, const volatile std::uint32_t& actual_width_slot,
    NativeGuiCurveStorageAcquired&);
// ABC040: required real AA7170 returns, then direct AA6A30 writes byte85=1.
// Does NOT imply geometry74/7C, material or texture creation is available.
void load_native_gui_curve_00abc040(void* actual_curve, NativeGuiCurveLoadedCalls&,
    NativeGuiCurveStorageAcquired&);
// ABDE90: captured material+04 decrement/current zero, authored130, points10C,
// pooled string104, widthsF8, controlEC, then genuine AA9730. Exact native EH
// cleanup state ordering is projected for C++ throws; second unwind throw
// terminates. Counted array pointers/capacities and texture header remain as
// native destruction leaves them, rather than being sanitized after free.
void destroy_native_gui_curve_00abde90(void* actual_curve,
    NativeGuiCurveStorageContext&, NativeGuiCurveStorageAcquired&);
// ABE010 explicit scalar: no count decrement/test. Read CURRENT flags low byte
// AFTER ABDE90; same F8BE5C slot return iff bit0. No return on interruption.
void* delete_native_gui_curve_00abe010(void* actual_curve,
    const volatile std::uint32_t& actual_flags_slot,
    NativeGuiCurveStorageContext&, NativeGuiCurveStorageAcquired&);
} // namespace bsp
