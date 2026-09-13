#include "bsp/native_render_group_lifetime.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void return_name(SizedStoragePool& strings, char* data, std::uint32_t size) {
    strings.release_00bd1510(data, size);
}
void return_name(ActualNativeStringPoolStorage& strings, char* data,
    std::uint32_t size) noexcept {
    strings.release(data, size);
}
template<class StringStorage>
class GroupNameCleanup final {
public:
    GroupNameCleanup(NativeRenderGroupStorage& group, StringStorage& strings) noexcept
        : group_(group), strings_(strings) {}
    ~GroupNameCleanup() { if (armed_) run(); }
    void run() {
        char* const data = group_.name_data_08;
        armed_ = false; // Native state -1 precedes the final normal pool return.
        if (data) return_name(strings_, data,
            static_cast<std::uint32_t>(group_.name_length_04) + 1u);
    }
private:
    NativeRenderGroupStorage& group_;
    StringStorage& strings_;
    bool armed_{true};
};

class GroupArraysCleanup final {
public:
    explicit GroupArraysCleanup(NativeRenderGroupStorage& group) noexcept : group_(group) {}
    ~GroupArraysCleanup() { if (armed_) run(); }
    void run() {
        armed_ = false; // Native group state0 before the vector destructor call.
        // These two actual array destructors cannot throw in their valid header
        // domain: resize0 allocates nothing and ordinary free is nonthrowing.
        destroy_native_instance_entry_pointers_00b1d1d0(group_.source_entries_24[1]);
        destroy_native_instance_entry_pointers_00b1d1d0(group_.source_entries_24[0]);
    }
private:
    NativeRenderGroupStorage& group_;
    bool armed_{true};
};

void release_model_then_clear(void*& field, NativeRenderGroupModels& models) {
    void* const captured = field;
    if (!captured) return;
    auto& reference = models.resolve_actual_model(captured);
    if (&reference.model_owner().storage.node != captured)
        throw std::invalid_argument("group model companion must borrow the actual captured raw prefix");
    unlink_and_release_render_model_00b6dfa0(reference);
    // The model and its reference companion may both have been retired. Only
    // the separately owned group remains accessible after that terminal call.
    field = nullptr;
}
template<class StringStorage>
void destroy_group(NativeRenderGroupStorage& group,
    NativeRenderActualOwners& owners, NativeRenderGroupModels& models,
    StringStorage& strings) {
    GroupNameCleanup<StringStorage> name(group, strings);
    GroupArraysCleanup arrays(group);
    void* const binding = group.binding_00;
    if (binding) {
        release_native_render_actual_owner(owners, binding);
        group.binding_00 = nullptr;
    }
    release_model_then_clear(group.models_1c[0], models);
    release_model_then_clear(group.models_1c[1], models);
    arrays.run();
    name.run();
}
} // namespace

void destroy_native_render_group_00b1d760(NativeRenderGroupStorage& group,
    NativeRenderActualOwners& owners, NativeRenderGroupModels& models,
    SizedStoragePool& strings) {
    destroy_group(group, owners, models, strings);
}
void destroy_native_render_group_00b1d760(NativeRenderGroupStorage& group,
    NativeRenderActualOwners& owners, NativeRenderGroupModels& models,
    ActualNativeStringPoolStorage& strings) {
    destroy_group(group, owners, models, strings);
}

NativeRenderGroupStorage* delete_native_render_group_00b1d8e0(
    NativeRenderGroupStorage* group, NativeRenderActualOwners& owners,
    NativeRenderGroupModels& models, SizedStoragePool& strings, std::uint32_t flags) {
    destroy_native_render_group_00b1d760(*group, owners, models, strings);
    if (flags & 1) singleton_lifetime_free(group);
    return group;
}
NativeRenderGroupStorage* delete_native_render_group_00b1d8e0(
    NativeRenderGroupStorage* group, NativeRenderActualOwners& owners,
    NativeRenderGroupModels& models, ActualNativeStringPoolStorage& strings,
    std::uint32_t flags) {
    destroy_native_render_group_00b1d760(*group, owners, models, strings);
    if (flags & 1) singleton_lifetime_free(group);
    return group;
}

} // namespace bsp
