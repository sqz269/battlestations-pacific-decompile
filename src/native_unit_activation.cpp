#include "bsp/native_unit_activation.hpp"
#include "bsp/native_alias_count_growth.hpp"

namespace bsp {
namespace {
template<class T> T read(const T& field) noexcept {
    return static_cast<const volatile T&>(field);
}
template<class T> void write(T& field, T value) noexcept {
    static_cast<volatile T&>(field) = value;
}
}

void activate_native_unit_0077f0e0(NativeUnitActivationView u,
    NativeUnitActivationGlobals g, NativeUnitActivationBindings& b)
{
    void* const unit = u.unit.canonical_unit;
    void* const sentinel = read(g.list_00f87194.sentinel_04);
    auto sentinel_node = b.list_node(sentinel);
    void* const previous = read(sentinel_node.previous_04);
    void* const inserted = b.call_004c2220(g.list_00f87194, sentinel, previous, &unit);
    grow_native_unit_registry_count_004cee30(g.list_00f87194.identity, 1);
    // No cleanup after count failure. Publish through the captured sentinel,
    // then through the LIVE new node's previous field, even if providers rebound.
    write(sentinel_node.previous_04, inserted);
    void* const actual_previous = read(b.list_node(inserted).previous_04);
    write(b.list_node(actual_previous).next_00, inserted);
    b.call_009277e0(u.unit);
    void* const holder = read(u.property_holder_c0);
    write(u.tick_294, read(g.tick_00f876b0));
    if (!holder) return;
    auto properties = b.property_holder(holder);
    const auto kind = read(properties.kind_04);
    if (kind == 3) {
        NativeLuaObjectStorage root;
        NativeUnitActivationReaderScratch reader;
        b.call_009238a0(unit, root);
        b.call_004425c0(reader, root);
        b.call_00441a20(reader);
        return;
    }
    if (kind == 2) {
        void* const saved_identity = read(properties.bag_08);
        auto saved = b.saved_state(saved_identity);
        const auto side = read(saved.side_b0);
        const auto entry = b.primary_table(unit)[0x144 / 4];
        write(u.side_54, side);
        write(u.word_28c, read(saved.word_40));
        const float* input = &saved.marker_108;
        float* output = &u.marker_304;
        __asm {
            mov eax, input
            mov edx, output
            fld dword ptr [eax]
            fstp dword ptr [edx]
        }
        const auto role = read(saved.role_10c);
        b.call_unit_144(entry, unit, role);
        // Native REP MOVSD, DF=0 ABI precondition: sequential live DWORDs,
        // not memmove or a snapshot. Keep overlap and callback mutations visible.
        for (unsigned i = 0; i != 9; ++i)
            write(u.roles_188[i], read(saved.roles_64[i]));
        return;
    }
    if (kind != 1) return;
    void* const bag = read(properties.bag_08);
    std::uint32_t role = 9;
    if (b.call_0048e9f0(bag, "OwnerPlayer") != 0) {
        void* const property = b.call_008f2260(bag, "OwnerPlayer");
        const auto record = b.property_record(property);
        role = *static_cast<const volatile std::uint32_t*>(record.payload_0c);
    }
    const auto role_entry = b.primary_table(unit)[0x144 / 4];
    b.call_unit_144(role_entry, unit, role);
    // Reload holder and bag after virtual144. Native adds no repeated kind gate.
    void* const current_holder = read(u.property_holder_c0);
    void* const current_bag = read(b.property_holder(current_holder).bag_08);
    void* const invincible = b.call_008f2260(current_bag, "Invincible");
    if (!invincible) return;
    auto record = b.property_record(invincible);
    const auto type = read(record.type_04);
    const void* payload = record.payload_0c;
    float value;
    if (type == 0) {
        __asm {
            mov eax, payload
            cvtsi2ss xmm0, dword ptr [eax]
            movss dword ptr [value], xmm0
        }
    } else {
        __asm {
            mov eax, payload
            movss xmm0, dword ptr [eax]
            movss dword ptr [value], xmm0
        }
    }
    const auto invincible_entry = b.primary_table(unit)[0xf4 / 4];
    __asm {
        fld dword ptr [value]
        fstp dword ptr [value]
    }
    b.call_unit_f4(invincible_entry, unit, value);
}
} // namespace bsp
