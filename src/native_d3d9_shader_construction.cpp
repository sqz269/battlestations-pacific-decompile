#include "bsp/native_d3d9_shader_construction.hpp"
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
void* pointer(const void* p, U offset = 0) noexcept { return reinterpret_cast<void*>(word(p, offset)); }
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
void reserve(void* array, std::int32_t request, Registry& a, bool pixel) {
    begin(a, array, pixel ? 0xb22e30 : 0xb22dd0);
    try {
        a.requested_capacity = request < 1 ? 1 : request;
        if (signed_word(word(array, 8)) < a.requested_capacity) {
            a.native_site = pixel ? 0xb22e50 : 0xb22df0;
            const U bytes = static_cast<U>(a.requested_capacity) * 4u;
            a.allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
            U destination = reinterpret_cast<U>(a.allocation); a.cursor = 0;
            while (signed_word(a.cursor) < signed_word(word(array, 4))) {
                if (destination) put(reinterpret_cast<void*>(destination), 0,
                    word(reinterpret_cast<void*>(word(array) + a.cursor * 4u)));
                ++a.cursor; destination += 4u;
            }
            a.native_site = pixel ? 0xb22e7c : 0xb22e1c;
            singleton_lifetime_free(pointer(array));
            put(array, 0, reinterpret_cast<U>(a.allocation));
            put(array, 8, static_cast<U>(a.requested_capacity)); a.allocation = nullptr;
        }
        a.phase = Registry::Phase::complete;
    } catch (...) { a.phase = Registry::Phase::failed; throw; }
}
void register_shader(void* renderer, void* shader, Registry& a, bool pixel) {
    auto* array = static_cast<char*>(renderer) + (pixel ? 0x1ad0 : 0x1ac4);
    begin(a, array, pixel ? 0xb289f0 : 0xb289a0); a.shader = shader;
    try {
        a.native_site = pixel ? 0xb289fe : 0xb289ae;
        remove(array, &shader);
        const U capacity = word(array, 8);
        if (word(array, 4) == capacity) {
            const auto doubled = signed_word(capacity + capacity);
            a.native_site = pixel ? 0xb28a1a : 0xb289ca;
            a.reserve_child = std::make_unique<Registry>();
            reserve(array, doubled > 1 ? doubled : 1, *a.reserve_child, pixel);
        }
        const U count = word(array, 4), data = word(array), slot = data + count * 4u;
        if (slot) put(reinterpret_cast<void*>(slot), 0, reinterpret_cast<U>(shader));
        put(array, 4, word(array, 4) + 1u);
        a.phase = Registry::Phase::complete;
    } catch (...) { a.phase = Registry::Phase::failed; throw; }
}
void register_owner(Op& a, bool pixel) {
    a.native_site = pixel ? 0xb5fad4 : 0xb5fb5d;
    void* renderer = a.context->actual_renderer_00f8d394;
    a.registry = std::make_unique<Registry>();
    register_shader(renderer, a.owner, *a.registry, pixel);
}
void support_names(Op& a, bool pixel) {
    auto& strings = a.context->strings;
    put(&a.first_name, 0, 0); put(&a.first_name, 4, 0); a.first_live = true;
    a.native_site = pixel ? 0xb5fa26 : 0xb5fb72;
    resize_native_string_header_0041dd40(&a.first_name, strings, pixel ? 11u : 12u, true);
    auto* first_data = pointer(&a.first_name, 4); a.first_length = word(&a.first_name);
    if (first_data) std::memmove(first_data, pixel ? "PixelShader" : "VertexShader", a.first_length + 1u);
    a.support_com_snapshot = pointer(a.owner, 8); // Stack local, NOT a B3E730 argument.
    put(&a.second_name, 0, 0); put(&a.second_name, 4, 0); a.second_live = true;
    a.native_site = pixel ? 0xb5fa64 : 0xb5fbb0;
    resize_native_string_header_0041dd40(&a.second_name, strings, a.first_length, true);
    a.second_data = static_cast<char*>(pointer(&a.second_name, 4)); a.second_captured = true;
    if (a.first_length) std::memmove(a.second_data, pointer(&a.first_name, 4), word(&a.second_name));
    a.native_site = pixel ? 0xb5fa89 : 0xb5fbd5;
    (void)resource_support_singleton_00b3e730(a.context->actual_support_0108fedc, a.context->actual_lifetime);
    a.native_site = pixel ? 0xb5faa9 : 0xb5fbf5;
    if (a.second_data) strings.release(a.second_data, word(&a.second_name) + 1u);
    a.second_live = false;
    a.native_site = pixel ? 0xb5fac8 : 0xb5fc14;
    if (auto* current_first = static_cast<char*>(pointer(&a.first_name, 4)))
        strings.release(current_first, a.first_length + 1u);
    a.first_live = false;
}
NativeD3d9ShaderStorage* construct(void* storage, void* com,
    NativeD3d9ShaderConstructionContext& c, Op& a, bool pixel) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("shader construction operation is one-shot");
    a.function = pixel ? 0xb5f9b0 : 0xb5faf0; a.context = &c; a.argument_com = com;
    a.phase = Op::Phase::running;
    try {
        a.owner = ::new (storage) NativeD3d9ShaderStorage;
        put(a.owner, 0, 0xceb130); a.owner->references_04.store(1, std::memory_order_relaxed);
        a.base_published = true;
        put(a.owner, 0, pixel ? 0xd62a60 : 0xd62a70); put(a.owner, 0xc, 0); put(a.owner, 8, 0);
        if (com) {
            put(a.owner, 8, reinterpret_cast<U>(com));
            a.native_site = pixel ? 0xb5fa08 : 0xb5fb48; a.com_addref_entered = true;
            using AddRef = U(__stdcall*)(void*);
            auto target = reinterpret_cast<AddRef>(word(pointer(com), 4));
            (void)target(com); a.com_addref_returned = true;
        }
        // Saved EDI was literal zero across the stdcall AddRef. The original
        // old-COM Release sites B5FA14/B5FB54 are therefore unreachable here.
        if (!pixel) register_owner(a, false);
        support_names(a, pixel);
        if (pixel) register_owner(a, true);
        a.phase = Op::Phase::complete; return a.owner;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
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
void reserve_native_vertex_shader_registry_00b22dd0(void* a, std::int32_t n, Registry& f) { reserve(a, n, f, false); }
void reserve_native_pixel_shader_registry_00b22e30(void* a, std::int32_t n, Registry& f) { reserve(a, n, f, true); }
void register_native_vertex_shader_00b289a0(void* r, void* s, Registry& f) { register_shader(r, s, f, false); }
void register_native_pixel_shader_00b289f0(void* r, void* s, Registry& f) { register_shader(r, s, f, true); }
NativeD3d9ShaderStorage* construct_native_pixel_shader_00b5f9b0(void* p, void* s, NativeD3d9ShaderConstructionContext& c, Op& a) { return construct(p, s, c, a, true); }
NativeD3d9ShaderStorage* construct_native_vertex_shader_00b5faf0(void* p, void* s, NativeD3d9ShaderConstructionContext& c, Op& a) { return construct(p, s, c, a, false); }
} // namespace bsp
