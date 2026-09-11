#pragma once

#include "bsp/native_string.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace bsp {

struct SoundConfigurationState;
class SoundLevelNameHost;

// Actual 10h Win32 record proved by producer 00A7D380. Vtable words are
// evidence tokens, not callable host pointers. New C++ APIs, not native ABIs.
// Native ECX=this; copy ctor RET4; destructor RET; scalar destructor RET4.
// NativeString has no automatic cleanup. Storage is supplied explicitly.
struct SoundListener {
    std::uint32_t native_vtable_00{0x00d5abb0u};
    std::atomic<std::uint32_t> references_04{1u};
    NativeString name_08;
};
static_assert(offsetof(SoundListener, references_04) == 4);
static_assert(offsetof(SoundListener, name_08) == 8);
static_assert(sizeof(SoundListener) == 0x10);

// Constructor, including the native destructive self-copy quirk: reset the
// destination header BEFORE checking aliasing, without releasing its old name.
SoundListener& construct_sound_listener_copy_00a7d380(SoundListener& destination,
    const SoundListener& source, NativeStringStorage&);
// Derived vtable, release nonnull name length+1 with header intact, base vtable.
// References and dead name fields remain untouched after the storage callback.
void destroy_sound_listener_00a7bce0(SoundListener&, NativeStringStorage&) noexcept;
// flags bit0 controls freeing; return the input address, including after free.
SoundListener* delete_sound_listener_00a7bd50(SoundListener*, std::uint8_t flags,
    NativeStringStorage&) noexcept;
void retain_sound_listener(SoundListener&) noexcept;
void release_sound_listener(SoundListener*, NativeStringStorage&) noexcept;

// Host owner of the native pointer array at manager+AC, count at+B0. Capacity
// remains configuration.listener_capacity_b4. Raw allocated slots remain alive
// during release callbacks, even after count has decremented. The allocator
// binding and live-allocation bit are host-only lifetime bookkeeping.
// No copying, concurrent structural edits, corrupted counts, allocation-failure
// emulation or foreign listener vtables. See SOUND_LISTENER_OWNERSHIP.md.
class SoundListenerTable final {
public:
    explicit SoundListenerTable(NativeStringStorage& = crt_string_storage()) noexcept;
    ~SoundListenerTable();
    SoundListenerTable(const SoundListenerTable&) = delete;
    SoundListenerTable& operator=(const SoundListenerTable&) = delete;
    std::size_t size() const noexcept { return static_cast<std::size_t>(count_); }
    bool empty() const noexcept { return count_ == 0; }
    SoundListener** data() noexcept { return data_; }
    SoundListener* const* data() const noexcept { return data_; }
    SoundListener* operator[](std::size_t index) const noexcept { return data_[index]; }
    NativeStringStorage& string_storage() const noexcept { return *strings_; }
    // Host convenience, same reverse-ref release as native resize(0).
    void clear() noexcept;
    void reserve_00a7d750(std::int32_t requested, std::int32_t& capacity,
        NativeStringStorage&);
    void resize_00a7d910(std::int32_t requested, std::int32_t& capacity,
        NativeStringStorage&);
    // Slot is zeroed before reading the reference argument; retain before count++.
    void append_00a7f050(SoundListener* const&, std::int32_t& capacity,
        NativeStringStorage&);
    // Native resize(0), free data without resetting data/capacity. Only the host
    // bookkeeping bit is cleared, preventing an implicit C++ double release.
    void destroy_00a7fe00(std::int32_t& capacity, NativeStringStorage&) noexcept;
private:
    void shrink(std::int32_t requested, NativeStringStorage&) noexcept;
    SoundListener** data_{};
    std::int32_t count_{};
    NativeStringStorage* strings_;
    bool allocation_live_{true};
};

// 00A7F9F0 ECX=manager+A4; source record*; RET4. Compare every equal-length
// nonempty name and ignore results; allocate/copy, append-retain, release temp.
void append_sound_listener_00a7f9f0(SoundConfigurationState&, const SoundListener&,
    SoundLevelNameHost&, NativeStringStorage& = crt_string_storage());
// 00A7D910 ECX=array header; requested count; RET4. Capacity only grows;
// growth initializes null slots, shrink decrements count before final release.
void resize_sound_listener_table_00a7d910(SoundConfigurationState&,
    std::int32_t requested, NativeStringStorage& = crt_string_storage());
// 00A7FE00 ECX=manager+A4, RET. Clears refs, frees table, then sets base vtable.
// Leaves dead array pointer/capacity and selected pointer/ordinal untouched.
void destroy_sound_listener_owner_00a7fe00(SoundConfigurationState&,
    std::uint32_t& native_vtable, NativeStringStorage& = crt_string_storage()) noexcept;

} // namespace bsp
