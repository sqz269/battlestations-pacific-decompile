#include "bsp/sound_class_ownership.hpp"

#include <cassert>
#include <limits>
#include <vector>

namespace bsp {

SoundClassDescriptor::SoundClassDescriptor(NativeStringStorage& storage) noexcept
    : storage_(&storage)
{
    // 00A7C37A ref=1; 00A7C391..00A7C3AE defaults. The native zero-length
    // resize at 00A7C3B1 sees an already empty header and is a no-op.
    class_index_08 = 0u;
    volume_10 = 1.0f;
    name_18 = &owned_name_;
}

SoundClassDescriptor::~SoundClassDescriptor() noexcept
{
    // 00A7C40D..00A7C42D releases only a nonnull name buffer with length+1.
    // The actual-header helper preserves the dying native string's header.
    destroy_native_string_header_0041dd20(&owned_name_, *storage_);
}

SoundClassDescriptor* create_sound_class_00a7c350(NativeStringStorage& storage)
{
    return new SoundClassDescriptor(storage);
}

void SoundClassDescriptor::retain() noexcept
{
    references_04_.fetch_add(1u, std::memory_order_seq_cst);
}

void SoundClassDescriptor::release() noexcept
{
    if (references_04_.fetch_sub(1u, std::memory_order_seq_cst) == 1u) {
        // Native vslot0 dispatch reaches 00A7C460(flags=1), which destroys the
        // descriptor and frees it. C++ delete supplies this host's allocation pair.
        delete this;
    }
}

std::uint32_t SoundClassDescriptor::reference_count() const noexcept
{
    return references_04_.load(std::memory_order_seq_cst);
}

SoundClassOwnership::SoundClassOwnership(SoundManagerLevels& manager) noexcept
    : manager_(&manager)
{
    assert(manager.classes_98.empty());
}

SoundClassOwnership::~SoundClassOwnership()
{
    // Host owner cleanup, not a reconstruction of the full manager destructor.
    resize_00a7c2c0(0);
}

void SoundClassOwnership::reserve_00a7bbe0(std::int32_t requested_capacity)
{
    if (requested_capacity < 1) {
        requested_capacity = 1;
    }
    if (requested_capacity <= capacity_) {
        return;
    }
    assert(requested_capacity <= std::numeric_limits<std::int32_t>::max() / 4);

    auto& slots = manager_->classes_98;
    std::vector<SoundClassLevel*> replacement;
    replacement.reserve(static_cast<std::size_t>(requested_capacity));
    for (SoundClassLevel* level : slots) {
        replacement.push_back(level);
        if (level != nullptr) {
            static_cast<SoundClassDescriptor*>(level)->retain();
        }
    }
    // 00A7BC70 second pass follows completion of every retained copy.
    for (SoundClassLevel*& level : slots) {
        if (level != nullptr) {
            static_cast<SoundClassDescriptor*>(level)->release();
            level = nullptr;
        }
    }
    // 00A7BCAF..00A7BCC1 is absent from the old decompilation after a false
    // no-return _free call. Disk and live bytes prove both header stores.
    slots.swap(replacement);
    capacity_ = requested_capacity;
}

void SoundClassOwnership::resize_00a7c2c0(std::int32_t requested_count)
{
    assert(requested_count >= 0);
    if (requested_count > capacity_) {
        reserve_00a7bbe0(requested_count);
    }
    auto& slots = manager_->classes_98;
    const auto count = static_cast<std::size_t>(requested_count);
    if (slots.size() < count) {
        slots.resize(count, nullptr);
    }
    while (slots.size() > count) {
        SoundClassLevel* removed = slots.back();
        // Native decrements the visible count before releasing the removed ref.
        slots.pop_back();
        if (removed != nullptr) {
            static_cast<SoundClassDescriptor*>(removed)->release();
        }
    }
}

void SoundClassOwnership::append_retained(SoundClassDescriptor& descriptor)
{
    auto& slots = manager_->classes_98;
    if (slots.size() == static_cast<std::size_t>(capacity_)) {
        assert(capacity_ <= std::numeric_limits<std::int32_t>::max() / 8);
        const auto doubled = capacity_ * 2;
        reserve_00a7bbe0(doubled > 1 ? doubled : 1);
    }
    slots.push_back(&descriptor);
    descriptor.retain();
}

void SoundClassOwnership::grow_classes_before_read(SoundManagerLevels& manager,
    std::int32_t requested_count)
{
    assert(&manager == manager_);
    (void)manager;
    resize_00a7c2c0(requested_count);
}

void SoundClassOwnership::grow_classes_before_write(SoundManagerLevels& manager,
    std::int32_t requested_count)
{
    assert(&manager == manager_);
    (void)manager;
    resize_00a7c2c0(requested_count);
}

} // namespace bsp
