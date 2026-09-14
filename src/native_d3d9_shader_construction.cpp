#include "bsp/native_d3d9_shader_construction_actual.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Registry = NativeD3d9ShaderRegistryOperation;
using Op = NativeD3d9ShaderConstructionOperation;
static_assert(sizeof(void*) == 4);
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + offset);
}
void put(void* p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(static_cast<char*>(p) + offset) = value;
}
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
std::int32_t signed_word(U value) noexcept {
    std::int32_t result; std::memcpy(&result, &value, 4); return result;
}
void begin(Registry& a, void* array, U function) {
    if (a.phase != Registry::Phase::fresh) throw std::logic_error("shader registry operation is one-shot");
    a.array = array; a.function = function; a.phase = Registry::Phase::running;
}
bool remove(void* array, const void* cell) noexcept {
    const U data = word(array), count = word(array, 4), end = data + count * 4u;
    U cursor = data;
    if (cursor >= end) return false;
    const U wanted = word(cell);
    while (word(reinterpret_cast<void*>(cursor)) != wanted) {
        cursor += 4u;
        if (cursor >= end) return false;
    }
    const U index = static_cast<U>(signed_word(cursor - data) >> 2);
    if (index == 0xffffffffu) return false;
    if (index != count - 1u)
        put(reinterpret_cast<void*>(data + index * 4u), 0, word(reinterpret_cast<void*>(data + count * 4u - 4u)));
    put(array, 4, word(array, 4) - 1u);
    return true;
}
// Optional diagnostics preserve the existing retained-operation API. The raw
// overloads use this same body without allocating an operation or undoing a
// completed remove/copy/allocation if a native dependency throws.
void reserve(void* array, std::int32_t request, Registry* a, bool pixel) {
    if(a)begin(*a,array,pixel?0x00b22e30u:0x00b22dd0u);
    try {
        if(request<1)request=1;
        if(a)a->requested_capacity=request;
        if(signed_word(word(array,8))<request) {
            const auto bytes=static_cast<std::uint32_t>(request)*4u;
            if(a)a->native_site=pixel?0x00b22e50u:0x00b22df0u;
            void* allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
            if(a)a->allocation=allocation;
            auto destination=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(allocation));
            std::uint32_t cursor=0;
            if(a)a->cursor=0;
            while(signed_word(cursor)<signed_word(word(array,4))) {
                if(destination)put(pointer(destination),0,word(pointer(word(array,0)+cursor*4u)));
                ++cursor;destination+=4u;
                if(a)a->cursor=cursor;
            }
            if(a)a->native_site=pixel?0x00b22e7cu:0x00b22e1cu;
            singleton_lifetime_free(pointer(word(array,0)));
            put(array,0,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(allocation)));
            put(array,8,static_cast<std::uint32_t>(request));
            if(a)a->allocation=nullptr;
        }
        if(a)a->phase=Registry::Phase::complete;
    } catch(...) { if(a)a->phase=Registry::Phase::failed;throw; }
}
void register_shader(void* renderer,void* shader,Registry* a,bool pixel) {
    void* array=static_cast<std::byte*>(renderer)+(pixel?0x1ad0u:0x1ac4u);
    if(a) {begin(*a,array,pixel?0x00b289f0u:0x00b289a0u);a->shader=shader;}
    try {
        if(a)a->native_site=pixel?0x00b289feu:0x00b289aeu;
        remove(array,&shader);
        const auto capacity=word(array,8);
        if(word(array,4)==capacity) {
            auto request=signed_word(capacity*2u);if(request<=1)request=1;
            if(a) {
                a->native_site=pixel?0x00b28a1au:0x00b289cau;
                a->reserve_child=std::make_unique<Registry>();
                reserve(array,request,a->reserve_child.get(),pixel);
            } else reserve(array,request,nullptr,pixel);
        }
        const auto count=word(array,4);const auto data=word(array,0);
        const auto destination=data+count*4u;
        if(destination)put(pointer(destination),0,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(shader)));
        put(array,4,word(array,4)+1u);
        if(a)a->phase=Registry::Phase::complete;
    } catch(...) { if(a)a->phase=Registry::Phase::failed;throw; }
}

} // namespace
NativeD3d9ShaderRegistryOperation::~NativeD3d9ShaderRegistryOperation() {
    if (phase == Phase::running || phase == Phase::failed || allocation) std::terminate();
}
void NativeD3d9ShaderRegistryOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running || allocation || (reserve_child &&
        (reserve_child->phase == Phase::running || reserve_child->phase == Phase::failed))) std::terminate();
    phase = Phase::diagnostic_retired;
}
NativeD3d9ShaderConstructionOperation::~NativeD3d9ShaderConstructionOperation() {
    if (phase == Phase::running || phase == Phase::failed || first_live || second_live) std::terminate();
}
void NativeD3d9ShaderConstructionOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running || first_live || second_live || (registry &&
        (registry->phase == Registry::Phase::running || registry->phase == Registry::Phase::failed))) std::terminate();
    phase = Phase::diagnostic_retired;
}
bool remove_native_vertex_shader_registry_00b253e0(void* a, const void* p) noexcept { return remove(a, p); }
bool remove_native_pixel_shader_registry_00b25450(void* a, const void* p) noexcept { return remove(a, p); }
void reserve_native_vertex_shader_registry_00b22dd0(void* a, std::int32_t n, Registry& f) { reserve(a, n, &f, false); }
void reserve_native_pixel_shader_registry_00b22e30(void* a, std::int32_t n, Registry& f) { reserve(a, n, &f, true); }
void register_native_vertex_shader_00b289a0(void* r, void* s, Registry& f) { register_shader(r, s, &f, false); }
void register_native_pixel_shader_00b289f0(void* r, void* s, Registry& f) { register_shader(r, s, &f, true); }
void reserve_native_vertex_shader_registry_00b22dd0(void* p,std::int32_t n){reserve(p,n,nullptr,false);}
void reserve_native_pixel_shader_registry_00b22e30(void* p,std::int32_t n){reserve(p,n,nullptr,true);}
void register_native_vertex_shader_00b289a0(void* r,void* s){register_shader(r,s,nullptr,false);}
void register_native_pixel_shader_00b289f0(void* r,void* s){register_shader(r,s,nullptr,true);}
NativeD3d9ShaderStorage* construct_native_pixel_shader_00b5f9b0(void* p, void* s, NativeD3d9ShaderConstructionContext& c, Op& a) { return detail::construct_native_shader_retained(p, s, c, a, true); }
NativeD3d9ShaderStorage* construct_native_vertex_shader_00b5faf0(void* p, void* s, NativeD3d9ShaderConstructionContext& c, Op& a) { return detail::construct_native_shader_retained(p, s, c, a, false); }
} // namespace bsp
