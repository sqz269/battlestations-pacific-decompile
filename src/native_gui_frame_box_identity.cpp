#include "bsp/native_gui_frame_box_identity.hpp"
#include "bsp/native_gui_frame_box_records.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_ref_counted.hpp"
#include <array>
#include <cstddef>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI FrameBox identity requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeGuiFrameBoxIdentityPhase;
struct Region { std::uintptr_t first, last; };

Region region(const volatile void* identity, std::size_t bytes) {
    const auto first = reinterpret_cast<std::uintptr_t>(identity);
    if (first == 0 || (first & 3u) != 0 ||
        bytes > (std::numeric_limits<std::uintptr_t>::max)() - first)
        throw std::invalid_argument("FrameBox identity requires aligned nonwrapping live regions");
    return {first, first + bytes};
}
bool overlaps(Region left, Region right) noexcept {
    return left.first < right.last && right.first < left.last;
}
Word current_profile(void* identity) noexcept {
    return *static_cast<const volatile Word*>(identity);
}
void require_slot(Word profile, NativeGuiFrameBoxIdentityContext& context,
    std::size_t index, Word expected) {
    if (profile != 0x00d5d130u || !context.actual_profile_00d5d130 ||
        context.actual_profile_00d5d130[index] != expected)
        throw std::logic_error("FrameBox identity has no binding for the current native slot");
}

__declspec(noinline) std::atomic<std::int32_t>& binding_count(void* identity,
    NativeGuiFrameBoxIdentityContext& context, const void* metadata) {
    // Evaluate this first base-initializer argument before constructing any
    // companion members. This does not legalize invalid C++ object placement.
    const std::array<Region, 8> known{{region(identity, 0x120),
        region(metadata, sizeof(NativeGuiFrameBoxReference)),
        region(&context, sizeof(context)), region(&context.lifetime, sizeof(context.lifetime)),
        region(&context.registry, sizeof(context.registry)),
        region(&context.actual_matching_pool, sizeof(context.actual_matching_pool)),
        region(&context.lifetime.vector, sizeof(context.lifetime.vector)),
        region(context.actual_profile_00d5d130, 2u * sizeof(Word))}};
    for (std::size_t i = 0; i != known.size(); ++i)
        for (std::size_t j = i + 1; j != known.size(); ++j)
            if (overlaps(known[i], known[j]))
                throw std::invalid_argument("FrameBox identity known regions overlap");
    if (&context.lifetime.owners != static_cast<NativeRenderActualOwners*>(&context.registry))
        throw std::invalid_argument("FrameBox identity requires its same canonical render registry");
    if (context.lifetime.vector.phase() != NativeGuiFrameBoxVectorTidyPhase::fresh ||
        context.lifetime.vector.header_identity() != static_cast<std::byte*>(identity) + 0xf0)
        throw std::invalid_argument("FrameBox identity requires its fresh exact-header119 preparation");
    require_slot(current_profile(identity), context, 0, 0x00bd30e0u);
    require_slot(current_profile(identity), context, 1, 0x00ad29b0u);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("FrameBox identity requires a positive live actual count");
    return count;
}

class FrameBoxDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    FrameBoxDeleteCalls(void* identity, NativeGuiFrameBoxReference& reference,
        NativeGuiFrameBoxIdentityContext& context)
        : identity_(identity), reference_(reference), context_(context) {}
    void delete_vslot04(void* identity, Word captured_profile, Word flags) override {
        if (identity != identity_ || flags != 1)
            throw std::logic_error("FrameBox terminal requires its same actual owner and flags1");
        // Genuine BD30E0 freshly loaded this profile after our slot00 check.
        require_slot(captured_profile, context_, 1, 0x00ad29b0u);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00ad29b0(actual_flags);
    }
private:
    void* const identity_;
    NativeGuiFrameBoxReference& reference_;
    NativeGuiFrameBoxIdentityContext& context_;
};
} // namespace

NativeGuiFrameBoxReference::NativeGuiFrameBoxReference(void* identity,
    NativeGuiFrameBoxIdentityContext& context)
    : RenderCommandReference(binding_count(identity, context, this)),
      identity_(identity), context_(context) {
    context_.registry.bind(identity_, *this);
}
NativeGuiFrameBoxReference::~NativeGuiFrameBoxReference() {
    if (!retired()) std::terminate();
    // The borrowed RenderCommandReference destructor never reads its atomic.
}
bool NativeGuiFrameBoxReference::retired() const noexcept {
    return phase_ == Phase::retired || phase_ == Phase::retired_after_failure;
}
void NativeGuiFrameBoxReference::retire(Phase phase) noexcept {
    phase_ = phase;
    context_.registry.unbind(identity_, *this); // Metadata only, no raw read.
}
void* NativeGuiFrameBoxReference::delete_scalar_00ad29b0(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("FrameBox scalar deletion requires its live canonical companion");
    phase_ = Phase::destroying;
    void* result;
    try {
        result = scalar_delete_native_gui_frame_box_00ad29b0(identity_, flags,
            context_.lifetime, context_.actual_matching_pool);
    } catch (...) {
        retire(Phase::retired_after_failure);
        throw;
    }
    retire(Phase::retired);
    return result; // Numeric identity only; native slot may already be returned.
}
void NativeGuiFrameBoxReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(current_profile(identity_), context_, 0, 0x00bd30e0u);
        FrameBoxDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) {
        std::terminate();
    }
}
} // namespace bsp
