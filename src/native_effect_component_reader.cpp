#include "bsp/native_effect_component_reader.hpp"

#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native effect component reading requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> volatile T& field(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(storage) + offset);
}
void load_x87_float(const void* source, float& destination) noexcept {
    float* output = &destination;
    __asm {
        mov eax, source
        mov edx, output
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
} // namespace

void read_native_effect_component_base_00868bf0(
    void* component, NativeLuaObjectStorage& definition,
    const bool& conversion_mode) {
    NativeLuaObjectStorage first;
    NativeLuaObjectStorage distance;
    NativeLuaObjectStorage* active = nullptr;
    try {
        auto* returned = native_lua_get_by_name_00b67800(definition, &first, "Autostart");
        active = &first;
        field<std::uint8_t>(component, 0x10) = native_lua_boolean_00b66250(*returned) ? 1 : 0;
        active = nullptr;
        destroy_native_lua_object_00b67700(first);

        returned = native_lua_get_by_name_00b67800(definition, &first, "Delay");
        active = &first;
        field<std::int32_t>(component, 0x14) = native_lua_integer_or_00b66380(*returned, 0, conversion_mode);
        active = nullptr;
        destroy_native_lua_object_00b67700(first);

        returned = native_lua_get_by_name_00b67800(definition, &distance, "NoFilterDist");
        float fallback;
        load_x87_float(static_cast<unsigned char*>(component) + 0x18, fallback);
        active = &distance;
        field<float>(component, 0x18) = native_lua_number_or_00b66330(*returned, fallback);
        active = nullptr;
        destroy_native_lua_object_00b67700(distance);
    } catch (...) {
        // C95170/C95178/C95180 are genuine FH3 unwind actions. Ordinary
        // destruction disarms the state before calling B67700, as above.
        if (active != nullptr) {
            try { destroy_native_lua_object_00b67700(*active); }
            catch (...) { std::terminate(); }
        }
        throw;
    }
}

void set_native_particle_resource_underwater_00af3e10(
    void* resource, std::uint8_t value) noexcept {
    field<std::uint8_t>(resource, 0x70) = value;
}
} // namespace bsp
