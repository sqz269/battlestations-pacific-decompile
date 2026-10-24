#include "bsp/native_gui_line_identity.hpp"
#include "bsp/native_gui_page_construction.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word profile(void* raw) noexcept { return *static_cast<const volatile Word*>(raw); }
void require_slot(Word captured, NativeGuiLineIdentityContext& context,
    std::size_t index, Word expected) {
    if (captured != 0x00d5c9e8 || !context.line_profile_00d5c9e8 ||
        context.line_profile_00d5c9e8[index] != expected)
        throw std::logic_error("Line identity has no genuine current native slot binding");
}
std::atomic<std::int32_t>& count(void* raw, NativeGuiLineIdentityContext& context) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4 || context.registry.find(raw))
        throw std::invalid_argument("Line identity requires aligned live unregistered actual storage");
    require_slot(profile(raw), context, 0, 0x00bd30e0);
    require_slot(profile(raw), context, 1, 0x00ac2ca0);
    auto& references = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    if (references.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Line identity requires its actual constructed positive count");
    return references;
}
class DeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    DeleteCalls(NativeGuiLineReference& owner, NativeGuiLineIdentityContext& context)
        : owner_(owner), context_(context) {}
    void delete_vslot04(void* actual, Word captured, Word flags) override {
        if (actual != owner_.actual_identity() || flags != 1)
            throw std::logic_error("Line zero terminal requires its same actual owner and flags1");
        require_slot(captured, context_, 1, 0x00ac2ca0);
        const volatile Word caller_flags = flags;
        owner_.delete_scalar_00ac2ca0(caller_flags);
    }
private:
    NativeGuiLineReference& owner_;
    NativeGuiLineIdentityContext& context_;
};
} // namespace
NativeGuiLineReference::NativeGuiLineReference(void* raw, NativeGuiLineIdentityContext& context)
    : RenderCommandReference(count(raw, context)), identity_(raw), context_(context) {
    context_.registry.bind(identity_, *this);
}
NativeGuiLineReference::~NativeGuiLineReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGuiLineReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeGuiLineReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.registry.unbind(identity_, *this); // opaque key only, no post-pool payload read
}
void* NativeGuiLineReference::delete_scalar_00ac2ca0(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("Line scalar deletion cannot replay entered or retired work");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_gui_line_00ac2ca0(identity_, flags, context_.lifetime); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeGuiLineReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(profile(identity_), context_, 0, 0x00bd30e0);
        DeleteCalls calls(*this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
void NativeGuiLineReference::bind_same_model_00aa6720(GuiWidgetOwner& widget) {
    if (phase_ != Phase::bound || profile(identity_) != 0x00d5c9e8 ||
        reference_count.load(std::memory_order_relaxed) <= 0 ||
        widget.layout().type != GuiWidgetType::Line ||
        widget.layout().transform.type_id != 4 || &widget.runtime().owner(widget.layout()) != &widget)
        throw std::invalid_argument("Line Model binding requires its canonical type4 owner");
    auto* node = widget.node_binding();
    auto* model = widget.model_reference();
    if (!node || !model || &model->model_owner().node != node ||
        model->model_owner().phase != NativeModelOwner::Phase::live ||
        model->reference_count.load(std::memory_order_relaxed) <= 0 ||
        &model->model_owner().environment.nodes != &context_.lifetime.nodes ||
        &model->model_owner().environment.retained_owners != &context_.registry ||
        &context_.registry.resolve_actual(&node->storage) != model)
        throw std::invalid_argument("Line must bind the same live actual renderer Model domain");
    bind_native_gui_widget_node_00aa6720(identity_, &node->storage);
}
} // namespace bsp
