#include "bsp/native_viewport_registry.hpp"
#include "bsp/native_viewport_owner.hpp"

#include <exception>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
// Explicit host provider installation, not a reconstructed native global.
NativeViewportRegistry* installed_registry;
}

NativeViewportRegistry::Storage::~Storage() noexcept {
    if (registry_ || phase_ != Phase::unused || view_) std::terminate();
}
NativeViewportRegistry::Admission::Admission(
    NativeViewportRegistry& registry, Storage& storage) noexcept
    : registry_(&registry), storage_(&storage) {}
NativeViewportRegistry::Admission::~Admission() noexcept { cancel(); }
NativeViewportRegistry::Admission::Admission(Admission&& other) noexcept
    : registry_(std::exchange(other.registry_, nullptr)),
      storage_(std::exchange(other.storage_, nullptr)) {}
NativeViewportRegistry::Admission& NativeViewportRegistry::Admission::operator=(
    Admission&& other) noexcept {
    if (this != &other) {
        cancel();
        registry_ = std::exchange(other.registry_, nullptr);
        storage_ = std::exchange(other.storage_, nullptr);
    }
    return *this;
}
void NativeViewportRegistry::Admission::cancel() noexcept {
    if (registry_) registry_->cancel(*this);
}
NativeViewportRegistry& NativeViewportRegistry::Admission::require_registry() const {
    if (!registry_ || installed_registry != registry_ || !storage_ ||
        storage_->registry_ != registry_ || storage_->phase_ != Phase::reserved)
        throw std::logic_error("viewport admission requires its installed runtime and reserved record");
    return *registry_;
}

NativeViewportRegistry::~NativeViewportRegistry() {
    if (records_ || installed_registry == this) std::terminate();
}
NativeViewportRegistry::Admission NativeViewportRegistry::admit(Storage& storage) {
    if (installed_registry != this)
        throw std::logic_error("viewport admission requires the installed registry");
    if (storage.registry_ || storage.phase_ != Phase::unused || storage.view_)
        throw std::invalid_argument("viewport admission requires unused quiescent storage");
    storage.registry_ = this;
    storage.phase_ = Phase::reserved;
    storage.next_ = records_;
    if (records_) records_->previous_ = &storage;
    records_ = &storage;
    return Admission(*this, storage);
}
void NativeViewportRegistry::cancel(Admission& admission) noexcept {
    auto* const storage = admission.storage_;
    if (admission.registry_ != this || !storage || storage->registry_ != this ||
        storage->phase_ != Phase::reserved) std::terminate();
    storage->phase_ = Phase::cancelled;
    admission.registry_ = nullptr;
    admission.storage_ = nullptr;
}
void NativeViewportRegistry::constructed(Admission& admission,
    NativeViewportOwner& owner) noexcept {
    auto* const storage = admission.storage_;
    if (installed_registry != this || admission.registry_ != this || !storage ||
        storage->registry_ != this || storage->phase_ != Phase::reserved ||
        owner.native_vtable_00 != kNativeViewportVtable || owner.references_04 <= 0)
        std::terminate();
    const auto key = reinterpret_cast<std::uintptr_t>(&owner);
    for (auto* record = records_; record; record = record->next_)
        if (record->phase_ == Phase::live && record->actual_key_ == key)
            std::terminate();
    // This constructor binds references directly; copying would make a detached
    // diagnostic viewport. Optional emplacement uses only the caller's storage.
    storage->view_.emplace(owner);
    storage->actual_key_ = key;
    storage->phase_ = Phase::live;
    admission.registry_ = nullptr;
    admission.storage_ = nullptr;
}
const CameraViewport* NativeViewportRegistry::resolve_viewport(
    NativeViewportOwner* owner) noexcept {
    const auto key = reinterpret_cast<std::uintptr_t>(owner);
    for (auto* record = records_; record; record = record->next_)
        if (record->phase_ == Phase::live && record->actual_key_ == key)
            return &*record->view_;
    return nullptr;
}
void NativeViewportRegistry::retire_before_destroy(NativeViewportOwner& owner) noexcept {
    const auto key = reinterpret_cast<std::uintptr_t>(&owner);
    for (auto* record = records_; record; record = record->next_) {
        if (record->phase_ != Phase::live || record->actual_key_ != key) continue;
        // Remove from the live identity set before typed destruction/free. The
        // all-record list keeps the inert host view alive without a native read.
        record->phase_ = Phase::retired;
        return;
    }
}
void NativeViewportRegistry::forget_quiescent(Storage& storage) noexcept {
    if (!storage.registry_ && storage.phase_ == Phase::unused) return;
    if (storage.registry_ != this ||
        (storage.phase_ != Phase::retired && storage.phase_ != Phase::cancelled))
        std::terminate();
    storage.view_.reset(); // reference members are not dereferenced by destruction
    if (storage.previous_) storage.previous_->next_ = storage.next_;
    else records_ = storage.next_;
    if (storage.next_) storage.next_->previous_ = storage.previous_;
    storage.previous_ = nullptr;
    storage.next_ = nullptr;
    storage.actual_key_ = 0;
    storage.phase_ = Phase::unused;
    storage.registry_ = nullptr;
}

NativeViewportRegistryBinding::NativeViewportRegistryBinding(NativeViewportRegistry& registry)
    : registry_(registry) {
    if (installed_registry || registry.records_)
        throw std::logic_error("native viewport registry already installed or not quiescent");
    installed_registry = &registry;
}
NativeViewportRegistryBinding::~NativeViewportRegistryBinding() noexcept {
    if (installed_registry != &registry_ || registry_.records_) std::terminate();
    installed_registry = nullptr;
}
void retire_registered_native_viewport_before_destroy(NativeViewportOwner& owner) noexcept {
    auto* const registry = installed_registry;
    if (registry) registry->retire_before_destroy(owner);
}
} // namespace bsp
