#include "bsp/dyn_body_creation.hpp"
#include <cfloat>
#include <cmath>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
static_assert(offsetof(DynBodyDescriptor, shape_count) == 0x78);
static_assert(sizeof(DynShapeDescriptor) == 0x48);
namespace {
template<class T> T& at(void* p, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p) + offset);
}
void* plus(void* p, std::size_t n) { return static_cast<unsigned char*>(p)+n; }
void* allocate(const AvoidZoneDynHullMemory& m, std::size_t n) {
    void* p=m.allocate(m.context,n);
    if (!p) throw std::bad_alloc();
    return p;
}
void append_pointer(void* vector, void* value, const AvoidZoneDynHullMemory& m) {
    auto& pointers=at<void**>(vector,0);
    auto& count=at<std::uint32_t>(vector,4);
    auto& capacity=at<std::uint32_t>(vector,8);
    if (count==capacity) {
        capacity=capacity*2+2;
        auto* grown=static_cast<void**>(allocate(m,capacity*4));
        if (count) std::memcpy(grown,pointers,count*4);
        if (pointers) m.release(m.context,pointers);
        pointers=grown;
    }
    pointers[count++]=value;
}
void add_pool_page(void* pool, std::size_t stride, const AvoidZoneDynHullMemory& m) {
    void* page=allocate(m,1000*stride);
    for (std::size_t i=0;i<999;++i)
        at<void*>(page,i*stride+stride-4)=plus(page,(i+1)*stride);
    at<void*>(page,1000*stride-4)=nullptr;
    at<void*>(pool,0xc)=page;
    append_pointer(pool,page,m);
}
void construct_pool(void* pool, std::size_t stride, const AvoidZoneDynHullMemory& m) {
    at<void*>(pool,0)=nullptr;
    at<std::uint32_t>(pool,4)=at<std::uint32_t>(pool,8)=0;
    at<void*>(pool,0x10+2*stride-8)=plus(pool,0x10);
    at<void*>(pool,0x10+2*stride-4)=nullptr;
    at<void*>(pool,0x10+stride-8)=nullptr;
    at<void*>(pool,0x10+stride-4)=plus(pool,0x10+stride);
    at<void*>(pool,0xc)=nullptr;
    at<std::uint32_t>(pool,0x10+2*stride)=0;
    add_pool_page(pool,stride,m);
}
void* take_pool_slot(void* pool, std::size_t stride, const AvoidZoneDynHullMemory& m) {
    if (!at<void*>(pool,0xc)) add_pool_page(pool,stride,m);
    void* slot=at<void*>(pool,0xc);
    at<void*>(pool,0xc)=at<void*>(slot,stride-4);
    ++at<std::uint32_t>(pool,0x10+2*stride);
    void* end=plus(pool,0x10+stride);
    void* tail=at<void*>(end,stride-8);
    at<void*>(slot,stride-4)=end;
    at<void*>(slot,stride-8)=tail;
    at<void*>(tail,stride-4)=slot;
    at<void*>(end,stride-8)=slot;
    return slot;
}
float add(float a,float b) { float r;
    __asm fld a
    __asm fadd b
    __asm fstp r
    return r;
}
float sub(float a,float b) { float r;
    __asm fld a
    __asm fsub b
    __asm fstp r
    return r;
}
float half(float a) { const double h=0.5; float r;
    __asm fld a
    __asm fmul h
    __asm fstp r
    return r;
}
float expand(float a,double b) { float r;
    __asm fld a
    __asm fadd b
    __asm fstp r
    return r;
}
float inverse(float a) { float r;
    __asm fld1
    __asm fdiv a
    __asm fstp r
    return r;
}
float transform_component(float x,float a,float y,float b,float z,float c,float t) {
    float r;
    __asm {
        fld x
        fmul a
        fld y
        fmul b
        faddp st(1),st(0)
        fld z
        fmul c
        faddp st(1),st(0)
        fadd t
        fstp r
    }
    return r;
}
float extent_component(float x,float a,float y,float b,float z,float c) {
    float r;
    __asm {
        fld x
        fmul a
        fld y
        fmul b
        faddp st(1),st(0)
        fld z
        fmul c
        faddp st(1),st(0)
        fstp r
    }
    return r;
}
DynAabb transform_bounds(const DynAabb& bounds, const float* transform) {
    float center[3],extent[3];
    const float lo[3]={bounds.min.x,bounds.min.y,bounds.min.z};
    const float hi[3]={bounds.max.x,bounds.max.y,bounds.max.z};
    for (int i=0;i<3;++i) {
        center[i]=half(add(lo[i],hi[i]));
        extent[i]=half(sub(hi[i],lo[i]));
    }
    float out[6];
    for (int i=0;i<3;++i) {
        const float c=transform_component(center[0],transform[i],center[1],transform[i+3],
            center[2],transform[i+6],transform[i+9]);
        const float e=extent_component(extent[0],std::fabs(transform[i]),
            extent[1],std::fabs(transform[i+3]),extent[2],std::fabs(transform[i+6]));
        out[i]=sub(c,e); out[i+3]=add(c,e);
    }
    return {{out[0],out[1],out[2]},{out[3],out[4],out[5]}};
}
void refresh_proxy(void* body) {
    void* proxy=at<void*>(body,0x60);
    at<DynAabb>(proxy,4)=transform_bounds(at<DynAabb>(body,0x38),
        &at<float>(body,8));
    void* scene=at<void*>(at<void*>(body,0),0x444);
    void* manager=at<void*>(scene,0xac);
    // Real borrowed virtual method, including the inserted-proxy endpoint path.
    using Refresh=void (__thiscall*)(void*,void*);
    auto table=at<void**>(manager,0);
    reinterpret_cast<Refresh>(table[2])(manager,proxy);
}
} // namespace

void dyn_body_pool_construct_00409170(void* p,const AvoidZoneDynHullMemory& m) { construct_pool(p,0x88,m); }
void dyn_motion_pool_construct_00409450(void* p,const AvoidZoneDynHullMemory& m) { construct_pool(p,0xc8,m); }
void dyn_endpoint_pool_construct_0040b4a0(void* p,const AvoidZoneDynHullMemory& m) { construct_pool(p,0x10,m); }
void dyn_pair_pool_construct_0040ac90(void* p,const AvoidZoneDynHullMemory& m) { construct_pool(p,0x10,m); }
void dyn_proxy_pool_construct_0040b750(void* p,const AvoidZoneDynHullMemory& m) { construct_pool(p,0x50,m); }

void dyn_world_body_pool_fragment_00c41ad0(void* world,const AvoidZoneDynHullMemory& m) {
    dyn_body_pool_construct_00409170(plus(world,0x4c),m);
    dyn_body_pool_construct_00409170(plus(world,0x170),m);
    dyn_motion_pool_construct_00409450(plus(world,0x294),m);
    void* shared=take_pool_slot(plus(world,0x294),0xc8,m);
    std::memset(shared,0,0xc0);
}

void dyn_sap_manager_construct_00c36f10(void* p,const void* vtable,const AvoidZoneDynHullMemory& m) {
    at<const void*>(p,0)=vtable;
    for (std::size_t i=0;i<3;++i) dyn_endpoint_pool_construct_0040b4a0(plus(p,4+i*0x34),m);
    dyn_pair_pool_construct_0040ac90(plus(p,0xa0),m);
    dyn_proxy_pool_construct_0040b750(plus(p,0xd4),m);
    dyn_proxy_pool_construct_0040b750(plus(p,0x188),m);
    at<void*>(p,0x23c)=nullptr;
    at<std::uint32_t>(p,0x240)=0;
    at<std::uint32_t>(p,0x244)=500;
    at<void*>(p,0x23c)=allocate(m,2000);
}

void dyn_convex_pool_construct_00407c70(DynConvexShapePoolStorage& p,void*& head,
    const void* vtable,const AvoidZoneDynHullMemory& m) {
    p.previous=nullptr; p.next=head;
    if (head) at<void*>(head,4)=&p;
    head=&p; p.vtable=vtable;
    InitializeCriticalSection(&p.critical_section);
    p.lock_depth=0; p.pages=nullptr; p.page_count=0;
    p.first_free_page=0xffffffffu; p.page_capacity=32;
    p.pages=static_cast<void**>(allocate(m,128));
}
void dyn_convex_pool_page_construct_004085f0(void* page,std::uint32_t page_index) {
    at<std::uint16_t>(page,0x10d00)=128;
    for (std::uint32_t i=0;i<128;++i) {
        at<std::uint16_t>(page,0x10c00+i*2)=static_cast<std::uint16_t>(127-i);
        at<std::uint32_t>(page,i*0x218+0x214)=page_index;
    }
}
DynConvexShapeStorage* dyn_convex_shape_allocate_00407ed0(DynConvexShapePoolStorage& p,
    const AvoidZoneDynHullMemory& m) {
    EnterCriticalSection(&p.critical_section); ++p.lock_depth;
    if (p.first_free_page==0xffffffffu) {
        p.first_free_page=p.page_count;
        void* page=allocate(m,0x10d04);
        dyn_convex_pool_page_construct_004085f0(page,p.first_free_page);
        append_pointer(&p.pages,page,m);
    }
    void* page=p.pages[p.first_free_page];
    auto& count=at<std::uint16_t>(page,0x10d00);
    const auto index=at<std::uint16_t>(page,0x10c00+2*--count);
    if (!count) {
        std::uint32_t i=p.first_free_page+1;
        while (i<p.page_count && !at<std::uint16_t>(p.pages[i],0x10d00)) ++i;
        p.first_free_page=i<p.page_count?i:0xffffffffu;
    }
    --p.lock_depth; LeaveCriticalSection(&p.critical_section);
    return static_cast<DynConvexShapeStorage*>(plus(page,index*0x218));
}
void dyn_convex_shape_free_00408040(DynConvexShapePoolStorage& p,DynConvexShapeStorage& shape) {
    EnterCriticalSection(&p.critical_section); ++p.lock_depth;
    const auto page_index=at<std::uint32_t>(&shape,0x214);
    void* page=p.pages[page_index];
    const auto index=(reinterpret_cast<unsigned char*>(&shape)-static_cast<unsigned char*>(page))/0x218;
    auto& count=at<std::uint16_t>(page,0x10d00);
    at<std::uint16_t>(page,0x10c00+count*2)=static_cast<std::uint16_t>(index);
    ++count;
    if (page_index<p.first_free_page) p.first_free_page=page_index;
    --p.lock_depth; LeaveCriticalSection(&p.critical_section);
}

void dyn_body_storage_init_00c43ca0(DynBodyStorage& body,const DynBodyDescriptor& desc) {
    void* b=&body;
    at<void*>(b,0x74)=nullptr;
    at<std::uint32_t>(b,0x78)=at<std::uint32_t>(b,0x7c)=0;
    at<std::uint32_t>(b,0x50)=desc.flags;
    if (!(desc.flags&1)) {
        void* m=at<void*>(b,4);
        at<bool>(m,0xb4)=desc.lock_torque_to_row1;
        std::memcpy(plus(m,0x84),desc.row0,0x30);
        at<OceanVec3>(m,0)=desc.linear_velocity;
        at<OceanVec3>(m,0xc)=desc.angular_velocity;
        at<float>(m,0x1c)=desc.max_angular_speed;
        at<float>(m,0x18)=desc.max_linear_speed;
        std::memset(plus(m,0x20),0,0x30);
        at<float>(m,0xb8)=desc.linear_damping; at<float>(m,0xbc)=desc.angular_damping;
        at<float>(m,0x50)=inverse(desc.mass);
        at<OceanVec3>(m,0x54)={desc.inertia.x>0?inverse(desc.inertia.x):0,
            desc.inertia.y>0?inverse(desc.inertia.y):0,desc.inertia.z>0?inverse(desc.inertia.z):0};
        std::memset(plus(m,0x60),0,0x24);
    }
    std::memcpy(plus(b,8),desc.row0,0x30);
    at<std::uint32_t>(b,0x54)=20;
    std::memset(plus(b,0x38),0,0x18);
    at<std::uint32_t>(b,0x58)=at<std::uint32_t>(b,0x5c)=0;
    at<std::uint16_t>(b,0x64)=0;
    at<void*>(b,0x60)=nullptr;
    at<const void*>(b,0x68)=desc.contact_listener;
    at<const void*>(b,0x6c)=desc.owner;
    at<void*>(b,0x70)=nullptr;
}
void dyn_sap_proxy_init_00c4cda0(DynSapProxyStorage& proxy,DynBodyStorage& body,
    const DynAabb& bounds,bool is_static,const AvoidZoneDynHullMemory& m) {
    at<void*>(&proxy,0)=&body; at<bool>(&proxy,0x1c)=is_static;
    at<DynAabb>(&proxy,4)=bounds;
    at<void*>(&proxy,0x3c)=nullptr;
    at<std::uint32_t>(&proxy,0x40)=0;
    at<std::uint32_t>(&proxy,0x44)=10;
    at<void*>(&proxy,0x3c)=allocate(m,40);
    at<std::uint8_t>(&proxy,0x38)=0;
}
DynSapProxyStorage* dyn_sap_create_proxy_00c54aa0(void* manager,DynBodyStorage& body,
    const DynAabb& bounds,bool is_static,const AvoidZoneDynHullMemory& m) {
    auto* proxy=static_cast<DynSapProxyStorage*>(take_pool_slot(
        plus(manager,is_static?0x188:0xd4),0x50,m));
    dyn_sap_proxy_init_00c4cda0(*proxy,body,bounds,is_static,m);
    append_pointer(plus(manager,0x23c),proxy,m);
    return proxy;
}
void dyn_body_register_broadphase_00c50470(DynBodyStorage& body,void* scene,
    const AvoidZoneDynHullMemory& m) {
    auto& flags=at<std::uint32_t>(&body,0x50); flags|=8;
    const auto bounds=transform_bounds(at<DynAabb>(&body,0x38),&at<float>(&body,8));
    // The identified concrete slot-0 target is 00C54AA0. Other manager classes
    // are outside this explicitly typed creation path.
    at<void*>(&body,0x60)=dyn_sap_create_proxy_00c54aa0(at<void*>(scene,0xac),body,
        bounds,(flags&1)!=0,m);
}
void dyn_body_recompute_bounds_00c55fc0(DynBodyStorage& body) {
    DynAabb bounds{{FLT_MAX,FLT_MAX,FLT_MAX},{-FLT_MAX,-FLT_MAX,-FLT_MAX}};
    for (void* s=at<void*>(&body,0x70);s;s=at<void*>(s,0x208)) {
        const auto& box=at<DynAabb>(s,0xc);
        const OceanVec3 corners[]={box.min,box.max};
        for (const auto& v:corners) {
            if (v.x<bounds.min.x) bounds.min.x=v.x;
            if (v.y<bounds.min.y) bounds.min.y=v.y;
            if (v.z<bounds.min.z) bounds.min.z=v.z;
            if (bounds.max.x<v.x) bounds.max.x=v.x;
            if (bounds.max.y<v.y) bounds.max.y=v.y;
            if (bounds.max.z<v.z) bounds.max.z=v.z;
        }
    }
    at<DynAabb>(&body,0x38)=bounds;
    if (at<std::uint32_t>(&body,0x50)&8) refresh_proxy(&body);
}
void dyn_convex_shape_refresh_bounds_00c57c40(DynConvexShapeStorage& shape) {
    const auto* mesh=at<const AvoidZoneDynHullData*>(&shape,0x210);
    const DynAabb expanded{{expand(mesh->minimum.x,-0.02),expand(mesh->minimum.y,-0.02),
        expand(mesh->minimum.z,-0.02)},{expand(mesh->maximum.x,0.02),
        expand(mesh->maximum.y,0.02),expand(mesh->maximum.z,0.02)}};
    at<DynAabb>(&shape,0xc)=transform_bounds(expanded,&at<float>(&shape,0x34));
    dyn_body_recompute_bounds_00c55fc0(*at<DynBodyStorage*>(&shape,4));
}
void dyn_convex_shape_construct_00c57f50(DynConvexShapeStorage& shape,DynBodyStorage& body,
    const DynShapeDescriptor& desc,const void* vtable) {
    at<const void*>(&shape,0)=vtable;
    at<std::uint32_t>(&shape,0x204)=0;
    at<void*>(&shape,4)=&body; at<DynShapeType>(&shape,8)=desc.type;
    at<float>(&shape,0x24)=desc.restitution; at<float>(&shape,0x28)=desc.friction;
    at<std::uint32_t>(&shape,0x2c)=desc.group; at<std::uint32_t>(&shape,0x30)=desc.mask;
    at<void*>(&shape,0x208)=at<void*>(&shape,0x20c)=nullptr;
    at<const void*>(&shape,0x210)=*static_cast<const void* const*>(desc.geometry);
    std::memcpy(plus(&shape,0x34),desc.row0,0x30);
    dyn_convex_shape_refresh_bounds_00c57c40(shape);
}
void dyn_body_attach_convex_shape_00c5c940(DynBodyStorage& body,
    const DynShapeDescriptor& desc,const DynBodyCreationContext& context) {
    if (desc.type!=DynShapeType::kConvexMesh) throw std::invalid_argument("convex shape required");
    void* old=at<void*>(&body,0x70);
    auto* shape=dyn_convex_shape_allocate_00407ed0(*context.convex_pool,context.memory);
    dyn_convex_shape_construct_00c57f50(*shape,body,desc,context.convex_shape_vtable);
    if (old) { at<void*>(shape,0x208)=old; at<void*>(old,0x20c)=shape; }
    at<void*>(&body,0x70)=shape;
    dyn_body_recompute_bounds_00c55fc0(body);
    if (!old) dyn_body_register_broadphase_00c50470(body,
        at<void*>(at<void*>(&body,0),0x444),context.memory);
}
DynBodyStorage* dyn_world_create_convex_body_00c5d580(void* world,const DynBodyDescriptor& desc,
    const DynShapeDescriptor* const* shapes,std::uint32_t count,const DynBodyCreationContext& context) {
    if (count!=desc.shape_count) throw std::invalid_argument("shape count mismatch");
    for (std::uint32_t i=0;i<count;++i)
        if (shapes[i]->type!=DynShapeType::kConvexMesh) throw std::invalid_argument("convex shape required");
    const bool is_static=(desc.flags&1)!=0;
    auto* body=static_cast<DynBodyStorage*>(take_pool_slot(plus(world,is_static?0x4c:0x170),
        0x88,context.memory));
    at<void*>(body,4)=is_static?at<void*>(world,0x368):
        take_pool_slot(plus(world,0x294),0xc8,context.memory);
    dyn_body_storage_init_00c43ca0(*body,desc);
    at<void*>(body,0)=world;
    for (std::uint32_t i=0;i<count;++i) dyn_body_attach_convex_shape_00c5c940(*body,*shapes[i],context);
    return body;
}
} // namespace bsp
