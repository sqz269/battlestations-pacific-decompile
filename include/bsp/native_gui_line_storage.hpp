#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include <cstdint>

namespace bsp {
namespace gui_widgets_cc13 {
struct GuiTableReader;
struct GuiReaderHost;
}

// Exact actual EC..103 tail. EC is an opaque constructor-unwritten vector
// proxy, F0/F4/F8 are begin/end/capacity POINTERS, FC is committed geometry
// count, and100 is desired GPU capacity. The pool's104 DWORD is separate.
struct NativeGuiLineTailStorage {
    std::uint32_t proxy_ec;
    void* first_f0;
    void* last_f4;
    void* end_f8;
    std::int32_t committed_fc;
    std::int32_t capacity_100;
};
NativeGuiLineTailStorage& native_gui_line_tail(void* actual_line);

// Required genuine raw STL-vector transport. No std::vector layout cast,
// guessed proxy, no-op fallback or source allocation is synthesized here.
// Copy is0079EE20 over actual {proxy,first,last,end}; append is004222C0 over
// that SAME header and an actual two-DWORD source. Copy owns its own native
// vector-constructor unwind and must preserve EC. Backing must be compatible
// with the existing singleton_lifetime_free/CRT-free binding used by AC28D0.
class NativeGuiLineVectorCalls {
public:
    virtual ~NativeGuiLineVectorCalls() = default;
    virtual void copy_construct_0079ee20(void* actual_destination_ec,
        const void* actual_source_ec) = 0;
    virtual void append_004222c0(void* actual_vector_ec, const void* actual_pair) = 0;
};
// Qualified stock substitution, not reconstructed game/vendor-library credit.
// Valid actual headers only, with backing from the SAME existing CRT allocation
// binding. Preserves EC; copies two floats via native FLD/FSTP (including current
// x87 exceptions), exact-size copy, and native 1.5x append growth. No projected
// std::vector, iterator proxy, alternate allocator or host point collection.
class NativeGuiLineStockVectorCalls final : public NativeGuiLineVectorCalls {
public:
    void copy_construct_0079ee20(void* actual_destination_ec,
        const void* actual_source_ec) override;
    void append_004222c0(void* actual_vector_ec, const void* actual_pair) override;
};
struct NativeGuiLineCopyContext {
    NativeGuiWidgetCopyContext& base;
    NativeGuiWidgetLifetimeContext& lifetime;
    NativeGuiLineVectorCalls& vectors;
};
struct NativeGuiLineCopyAcquired {
    NativeGuiWidgetCopyAcquired base;
    bool base_returned{}, vector_entered{}, complete{}, base_cleanup_entered{};
};

// AC2D00, ECX actual104h, EAX same, RET. Full existing AA9390(type4) then
// D5C9E8/F0/F4/F8/FC=0/100=4. Preserve EC, all base preimages and pool+104.
void* construct_native_gui_line_default_00ac2d00(void* actual_line,
    const volatile std::uint32_t& actual_one_00d7a24c);
// AC2D60, ECX destination, stack source, EAX same, RET4. Real AA9520 first,
// stamp D5C9E8, then required genuine vector copy. FC/100 remain unwritten.
// Failed vector construction consumes base cleanup; its own vector cleanup
// belongs to that required transport. Preserve failed clone/provider frames.
void* construct_native_gui_line_copy_00ac2d60(void* actual_destination,
    const void* actual_source, NativeGuiLineCopyContext&, NativeGuiLineCopyAcquired&);
// Complete null/non-null arms of AA1530, split to require only reached inputs.
// Original ECX optional source, EAX actual108h pool slot or null, RET. Require
// explicit GuiLinePoolTraits process startup; failed constructor returns only
// its captured failed slot, after that constructor's own cleanup.
void* create_native_gui_line_default_00aa1530(
    const volatile std::uint32_t& actual_one_00d7a24c);
void* create_native_gui_line_copy_00aa1530(const void* actual_source,
    NativeGuiLineCopyContext&, NativeGuiLineCopyAcquired&);

// AC28D0 ECX actual, RET: stamp Line, free CURRENT F0, clear F0/F4/F8,
// then genuine AA9730. Preserve EC/FC/100. No physical widget-slot return.
void destroy_native_gui_line_00ac28d0(void* actual_line, NativeGuiWidgetLifetimeContext&);
// AC2CA0 ECX actual, stack flags, EAX saved identity, RET4. Same full direct
// destructor, then CURRENT flags low byte; return same Line slot iff bit0.
// Explicit scalar deletion neither decrements nor tests actual+04.
void* delete_native_gui_line_00ac2ca0(void* actual_line,
    const volatile std::uint32_t& actual_flags_slot, NativeGuiWidgetLifetimeContext&);
// AC2920 current0C: CURRENT F8BF34/38/3C in order, AL result, RET4.
bool native_gui_line_is_kind_of_00ac2920(std::uint32_t,
    const volatile std::uint32_t (&actual_lineage_00f8bf34)[3]) noexcept;
// AC2DC0 ECX Line, two stacked float DWORDs, RET8. New explicit pointer API
// forwards the SAME pair to actual vectorEC, with no arithmetic/float copy.
void append_native_gui_line_point_00ac2dc0(void* actual_line,
    const void* actual_pair, NativeGuiLineVectorCalls&);
// AC2DE0 ECX actual Line, stack actual visitor, RET4. Existing visitor source
// ABI performs base AAA710 then Points. Required host supplies real base and
// raw004222C0 transport; no projected vector is created.
void read_native_gui_line_properties_00ac2de0(void* actual_line,
    gui_widgets_cc13::GuiTableReader&, gui_widgets_cc13::GuiReaderHost&);
// Derived tail only, after the SAME caller's completed base/property stage.
void read_native_gui_line_points_after_base_00ac2de0(void* actual_line,
    gui_widgets_cc13::GuiTableReader&, NativeGuiLineVectorCalls&);
} // namespace bsp
