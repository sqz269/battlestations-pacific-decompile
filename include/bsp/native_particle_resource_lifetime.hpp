#pragma once
#include <cstdint>
namespace bsp {
struct NativeStringRawPoolContext;
// Actual resource90h: reference count+4, name header+8, eight inline emitter
// pointers+10/count+30, eight inline Layer pointers+34/count+54. Other bytes
// remain untouched. Child virtual0 is a callable thiscall terminal without flags.
// AF4280 native ECX resource, RET. Stamp D5D958, release children FORWARD using
// current signed counts, clear each count, release actual name, stamp CEB130.
// Name/base are genuine unwind actions; second cleanup exceptions terminate.
void destroy_native_particle_resource_00af4280(void*, NativeStringRawPoolContext&);
// AF46E0 native ECX resource, stack flags, EAX original pointer, RET4.
void* delete_native_particle_resource_00af46e0(void*, std::uint32_t flags, NativeStringRawPoolContext&);
}
