#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Complete216B native868BF0: ECX component, stack14h Lua object pointer, RET4.
// Actual tracked stack objects and concrete Lua getters; Autostart+10, numeric
// Delay+14 default0, numeric NoFilterDist+18 with CURRENT+18 captured after
// field lookup. The borrowed CRT conversion mode is read by B66380 only after
// numeric conversion. Three EH states clean only their completed Lua object.
void read_native_effect_component_base_00868bf0(
    void* actual_component, NativeLuaObjectStorage& definition,
    const bool& actual_crt_sse2_conversion);

// Complete10B AF3E10: ECX actual particle resource, byte stack argument, RET4.
// Store the entire supplied byte at+70; no truthiness normalization or reads.
void set_native_particle_resource_underwater_00af3e10(
    void* actual_resource, std::uint8_t value) noexcept;

// Source interfaces only; no original thiscall/FH3/SEH or gameplay claim.
} // namespace bsp
