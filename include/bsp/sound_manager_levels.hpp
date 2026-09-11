#pragma once

#include "bsp/native_string.hpp"

#include <cstdint>
#include <vector>

namespace bsp {

// Behavioral projections, not native layouts or replacement ABIs. Names are
// hypotheses. Evidence, original calling conventions and limits are recorded in
// docs/SOUND_MANAGER_LEVELS.md. The enabled byte is already represented by
// SoundSystemState::sound_enabled and AudioSettings::enabled_24; it is not a gain.
struct SoundClassLevel {
    std::uint32_t class_index_08{};
    float volume_10{};
    const NativeString* name_18{}; // view of the embedded +18/+1C native string
};

struct SoundLevelEntry {
    std::uint8_t dirty_14{};
    SoundClassLevel* class_44{};
};

struct SoundManagerLevels {
    float global_4c{};
    std::vector<SoundLevelEntry*> entries_8c; // native pointer array, count +90
    std::vector<SoundClassLevel*> classes_98; // different array, count +9C
};

// x86 SHL r32,CL uses only the low five bits, including for class indices >=32.
constexpr std::uint32_t sound_class_bit(std::uint32_t index) noexcept
{
    return std::uint32_t{1} << (index & 31u);
}

// 00A7A3F0, __thiscall(manager, uint32 mask), RET 4. Entries and descriptors
// must be valid. Only matching dirty bytes become 1; unmatched bytes stay as-is.
void dirty_sound_classes_00a7a3f0(SoundManagerLevels& manager,
    std::uint32_t mask) noexcept;

// 00A7A440, __thiscall(manager, float), tail JMP to 00A7A3F0 / RET 4.
// Stores the argument without clamping or an equality check, then uses FFFF.
void apply_sound_global_level_00a7a440(SoundManagerLevels& manager,
    float volume) noexcept;

// 00A7ABF0, __thiscall(manager, uint32 mask, float), RET 8. Bit positions
// follow array ordinals, not class_index_08. Ordered equality removes the bit
// from the still-pending mask. NaN is a change; +0 and -0 compare unchanged.
// Residual bits, even ones with no descriptor, still reach the dirty walker.
void set_sound_class_levels_00a7abf0(SoundManagerLevels& manager,
    std::uint32_t mask, float volume) noexcept;

class SoundLevelNameHost {
public:
    virtual ~SoundLevelNameHost() = default;
    // 00A7AD3B: CRT __stricmp 00BF7FBF. Keep CRT/locale behavior external.
    virtual int compare_class_name_case_insensitive(const char* left,
        const char* right) = 0;
};

// 00A7ACF0, __thiscall(manager, native-string*), RET 4, index in EAX.
// Exact stored-length equality precedes __stricmp; two empty names match
// without reading their buffers. First match wins; a miss returns -1.
std::int32_t find_sound_class_00a7acf0(const SoundManagerLevels& manager,
    const NativeString& name, SoundLevelNameHost& host);

class SoundLevelTableHost {
public:
    virtual ~SoundLevelTableHost() = default;
    // Both call 00A7C2C0 with ECX=manager+98 and requested count=index+1.
    // The callee's allocation/initialization behavior is not reconstructed.
    virtual void grow_classes_before_read(SoundManagerLevels& manager,
        std::int32_t requested_count) = 0; // 00A7F8FD
    virtual void grow_classes_before_write(SoundManagerLevels& manager,
        std::int32_t requested_count) = 0; // 00A7F923
};

// 00A7F8E0, __thiscall(manager, signed index, float), RET 8. Native has no
// negative-index check: passing the lookup-miss value -1 reads before the
// array. This interface requires index>=0, index<INT_MAX, count<=INT_MAX,
// and a valid descriptor at index after any growth call. It adds no recovery.
void set_sound_class_level_00a7f8e0(SoundManagerLevels& manager,
    std::int32_t index, float volume, SoundLevelTableHost& host);

} // namespace bsp
