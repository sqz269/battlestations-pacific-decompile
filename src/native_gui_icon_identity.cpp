#include "bsp/native_gui_icon_identity.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word profile(void* raw) noexcept { return *static_cast<const volatile Word*>(raw); }
void require_slot(Word captured, NativeGuiIconIdentityContext& context,
    std::size_t index, Word expected) {
    const auto* const table = context.storage.icon_profile_00d5c4c0;
    if (captured != 0x00d5c4c0 || !table || table[index] != expected)
        throw std::logic_error("Icon identity requires its genuine selected current native slot");
}
std::atomic<std::int32_t>& count(void* raw, NativeGuiIconIdentityContext& context) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4 || context.storage.actual_owners.find(raw))
        throw std::invalid_argument("Icon identity requires aligned live unregistered actual storage");
    if (&context.actual_matching_pool != &game::GameNativeGuiWidgetPoolProcess<GuiIconPoolTraits>::instance().pool())
        throw std::invalid_argument("Icon identity requires the same initialized F8BDA0 process pool");
    require_slot(profile(raw), context, 0, 0x00bd30e0);
    require_slot(profile(raw), context, 1, 0x00ab6100);
    auto& references = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    if (references.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Icon identity requires its actual constructed positive count");
    return references;
}
class DeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    DeleteCalls(NativeGuiIconReference& reference, NativeGuiIconIdentityContext& context)
        : reference_(reference), context_(context) {}
    void delete_vslot04(void* raw, Word captured, Word flags) override {
        if (raw != reference_.actual_identity() || flags != 1)
            throw std::logic_error("Icon zero terminal requires its same actual owner and flags1");
        require_slot(captured, context_, 1, 0x00ab6100);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00ab6100(actual_flags);
    }
private:
    NativeGuiIconReference& reference_;
    NativeGuiIconIdentityContext& context_;
};
} // namespace
NativeGuiIconReference::NativeGuiIconReference(void* raw, NativeGuiIconIdentityContext& context)
    : RenderCommandReference(count(raw, context)), identity_(raw), context_(context) {
    context_.storage.actual_owners.bind(identity_, *this);
}
NativeGuiIconReference::~NativeGuiIconReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGuiIconReference::retired() const noexcept { return phase_ == Phase::retired; }
bool NativeGuiIconReference::failed() const noexcept { return phase_ == Phase::failed; }
void NativeGuiIconReference::retire() noexcept {
    phase_ = Phase::retired;
    // Opaque key only; native payload may already be in the returned pool slot.
    context_.storage.actual_owners.unbind(identity_, *this);
}
void NativeGuiIconReference::prepare_cold_delete_call() {
    const bool fresh = phase_ == Phase::bound && !cold_delete_preparation_entered_;
    cold_delete_preparation_entered_ = true; // Before even late Source admission.
    try {
        if (!fresh)
            throw std::logic_error("Icon cold deletion preparation requires its retained fresh bound reference");
        auto& prepared = context_.storage.delayed.prepare_icon_cancel_call(
            identity_, context_.storage, destruction_);
        // Preparation owns no Native work. A nested Source denial cannot be
        // swallowed into a successful preparation/pointer publication.
        if (phase_ != Phase::bound)
            throw std::logic_error("Icon cold deletion preparation lost its exclusive Source scope");
        prepared_delete_call_ = &prepared; // Publish only after normal preparation.
    } catch (...) {
        phase_ = Phase::failed; // Source denial only; leave destruction_ untouched.
        throw; // Caller retains the already constructed graph; no cleanup/retry.
    }
}
void* NativeGuiIconReference::delete_scalar_00ab6100(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("Icon scalar deletion cannot replay entered or retired work");
    if (cold_delete_preparation_entered_ && !prepared_delete_call_) {
        phase_ = Phase::failed; // Reject an in-progress/failed explicit cold route.
        throw std::logic_error("Icon scalar deletion requires its completed cold preparation");
    }
    phase_ = Phase::destroying;
    void* result;
    try {
        if (prepared_delete_call_)
            result = delete_native_gui_icon_prepared_00ab6100(
                identity_, flags, context_.storage, destruction_, *prepared_delete_call_);
        else
            result = delete_native_gui_icon_00ab6100(identity_, flags, context_.storage, destruction_);
    }
    catch (...) {
        phase_ = Phase::failed;
        destruction_.failed = true;
        if (!destruction_.failure) destruction_.failure = std::current_exception();
        // Keep SAME canonical authority without rereading possibly consumed
        // native storage. The failed binding blocks cleanup, replay and reuse.
        throw;
    }
    retire();
    return result;
}
void NativeGuiIconReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(profile(identity_), context_, 0, 0x00bd30e0);
        DeleteCalls calls(*this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
