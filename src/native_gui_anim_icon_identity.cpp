#include "bsp/native_gui_anim_icon_identity.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word profile(void* raw) noexcept { return *static_cast<const volatile Word*>(raw); }
void require_slot(Word captured, NativeGuiAnimIconIdentityContext& context,
    std::size_t index, Word expected) {
    const auto* const table = context.storage.anim_icon_profile_00d5bce0;
    if (captured != 0x00d5bce0 || !table || table[index] != expected)
        throw std::logic_error("AnimIcon identity requires its genuine selected current native slot");
}
std::atomic<std::int32_t>& count(void* raw, NativeGuiAnimIconIdentityContext& context) {
    auto& registry = context.storage.icon.actual_owners;
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4 || registry.find(raw))
        throw std::invalid_argument("AnimIcon identity requires aligned live unregistered actual storage");
    if (&context.actual_matching_pool != &game::GameNativeGuiWidgetPoolProcess<GuiAnimIconPoolTraits>::instance().pool())
        throw std::invalid_argument("AnimIcon identity requires its SAME initialized F8C1D4 pool");
    require_slot(profile(raw), context, 0, 0x00bd30e0);
    require_slot(profile(raw), context, 1, 0x00aa1d50);
    auto& references = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    if (references.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("AnimIcon identity requires its actual constructed positive count");
    return references;
}
class DeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    DeleteCalls(NativeGuiAnimIconReference& reference, NativeGuiAnimIconIdentityContext& context)
        : reference_(reference), context_(context) {}
    void delete_vslot04(void* raw, Word captured, Word flags) override {
        if (raw != reference_.actual_identity() || flags != 1)
            throw std::logic_error("AnimIcon zero terminal requires its SAME actual owner and flags1");
        require_slot(captured, context_, 1, 0x00aa1d50);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00aa1d50(actual_flags);
    }
private:
    NativeGuiAnimIconReference& reference_;
    NativeGuiAnimIconIdentityContext& context_;
};
} // namespace
NativeGuiAnimIconReference::NativeGuiAnimIconReference(void* raw, NativeGuiAnimIconIdentityContext& context)
    : RenderCommandReference(count(raw, context)), identity_(raw), context_(context) {
    context_.storage.icon.actual_owners.bind(identity_, *this);
}
NativeGuiAnimIconReference::~NativeGuiAnimIconReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGuiAnimIconReference::retired() const noexcept { return phase_ == Phase::retired; }
bool NativeGuiAnimIconReference::failed() const noexcept { return phase_ == Phase::failed; }
void NativeGuiAnimIconReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.storage.icon.actual_owners.unbind(identity_, *this); // Opaque key only, no returned payload read.
}
void* NativeGuiAnimIconReference::delete_scalar_00aa1d50(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("AnimIcon scalar deletion cannot replay entered or retired work");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_gui_anim_icon_00aa1d50(identity_, flags, context_.storage, destruction_); }
    catch (...) {
        phase_ = Phase::failed;
        destruction_.failed = true;
        if (!destruction_.failure) destruction_.failure = std::current_exception();
        // Preserve SAME canonical authority without reading possibly consumed
        // payload. The failed binding prevents replay, cleanup and slot reuse.
        throw;
    }
    retire();
    return result;
}
void NativeGuiAnimIconReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(profile(identity_), context_, 0, 0x00bd30e0);
        DeleteCalls calls(*this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
