#include "bsp/native_render_context.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <mutex>
#include <new>
#include <stdexcept>
#include <unordered_map>

namespace bsp {
namespace {
constexpr std::uint32_t reference_table = 0x00ceb130;
constexpr std::uint32_t context_table = 0x00d5e5c4;

class ContextBaseCleanup final {
public:
    explicit ContextBaseCleanup(NativeRenderContextStorage& storage) noexcept : storage_(storage) {}
    ~ContextBaseCleanup() { storage_.native_vtable_00 = reference_table; }
private:
    NativeRenderContextStorage& storage_;
};

void release_then_clear(void*& slot, NativeRenderActualOwners& owners) {
    void* const captured = slot;
    if (captured) {
        release_native_render_actual_owner(owners, captured);
        slot = nullptr;
    }
}
} // namespace

struct NativeRenderActualOwnerRegistry::Impl final {
    mutable std::mutex mutex;
    std::unordered_map<void*, RenderCommandReference*> entries;
};

NativeRenderActualOwnerRegistry::NativeRenderActualOwnerRegistry()
    : impl_(std::make_unique<Impl>()) {}

NativeRenderActualOwnerRegistry::~NativeRenderActualOwnerRegistry() {
    if (!empty()) std::terminate();
}

void NativeRenderActualOwnerRegistry::bind(void* identity,
    RenderCommandReference& reference) {
    if (!identity || reinterpret_cast<std::uintptr_t>(identity) % alignof(std::uint32_t) != 0)
        throw std::invalid_argument("native owner registry requires an aligned actual identity");
    auto* const actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    if (&reference.reference_count != actual ||
        actual->load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument(
            "native owner registry companion must borrow the live actual +04 atomic");
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->entries.emplace(identity, &reference).second)
        throw std::logic_error("native owner identity already has a canonical companion");
}

void NativeRenderActualOwnerRegistry::unbind(void* identity,
    RenderCommandReference& reference) noexcept {
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    const auto found = impl_->entries.find(identity);
    if (found == impl_->entries.end() || found->second != &reference)
        std::terminate();
    impl_->entries.erase(found);
}

RenderCommandReference* NativeRenderActualOwnerRegistry::find(void* identity) {
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    const auto found = impl_->entries.find(identity);
    return found == impl_->entries.end() ? nullptr : found->second;
}

RenderCommandReference& NativeRenderActualOwnerRegistry::resolve_actual(void* identity) {
    auto* const found = find(identity);
    if (!found)
        throw std::logic_error("actual render owner has no canonical companion");
    return *found;
}

std::size_t NativeRenderActualOwnerRegistry::size() const {
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->entries.size();
}

bool NativeRenderActualOwnerRegistry::empty() const {
    return size() == 0;
}

void NativeRenderActualOwnerRegistry::bind_callback(void* context, void* identity,
    RenderCommandReference& reference) {
    if (!context) throw std::invalid_argument("native owner registry context is null");
    static_cast<NativeRenderActualOwnerRegistry*>(context)->bind(identity, reference);
}

void NativeRenderActualOwnerRegistry::unbind_callback(void* context, void* identity,
    RenderCommandReference& reference) noexcept {
    if (!context) std::terminate();
    static_cast<NativeRenderActualOwnerRegistry*>(context)->unbind(identity, reference);
}

RenderCommandReference* NativeRenderActualOwnerRegistry::find_callback(
    void* context, void* identity) {
    if (!context) throw std::invalid_argument("native owner registry context is null");
    return static_cast<NativeRenderActualOwnerRegistry*>(context)->find(identity);
}

void release_native_render_actual_owner(NativeRenderActualOwners& owners, void* identity) {
    auto* const actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    if (actual->fetch_sub(1, std::memory_order_seq_cst) == 1) {
        auto& reference = owners.resolve_actual(identity);
        if (&reference.reference_count != actual)
            throw std::invalid_argument("native owner companion must borrow the same actual +04 atomic");
        reference.release_zero_references();
    }
}

NativeRenderContextStorage* initialize_native_render_context_00b1edc0_fragment(void* raw) noexcept {
    auto* context = ::new (raw) NativeRenderContextStorage;
    context->native_vtable_00 = reference_table;
    context->references_04.store(1, std::memory_order_relaxed);
    context->native_vtable_00 = context_table;
    context->camera_08 = nullptr;
    context->second_owner_0c = nullptr;
    context->borrowed_command_10 = nullptr;
    context->target_14 = nullptr;
    return context;
}

void destroy_native_render_context_00b1d120(
    NativeRenderContextStorage& context, NativeRenderActualOwners& owners) {
    context.native_vtable_00 = context_table;
    const ContextBaseCleanup base(context);
    release_then_clear(context.camera_08, owners);
    release_then_clear(context.second_owner_0c, owners);
    release_then_clear(context.target_14, owners);
}

NativeRenderContextStorage* delete_native_render_context_00b1d570(
    NativeRenderContextStorage* context, NativeRenderActualOwners& owners, std::uint32_t flags) {
    destroy_native_render_context_00b1d120(*context, owners);
    if (flags & 1) singleton_lifetime_free(context);
    return context;
}

NativeRenderContextReference::NativeRenderContextReference(NativeRenderContextStorage& storage,
    NativeRenderActualOwners& owners, const volatile std::uint32_t* table,
    NativeRenderContextCompanionDisposal disposal)
    : RenderCommandReference(storage.references_04), storage_(storage), owners_(owners),
      vtable_00d5e5c4_(table), disposal_(disposal) {
    if (!disposal.retire || storage.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("context reference requires live actual storage and explicit companion retirement");
    require_current_profile();
}

NativeRenderContextReference::~NativeRenderContextReference() {
    if (phase_ != Phase::retired) std::terminate();
}

void NativeRenderContextReference::require_current_profile() const noexcept {
    if (storage_.native_vtable_00 != context_table || !vtable_00d5e5c4_ ||
        vtable_00d5e5c4_[0] != 0x00bd30e0 || vtable_00d5e5c4_[1] != 0x00b1d570)
        std::terminate();
}

void NativeRenderContextReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    delete_native_render_context_00b1d570(&storage_, owners_, 1);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}

} // namespace bsp
