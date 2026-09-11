#pragma once

#include "bsp/sound_manager_levels.hpp"

#include <atomic>
#include <cstdint>

namespace bsp {

// Behavioral owner for the native 20h descriptor, not its layout or ABI.
// Descriptive names are hypotheses; see docs/SOUND_CLASS_OWNERSHIP.md.
// The inherited level is the actual object read by the existing level routines.
class SoundClassDescriptor final : public SoundClassLevel {
public:
    SoundClassDescriptor(const SoundClassDescriptor&) = delete;
    SoundClassDescriptor& operator=(const SoundClassDescriptor&) = delete;

    std::uint8_t flags_0c{}; // meaning beyond the constructor's zero is unknown
    float secondary_level_14{1.0f}; // meaning beyond default 1 is unknown

    NativeString& name() noexcept { return owned_name_; }
    const NativeString& name() const noexcept { return owned_name_; }
    NativeStringStorage& name_storage() noexcept { return *storage_; }

    // Native callers use InterlockedIncrement/Decrement on descriptor+4.
    // The final release follows vslot0 -> 00BD30E0 -> vslot4(flags=1).
    void retain() noexcept;
    void release() noexcept;
    std::uint32_t reference_count() const noexcept;

private:
    friend SoundClassDescriptor* create_sound_class_00a7c350(NativeStringStorage&);
    explicit SoundClassDescriptor(NativeStringStorage& storage) noexcept;
    ~SoundClassDescriptor() noexcept;

    std::atomic<std::uint32_t> references_04_{1u};
    NativeString owned_name_;
    NativeStringStorage* storage_;
};

// 00A7C350: ECX allocated 20h descriptor, RET, returns this in EAX.
// Factory allocation is a host convenience: native setup allocates separately
// at 00A80D39. Returns one owned reference; call release when it is no longer held.
// The supplied string allocator must outlive every descriptor created with it.
SoundClassDescriptor* create_sound_class_00a7c350(
    NativeStringStorage& storage = crt_string_storage());

// Concrete host for manager+98/+9C/+A0. manager must initially have an empty
// classes_98 and must outlive this owner. Once bound, all table structural edits
// go through this object and every nonnull slot points to SoundClassDescriptor.
// External entry references must be retained separately if they outlive the table.
// Counts/capacities are nonnegative signed 32-bit values with native byte-size
// multiplication in range. Calls are serialized; atomic descriptor refs alone
// do not make table mutation concurrent. No allocation-failure emulation.
class SoundClassOwnership final : public SoundLevelTableHost {
public:
    explicit SoundClassOwnership(SoundManagerLevels& manager) noexcept;
    ~SoundClassOwnership() override;
    SoundClassOwnership(const SoundClassOwnership&) = delete;
    SoundClassOwnership& operator=(const SoundClassOwnership&) = delete;

    SoundManagerLevels& manager() const noexcept { return *manager_; }
    std::int32_t capacity() const noexcept { return capacity_; }

    // 00A7BBE0: ECX table header, signed requested capacity, RET 4.
    // Clamp requests to >=1; never reduce capacity. Retain all copies before
    // releasing old slots, then install storage. Count and descriptors survive.
    void reserve_00a7bbe0(std::int32_t requested_capacity);

    // 00A7C2C0: ECX table header, signed requested count, RET 4.
    // Grow with null slots. Shrink in reverse order, releasing each removed ref.
    // Capacity does not shrink. This does not construct missing descriptors.
    void resize_00a7c2c0(std::int32_t requested_count);

    // Support for setup's 00A80EBA..00A80EF5 insertion fragment. Grow only at
    // count==capacity to max(2*capacity,1), store and retain, then increment count.
    // The caller owns name/volume/index initialization and its temporary refs.
    void append_retained(SoundClassDescriptor& descriptor);

    void grow_classes_before_read(SoundManagerLevels& manager,
        std::int32_t requested_count) override;
    void grow_classes_before_write(SoundManagerLevels& manager,
        std::int32_t requested_count) override;

private:
    SoundManagerLevels* manager_;
    std::int32_t capacity_{};
};

} // namespace bsp
