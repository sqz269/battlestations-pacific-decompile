#include "bsp/native_gui_clip_box_identity.hpp"
#include "bsp/native_gui_clip_box_factory.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word current_profile(void* identity) noexcept {
    return *static_cast<const volatile Word*>(identity);
}
void require_slot(Word profile, NativeGuiClipBoxIdentityContext& c,
    std::size_t index, Word expected) {
    if (profile != 0x00d5d058u || !c.clip_box_profile_00d5d058 ||
        c.clip_box_profile_00d5d058[index] != expected)
        throw std::logic_error("ClipBox identity has no binding for the current native slot");
}
std::atomic<std::int32_t>& binding_count(void* identity, NativeGuiClipBoxIdentityContext& c) {
    if (!identity || reinterpret_cast<std::uintptr_t>(identity) % 4 || c.registry.find(identity))
        throw std::invalid_argument("ClipBox identity requires aligned unregistered actual storage");
    if (&c.actual_matching_pool !=
        &game::GameNativeGuiWidgetPoolProcess<GuiClipBoxPoolTraits>::instance().pool())
        throw std::invalid_argument("ClipBox identity requires its same initialized process pool");
    require_slot(current_profile(identity), c, 0, 0x00bd30e0u);
    require_slot(current_profile(identity), c, 1, 0x00ace620u);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("ClipBox identity requires a completed live actual count");
    return count;
}
class ClipBoxDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    ClipBoxDeleteCalls(void* identity, NativeGuiClipBoxReference& reference,
        NativeGuiClipBoxIdentityContext& context)
        : identity_(identity), reference_(reference), context_(context) {}
    void delete_vslot04(void* identity, Word captured_profile, Word flags) override {
        if (identity != identity_ || flags != 1)
            throw std::logic_error("ClipBox terminal requires its same actual owner and flags1");
        require_slot(captured_profile, context_, 1, 0x00ace620u);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00ace620(actual_flags);
    }
private:
    void* const identity_;
    NativeGuiClipBoxReference& reference_;
    NativeGuiClipBoxIdentityContext& context_;
};
} // namespace

NativeGuiClipBoxReference::NativeGuiClipBoxReference(void* identity,
    NativeGuiClipBoxIdentityContext& context)
    : RenderCommandReference(binding_count(identity, context)), identity_(identity), context_(context) {
    context_.registry.bind(identity_, *this);
}
NativeGuiClipBoxReference::~NativeGuiClipBoxReference() {
    if (phase_ != Phase::retired) std::terminate();
    // Borrowed RenderCommandReference destruction never reads the ended atomic.
}
bool NativeGuiClipBoxReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeGuiClipBoxReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.registry.unbind(identity_, *this);
}
void* NativeGuiClipBoxReference::delete_scalar_00ace620(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("ClipBox scalar deletion requires its live canonical companion");
    phase_ = Phase::destroying;
    void* result;
    try {
        result = delete_native_gui_clip_box_00ace620(identity_, flags, context_.lifetime);
    } catch (...) {
        retire();
        throw;
    }
    retire();
    return result;
}
void NativeGuiClipBoxReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(current_profile(identity_), context_, 0, 0x00bd30e0u);
        ClipBoxDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
