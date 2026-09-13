#include "bsp/native_input_binding_slots.hpp"
#include "bsp/native_input_action_binding_runtime.hpp"
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> T read(Word p, Word offset = 0) noexcept {
    T result; std::memcpy(&result, pointer(p + offset), sizeof result); return result;
}
template<class T = Word> void write(Word p, Word offset, T value) noexcept {
    std::memcpy(pointer(p + offset), &value, sizeof value);
}
Word action_record(const void* owner, Word action) noexcept {
    return read(address(owner), 4) + action * 0x30u;
}
} // namespace

std::int32_t count_native_input_binding_slots_00a92820(
    const void* owner, Word action) noexcept {
    return read<std::int32_t>(action_record(owner, action), 0x14);
}

void read_native_input_binding_slot_00a92790(const void* owner, Word action,
    std::int32_t slot, void* descriptor14, float* scale, Word missing_preimage) noexcept {
    const auto header = action_record(owner, action) + 0x10u;
    const auto output = address(descriptor14);
    if (read<std::int32_t>(header, 4) <= slot) {
        write(output, 0, 0xffffffffu);
        write(output, 4, 0u);
        write(output, 8, 0u);
        write(output, 0xc, 0xffffffffu);
        write(output, 0x10, missing_preimage & 0xffffff00u);
        write(address(scale), 0, 0u);
        return;
    }
    const auto binding = read(header) + static_cast<Word>(slot) * 0x34u;
    // Separate loads/stores preserve supported overlaps with the source record.
    write(output, 0, read(binding, 4));
    write(output, 4, read(binding, 8));
    write(output, 8, read(binding, 0xc));
    write(output, 0xc, read(binding, 0x10));
    write(output, 0x10, read(binding, 0x14));
    const void* source_scale = pointer(binding + 0x30u);
    __asm {
        mov eax, source_scale
        mov edx, scale
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}

void install_native_input_binding_slot_00a93750(void* owner, Word action,
    std::int32_t slot, std::array<Word, 5> descriptor, float scale,
    const NativeInputBindingStorageContext& storage, void* volatile& backend) {
    const auto record = action_record(owner, action), header = record + 0x10u;
    if (read<std::int32_t>(header, 4) <= slot) {
        const auto requested = static_cast<std::int32_t>(static_cast<Word>(slot) + 1u);
        resize_native_input_bindings_00a93500(pointer(header), requested, storage);
    }
    const auto binding = read(header) + static_cast<Word>(slot) * 0x34u;
    write(binding, 4, descriptor[0]);
    write(binding, 8, descriptor[1]);
    write(binding, 0xc, descriptor[2]);
    write(binding, 0x10, descriptor[3]);
    write<std::uint8_t>(binding, 1, descriptor[0] == 2);
    write(binding, 0x14, descriptor[4]);
    // Native MOVSS stores all bits without the reader's x87 conversion.
    std::memcpy(pointer(binding + 0x30u), &scale, sizeof scale);
    rebind_native_input_action_00a91e80(pointer(record), backend);
}
} // namespace bsp
