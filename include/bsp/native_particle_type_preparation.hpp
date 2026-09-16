#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete B075D0..B07653. Original ECX actual Layer, two stack DWORDs forming
// an owned 8h NativeString argument, RET8. This source interface receives the
// ACTUAL consumed argument header, retaining the native alias comparison with
// layer+14h and its unwind action. The caller must not release it afterward.
// Capture incoming length/data once. Unless destination aliases that header,
// resize layer+14 using captured length, then copy captured data using CURRENT
// destination length. Normal cleanup returns captured data/length+1; an exception
// during copy/resize instead destroys the CURRENT consumed argument header.
void set_native_particle_layer_material_00b075d0(void* actual_layer,
    void* actual_consumed_name_8h, NativeStringRawPoolContext& strings);

// Complete 41-byte ECX actual particle-type preparation methods, plain RET.
// Capture type+74 index, then type+14 parent and its inline Layer slot+34+index*4
// BEFORE constructing the owned name. Construct the actual8h by-value argument
// using the full 41E870 schedule through concrete raw-pool string providers,
// then transfer it to B075D0. No layer/type/parent replacement or extra refcount.
void prepare_native_sprite_particle_type_00b0a000(void* actual_type,
    NativeStringRawPoolContext& strings);
void prepare_native_axial_particle_type_00b07660(void* actual_type,
    NativeStringRawPoolContext& strings);
void prepare_native_floating_particle_type_00b087b0(void* actual_type,
    NativeStringRawPoolContext& strings);

// Each is a genuine one-byte RET body. ECX is ignored; no fields or pointees
// are read. These are required actual vtable14/0C targets, not parser substitutes.
void __fastcall prepare_native_object_particle_type_00af8a30(void*) noexcept;
void __fastcall prepare_native_tracer_particle_type_00b0a0f0(void*) noexcept;
void __fastcall clamp_native_tracer_particle_type_00b0a100(void*) noexcept;

// The owning source interfaces add the raw pool context and an explicit
// consumed-header address. C++ cleanup follows the recovered single FH3 state,
// terminating on a second unwind exception. Original FH3/SEH identity, arbitrary
// memory faults, binary substitution and gameplay are not established.
} // namespace bsp
