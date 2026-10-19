#include "bsp/oct18_gunnery_target.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_particle_object_state.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Original x87/register schedules require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
template<class T> T read(const void* p, U offset) {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p)+offset);
}
template<class T> void write(void* p, U offset, T value) {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p)+offset)=value;
}
void* at(void* p, U offset) { return static_cast<std::byte*>(p)+offset; }
const void* at(const void* p, U offset) { return static_cast<const std::byte*>(p)+offset; }
void copy_float(void* destination, const void* source) {
    __asm { mov eax, source }
    __asm { mov edx, destination }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [edx] }
}

// 00864930..008649AF. DCC8 at 00864979 is FMUL ST0,ST0 (the Ghidra
// listing misprints its opcode class). Decode the PE bytes, not that listing.
__declspec(noinline) float distance_x87(const void* owner, const void* target,
    const volatile double* cutoff, const CameraAxesCrtAccess* crt) {
    float delta_x,delta_y,delta_z,square,result;
    __asm {
        mov eax, owner
        mov edx, target
        fld dword ptr [eax+0fch]
        fsub dword ptr [edx+0fch]
        fstp delta_x
        fld dword ptr [eax+100h]
        fsub dword ptr [edx+100h]
        fstp delta_y
        fld dword ptr [eax+104h]
        fsub dword ptr [edx+104h]
        fstp delta_z
        fld delta_y
        fld delta_x
        fld delta_z
        fld st(1)
        fmulp st(2),st(0)
        fld st(2)
        fmulp st(3),st(0)
        fxch st(1)
        faddp st(2),st(0)
        // Retain the actual DC form, independently byte-auditable.
        _emit 0dch
        _emit 0c8h
        faddp st(1),st(0)
        fstp square
        mov eax, cutoff
        fld qword ptr [eax]
        fld square
        fcomi st(0),st(1)
        fstp st(1)
        jbe distance_zero
        mov ecx, crt
        call native_crt_sqrt_st0_00bf7030
        fstp result
        fld result
        fstp result
        jmp distance_done
    distance_zero:
        xorps xmm0,xmm0
        fstp st(0)
        movss result,xmm0
    distance_done:
    }
    return result;
}

__declspec(noinline) float cap_x87(const void* row, float distance, U maximum_bits) {
    float scaled,result;
    __asm {
        mov eax,row
        fld dword ptr [eax+10h]
        fmul distance
        fstp scaled
        fld scaled
        fld maximum_bits
        fcomip st(0),st(1)
        fstp st(0)
        jbe cap_maximum
        movss xmm0,scaled
        jmp cap_done
    cap_maximum:
        movss xmm0,maximum_bits
    cap_done:
        movss result,xmm0
    }
    return result;
}

// Original 008649E6..00864A36. Even a zero-width interval consumes its draw.
void draw_radius(void* record, const void* row, float cap,
    const NativeParticleUnitRandomAccess* random) {
    float multiplier,minimum,maximum;
    __asm {
        mov eax,row
        fld dword ptr [eax+14h]
        fstp multiplier
        fld cap
        fst maximum
        fmul multiplier
        fstp minimum
        sub esp,8
        fld maximum
        fstp dword ptr [esp+4]
        fld minimum
        fstp dword ptr [esp]
        mov ecx,1
        mov edx,random
        call native_particle_random_range_00bd2f10
        mov eax,record
        fstp dword ptr [eax+70h]
    }
}
void draw_bearing(void* record, const volatile float* full_angle,
    const NativeParticleUnitRandomAccess* random) {
    __asm {
        sub esp,8
        mov eax,full_angle
        fld dword ptr [eax]
        fstp dword ptr [esp+4]
        fldz
        fstp dword ptr [esp]
        mov ecx,1
        mov edx,random
        call native_particle_random_range_00bd2f10
        mov eax,record
        fstp dword ptr [eax+6ch]
    }
}

// Writes x, y, z sequentially. Do not snapshot the velocity vector: the
// returned pointer is allowed to alias a scratch destination or the record.
void scale_velocity(void* scratch, const float* velocity, const float* time) {
    __asm {
        mov eax,velocity
        mov edx,scratch
        mov ecx,time
        fld dword ptr [eax]
        fld dword ptr [ecx]
        fld st(0)
        fmulp st(2),st(0)
        fxch st(1)
        fstp dword ptr [edx]
        fld dword ptr [eax+4]
        fmul st(0),st(1)
        fstp dword ptr [edx+4]
        fmul dword ptr [eax+8]
        fstp dword ptr [edx+8]
    }
}
void subtract_previous(void* record, const void* scratch) {
    __asm {
        mov eax,record
        mov edx,scratch
        fld dword ptr [eax+50h]
        fsub dword ptr [edx]
        fstp dword ptr [eax+50h]
        fld dword ptr [eax+54h]
        fsub dword ptr [edx+4]
        fstp dword ptr [eax+54h]
        fld dword ptr [eax+58h]
        fsub dword ptr [edx+8]
        fstp dword ptr [eax+58h]
    }
}
void predict_position(void* record, const void* target, void* scratch) {
    __asm {
        mov eax,target
        mov edx,scratch
        fld dword ptr [eax+0fch]
        fadd dword ptr [edx]
        fstp dword ptr [edx+0ch]
        fld dword ptr [eax+100h]
        fadd dword ptr [edx+4]
        fstp dword ptr [edx+10h]
        fld dword ptr [eax+104h]
        fadd dword ptr [edx+8]
        fstp dword ptr [edx+14h]
        mov eax,record
        fld dword ptr [edx+0ch]
        fstp dword ptr [eax+60h]
        fld dword ptr [edx+10h]
        fstp dword ptr [eax+64h]
        fld dword ptr [edx+14h]
        fstp dword ptr [eax+68h]
        xorps xmm0,xmm0
        movss dword ptr [eax+64h],xmm0
    }
}

struct ObserverUnwind {
    const RecoveredGunneryTargetCalls& calls;
    void* record;
    bool active=true;
    ~ObserverUnwind() noexcept {
        if(active)calls.destroy_callback_owner_00695870(calls.context,record);
    }
};
} // namespace

void* construct_recovered_gunnery_target_00864880(void* record,
    void* owner, void* target, const RecoveredGunneryTargetAccess& access) {
    const auto& calls=*access.calls;
    write<U>(record,4,0);
    write<U>(record,8,0);
    write<U>(record,0xc,0);
    write<std::uint8_t>(record,0x10,0);
    write<U>(record,0,0x00d0d2fc);
    write<void*>(record,0x18,owner);
    const U level=read<U>(owner,0x390);
    ObserverUnwind unwind{calls,record}; // Original state 0, before registration.
    write<U>(record,0x14,level);
    write<void*>(record,0x1c,target);
    calls.register_observer_00694a60(calls.context,target,record);

    const U clock=read<U>(const_cast<const float*>(access.clock_00f876a4),0);
    const U captured_level=read<U>(record,0x14);
    void* const captured_target=read<void*>(record,0x1c);
    write<U>(record,0x84,clock);
    copy_float(at(record,0x80),at(record,0x84));
    const bool target_ready=read<std::uint8_t>(captured_target,0xc8)!=0;
    void* const initial_rows=*access.rows_00e19994;
    const U maximum=read<U>(initial_rows,captured_level*0x28u+0xc);
    if(!target_ready)calls.refresh_world_00414db0(calls.context,captured_target);
    void* const captured_owner=read<void*>(record,0x18);
    if(!read<std::uint8_t>(captured_owner,0xc8))
        calls.refresh_world_00414db0(calls.context,captured_owner);
    const float distance=distance_x87(captured_owner,captured_target,
                                    access.length_cutoff_00ce3820,access.crt);
    const U current_level=read<U>(record,0x14);
    const void* const row=at(*access.rows_00e19994,current_level*0x28u);
    const float cap=cap_x87(row,distance,maximum);
    draw_radius(record,row,cap,access.random);
    draw_bearing(record,access.full_angle_00ce3d9c,access.random);

    // Original B+10h..B+33h scratch. In particular the axis and first velocity
    // share B+1Ch; products occupy B+10h..18h and the second output starts B+28h.
    alignas(4) std::byte scratch[0x24];
    const U radius=read<U>(record,0x70);
    write<U>(record,0x7c,radius);
    const U one=read<U>(const_cast<const float*>(access.one_00d7a24c),0);
    write<U>(record,0x74,0);
    write<U>(record,0x78,0);
    float angle;
    copy_float(&angle,at(record,0x6c));
    write<U>(scratch,0xc,0);
    write<U>(scratch,0x10,one);
    write<U>(scratch,0x14,0);
    calls.rotate_0085c3f0(calls.context,at(record,0x74),at(record,0x74),
                        reinterpret_cast<const float*>(scratch+0xc),angle);
    void* const config=calls.global_config_00432650(calls.context);
    void* current_target=read<void*>(record,0x1c);
    const bool first_ready=read<std::uint8_t>(current_target,0xc8)!=0;
    const U time_bits=read<U>(config,0x88);
    if(!first_ready)calls.refresh_world_00414db0(calls.context,current_target);
    copy_native_camera_matrix_004134f0(at(record,0x20),nullptr,at(current_target,0xcc));
    current_target=read<void*>(record,0x1c);
    const float* velocity=calls.velocity_slot34(calls.context,current_target,
                        reinterpret_cast<float*>(scratch+0xc));
    const auto* const time=reinterpret_cast<const float*>(&time_bits);
    scale_velocity(scratch,velocity,time);
    subtract_previous(record,scratch);
    current_target=read<void*>(record,0x1c);
    velocity=calls.velocity_slot34(calls.context,current_target,
                        reinterpret_cast<float*>(scratch+0x18));
    // EDI and its dirty test are captured AFTER slot34, before the product stores.
    current_target=read<void*>(record,0x1c);
    const bool second_ready=read<std::uint8_t>(current_target,0xc8)!=0;
    scale_velocity(scratch,velocity,time);
    if(!second_ready)calls.refresh_world_00414db0(calls.context,current_target);
    predict_position(record,current_target,scratch);
    unwind.active=false;
    return record;
}
} // namespace bsp
