#pragma once

#include "bsp/light_type_bootstrap.hpp"

#include <cstdint>

namespace bsp {

// Actual cFile descriptor: 0109DB58 own ID, 0109DB5C root ID, 0109DB60 name.
// No member initializers: binding an existing process must preserve its state.
struct NativeFileTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};

// cMemoryFile at 0109DBA0 and cFileX86 at 0109DC30 have the same four-word
// shape. The final word is an original image address, not a host string pointer.
struct NativeDerivedFileTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t file_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};

struct NativeStreamTypeIdStorage {
    volatile std::uint8_t& file_guard_0109db54;
    volatile NativeFileTypeDescriptor& file_0109db58;
    volatile std::uint8_t& memory_guard_0109db94;
    volatile NativeDerivedFileTypeDescriptor& memory_0109dba0;
    volatile std::uint8_t& physical_guard_0109dc2c;
    volatile NativeDerivedFileTypeDescriptor& physical_0109dc30;
};

class NativeStreamTypeIds final {
public:
    // Counter and shared_types must belong to the same process lifetime domain
    // and bind its actual publication slot0109DB7C and root guard/descriptor.
    // Construction only retains references; it performs no initialization.
    NativeStreamTypeIds(TypeIdCounterLifetime&, LightTypeBootstrap& shared_types,
        NativeStreamTypeIdStorage) noexcept;

    // Complete native body00BE4530..00BE456D: ECX is the target descriptor,
    // no stack arguments, RET. The guard remains process-wide for any target.
    void initialize_file_00be4530(volatile NativeFileTypeDescriptor& target);
    // Complete raw entries, currently undefined in Ghidra: no arguments, RET.
    // Memory: [00CD8FC0,00CD900F); physical: [00CD9030,00CD907F).
    void initialize_memory_00cd8fc0();
    void initialize_physical_00cd9030();

    NativeStreamTypeIdStorage storage() const noexcept { return storage_; }

private:
    std::uint32_t consume_type_id();
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& shared_types_;
    NativeStreamTypeIdStorage storage_;
};

} // namespace bsp
