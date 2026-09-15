#include "bsp/native_damageable_class_binding.hpp"
#include "bsp/entity_identity.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }
void* pointer(U p) noexcept { return reinterpret_cast<void*>(p); }
U read(U p, U offset=0) noexcept { return *reinterpret_cast<const volatile U*>(p+offset); }
void write(U p,U offset,U value) noexcept { *reinterpret_cast<volatile U*>(p+offset)=value; }
void invalid(const SingletonLifetimeCallbacks& c) {
    if(c.invalid_parameter) c.invalid_parameter(c.context);
    else _invalid_parameter_noinfo();
}
U count(U begin,U end,U stride) noexcept {
    return static_cast<U>(static_cast<std::int32_t>(end-begin)/static_cast<std::int32_t>(stride));
}
void copy_raw3(U destination,U source) noexcept {
    const U x=read(source), y=read(source,4), z=read(source,8);
    write(destination,0,x);write(destination,4,y);write(destination,8,z);
}
void copy_x87_3(U destination,U source) noexcept {
    __asm {
        mov eax,destination
        mov edx,source
        fld dword ptr [edx]
        fstp dword ptr [eax]
        fld dword ptr [edx+4]
        fstp dword ptr [eax+4]
        fld dword ptr [edx+8]
        fstp dword ptr [eax+8]
    }
}
void point_delta(U output,U source,U origin) noexcept {
    U temp[3];copy_x87_3(address(temp),source);
    __asm {
        mov eax,output
        mov edx,origin
        lea ecx,temp
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        fstp dword ptr [eax]
        fld dword ptr [ecx+4]
        fsub dword ptr [edx+4]
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+8]
        fsub dword ptr [edx+8]
        fstp dword ptr [eax+8]
    }
}
// 879EFB..879F6D: keep the inline cross-product operand and spill schedule.
__declspec(naked) void __fastcall frame_cross(U*,const U*,const U*) {
    __asm {
        sub esp,12
        mov eax,dword ptr [esp+16]
        fld dword ptr [edx+4]
        fld st(0)
        fld dword ptr [eax+8]
        fld st(0)
        fmulp st(2),st(0)
        fld dword ptr [edx+8]
        fld st(0)
        fld dword ptr [eax+4]
        fld st(0)
        fmulp st(2),st(0)
        fxch st(4)
        fsubrp st(1),st(0)
        fstp dword ptr [esp]
        fld dword ptr [eax]
        movss xmm0,dword ptr [esp]
        fld st(0)
        movss dword ptr [ecx+16],xmm0
        fmulp st(2),st(0)
        fld dword ptr [edx]
        fld st(0)
        fmulp st(4),st(0)
        fxch st(2)
        fsubrp st(3),st(0)
        fxch st(2)
        fstp dword ptr [esp+4]
        fmulp st(2),st(0)
        fmulp st(2),st(0)
        fsubrp st(1),st(0)
        fstp dword ptr [esp+8]
        fld dword ptr [esp+4]
        fstp dword ptr [ecx+20]
        fld dword ptr [esp+8]
        fstp dword ptr [ecx+24]
        add esp,12
        ret 4
    }
}
U checked_points(U item,U captured_begin,U index,const SingletonLifetimeCallbacks& c) {
    if(!captured_begin || index>=count(captured_begin,read(item,0x4c),12)) invalid(c);
    return read(item,0x48);
}
U first_point(U item,const SingletonLifetimeCallbacks& c) {
    return checked_points(item,read(item,0x48),0,c);
}
void empty_name(NativeLegacySboStringStorage& name) noexcept {
    write(address(&name),0x18,15);write(address(&name),0x14,0);
    name.buffer_04.inline_bytes[0]=0;
}
void release_name(NativeLegacySboStringStorage& name) noexcept {
    if(read(address(&name),0x18)>=16) singleton_lifetime_free(pointer(read(address(&name),4)));
}
U rows(U actual_class) noexcept {
    const U begin=read(actual_class,0x1c);
    return begin ? count(begin,read(actual_class,0x20),0x30) : 0;
}
} // namespace

NativeLegacySboStringStorage& copy_native_point_group_category_00879a70(
    const void* item,NativeLegacySboStringStorage& output) {
    empty_name(output);
    return native_legacy_sbo_string_assign_substring_00408120(output,
        *static_cast<const NativeLegacySboStringStorage*>(pointer(address(item)+0x28)),0,0xffffffffu);
}
void* find_native_damageable_effect_row_00876da0(const void* object,U category,U index,
                                               const SingletonLifetimeCallbacks& c) {
    const U actual_class=address(object);U offset=0;
    for(U i=0;i<rows(actual_class);++i,offset+=0x30) {
        if(i>=rows(actual_class)) invalid(c);
        const U row=read(actual_class,0x1c)+offset;
        if(read(row,4)==category && read(row,8)==index) return pointer(row);
    }
    return nullptr;
}
void advance_native_class_frame_008772b0(NativeKeyboardTreeIterator* cursor,
                                        const SingletonLifetimeCallbacks& c) {
    advance_native_checked_tree_iterator(cursor,0x55,c);
}
void reserve_native_class_point_array_0074d190(void* object,std::int32_t requested) {
    const U header=address(object);
    const std::int32_t capacity=requested<1 ? 1 : requested;
    if(static_cast<std::int32_t>(read(header,8))>=capacity) return;
    const U bytes=static_cast<U>(capacity)*12u;
    const U replacement=address(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    U destination=replacement,offset=0;
    for(U i=0;static_cast<std::int32_t>(i)<static_cast<std::int32_t>(read(header,4));++i,offset+=12,destination+=12)
        if(destination) copy_x87_3(destination,read(header)+offset);
    singleton_lifetime_free(pointer(read(header)));
    write(header,0,replacement);write(header,8,static_cast<U>(capacity));
}

void bind_native_damageable_class_model_00879ad0(
    void* object,const NativeDamageableClassBindingContext& context) {
    const U actual_class=address(object);const auto& c=context.invalid_parameters;
    NativeLegacySboStringStorage name, fake_name;
    int state=-1;
    try {
        NativeKeyboardTreeIterator cursor{pointer(actual_class+0x60),pointer(read(read(actual_class,0x64)))};
        for(;;) {
            const U head=read(actual_class,0x64);
            if(!cursor.owner || address(cursor.owner)!=actual_class+0x60) invalid(c);
            const U node=address(cursor.node);
            if(node==head) break;
            if(!cursor.owner) invalid(c);
            if(node==read(address(cursor.owner),4)) invalid(c);
            const U index=read(node,0x0c);
            empty_name(name);
            native_legacy_sbo_string_assign_counted_00408720(name,"emberke",7);state=0;
            const U group=address(lookup_native_game_group_00718870(pointer(read(actual_class,0x50)),name,index,c));
            state=-1;release_name(name);empty_name(name);
            U matrix[16]{};matrix[0]=matrix[5]=matrix[10]=matrix[15]=context.one_00d7a24c;
            copy_raw3(address(matrix+12),first_point(group,c));
            U origin[3],delta[3],a[3],b[3];
            U begin=first_point(group,c);copy_raw3(address(origin),begin);
            begin=checked_points(group,begin,2,c);
            point_delta(address(delta),begin+24,address(origin));
            copy_x87_3(address(matrix+8),address(delta));
            begin=first_point(group,c);copy_raw3(address(origin),begin);
            begin=checked_points(group,begin,1,c);
            // The original captures begin for its next validation before the
            // final delta stores; no external call occurs between those reads.
            point_delta(address(a),begin+12,address(origin));
            begin=first_point(group,c);copy_raw3(address(origin),begin);
            begin=checked_points(group,begin,2,c);
            point_delta(address(b),begin+24,address(origin));
            frame_cross(matrix,b,a);
            orthonormalize_native_pose_matrix_0085dc80(reinterpret_cast<float*>(matrix),&context.pose);
            if(node==read(address(cursor.owner),4)) invalid(c);
            for(U i=0;i!=16;++i) write(node,0x10+i*4,read(address(matrix),i*4));
            advance_native_class_frame_008772b0(&cursor,c);
        }
        U resource=read(actual_class,0x50);U current=read(resource,0x68);
        const U captured_owner=resource+0x64;
        if(current>read(resource,0x6c)) invalid(c);
        for(;;) {
            resource=read(actual_class,0x50);
            const U end=read(resource,0x6c);
            if(read(resource,0x68)>end) invalid(c);
            if(captured_owner!=resource+0x64) invalid(c);
            if(current==end) break;
            if(current>=read(captured_owner,8)) invalid(c);
            const U item=read(current);
            copy_native_point_group_category_00879a70(pointer(item),name);release_name(name);
            const U length=read(item,0x1c),compared=length<9 ? length : 9;
            const U data=read(item,0x20)<16 ? item+0x0c : read(item,0x0c);
            if((!compared || std::memcmp(pointer(data),"explosion",compared)==0) && length==9) {
                auto& category=copy_native_point_group_category_00879a70(pointer(item),name);state=1;
                const char* text=category.data();U kind=0;
                if(!context.explosion_categories_00e08138[0]) kind=0xffffffff;
                else for(;;) {
                    if(compare_insensitive_00438e10(text,context.explosion_categories_00e08138[kind])==0) break;
                    ++kind;
                    if(!context.explosion_categories_00e08138[kind]) { kind=0xffffffff;break; }
                }
                state=-1;release_name(name);
                const U row=address(find_native_damageable_effect_row_00876da0(object,kind,read(item,0x24),c));
                if(row) copy_raw3(row+12,first_point(item,c));
            }
            if(current>=read(captured_owner,8)) invalid(c);
            current+=4;
        }
        for(U index=1;;++index) {
            empty_name(fake_name);
            native_legacy_sbo_string_assign_counted_00408720(fake_name,"fakeexplosion",13);state=2;
            const U item=address(lookup_native_game_group_00718870(pointer(read(actual_class,0x50)),fake_name,index,c));
            state=-1;release_name(fake_name);empty_name(fake_name);
            if(!item) break;
            const U point=first_point(item,c);U temporary[3];temporary[0]=read(point);
            const U capacity=read(actual_class,0x30);
            const bool grow=read(actual_class,0x2c)==capacity;
            temporary[1]=read(point,4);temporary[2]=read(point,8);
            if(grow) {
                const std::int32_t doubled=static_cast<std::int32_t>(capacity*2);
                reserve_native_class_point_array_0074d190(pointer(actual_class+0x28),doubled>1 ? doubled : 1);
            }
            const U append_index=read(actual_class,0x2c);
            const U append_begin=read(actual_class,0x28);
            const U destination=append_begin+append_index*12;
            if(destination) copy_raw3(destination,address(temporary));
            write(actual_class,0x2c,read(actual_class,0x2c)+1);
        }
        U offset=0;
        for(U i=0;i<rows(actual_class);++i,offset+=0x30) {
            if(i>=rows(actual_class)) invalid(c);
            const U row=read(actual_class,0x1c)+offset;
            if(i>=rows(actual_class)) invalid(c);
            copy_x87_3(read(actual_class,0x1c)+offset+0x18,row+0x0c);
        }
    } catch(...) {
        if(state==0 || state==1) native_legacy_sbo_string_destroy_004072d0(name);
        else if(state==2) native_legacy_sbo_string_destroy_004072d0(fake_name);
        throw;
    }
}
} // namespace bsp
