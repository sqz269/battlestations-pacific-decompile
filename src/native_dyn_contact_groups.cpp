#include "bsp/native_dyn_contact_groups.hpp"
#include <cstring>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
template<class T=U> T read(const void* p,U offset=0) {
    T value; std::memcpy(&value,static_cast<const unsigned char*>(p)+offset,sizeof value);
    return value;
}
template<class T> void write(void* p,U offset,T value) {
    std::memcpy(static_cast<unsigned char*>(p)+offset,&value,sizeof value);
}
void* at(const void* p,U offset=0) {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
void* allocate(const NativeDynContactGroupContext& c,U site,U bytes) {
    return c.calls.allocate_00bf55be(site,bytes,c.memory);
}
void release(const NativeDynContactGroupContext& c,U site,void* p) {
    if(p)c.calls.free_00bf6989(site,p,c.memory);
}
void visit_body(void* body,void* stack,U& top) {
    if((read<unsigned char>(body,0x50)&1)!=0 || read<std::uint16_t>(body,0x64)!=0xffff)return;
    write<std::uint16_t>(body,0x64,0);
    for(U i=0;i<read(body,0x78);++i) {
        void* contact=read<void*>(read<void*>(body,0x74),i*4);
        if(!contact || read(contact,0xc8)==0 || read<std::uint16_t>(contact,0xd4)!=0xffff)continue;
        write(stack,(top+1)*4,contact); ++top;
        write<std::uint16_t>(contact,0xd4,0);
        void* a=read<void*>(contact,0xcc);
        write(a,0x50,read(a,0x50)&0xffffffedu);
        void* b=read<void*>(contact,0xd0);
        write(b,0x50,read(b,0x50)&0xffffffedu);
    }
}
} // namespace
void* NativeDynContactGroupCalls::allocate_00bf55be(U,U n,const AvoidZoneDynHullMemory& m) {
    return m.allocate(m.context,n);
}
void NativeDynContactGroupCalls::free_00bf6989(U,void* p,const AvoidZoneDynHullMemory& m) {
    m.release(m.context,p);
}
void* native_dyn_group_copy_0040f810(void* dst,const void* src,const NativeDynContactGroupContext& c) {
    U count=read(src,4);
    if(count==0) {write<U>(dst,0,0);write<U>(dst,8,0);write<U>(dst,4,0);return dst;}
    write(dst,8,count);
    write(dst,0,allocate(c,0x0040f822,count*4));
    write(dst,4,read(dst,8));
    for(U i=0;i<read(dst,4);++i) {
        void* slot=at(read<void*>(dst),i*4);
        if(slot)write(slot,0,read(read<void*>(src),i*4));
    }
    return dst;
}
void native_dyn_reset_group_marks_00c36ac0(void* manager) {
    void* world=read<void*>(manager);
    void* body=read<void*>(world,0x204);
    while(body!=at(world,0x208)) {
        write<std::uint16_t>(body,0x64,0xffff);
        world=read<void*>(manager);
        body=read<void*>(body,0x84);
    }
    void* pool=read<void*>(read<void*>(read<void*>(manager),0x444),0xb0);
    for(void* p=read<void*>(pool,0xec);p!=at(pool,0xf0);p=read<void*>(p,0xdc))
        write<std::uint16_t>(p,0xd4,0xffff);
}
void native_dyn_append_manifold_00c36b60(void* group,void* manifold,const NativeDynContactGroupContext& c) {
    if(read(group,4)==read(group,8)) {
        U capacity=read(group,8)*2+2;
        write(group,8,capacity);
        void* data=allocate(c,0x00c36b75,capacity*4);
        for(U i=0;i<read(group,4);++i) {
            void* slot=at(data,i*4);
            if(slot)write(slot,0,read(read<void*>(group),i*4));
        }
        release(c,0x00c36ba5,read<void*>(group));
        write(group,0,data);
    }
    void* slot=at(read<void*>(group),read(group,4)*4);
    if(slot)write(slot,0,manifold);
    write(group,4,read(group,4)+1);
}
void native_dyn_clear_contact_groups_00c3f410(void* manager,const NativeDynContactGroupContext& c) {
    for(U i=0;i<read(manager,8);++i)
        release(c,0x00c3f43f,read<void*>(read<void*>(manager,4),i*12));
    write<U>(manager,8,0);write<U>(manager,0x10,0);
}
void native_dyn_create_contact_groups_00c4b610(void* manager,const NativeDynContactGroupContext& c) {
    native_dyn_clear_contact_groups_00c3f410(manager,c);
    native_dyn_reset_group_marks_00c36ac0(manager);
    void* pool=read<void*>(read<void*>(read<void*>(manager),0x444),0xb0);
    U count=read(pool,0x1d0);
    void* stack=nullptr;
    if(count) {
        stack=allocate(c,0x00c4b674,count*4);
        for(U i=0;i<count;++i)if(void* slot=at(stack,i*4))write<U>(slot,0,0);
    }
    U top=0xffffffffu;
    void* sentinel=at(pool,0xf0);
    for(void* seed=read<void*>(pool,0xec);seed!=sentinel;seed=read<void*>(seed,0xdc)) {
        if(read(seed,0xc8)==0 || read<std::uint16_t>(seed,0xd4)!=0xffff)continue;
        if((read<unsigned char>(read<void*>(seed,0xcc),0x50)&3)!=0 &&
           (read<unsigned char>(read<void*>(seed,0xd0),0x50)&3)!=0)continue;
        write(manager,0x10,read(manager,0x10)+1);
        U desired=read(manager,0x10);
        if(read(manager,8)<desired) {
            if(read(manager,12)<desired) {
                write(manager,12,desired);
                void* data=allocate(c,0x00c4b72a,desired*12);
                for(U i=0;i<read(manager,8);++i) {
                    void* dst=at(data,i*12);
                    if(dst)native_dyn_group_copy_0040f810(dst,at(read<void*>(manager,4),i*12),c);
                    release(c,0x00c4b787,read<void*>(read<void*>(manager,4),i*12));
                }
                release(c,0x00c4b7ab,read<void*>(manager,4));
                write(manager,4,data);
            }
            U size=read(manager,8);
            if(size<desired)for(U i=size;i<desired;++i) {
                void* dst=at(read<void*>(manager,4),i*12);
                if(dst){write<U>(dst,0,0);write<U>(dst,4,0);write<U>(dst,8,0);}
            }
        } else if(desired<read(manager,8)) {
            for(U i=desired;i<read(manager,8);++i)
                release(c,0x00c4b816,read<void*>(read<void*>(manager,4),i*12));
        }
        write(manager,8,desired);
        ++top;write(stack,top*4,seed);write<std::uint16_t>(seed,0xd4,0);
        while(static_cast<std::int32_t>(top)>=0) {
            void* p=read<void*>(stack,top*4);
            void* group=at(read<void*>(manager,4),read(manager,0x10)*12-12);
            --top;
            native_dyn_append_manifold_00c36b60(group,p,c);
            void* a=read<void*>(p,0xcc);void* b=read<void*>(p,0xd0);
            visit_body(a,stack,top);visit_body(b,stack,top);
        }
    }
    release(c,0x00c4b992,stack);
}
void native_dyn_sleep_contact_groups_00c4b550(void* manager,const NativeDynContactGroupContext& c) {
    void* data=read<void*>(manager,4);
    for(U g=0;g<read(manager,8);++g) {
        void* group=at(data,g*12);
        std::int32_t count=read<std::int32_t>(group,4),i=0;
        if(count>0) {
            void* list=read<void*>(group);
            for(;i<count;++i) {
                void* p=read<void*>(list,static_cast<U>(i)*4);
                if((read<unsigned char>(read<void*>(p,0xcc),0x50)&3)==0 ||
                   (read<unsigned char>(read<void*>(p,0xd0),0x50)&3)==0)break;
            }
        }
        if(i>=count && count>0) {
            i=0;
            do {
                void* p=read<void*>(read<void*>(group),static_cast<U>(i)*4);
                void* a=read<void*>(p,0xcc);write(a,0x50,read(a,0x50)|0x10);
                void* b=read<void*>(p,0xd0);write(b,0x50,read(b,0x50)|0x10);
                data=read<void*>(manager,4);group=at(data,g*12);++i;
            } while(i<read<std::int32_t>(group,4));
        }
    }
    native_dyn_clear_contact_groups_00c3f410(manager,c);
}
} // namespace bsp
