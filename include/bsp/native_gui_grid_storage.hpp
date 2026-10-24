#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Same actual14h column: width, UNWRITTEN proxy, pointer begin/end/capacity.
// Cell pointers are copied as raw DWORDs, with NO retain, clone or tree repair.
struct NativeGuiGridColumnStorage {
    float width_00;
    std::uint32_t proxy_04;
    void* first_08;
    void* last_0c;
    void* end_10;
};
struct NativeGuiGridTailStorage {
    std::uint32_t proxy_ec;
    NativeGuiGridColumnStorage* first_f0;
    NativeGuiGridColumnStorage* last_f4;
    NativeGuiGridColumnStorage* end_f8;
    std::int32_t columns_fc;
    std::int32_t rows_100;
    float cell_width_104, cell_height_108;
    std::uint8_t flag_10c;
    std::uint8_t unwritten_10d[3];
    float scroll_x_110, scroll_y_114;
    void* visual_group_118;
    float content_width_11c, content_height_120;
};
NativeGuiGridTailStorage& native_gui_grid_tail(void* actual_grid);

class NativeGuiGridLookupCalls {
public:
    virtual ~NativeGuiGridLookupCalls() = default;
    // Genuine raw AA7E00 over SAME actual direct-child list and node names.
    // Second stack DWORD1 is UNUSED, not recursion/factory permission. Return
    // an actual child or null; no logical-owner cast or successful fake lookup.
    virtual void* find_child_00aa7e00(void* actual_grid,
        const void* actual_name_header, std::uint32_t unused_one) = 0;
};
struct NativeGuiGridContext {
    NativeGuiWidgetLifetimeContext& lifetime;
    NativeStringRawPoolContext& strings;
    NativeRenderActualOwnerRegistry& actual_owners; // SAME child +04 companions
    NativeGuiGridLookupCalls& lookup;
};
struct NativeGuiGridColumnCopyAcquired {
    bool started{}, complete{}, range_cleanup_entered{}, outer_cleanup_entered{};
    void* outer_allocation{};
    void* current_column{};
    void* completed_prefix_end{};
    void* current_inner_allocation{}; // diagnostics only, never another owner
};
struct NativeGuiGridCopyContext {
    NativeGuiWidgetCopyContext& base;
    NativeGuiGridContext& grid;
};
struct NativeGuiGridCopyAcquired {
    NativeGuiWidgetCopyAcquired base;
    NativeGuiGridColumnCopyAcquired columns;
    bool base_returned{}, columns_returned{}, complete{};
    bool string_cleanup_entered{}, columns_cleanup_entered{}, base_cleanup_entered{};
    std::int32_t native_eh_state{-1};
};
// Fresh external record before entry. No native ownership/credits. Interrupted
// work is sticky: preserve SAME raw graph/providers and record; do not retry or
// drain dependencies. Destroying entered/failed diagnostics terminates.
struct NativeGuiGridRetirementAcquired {
    enum class Phase { fresh, entered, complete, failed };
    Phase phase{Phase::fresh};
    bool unsupported_growth{}, columns_cleanup_entered{}, base_cleanup_entered{};
    bool base_entered{}, slot_returned{};
    std::int32_t column_index{};
    void* active_child{};
    std::uint32_t child_target{};
    bool child_call_entered{}, child_call_returned{};
    NativeGuiGridRetirementAcquired() = default;
    ~NativeGuiGridRetirementAcquired() noexcept;
    NativeGuiGridRetirementAcquired(const NativeGuiGridRetirementAcquired&) = delete;
    NativeGuiGridRetirementAcquired& operator=(const NativeGuiGridRetirementAcquired&) = delete;
};

// ACB290 exact-size actual column-vector copy. Qualified stock substitution:
// x87 width FLD/FSTP, real nested pointer backings, proxies untouched, no child
// retain; partial-range then current outer tidy follows original catch order.
// Uses the SAME singleton_lifetime CRT allocator/free, no vendor-body credit.
void* copy_native_gui_grid_columns_00acb290(void* actual_destination_ec,
    const void* actual_source_ec, NativeGuiGridColumnCopyAcquired&);
// ACA730 ECX first/EDX end, two unused stack DWORDs, RET8. Free CURRENT inner
// first, clear08/0C/10 per14h record; preserve width/proxy and child payloads.
void destroy_native_gui_grid_columns_00aca730(void* actual_first, void* actual_end);

// ACB0A0 raw AA9390(type8), D5CE60, native tail writes. Preserve EC/10D..10F.
void* construct_native_gui_grid_default_00acb0a0(void* actual_grid,
    const volatile std::uint32_t& actual_one_00d7a24c);
// ACB3F0 raw AA9520, columns, x87 tail copies and actual Visual_Group lookup.
// FC/100 remain UNWRITTEN, as in source; no repaired count/child ownership.
void* construct_native_gui_grid_copy_00acb3f0(void* actual_destination,
    const void* actual_source, NativeGuiGridCopyContext&, NativeGuiGridCopyAcquired&);
void* create_native_gui_grid_default_00aa1650(const volatile std::uint32_t& actual_one);
void* create_native_gui_grid_copy_00aa1650(const void* actual_source,
    NativeGuiGridCopyContext&, NativeGuiGridCopyAcquired&);

// Explicit supported-domain caller preflight, before native entry: valid actual
// outer/inner ranges, nonnegative FC <= actual column size. Does not alter or
// initialize unwritten fields, discover aliases, or authorize scalar dispatch.
void preflight_native_gui_grid_clear_zero(void* actual_grid);
// ONLY ACB9B0 argument0 shrink/clear path. Current child04(flags1), current
// bounds/rereads, then concrete zero erase. If callbacks change FC to nonzero,
// reject before unimplemented resize/growth and retain reached effects. This
// is NOT the full nonzero count setter/current84/growing-row implementation.
void clear_native_gui_grid_zero_00acb9b0(void* actual_grid,
    NativeGuiGridContext&, NativeGuiGridRetirementAcquired&);
// ACBBF0 supported count0 domain; columns tidy, then genuine AA9730. Ordinary
// derived failure consumes native columns/base unwind. Unsupported reentry
// retains the partially reached graph instead of fabricating native growth.
void destroy_native_gui_grid_00acbbf0(void* actual_grid,
    NativeGuiGridContext&, NativeGuiGridRetirementAcquired&);
// ACBC90 explicit scalar; CURRENT flags low byte after dtor; SAME F8C050 slot
// return iff bit0. No count decrement/test. No slot return on interruption.
void* delete_native_gui_grid_00acbc90(void* actual_grid,
    const volatile std::uint32_t& actual_flags_slot,
    NativeGuiGridContext&, NativeGuiGridRetirementAcquired&);
} // namespace bsp
