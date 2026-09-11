#include "bsp/sound_listener.hpp"
#include "bsp/sound_configuration.hpp"

#include <cassert>
#include <cstdlib>
#include <limits>
#include <new>

namespace bsp {

SoundListener& construct_sound_listener_copy_00a7d380(SoundListener& destination,
    const SoundListener& source, NativeStringStorage& strings)
{
    destination.native_vtable_00 = 0x00ceb130u;
    destination.references_04.store(1u, std::memory_order_seq_cst);
    destination.native_vtable_00 = 0x00d5abb0u;
    // Placement construction writes the two zero fields without releasing them.
    new (&destination.name_08) NativeString();
    if (&destination.name_08 != &source.name_08) {
        try {
            destination.name_08.copy_from_00be0a30_fragment(strings, source.name_08);
        } catch (...) {
            destination.native_vtable_00 = 0x00ceb130u;
            throw;
        }
    }
    return destination;
}

void destroy_sound_listener_00a7bce0(SoundListener& listener,
    NativeStringStorage& strings) noexcept
{
    listener.native_vtable_00 = 0x00d5abb0u;
    destroy_native_string_header_0041dd20(&listener.name_08, strings);
    listener.native_vtable_00 = 0x00ceb130u;
}

SoundListener* delete_sound_listener_00a7bd50(SoundListener* listener,
    std::uint8_t flags, NativeStringStorage& strings) noexcept
{
    destroy_sound_listener_00a7bce0(*listener, strings);
    if ((flags & 1u) != 0) {
        listener->~SoundListener();
        std::free(listener);
    }
    return listener;
}

void retain_sound_listener(SoundListener& listener) noexcept
{
    listener.references_04.fetch_add(1u, std::memory_order_seq_cst);
}

void release_sound_listener(SoundListener* listener, NativeStringStorage& strings) noexcept
{
    if (listener && listener->references_04.fetch_sub(1u,
        std::memory_order_seq_cst) == 1u) {
        // D5ABB0 bytes establish vslot0 BD30E0 -> vslot4 A7BD50(flags=1).
        (void)delete_sound_listener_00a7bd50(listener, 1u, strings);
    }
}

SoundListenerTable::SoundListenerTable(NativeStringStorage& strings) noexcept
    : strings_(&strings) {}

SoundListenerTable::~SoundListenerTable()
{
    if (allocation_live_) {
        clear();
        std::free(data_);
    }
}

void SoundListenerTable::shrink(std::int32_t requested,
    NativeStringStorage& strings) noexcept
{
    while (requested < count_) {
        --count_;
        SoundListener** slot = data_ + count_;
        SoundListener* removed = *slot;
        if (removed) {
            release_sound_listener(removed, strings);
            // Native captures this slot BEFORE the callback; zero AFTER it.
            *slot = nullptr;
        }
    }
    count_ = requested;
}

void SoundListenerTable::clear() noexcept
{
    if (allocation_live_) shrink(0, *strings_);
}

void SoundListenerTable::reserve_00a7d750(std::int32_t requested,
    std::int32_t& capacity, NativeStringStorage& strings)
{
    assert(&strings == strings_ && allocation_live_);
    if (requested < 1) requested = 1;
    if (requested <= capacity) return;
    assert(requested <= std::numeric_limits<std::int32_t>::max() / 4);
    auto* replacement = static_cast<SoundListener**>(
        std::malloc(static_cast<std::size_t>(requested) * sizeof(SoundListener*)));
    if (!replacement) throw std::bad_alloc();
    // All new slots acquire references before any old slot loses its reference.
    for (std::int32_t index = 0; index < count_; ++index) {
        replacement[index] = nullptr;
        SoundListener* listener = data_[index];
        if (listener) {
            replacement[index] = listener;
            retain_sound_listener(*listener);
        }
    }
    for (std::int32_t index = 0; index < count_; ++index) {
        SoundListener** slot = data_ + index;
        SoundListener* listener = *slot;
        if (listener) {
            release_sound_listener(listener, strings);
            *slot = nullptr;
        }
    }
    std::free(data_);
    // Raw disk tail 00A7D81F..31, absent from Ghidra's function body.
    data_ = replacement;
    capacity = requested;
}

void SoundListenerTable::resize_00a7d910(std::int32_t requested,
    std::int32_t& capacity, NativeStringStorage& strings)
{
    assert(&strings == strings_ && allocation_live_ && requested >= 0);
    if (capacity < requested) reserve_00a7d750(requested, capacity, strings);
    for (std::int32_t index = count_; index < requested; ++index)
        data_[index] = nullptr;
    shrink(requested, strings);
}

void SoundListenerTable::append_00a7f050(SoundListener* const& source,
    std::int32_t& capacity, NativeStringStorage& strings)
{
    assert(&strings == strings_ && allocation_live_);
    if (count_ == capacity) {
        assert(capacity <= std::numeric_limits<std::int32_t>::max() / 8);
        const std::int32_t doubled = 2 * capacity;
        reserve_00a7d750(doubled > 1 ? doubled : 1, capacity, strings);
    }
    SoundListener** slot = data_ + count_;
    *slot = nullptr;
    SoundListener* listener = source;
    if (listener) {
        *slot = listener;
        retain_sound_listener(*listener);
    }
    ++count_;
}

void SoundListenerTable::destroy_00a7fe00(std::int32_t& capacity,
    NativeStringStorage& strings) noexcept
{
    assert(&strings == strings_ && allocation_live_);
    resize_00a7d910(0, capacity, strings);
    std::free(data_);
    allocation_live_ = false;
}

void append_sound_listener_00a7f9f0(SoundConfigurationState& state,
    const SoundListener& source, SoundLevelNameHost& names, NativeStringStorage& strings)
{
    // Native snapshots begin/end once. Reentrant table mutation from comparison
    // would invalidate the native walk and is outside this owner's contract.
    const auto count = state.listeners_ac.size();
    for (std::size_t index = 0; index < count; ++index) {
        const NativeString& existing = state.listeners_ac[index]->name_08;
        if (existing.length() == source.name_08.length() && existing.length() != 0)
            (void)names.compare_class_name_case_insensitive(
                existing.data(), source.name_08.data());
    }
    auto* memory = std::malloc(sizeof(SoundListener));
    if (!memory) throw std::bad_alloc();
    auto* copied = new (memory) SoundListener();
    try {
        construct_sound_listener_copy_00a7d380(*copied, source, strings);
    } catch (...) {
        copied->~SoundListener();
        std::free(copied);
        throw;
    }
    try {
        state.listeners_ac.append_00a7f050(copied, state.listener_capacity_b4, strings);
    } catch (...) {
        release_sound_listener(copied, strings);
        throw;
    }
    release_sound_listener(copied, strings);
}

void resize_sound_listener_table_00a7d910(SoundConfigurationState& state,
    std::int32_t requested, NativeStringStorage& strings)
{
    state.listeners_ac.resize_00a7d910(requested, state.listener_capacity_b4, strings);
}

void destroy_sound_listener_owner_00a7fe00(SoundConfigurationState& state,
    std::uint32_t& native_vtable, NativeStringStorage& strings) noexcept
{
    state.listeners_ac.destroy_00a7fe00(state.listener_capacity_b4, strings);
    // Native A7FE00 does NOT first write D5AEC4. Base destruction is only here,
    // after releases and free, at the raw-tail call 00A7FE47 -> 00BD30F0.
    native_vtable = 0x00ceb130u;
}

} // namespace bsp
