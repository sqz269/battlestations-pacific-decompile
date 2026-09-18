#include "bsp/native_game_grid.hpp"
#include "bsp/camera_decomposition.hpp"
#include <array>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include "bsp/native_vertex_declaration_loading.hpp"
#include "bsp/native_string_pool_storage.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U bits(const void* p) noexcept {return static_cast<U>(reinterpret_cast<std::uintptr_t>(p));}
void* at(const void* p,U offset=0) noexcept {return reinterpret_cast<void*>(bits(p)+offset);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* pointer(const void* p,U offset=0) noexcept {return reinterpret_cast<void*>(word(p,offset));}
std::int32_t signed_word(U value) noexcept {std::int32_t result;std::memcpy(&result,&value,4);return result;}
bool below(U value,U limit) noexcept {return signed_word(value)<signed_word(limit);}

// Keep the load/store sequence rather than assigning C++ floats: even the
// copies quiet SNaNs, and a destination may overlap a later source component.
__declspec(naked) void __fastcall copy_float(void*,const void*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
__declspec(naked) void __fastcall copy_vector(void*,const void*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [edx+8]
        fstp dword ptr [ecx+8]
        ret
    }
}
// ECX=three binary32 differences, EDX=left, stack=right. No C++ arithmetic
// intervenes between each x87 subtraction and its native binary32 spill.
__declspec(naked) void __fastcall difference(void*,const void*,const void*) {
    __asm {
        mov eax,dword ptr [esp+4]
        fld dword ptr [edx]
        fsub dword ptr [eax]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fsub dword ptr [eax+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [edx+8]
        fsub dword ptr [eax+8]
        fstp dword ptr [ecx+8]
        ret 4
    }
}
// ECX=normal, EDX={row difference xyz, column difference xyz}. Preserve the
// exact 709D99..709DDA stack order and extended products, including NaN operand
// selection; independent C++ expressions can change both rounding and NaNs.
__declspec(naked) void __fastcall cross(void*,const void*) {
    __asm {
        fld dword ptr [edx+20]
        fld st(0)
        fld dword ptr [edx+4]
        fld st(0)
        fmulp st(2),st(0)
        fld dword ptr [edx+16]
        fld st(0)
        fld dword ptr [edx+8]
        fld st(0)
        fmulp st(2),st(0)
        fxch st(4)
        fsubrp st(1),st(0)
        fstp dword ptr [ecx]
        fld dword ptr [edx+12]
        fld st(0)
        fmulp st(4),st(0)
        fld dword ptr [edx]
        fld st(0)
        fmulp st(6),st(0)
        fxch st(4)
        fsubrp st(5),st(0)
        fxch st(4)
        fstp dword ptr [ecx+4]
        fmulp st(2),st(0)
        fmulp st(2),st(0)
        fsubrp st(1),st(0)
        fstp dword ptr [ecx+8]
        ret
    }
}
}

void NativeGameGridCpuCalls::free_00bf6989(void* allocation){std::free(allocation);}

void* copy_native_game_grid_descriptor_00709c90(void* destination,const void* source) {
    put(destination,0,word(source));put(destination,4,word(source,4));
    for(U offset=8;offset!=0x38;offset+=4)copy_float(at(destination,offset),at(source,offset));
    return destination;
}

void calculate_native_game_grid_normals_00709cf0(void* grid) {
    U row=1;
    if(below(row,word(grid,0x40)-1u)) {
        U column_limit=word(grid,0x3c)-1u;
        do {
            U column=1;
            if(below(column,column_limit)) {
                const U next_row=row+1u;
                do {
                    const U columns=word(grid,0x3c);
                    void* const positions=pointer(grid,8);
                    void* const after=at(positions,(next_row*columns+column)*12u);
                    void* const before=at(positions,((row-1u)*columns+column)*12u);
                    const U offset=(row*columns+column)*12u;
                    std::array<float,6> differences;
                    difference(differences.data(),before,after);
                    difference(differences.data()+3,at(positions,offset-12u),at(positions,offset+12u));
                    cross(at(pointer(grid,0x1c),offset),differences.data());
                    // Original reloads BOTH columns and output pointer after
                    // the cross stores, before calling the existing callee.
                    const U current_offset=(word(grid,0x3c)*row+column)*12u;
                    auto* const normal=static_cast<std::array<float,3>*>(at(pointer(grid,0x1c),current_offset));
                    normalize_camera_basis_0042b260(*normal);
                    column_limit=word(grid,0x3c)-1u;
                    ++column;
                } while(below(column,column_limit));
            }
            ++row;
        } while(below(row,word(grid,0x40)-1u));
    }
    U row_offset=12;
    for(row=0;below(row,word(grid,0x40));++row,row_offset+=12) {
        const U offset=word(grid,0x3c)*row*12u;
        void* const first=at(pointer(grid,0x1c),offset);
        copy_vector(first,at(first,12));
        const U end=word(grid,0x3c)*row_offset;
        void* const normals=pointer(grid,0x1c);
        copy_vector(at(normals,end-12u),at(normals,end-24u));
    }
    U byte_offset=0;
    for(U column=0;below(column,word(grid,0x3c));++column,byte_offset+=12) {
        const U next=(word(grid,0x3c)+column)*12u;
        void* const normals=pointer(grid,0x1c);
        copy_vector(at(normals,byte_offset),at(normals,next));
        const U rows=word(grid,0x40),columns=word(grid,0x3c);
        void* const before=at(pointer(grid,0x1c),((rows-2u)*columns+column)*12u);
        const U last=((rows-1u)*columns+column)*12u;
        copy_vector(at(pointer(grid,0x1c),last),before);
    }
}

void release_native_game_grid_cpu_arrays_00709760(void* grid,NativeGameGridCpuCalls& calls) {
    for(U offset=8;offset!=0x20;offset+=4) {
        void* const allocation=pointer(grid,offset);
        if(allocation)calls.free_00bf6989(allocation);
    }
    for(U offset=8;offset!=0x20;offset+=4)put(grid,offset,0);
}

void* NativeGameGridCpuCalls::allocate_00bf55be(U bytes){return ::operator new(bytes);}
void NativeGameGridCpuCalls::free_00bf65ac(void* allocation){::operator delete(allocation);}
NativeGameGridOperation::~NativeGameGridOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeGameGridOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}

namespace {
using GridOp=NativeGameGridOperation;
void require_grid(bool condition,const char* reason){if(!condition)throw std::logic_error(reason);}
void grid_begin(void* grid,NativeGameGridContext& c,GridOp& op){
    require_grid(op.phase==GridOp::Phase::fresh,"Native grid operation cannot be replayed");
    auto& s=c.graphics.streams;
    require_grid(&c.strings==&c.graphics.declarations.strings &&
        &c.strings==&c.graphics.declarations.declarations.strings &&
        &s.geometry.actual_owners()==&s.vertices.actual_owners &&
        &s.vertices.actual_renderer_00f8d394==&s.indices.lifetime.actual_renderer_00f8d394 &&
        &s.vertices.actual_renderer_00f8d394==&s.mapping.actual_renderer_00f8d394 &&
        &s.vertices.actual_synchronization_0108d6dc==&s.indices.lifetime.actual_synchronization_0108d6dc &&
        &s.vertices.actual_synchronization_0108d6dc==&s.mapping.actual_synchronization_0108d6dc &&
        &s.vertices.actual_physical==&s.indices.lifetime.actual_physical &&
        &s.vertices.actual_physical_profiles==&s.mapping.actual_physical_profiles &&
        s.vertices.actual_physical.actual_lifetime_01090aa0.borrows_same_domain(s.mapping.actual_physical_lock.actual_lifetime_01090aa0) &&
        s.vertices.actual_renderer_profile_00d5f0a8==s.indices.actual_renderer_profile_00d5f0a8 &&
        s.vertices.actual_type_sizes_00d61cc0==c.graphics.declarations.declarations.type_sizes_00d61cc0,
        "Native grid requires one actual renderer, string, owner and mapping domain");
    op.phase=GridOp::Phase::running;op.owner=grid;op.context=&c;
}
void grid_failed(NativeGameGridContext& c,GridOp& op) noexcept {
    // Only the completed local string is in 70B330's native unwind state.
    // Other partial arrays, factories, maps and references remain recorded.
    if(op.name_cleanup_active){op.name_cleanup_active=false;destroy_native_string_header_0041dd20(&op.format_name,c.strings);}
    op.phase=GridOp::Phase::failed;
}
void stream_slot(void* stream,bool vertex,U offset,U expected,NativeGameGridContext& c){
    auto& s=c.graphics.streams;
    auto* profile=vertex?s.vertices.actual_logical_profile_00d61d6c:s.indices.actual_logical_profile_00d61de0;
    require_grid(stream && word(stream)==(vertex?0x00d61d6cu:0x00d61de0u) && profile && profile[offset/4]==expected,
        "Native grid reached an unsupported current logical stream slot");
}
void publish_stream(void* grid,U offset,void* stream,NativeStreamCloneAcquired& acquired){
    put(grid,offset,bits(stream));
    // The creator reference is now owned by the actual grid field. The source
    // frame continues to record entered mapping calls but owns no second ref.
    acquired.creator=nullptr;acquired.companion=nullptr;acquired.canonical_registration=false;
    acquired.phase=NativeStreamClonePhase::consumed;
}
void release_field(void* grid,U offset,NativeGameGridContext& c){
    void* const captured=pointer(grid,offset);
    if(captured){release_native_render_actual_owner(c.graphics.streams.geometry.actual_owners(),captured);put(grid,offset,0);}
}
void* grid_map(void* stream,bool vertex,NativeGameGridContext& c,GridOp& op){
    stream_slot(stream,vertex,vertex?0x10u:0x0cu,vertex?0x00b49980u:0x00b49b60u,c);
    auto& acquired=vertex?op.vertex:op.index;
    acquired.destination_map=NativeStreamCloneMapPhase::call_in_progress;
    acquired.destination_mapping=vertex?lock_native_logical_vertex_stream_00b49980(stream,c.graphics.streams.mapping,0,0,0):
        lock_native_logical_index_stream_00b49b60(stream,c.graphics.streams.mapping,0,0,0);
    acquired.destination_map=NativeStreamCloneMapPhase::returned;
    return acquired.destination_mapping;
}
void grid_unmap(void* stream,bool vertex,NativeGameGridContext& c,GridOp& op){
    stream_slot(stream,vertex,vertex?0x14u:0x10u,vertex?0x00b49a80u:0x00b49c70u,c);
    auto& acquired=vertex?op.vertex:op.index;
    acquired.destination_map=NativeStreamCloneMapPhase::unlock_in_progress;
    if(vertex)unlock_native_logical_vertex_stream_00b49a80(stream,c.graphics.streams.mapping);
    else unlock_native_logical_index_stream_00b49c70(stream,c.graphics.streams.mapping);
    acquired.destination_map=NativeStreamCloneMapPhase::unlocked;
}
void grid_steps(void* grid){
    __asm {
        mov edx,grid
        fld dword ptr [edx+0x48]
        mov eax,dword ptr [edx+0x40]
        lea ecx,[eax-1]
        push ecx
        fidiv dword ptr [esp]
        mov eax,dword ptr [edx+0x3c]
        fstp dword ptr [edx+0x60]
        sub eax,1
        mov dword ptr [esp],eax
        fld dword ptr [edx+0x44]
        fidiv dword ptr [esp]
        fstp dword ptr [edx+0x5c]
        add esp,4
    }
}
void cpu_position(void* grid,U row,U column,void* output){
    const U negative_row=0u-row,negative_column=0u-column;
    float y,z,row_float;
    __asm {
        mov eax,negative_row
        cvtsi2ss xmm0,eax
        movss row_float,xmm0
        mov edx,grid
        fld row_float
        fmul dword ptr [edx+0x60]
        fstp y
        fild negative_column
        fmul dword ptr [edx+0x5c]
        fstp z
        mov eax,output
        fld y
        mov dword ptr [eax],0
        fstp dword ptr [eax+4]
        fld z
        fstp dword ptr [eax+8]
    }
}
void curve_row(void* grid,U row,const volatile double* period,const volatile double* amplitude){
    float value,sine;
    __asm {
        mov edx,grid
        fild row
        mov eax,period
        fmul qword ptr [eax]
        fidiv dword ptr [edx+0x40]
        fstp value
        fld value
        fsin
        fstp sine
        fld dword ptr [edx+0x44]
        fchs
        mov eax,amplitude
        fmul qword ptr [eax]
        fmul sine
        fstp value
        fld value
        mov eax,dword ptr [edx+0x3c]
        imul eax,row
        lea eax,[eax+eax*2]
        mov edx,dword ptr [edx+8]
        fstp dword ptr [edx+eax*4+8]
    }
}
void gpu_position(void* grid,U index,U count){
    const auto rows=signed_word(word(grid,0x40));
    const U negative_row=0u-static_cast<U>(signed_word(index)/rows);
    const U negative_column=0u-static_cast<U>(signed_word(index)%rows);
    float y,z;
    __asm {
        mov edx,grid
        fild negative_row
        fmul dword ptr [edx+0x60]
        fstp y
        fild negative_column
        fmul dword ptr [edx+0x5c]
        fstp z
    }
    void* stream=pointer(grid,0x78);
    void* front=at(pointer(stream,8),word(stream,0xc)*index+word(stream,0x10));
    copy_float(at(front,4),&y);put(front,0,0);copy_float(at(front,8),&z);
    stream=pointer(grid,0x78);
    void* back=at(pointer(stream,8),word(stream,0xc)*(index+count)+word(stream,0x10));
    copy_float(at(back,4),&y);put(back,0,0);copy_float(at(back,8),&z);
}
void grid_uv(void* grid,U index,U* output){
    const auto columns=signed_word(word(grid,0x3c));
    const U quotient=static_cast<U>(signed_word(index)/columns),remainder=static_cast<U>(signed_word(index)%columns);
    const U column_divisor=static_cast<U>(columns)-1u,row_divisor=word(grid,0x40)-1u;
    float start_u,start_v,fraction;
    __asm {
        mov edx,grid
        fld1
        fldz
        fsub st(1),st(0)
        fld dword ptr [edx+0x64]
        fstp start_u
        fild remainder
        fidiv column_divisor
        fstp fraction
        fld fraction
        fsub st(0),st(1)
        fld dword ptr [edx+0x6c]
        fld start_u
        fld st(0)
        fsubp st(2),st(0)
        fxch st(1)
        fdiv st(0),st(4)
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fstp start_u
        fld dword ptr [edx+0x68]
        fstp start_v
        fild quotient
        fidiv row_divisor
        fstp fraction
        fld fraction
        fsub st(0),st(1)
        fld dword ptr [edx+0x70]
        fld start_v
        fld st(0)
        fsubp st(2),st(0)
        fxch st(1)
        fdiv st(0),st(4)
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fstp start_v
        mov eax,output
        mov ecx,start_u
        mov dword ptr [eax],ecx
        mov ecx,start_v
        mov dword ptr [eax+4],ecx
        fstp st(0)
        fstp st(0)
    }
}
void grid_color(void* grid,U index,U white){
    void* stream=pointer(grid,0x78);const U packed=word(stream,0x34);
    void* destination=at(pointer(stream,8),word(stream,0xc)*index);
    if(signed_word(packed)>=0)put(destination,packed,0xffffffffu);
    else for(U offset=0x38;offset!=0x48;offset+=4)put(destination,word(stream,offset),white);
}
void emit_index(void*& destination,U value){*static_cast<volatile std::uint16_t*>(destination)=static_cast<std::uint16_t>(value);destination=at(destination,2);}
void initialize_grid_body(void* grid,const void* descriptor,NativeGameGridContext& c,GridOp& op){
    op.native_site=0x0070b358;copy_native_game_grid_descriptor_00709c90(at(grid,0x3c),descriptor);
    put(grid,4,word(descriptor,4)*word(descriptor));
    constexpr U sites[]={0x70b377,0x70b393,0x70b3af,0x70b3cb,0x70b3e7,0x70b403};
    for(U i=0;i<6;++i){
        const std::uint64_t bytes=static_cast<std::uint64_t>(word(grid,4))*12u;
        op.native_site=sites[i];void* const array=c.cpu.allocate_00bf55be(bytes>0xffffffffu?0xffffffffu:static_cast<U>(bytes));
        put(grid,8+i*4,bits(array));
    }
    grid_steps(grid);
    for(U row=0;below(row,word(grid,0x40));++row){
        for(U column=0;below(column,word(grid,0x3c));++column){
            cpu_position(grid,row,column,at(pointer(grid,8),(word(grid,0x3c)*row+column)*12u));
            void* velocity=at(pointer(grid,0x10),(word(grid,0x3c)*row+column)*12u);
            put(velocity,0,0);put(velocity,4,0);put(velocity,8,0);
        }
        curve_row(grid,row,&c.constants.value_00ce3d28,&c.constants.value_00d7a3a0);
    }
    op.native_site=0x0070b524;calculate_native_game_grid_normals_00709cf0(grid);
    op.native_site=0x0070b539;resize_native_string_header_0041dd40(&op.format_name,c.strings,10,true);
    if(op.format_name.data())std::memmove(op.format_name.data(),c.constants.literal_00cfd4e4,op.format_name.length()+1u);
    void* renderer=const_cast<void*>(c.graphics.streams.vertices.actual_renderer_00f8d394);
    op.name_cleanup_active=true;op.native_site=0x0070b570;
    void* const declaration=load_gui_text_native_declaration_current38(renderer,op.format_name,c.graphics,op.declaration);
    op.name_cleanup_active=false;op.native_site=0x0070b591;
    destroy_native_string_header_0041dd20(&op.format_name,c.strings);
    const U count=word(grid,4);
    put(grid,0x80,(word(grid,0x40)*4u-4u)*(word(grid,0x3c)-1u));
    // EBX captures one renderer before either old stream is released.
    renderer=const_cast<void*>(c.graphics.streams.vertices.actual_renderer_00f8d394);
    op.native_site=0x0070b5cd;release_field(grid,0x78,c);
    op.native_site=0x0070b600;
    void* stream=create_gui_text_native_vertex_current5c(renderer,count*2u,0x1000,declaration,c.graphics,op.vertex);
    publish_stream(grid,0x78,stream,op.vertex);
    op.native_site=0x0070b610;release_field(grid,0x7c,c);
    op.native_site=0x0070b63f;
    stream=create_gui_text_native_index_current60(renderer,word(grid,0x80)*3u,1,0x65,c.graphics,op.index);
    publish_stream(grid,0x7c,stream,op.index);
    op.native_site=0x0070b651;void* destination=grid_map(stream,false,c,op);
    U cursor=0;
    for(U row=0;below(row,word(grid,0x40)-1u);++row,++cursor){
        for(U column=0;below(column,word(grid,0x3c)-1u);++column,++cursor){
            const U n=static_cast<std::uint16_t>(cursor);
            emit_index(destination,n);emit_index(destination,n+1u);emit_index(destination,n+word(grid,0x3c)+1u);
            emit_index(destination,n);emit_index(destination,n+word(grid,0x3c)+1u);emit_index(destination,n+word(grid,0x3c));
        }
    }
    cursor=count;
    for(U row=0;below(row,word(grid,0x40)-1u);++row,++cursor){
        for(U column=0;below(column,word(grid,0x3c)-1u);++column,++cursor){
            const U n=static_cast<std::uint16_t>(cursor);
            emit_index(destination,n);emit_index(destination,n+word(grid,0x40)+1u);emit_index(destination,n+1u);
            emit_index(destination,n);emit_index(destination,n+word(grid,0x3c));emit_index(destination,n+word(grid,0x3c)+1u);
        }
    }
    op.native_site=0x0070b777;grid_unmap(pointer(grid,0x7c),false,c,op);
    op.native_site=0x0070b787;(void)grid_map(pointer(grid,0x78),true,c,op);
    if(below(0,count)){
        const U white=c.constants.bits_00d7a24c;
        for(U i=0;below(i,count);++i){
            gpu_position(grid,i,count);U uv[2];grid_uv(grid,i,uv);
            stream=pointer(grid,0x78);destination=at(pointer(stream,8),word(stream,0xc)*i+word(stream,0x28));
            copy_float(destination,uv);copy_float(at(destination,4),uv+1);grid_color(grid,i,white);
            stream=pointer(grid,0x78);destination=at(pointer(stream,8),word(stream,0xc)*(i+count)+word(stream,0x28));
            put(destination,0,uv[0]);put(destination,4,uv[1]);grid_color(grid,i+count,white);
        }
    }
    op.native_site=0x0070b99a;grid_unmap(pointer(grid,0x78),true,c,op);
    op.native_site=0x0070b9a4;op.declaration={};
    release_native_render_actual_owner(c.graphics.streams.geometry.actual_owners(),declaration);
}
void orientation(void* grid,const float* angle){
    float sine,sine_copy,cosine,product;
    __asm {
        mov eax,angle
        fld dword ptr [eax]
        fsin
        fstp sine
        fld sine
        fstp sine_copy
        fld dword ptr [eax]
        fcos
        fstp cosine
        mov edx,grid
        fld dword ptr [edx+0x58]
        fmul sine_copy
        mov dword ptr [edx+0x30],0
        mov eax,cosine
        mov dword ptr [edx+0x34],eax
        fstp product
        fld product
        fstp dword ptr [edx+0x38]
    }
}
}
void initialize_native_game_grid_0070b330(void* grid,const void* descriptor,NativeGameGridContext& c,GridOp& op){
    grid_begin(grid,c,op);
    try {initialize_grid_body(grid,descriptor,c,op);op.phase=GridOp::Phase::complete;}
    catch(...){grid_failed(c,op);throw;}
}
void* construct_native_game_grid_0070bd70(void* grid,float angle,const NativeGameGridDescriptorPreimage& preimage,NativeGameGridContext& c,GridOp& op){
    grid_begin(grid,c,op);
    try {
        const U one=c.constants.bits_00d7a24c,size=c.constants.bits_00cfd470,parameter54=c.constants.bits_00d7a260;
        const U parameter50=c.constants.bits_00ce5380,parameter4c=c.constants.bits_00ce397c,parameter58=c.constants.bits_00cfd46c;
        put(grid,0,0x00cfd484);put(grid,0x64,0);put(grid,0x68,0);put(grid,0x6c,one);put(grid,0x70,one);
        put(grid,0x44,size);put(grid,0x48,size);put(grid,0x54,parameter54);put(grid,0x50,parameter50);put(grid,0x4c,parameter4c);put(grid,0x58,parameter58);
        put(grid,0x3c,16);put(grid,0x40,16);for(U offset=4;offset!=0x20;offset+=4)put(grid,offset,0);
        put(grid,0x20,c.constants.bits_00f87574);put(grid,0x24,c.constants.bits_00f87578);put(grid,0x28,c.constants.bits_00f8757c);
        put(grid,0x78,0);put(grid,0x7c,0);U angle_bits;std::memcpy(&angle_bits,&angle,4);put(grid,0x2c,angle_bits);put(grid,0x74,0xffffffffu);
        U descriptor[14]={16,16,size,size,parameter4c,parameter50,parameter54,parameter58,preimage.word20,preimage.word24,0,0,one,one};
        op.native_site=0x0070be8d;initialize_grid_body(grid,descriptor,c,op);
        orientation(grid,&angle);op.phase=GridOp::Phase::complete;return grid;
    } catch(...){grid_failed(c,op);throw;}
}
void destroy_native_game_grid_0070b220(void* grid,NativeGameGridContext& c){
    put(grid,0,0x00cfd484);release_native_game_grid_cpu_arrays_00709760(grid,c.cpu);
    release_field(grid,0x78,c);release_field(grid,0x7c,c);
}
void* delete_native_game_grid_0070b280(void* grid,std::uint8_t flags,NativeGameGridContext& c){
    destroy_native_game_grid_0070b220(grid,c);if((flags&1u)!=0)c.cpu.free_00bf65ac(grid);return grid;
}
} // namespace bsp
