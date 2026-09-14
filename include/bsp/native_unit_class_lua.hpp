#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {
// Complete009292B0..00929454[421], native ECX=actual entity, stack int32
// class_id/const char* name, RET8. Borrow the SAME actual entity NativeString
// at+178 and application's actual E188A8 and native string pool publications.
// Self comes from00927B40; after ClassID and Name callbacks, reload the world
// for VehicleClass[class_id]. All objects retain stable tracked stack identity.
void bind_native_unit_class_lua_009292b0(
    const NativeString& actual_entity_key_178, std::int32_t class_id,
    const char* name, void* volatile& actual_world_publication_00e188a8,
    NativeStringRawPoolContext& strings);
// C++ unwind reproduces completed-object cleanup and the native state changes
// before normal cleanup calls. Original FH3, Lua longjmp, simultaneous cleanup
// exceptions, hardware faults and arbitrary native stack aliases are unproved.
} // namespace bsp
