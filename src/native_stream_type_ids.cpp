#include "bsp/native_stream_type_ids.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native stream type IDs require MSVC Win32 pointer widths.
#endif

namespace bsp {

static_assert(sizeof(NativeFileTypeDescriptor) == 12);
static_assert(offsetof(NativeFileTypeDescriptor, own_id) == 0);
static_assert(offsetof(NativeFileTypeDescriptor, root_id) == 4);
static_assert(offsetof(NativeFileTypeDescriptor, native_name_address) == 8);
static_assert(sizeof(NativeDerivedFileTypeDescriptor) == 16);
static_assert(offsetof(NativeDerivedFileTypeDescriptor, own_id) == 0);
static_assert(offsetof(NativeDerivedFileTypeDescriptor, file_id) == 4);
static_assert(offsetof(NativeDerivedFileTypeDescriptor, root_id) == 8);
static_assert(offsetof(NativeDerivedFileTypeDescriptor, native_name_address) == 12);

NativeStreamTypeIds::NativeStreamTypeIds(TypeIdCounterLifetime& counter,
    LightTypeBootstrap& shared_types, NativeStreamTypeIdStorage storage) noexcept
    : counter_(counter), shared_types_(shared_types), storage_(storage) {}

std::uint32_t NativeStreamTypeIds::consume_type_id() {
    volatile auto* counter = counter_.get_006fac20();
    const auto result = counter->next_id_04;
    counter->next_id_04 = result + 1u;
    return result;
}

void NativeStreamTypeIds::initialize_file_00be4530(
    volatile NativeFileTypeDescriptor& target) {
    if (storage_.file_guard_0109db54 == 0) {
        storage_.file_guard_0109db54 = 1;
        target.native_name_address = 0x00d6888cu;
        auto& root = shared_types_.storage().root_0109db84;
        shared_types_.initialize_root_00bea780(root);
        target.root_id = root.own_id;
        target.own_id = consume_type_id();
    }
}

void NativeStreamTypeIds::initialize_memory_00cd8fc0() {
    if (storage_.memory_guard_0109db94 == 0) {
        storage_.memory_guard_0109db94 = 1;
        storage_.memory_0109dba0.native_name_address = 0x00d68d7cu;
        initialize_file_00be4530(storage_.file_0109db58);
        // Both source loads precede either destination store at00CD8FE4..8FF4.
        const auto file_id = storage_.file_0109db58.own_id;
        const auto root_id = storage_.file_0109db58.root_id;
        storage_.memory_0109dba0.file_id = file_id;
        storage_.memory_0109dba0.root_id = root_id;
        storage_.memory_0109dba0.own_id = consume_type_id();
    }
}

void NativeStreamTypeIds::initialize_physical_00cd9030() {
    if (storage_.physical_guard_0109dc2c == 0) {
        storage_.physical_guard_0109dc2c = 1;
        storage_.physical_0109dc30.native_name_address = 0x00d69218u;
        initialize_file_00be4530(storage_.file_0109db58);
        // Both source loads precede either destination store at00CD9054..9064.
        const auto file_id = storage_.file_0109db58.own_id;
        const auto root_id = storage_.file_0109db58.root_id;
        storage_.physical_0109dc30.file_id = file_id;
        storage_.physical_0109dc30.root_id = root_id;
        storage_.physical_0109dc30.own_id = consume_type_id();
    }
}

} // namespace bsp
