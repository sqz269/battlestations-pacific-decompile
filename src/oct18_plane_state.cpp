#include "bsp/oct18_plane_state.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Recovered x87/register contracts require MSVC Win32.
#endif

namespace bsp {
namespace {
using U=std::uint32_t;
template<class T> T read(const void* p,U offset) {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p)+offset);
}
template<class T> void write(void* p,U offset,T value) {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p)+offset)=value;
}
void* at(void* p,U offset) { return static_cast<std::byte*>(p)+offset; }
const void* at(const void* p,U offset) { return static_cast<const std::byte*>(p)+offset; }
bool flying(U state) { return state==7||state==6||state==4||state==5; }
void copy_vector_x87(void* destination,const void* source) {
    __asm {
        mov eax,source
        mov edx,destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax+4]
        fstp dword ptr [edx+4]
        fld dword ptr [eax+8]
        fstp dword ptr [edx+8]
    }
}
void copy_controls(void* unit,const void* message) {
    // Two passes, no snapshot/memcpy: later reads can see earlier stores.
    for(U offset=0;offset<24;offset+=4)
        write<U>(unit,0x9e4+offset,read<U>(message,0x38+offset));
    for(U offset=0;offset<24;offset+=4)
        write<U>(unit,0x9fc+offset,read<U>(message,0x38+offset));
}

// f maps original ESP-after-PUSH-EBP +10h..+9Bh, exactly 8Ch bytes. Keep
// all saved vectors and the old factor as DWORDs, not converted C++ floats.
void save_and_reset(void* unit,void* f,const volatile U* zero) {
    __asm {
        mov esi,unit
        mov edx,f
        mov edi,zero
        movss xmm0,dword ptr [esi+4c0h]
        movss dword ptr [edx+8],xmm0
        movss xmm0,dword ptr [esi+4c4h]
        movss dword ptr [edx+0ch],xmm0
        movss xmm0,dword ptr [esi+4c8h]
        movss dword ptr [edx+10h],xmm0
        movss xmm0,dword ptr [esi+4cch]
        movss dword ptr [edx+28h],xmm0
        movss xmm0,dword ptr [esi+4d0h]
        movss dword ptr [edx+2ch],xmm0
        movss xmm0,dword ptr [esi+4d4h]
        movss dword ptr [edx+30h],xmm0
        movss xmm0,dword ptr [esi+4dch]
        movss xmm1,dword ptr [edi]
        movss dword ptr [esi+4cch],xmm1
        fld dword ptr [esi+4cch]
        movss xmm1,dword ptr [edi+4]
        movss dword ptr [esi+4d0h],xmm1
        movss xmm1,dword ptr [edi+8]
        movss dword ptr [esi+4d4h],xmm1
        fstp dword ptr [esi+4b4h]
        fld dword ptr [esi+4d0h]
        fstp dword ptr [esi+4b8h]
        movss dword ptr [edx+4],xmm0
        fld dword ptr [esi+4d4h]
        fstp dword ptr [esi+4bch]
        movss xmm1,dword ptr [edi]
        movss dword ptr [esi+4c0h],xmm1
        fld dword ptr [esi+4c0h]
        movss xmm1,dword ptr [edi+4]
        movss dword ptr [esi+4c4h],xmm1
        movss xmm1,dword ptr [edi+8]
        movss dword ptr [esi+4c8h],xmm1
        fstp dword ptr [esi+4a8h]
        fld dword ptr [esi+4c4h]
        xorps xmm1,xmm1
        fstp dword ptr [esi+4ach]
        fld dword ptr [esi+4c8h]
        fstp dword ptr [esi+4b0h]
        movss dword ptr [esi+4dch],xmm1
    }
}

bool above_restore_cutoff(U old_bits,const volatile float* cutoff) {
    bool result;
    __asm { mov eax,cutoff }
    __asm { movss xmm0,old_bits }
    __asm { comiss xmm0,dword ptr [eax] }
    __asm { seta al }
    __asm { mov result,al }
    return result;
}

bool clamp_correction(void* unit,void* f,const RecoveredPlaneStateAccess& a) {
    const auto entry=a.math->clamp_00415620;
    const auto one=a.one_00d7a24c;
    const auto correction_offset=a.correction_offset_00d7a308;
    bool result;
    __asm {
        mov esi,unit
        mov edx,f
        fld dword ptr [esi+908h]
        mov eax,one
        movss xmm0,dword ptr [eax]
        mov eax,correction_offset
        // Original DC25: FSUB qword [00D7A308], not a float subtraction.
        fsub qword ptr [eax]
        movss dword ptr [edx+18h],xmm0
        xorps xmm0,xmm0
        lea eax,[edx+18h]
        push eax
        fstp dword ptr [edx+14h]
        movss dword ptr [edx+4],xmm0
        lea ecx,[edx+14h]
        add edx,4
        call entry
        mov edx,f
        fstp dword ptr [edx+14h]
        fld dword ptr [edx+14h]
        fst dword ptr [esi+4dch]
        fldz
        fxch st(1)
        fcomip st(0),st(1)
        fstp st(0)
        seta al
        mov result,al
    }
    return result;
}

void position_delta(void* unit,void* f) {
    __asm {
        mov eax,unit
        mov edx,f
        fld dword ptr [edx+1ch]
        fsub dword ptr [eax+6a4h]
        fstp dword ptr [edx+8]
        fld dword ptr [edx+20h]
        fsub dword ptr [eax+6a8h]
        fstp dword ptr [edx+0ch]
        fld dword ptr [edx+24h]
        fsub dword ptr [eax+6ach]
        fstp dword ptr [edx+10h]
    }
    copy_vector_x87(at(unit,0x4a8),at(f,8));
}
bool length_exceeds(const void* vector,const RecoveredPlaneStateAccess& a) {
    const auto entry=a.math->length_squared_00427e30;
    const auto threshold=a.correction_distance_sq_00ce3d64;
    bool result;
    __asm {
        mov ecx,vector
        call entry
        mov eax,threshold
        fld dword ptr [eax]
        fxch st(1)
        fcomip st(0),st(1)
        fstp st(0)
        seta al
        mov result,al
    }
    return result;
}
void store_angle(void* destination,const void* left,const void* right,
                 PlaneStateWrappedAngleEntry entry) {
    __asm {
        sub esp,8
        mov eax,right
        fld dword ptr [eax]
        fstp dword ptr [esp+4]
        mov eax,left
        fld dword ptr [eax]
        fstp dword ptr [esp]
        call entry
        mov eax,destination
        fstp dword ptr [eax]
    }
}
void zero_delta_bits(void* unit,const volatile U* zero) {
    write<U>(unit,0x4a8,zero[0]);
    write<U>(unit,0x4ac,zero[1]);
    write<U>(unit,0x4b0,zero[2]);
}
void reset_after_update(void* unit,const volatile U* zero) {
    __asm {
        mov eax,unit
        mov edx,zero
        movss xmm0,dword ptr [edx]
        movss dword ptr [eax+4c0h],xmm0
        fld dword ptr [eax+4c0h]
        movss xmm0,dword ptr [edx+4]
        movss dword ptr [eax+4c4h],xmm0
        movss xmm0,dword ptr [edx+8]
        movss dword ptr [eax+4c8h],xmm0
        fstp dword ptr [eax+4a8h]
        fld dword ptr [eax+4c4h]
        xorps xmm0,xmm0
        fstp dword ptr [eax+4ach]
        fld dword ptr [eax+4c8h]
        fstp dword ptr [eax+4b0h]
        fld dword ptr [eax+4a8h]
        fstp dword ptr [eax+4cch]
        fld dword ptr [eax+4ach]
        fstp dword ptr [eax+4d0h]
        fld dword ptr [eax+4b0h]
        fstp dword ptr [eax+4d4h]
        fld dword ptr [eax+4cch]
        fstp dword ptr [eax+4b4h]
        fld dword ptr [eax+4d0h]
        fstp dword ptr [eax+4b8h]
        fld dword ptr [eax+4d4h]
        fstp dword ptr [eax+4bch]
        movss dword ptr [eax+4dch],xmm0
        mov byte ptr [eax+6e8h],0
    }
}
} // namespace

void apply_recovered_plane_state_007d1360(void* unit,void* message,
                                        const RecoveredPlaneStateAccess& a) {
    const auto& c=*a.calls;
    alignas(4) std::byte f[0x8c]; // Original shared frame; no blanket zeroing.
    void* const game=*a.game_00e188a8;
    const bool host=game&&read<U>(game,0x1fe4)==1;
    const U state=read<U>(unit,0x900);
    const bool active=flying(state);
    write<std::uint8_t>(f,3,active?1:0); // Original B+0Fh / T+13h.
    if(host&&(!active||read<std::uint8_t>(message,0x8c)!=state))return;
    if(!host&&active&&c.local_role_00927f30(c.context,unit,1)
       &&read<std::uint8_t>(unit,0x6e8)==0) {
        write<std::uint8_t>(f,2,1);
        if(!read<std::uint8_t>(unit,0x9e1)) {
            write<std::uint8_t>(unit,0x9e1,1);
            write<std::uint8_t>(f,2,0);
        }
    } else {
        write<std::uint8_t>(f,2,0);
        write<std::uint8_t>(unit,0x9e1,0);
    }
    if(!read<std::uint8_t>(unit,0x9e1)&&read<std::uint8_t>(message,0x9d))return;
    if(!host) {
        // MOVZX/MOV: integer ID bits, NOT a conversion to float.
        write<U>(f,4,read<std::uint16_t>(unit,0x174));
        c.enqueue_id_007cf380(c.context,a.received_ids_00f87284,
                            reinterpret_cast<const U*>(f+4));
    }
    write<U>(f,0x18,read<U>(message,0xc)-*a.clock_ticks_00f876b0);
    void* const found=c.find_state_0077c6b0(c.context,unit,read<U>(message,0x14));
    c.serialize_007c2af0(c.context,message,unit,found);
    if(!host&&read<std::uint8_t>(message,0x9f))
        c.gravity_007d8180(c.context,at(unit,0xab0),1);
    void* child=read<void*>(unit,0x48);
    write<std::uint8_t>(unit,0xc8,0);
    write<std::uint8_t>(unit,0x10c,0);
    while(child) {
        c.invalidate_0042ed50(c.context,child);
        child=read<void*>(child,0x44);
    }
    if(!read<std::uint8_t>(unit,0xc8))c.refresh_world_00414db0(c.context,unit);
    for(U i=0;i<12;i+=4)write<U>(f,0x1c+i,read<U>(unit,0xfc+i));
    c.velocity_slot34(c.context,unit,reinterpret_cast<float*>(f+0x40));
    if(!read<std::uint8_t>(unit,0xc8))c.refresh_world_00414db0(c.context,unit);
    c.angles_0042d2e0(c.context,at(unit,0xcc),reinterpret_cast<float*>(f+0x34),
                    reinterpret_cast<float*>(f+0x38),reinterpret_cast<float*>(f+0x3c));
    save_and_reset(unit,f,a.zero_vector_00f87574);
    if(!host) {
        const auto mode=read<std::uint8_t>(message,0x9e);
        const auto next_state=read<std::uint8_t>(message,0x8c);
        c.set_state_007c78a0(c.context,unit,next_state,mode);
        if(!flying(read<U>(unit,0x900))) {
            write<std::uint8_t>(unit,0x9e1,0);
            void* matrix=c.identity_007bdf50(c.context,message,reinterpret_cast<float*>(f+0x4c));
            matrix=copy_native_camera_matrix_004134f0(at(unit,0x674),nullptr,matrix);
            copy_native_camera_matrix_004134f0(at(unit,0x4e0),nullptr,matrix);
            copy_controls(unit,message);
            copy_native_camera_matrix_004134f0(at(unit,0x74),nullptr,at(unit,0x674));
            return;
        }
    }
    const bool restore=read<std::uint8_t>(f,2)!=0;
    write<std::uint8_t>(unit,0xc49,1);
    if(restore) {
        const U old_factor=read<U>(f,4);
        if(above_restore_cutoff(old_factor,a.restore_cutoff_00d7a218)) {
            for(U i=0;i<12;i+=4)write<U>(unit,0x4c0+i,read<U>(f,8+i));
            for(U i=0;i<12;i+=4)write<U>(unit,0x4cc+i,read<U>(f,0x28+i));
            write<U>(unit,0x4dc,old_factor);
            c.restore_slot1dc(c.context,unit,0.0f);
        }
        write<std::uint8_t>(unit,0xc49,0);
        return;
    }
    write<std::uint8_t>(unit,0x9e0,1);
    copy_controls(unit,message);
    void* matrix=c.identity_007bdf50(c.context,message,reinterpret_cast<float*>(f+0x4c));
    matrix=copy_native_camera_matrix_004134f0(at(unit,0x674),nullptr,matrix);
    copy_native_camera_matrix_004134f0(at(unit,0x4e0),nullptr,matrix);
    if(!read<std::uint8_t>(f,3))
        copy_native_camera_matrix_004134f0(at(unit,0x74),nullptr,at(unit,0x674));
    c.copy_state_007baaf0(c.context,f+0x4c,at(message,0x50));
    c.apply_controller_007db1f0(c.context,at(unit,0xab0),f+0x4c);
    c.invalidate_0042ed50(c.context,unit);
    c.update_007c18b0(c.context,unit,0.0f);
    const U delta=read<U>(f,0x18);
    if(delta) {
        const U negative=0u-delta;
        std::int32_t signed_negative;
        std::memcpy(&signed_negative,&negative,4);
        c.fixed_steps_008746a0(c.context,at(unit,0x310),signed_negative);
    } else c.simulation_slot04(c.context,at(unit,0x310),0.0f);
    if(c.controller_slot38(c.context,at(unit,0x72c))&&clamp_correction(unit,f,a)) {
        position_delta(unit,f);
        if(length_exceeds(at(unit,0x4a8),a)||read<std::int32_t>(message,0x90)>5)
            zero_delta_bits(unit,a.zero_vector_00f87574);
        copy_vector_x87(at(unit,0x4c0),at(unit,0x4a8));
        const void* world=c.world_matrix_0042d7e0(c.context,unit);
        c.angles_0042d2e0(c.context,world,reinterpret_cast<float*>(f+8),
                        reinterpret_cast<float*>(f+0xc),reinterpret_cast<float*>(f+0x10));
        store_angle(at(unit,0x4b4),f+0x34,f+8,a.math->wrapped_angle_00438b10);
        store_angle(at(unit,0x4b8),f+0x38,f+0xc,a.math->wrapped_angle_00438b10);
        store_angle(at(unit,0x4bc),f+0x3c,f+0x10,a.math->wrapped_angle_00438b10);
        copy_vector_x87(at(unit,0x4cc),at(unit,0x4b4));
    }
    c.update_007c18b0(c.context,unit,0.0f);
    if(read<std::uint8_t>(unit,0x6e8))reset_after_update(unit,a.zero_vector_00f87574);
    write<std::uint8_t>(unit,0x9e0,0);
    write<std::uint8_t>(unit,0xc49,0);
}
} // namespace bsp
