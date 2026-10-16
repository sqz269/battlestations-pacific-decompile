#include "bsp/native_gui_frame_box_lifetime.hpp"
#include "bsp/native_gui_frame_box_records.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <array>
#include <cstddef>
#include <exception>
#include <limits>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI FrameBox lifetime requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
namespace {
using Word = std::uint32_t;
struct Region { std::uintptr_t first, last; };
using KnownRegions = std::array<Region, 15>;

Region region(const volatile void* identity, std::size_t bytes) {
    const auto first = reinterpret_cast<std::uintptr_t>(identity);
    if ((bytes != 0 && first == 0) ||
        bytes > (std::numeric_limits<std::uintptr_t>::max)() - first)
        throw std::invalid_argument("raw FrameBox lifetime region is null or wraps");
    return {first, first + bytes};
}
bool overlaps(Region left, Region right) noexcept {
    return left.first != left.last && right.first != right.last &&
        left.first < right.last && right.first < left.last;
}
bool contains(Region value, std::uintptr_t point) noexcept {
    return value.first <= point && point < value.last;
}
void validate_extra(const KnownRegions& known, Region extra) {
    for (const auto value : known) {
        if (overlaps(value, extra))
            throw std::invalid_argument("raw FrameBox lifetime known regions overlap");
    }
    // Even an empty nonnull constructed range reaches CURRENT begin/free.
    if (known[1].first != 0 && contains(extra, known[1].first))
        throw std::invalid_argument("raw FrameBox lifetime backing aliases metadata");
}
KnownRegions validate_common(void* owner, NativeGuiFrameBoxLifetimeContext& c) {
    const auto payload = region(owner, 0x11c);
    const auto context = region(&c, sizeof(c));
    if ((payload.first & 3u) != 0 || (context.first & 3u) != 0 || overlaps(payload, context))
        throw std::invalid_argument("raw FrameBox requires aligned disjoint actual storage");
    const auto operation = region(&c.vector, sizeof(c.vector));
    if ((operation.first & 3u) != 0 || overlaps(payload, operation) || overlaps(context, operation))
        throw std::invalid_argument("raw FrameBox aliases its prepared operation");
    if (c.vector.phase() != NativeGuiFrameBoxVectorTidyPhase::fresh ||
        reinterpret_cast<std::uintptr_t>(c.vector.header_identity()) != payload.first + 0xf0u)
        throw std::logic_error("raw FrameBox requires its fresh exact-header operation");
    const auto first = reinterpret_cast<std::uintptr_t>(c.vector.expected_first_identity());
    const auto last = reinterpret_cast<std::uintptr_t>(c.vector.expected_end_identity());
    if ((first & 3u) != 0 || (last & 3u) != 0 || last < first ||
        (first == 0 && last != 0) || (last - first) % 0x2cu != 0 ||
        (last - first) / 0x2cu != c.vector.prepared_record_count())
        throw std::invalid_argument("raw FrameBox requires an exact constructed record span");
    const KnownRegions known{{payload, region(c.vector.expected_first_identity(), last - first),
        context, operation, region(&c.widgets, sizeof(c.widgets)),
        region(&c.strings, sizeof(c.strings)), region(&c.owners, sizeof(c.owners)),
        region(&c.strings.actual_published_01090aa8, sizeof(void*)),
        region(&c.strings.actual_small_returns_disabled_01090aa4, sizeof(Word)),
        region(&c.strings.actual_manager_publication_01090aa0, sizeof(void*)),
        region(&c.widgets.nodes, sizeof(c.widgets.nodes)),
        region(&c.widgets.bindings, sizeof(c.widgets.bindings)),
        region(c.widgets.actual_base_table_00d5c130, 9u * sizeof(Word)),
        region(&c.widgets.actual_base_lineage_00f8bc88, 2u * sizeof(Word)),
        region(&c.widgets.actual_text_descriptor_00f8be28, sizeof(Word))}};
    for (std::size_t i = 0; i != known.size(); ++i) {
        if ((known[i].first & 3u) != 0)
            throw std::invalid_argument("raw FrameBox lifetime known region is misaligned");
        for (std::size_t j = i + 1; j != known.size(); ++j) {
            if (overlaps(known[i], known[j]))
                throw std::invalid_argument("raw FrameBox lifetime known regions overlap");
        }
        if (i != 1 && first != 0 && contains(known[i], first))
            throw std::invalid_argument("raw FrameBox backing aliases metadata or payload");
    }
    return known;
}
void* at(void* owner, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
volatile Word& word(void* owner, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(owner, offset));
}
void unwind_frame_box(void* owner, int captured_state,
    NativeGuiFrameBoxLifetimeContext& c) noexcept {
    volatile int state = captured_state;
    try {
        if (state == 1) {
            state = 0; // Consume BEFORE the genuine119 cleanup entry.
            cleanup_native_gui_frame_box_vector_00ad2320(at(owner, 0xf0),
                c.owners, c.strings, c.vector);
        }
        if (state == 0) {
            state = -1; // Consume BEFORE the genuine base.
            destroy_native_gui_widget_base_00aa9730(owner, c.widgets);
        }
    } catch (...) {
        std::terminate();
    }
}
__declspec(noinline) void destroy_frame_box_body(void* owner,
    NativeGuiFrameBoxLifetimeContext& c) {
    word(owner) = 0x00d5d130u;
    void* const data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(word(owner, 0x108)));
    volatile int state = 1; // Native DATA capture precedes state1.
    try {
        if (data != nullptr) {
            const Word size = word(owner, 0x104) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.strings.actual_published_01090aa8,
                c.strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, data, size,
                c.strings.actual_small_returns_disabled_01090aa4);
        }
        state = 0; // Consume BEFORE normal119: failure does base ONLY.
        tidy_native_gui_frame_box_vector_00ad21b0(at(owner, 0xf0),
            c.owners, c.strings, c.vector);
        state = -1; // Consume BEFORE base: no repeated outer cleanup.
        destroy_native_gui_widget_base_00aa9730(owner, c.widgets);
    } catch (...) {
        unwind_frame_box(owner, state, c);
        throw;
    }
}
} // namespace

void destroy_native_gui_frame_box_00ad2880(void* owner,
    NativeGuiFrameBoxLifetimeContext& c) {
    (void)validate_common(owner, c);
    destroy_frame_box_body(owner, c);
}
void* scalar_delete_native_gui_frame_box_00ad29b0(void* owner,
    const volatile Word& flags, NativeGuiFrameBoxLifetimeContext& c,
    NativeGuiWidgetPool<GuiFrameBoxPoolTraits>& pool) {
    const auto known = validate_common(owner, c);
    const auto flags_region = region(&flags, sizeof(flags));
    const auto pool_metadata = region(&pool, sizeof(pool)); // No pool state read.
    if ((flags_region.first & 3u) != 0 || overlaps(flags_region, pool_metadata))
        throw std::invalid_argument("raw FrameBox flags/pool metadata must be disjoint");
    validate_extra(known, flags_region);
    validate_extra(known, pool_metadata);
    destroy_frame_box_body(owner, c);
    const auto low_flags = *reinterpret_cast<const volatile unsigned char*>(&flags);
    if ((low_flags & 1u) != 0)
        pool.free(owner);
    return owner;
}
} // namespace bsp
