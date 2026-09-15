#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Genuine raw 0Ch pointer-vector header: +0 data, +4 signed count, +8 signed
// capacity. The Particle component embeds this at +28h in its actual 34h owner.
// ECX header, stack signed request, RET4. Reserve clamps to one and copies
// pointers without retain/release. Resize zeroes added slots; shrinking only
// decrements count. Allocation sizes and pointer arithmetic wrap as DWORDs.
void reserve_native_particle_component_resources_0086a4d0(void* actual_header,
    std::int32_t requested_capacity);
void resize_native_particle_component_resources_0086abb0(void* actual_header,
    std::int32_t requested_count);

// 0086B6C0: ECX actual header, RET. resize(0), free current data; retain the
// dangling data and capacity fields. This is also BB80's state2 unwind action.
void destroy_native_particle_component_resources_0086b6c0(void* actual_header);

// Raw-pool overload of the existing component base body. ECX actual owner, RET.
// Stamp D0D570, release actual name+8, then BD30F0 stamps CEB130. Base cleanup
// also runs if the string pool getter throws. No host NativeStringStorage owner.
void destroy_effect_component_base_0086b7e0(void* actual_component,
    NativeStringRawPoolContext& strings);

// 0086BB80: ECX actual 34h component, RET. Stamp D0D5B4; release resource refs
// in reverse via InterlockedDecrement(+4) and CURRENT resource vtable slot0 on
// zero; reread live count after dispatch. Then vector, name+20h, base cleanup.
// Actual resource vtables must contain executable Win32 __thiscall entries.
// Unknown bytes, dangling headers and callback writes survive as in the native
// body. No replacement owner, implicit release, null guard or facade is added.
void destroy_native_particle_component_0086bb80(void* actual_component,
    NativeStringRawPoolContext& strings);

// 0086BC60: ECX owner, stack flags, EAX original owner, RET4. After successful
// destruction only, flag bit0 frees the allocation. Remaining bits are ignored.
void* scalar_delete_native_particle_component_0086bc60(void* actual_component,
    std::uint32_t flags, NativeStringRawPoolContext& strings);

// These owning source interfaces compose the concrete raw pool and native heap
// providers. C++ unwinding follows the recovered FH3 cleanup states, including
// termination on a second cleanup exception. They are not original FH3 frames
// or drop-in binary/SEH entry points; CRT faults/OOM and game parity are unproved.
} // namespace bsp
