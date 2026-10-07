#include "bsp/native_session_message84.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/native_bit_cursor_write.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;

// Native raw address arithmetic, including wrapping index products. Every
// selected address must be valid in the caller's admitted ordinary domain.
template<class T> T load_at(const void* p, U offset) noexcept {
    return *reinterpret_cast<const volatile T*>(reinterpret_cast<U>(p) + offset);
}

NativeSessionMessage84* __fastcall scalar(NativeSessionMessage84* m, void*, U flags) {
    auto* const release = reinterpret_cast<const NativeSessionMessage84Profile*>(m->base.profile_00)->release;
    return delete_native_session_message84_006bd600(m, flags, *release);
}
void __fastcall write(NativeSessionMessage84* m, void*, NativeBitCursor* c) {
    write_native_session_message84_006bd680(m, c);
}
void __fastcall read(NativeSessionMessage84* m, void*, NativeSessionReadStream* s) {
    read_native_session_message84_006bd710(m, s);
}
bool __fastcall accepts(const NativeSessionMessage84* m, void*, U type) {
    return native_session_message84_accepts_type_006bd5d0(m, type);
}
bool __fastcall valid(const NativeSessionMessage84* m, void*) {
    const auto* const banks = reinterpret_cast<const NativeSessionMessage84Profile*>(m->base.profile_00)->banks;
    return native_session_message84_is_valid_006bd7a0(m, *banks);
}
} // namespace

NativeSessionMessage84Profile::NativeSessionMessage84Profile(
    const ObjectHandleTables& actual_banks, NativeMessage84Release& actual_release)
    : slots{reinterpret_cast<U>(&scalar), reinterpret_cast<U>(&write), reinterpret_cast<U>(&read),
            reinterpret_cast<U>(&accepts), reinterpret_cast<U>(&valid)},
      banks(&actual_banks), release(&actual_release) {}

U native_vehicle_class_id_0095b9c0(const void* descriptor, NativeMessage84ClassRegistryAccess& access) {
    if (descriptor == nullptr) return 0;
    void* const registry = access.get_00437f50();
    const U index = load_at<U>(descriptor, 0x70);
    return load_at<U>(registry, 0x2010u + index * 4u);
}

NativeSessionMessage84* construct_native_session_message84_006bd520(
    NativeSessionMessage84* m, const void* block, U index,
    const NativeSessionMessageContext& base_context, const NativeSessionMessage84Profile& profile,
    NativeMessage84ClassRegistryAccess& access) {
    construct_native_session_message_0075b430(&m->base, 0x84, base_context);
    volatile auto& v = *m;
    v.sender_18 = 0;
    v.relay_1a = 0;
    v.base.delivery_04 = 1;
    v.flag_1c = 0;
    v.slot_index_20 = index;
    v.base.profile_00 = profile.slots; //006BD56A, before actual block+4C load.
    const U slots = load_at<U>(block, 0x4c);
    const void* const slot = reinterpret_cast<const void*>(slots + index * 0x58u);
    v.state_24 = load_at<U>(slot, 0x2c);
    const void* const descriptor = load_at<const void*>(slot, 4);
    v.vehicle_class_28 = native_vehicle_class_id_0095b9c0(descriptor, access);
    v.count_2c = load_at<U>(slot, 8);
    v.raw_30 = load_at<U>(slot, 0x10);
    const void* const squad = load_at<const void*>(slot, 0x28);
    v.squad_id_34 = squad != nullptr ? load_at<std::uint16_t>(squad, 0x174) : 0;
    return m;
}

NativeSessionMessage84* delete_native_session_message84_006bd600(
    NativeSessionMessage84* m, U flags, NativeMessage84Release& release) noexcept {
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) release.free_00bf65ac(m);
    return m;
}

void write_native_session_message84_006bd680(const NativeSessionMessage84* m, NativeBitCursor* c) {
    write_native_session_message_header_0075b480(m, c);
    const volatile auto& v = *m;
    write_native_bool_bit_004290b0(c, v.flag_1c);
    write_native_unsigned_dword_bits_00429070(c, v.slot_index_20, 6);
    write_native_unsigned_dword_bits_00429070(c, v.state_24, 4);
    write_native_unsigned_dword_bits_00429070(c, v.vehicle_class_28, 10);
    write_native_unsigned_dword_bits_00429070(c, v.count_2c, 6);
    write_native_unsigned_dword_bits_00429070(c, v.raw_30, 4);
    const bool present = v.squad_id_34 != 0;
    write_native_bool_bit_004290b0(c, present);
    if (present) write_native_word_bits_00429120(c, v.squad_id_34, 12);
}

void read_native_session_message84_006bd710(NativeSessionMessage84* m, NativeSessionReadStream* stream) {
    read_native_session_message_header_0075b4c0(m, stream);
    auto* const c = &stream->cursor_04;
    read_native_bool_bit_00428d70(c, reinterpret_cast<bool*>(&m->flag_1c));
    read_native_u32_bits_00428d10(c, &m->slot_index_20, 6);
    read_native_u32_bits_00428d10(c, &m->state_24, 4);
    read_native_u32_bits_00428d10(c, &m->vehicle_class_28, 10);
    read_native_u32_bits_00428d10(c, &m->count_2c, 6);
    read_native_u32_bits_00428d10(c, &m->raw_30, 4);
    bool present;
    read_native_bool_bit_00428d70(c, &present);
    if (present) read_native_word_bits_00428e30(c, &m->squad_id_34, 12);
    else static_cast<volatile NativeSessionMessage84&>(*m).squad_id_34 = 0;
}

bool native_session_message84_accepts_type_006bd5d0(const NativeSessionMessage84*, U type) {
    return type == 0x84 || type == 0x49 || type == 0x46;
}

bool native_session_message84_is_valid_006bd7a0(const NativeSessionMessage84* m,
                                               const ObjectHandleTables& actual_banks) noexcept {
    const auto id = static_cast<const volatile NativeSessionMessage84&>(*m).squad_id_34;
    return id == 0 || object_from_handle_006ad080(id, actual_banks) != nullptr;
}

} // namespace bsp
