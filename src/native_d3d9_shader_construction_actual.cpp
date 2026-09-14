#include "bsp/native_d3d9_shader_construction_actual.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeD3d9ShaderConstructionOperation;
static_assert(sizeof(void*) == 4);
U word(const void* p, U byte_offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + byte_offset);
}
void put(void* p, U byte_offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(static_cast<char*>(p) + byte_offset) = value;
}
void* pointer(const void* p, U byte_offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, byte_offset));
}
void return_raw_string(void* data, U size, NativeStringRawPoolContext& c) {
    auto* pool = native_string_pool_get_or_create_00419cc0(
        c.actual_published_01090aa8, c.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, size,
        c.actual_small_returns_disabled_01090aa4);
}
struct SupportTemporary {
    void* borrowed_com;
    NativeString name;
};
static_assert(sizeof(SupportTemporary) == 12 && offsetof(SupportTemporary, name) == 4);

// One concrete shared source schedule. The optional retained frame is the old
// API's diagnostic storage; the actual route has only local native temporaries
// and its three recovered cleanup states. No provider is a host callback.
struct ConstructionExecution {
    NativeD3d9ShaderConstructionActualContext* actual;
    Op* retained;
    bool pixel;
    int state{-1};
    NativeD3d9ShaderStorage* owner{};
    NativeString local_first;
    SupportTemporary local_second;
    U first_length{};
    char* second_data{};
    NativeString& first() noexcept { return retained ? retained->first_name : local_first; }
    NativeString& second() noexcept { return retained ? retained->second_name : local_second.name; }
    void site(U pixel_site, U vertex_site) noexcept {
        if (retained) retained->native_site = pixel ? pixel_site : vertex_site;
    }
    void resize(NativeString& name, U length) {
        if (actual) resize_native_string_header_0041dd40(&name, actual->strings, length, true);
        else resize_native_string_header_0041dd40(&name, retained->context->strings, length, true);
    }
    void return_string(void* data, U size) {
        if (actual) return_raw_string(data, size, actual->strings);
        else retained->context->strings.release(static_cast<char*>(data), size);
    }
    void support() {
        if (actual) (void)resource_support_singleton_00b3e730(
            actual->actual_support_0108fedc, actual->strings.actual_manager_publication_01090aa0);
        else (void)resource_support_singleton_00b3e730(
            retained->context->actual_support_0108fedc, retained->context->actual_lifetime);
    }
    void register_owner() {
        site(0xb5fad4, 0xb5fb5d);
        void* renderer = actual ? actual->actual_renderer_00f8d394
            : retained->context->actual_renderer_00f8d394;
        if (actual) {
            if (pixel) register_native_pixel_shader_00b289f0(renderer, owner);
            else register_native_vertex_shader_00b289a0(renderer, owner);
        } else {
            retained->registry = std::make_unique<NativeD3d9ShaderRegistryOperation>();
            if (pixel) register_native_pixel_shader_00b289f0(renderer, owner, *retained->registry);
            else register_native_vertex_shader_00b289a0(renderer, owner, *retained->registry);
        }
    }
    ~ConstructionExecution() noexcept {
        if (!actual) return; // The legacy interface deliberately retains failures.
        if (state == 2) {
            state = 1;
            destroy_native_buffer_diagnostic_record_00b3f4c0(&local_second, actual->strings);
        }
        if (state == 1) {
            state = 0;
            destroy_native_string_header_0041dd20(&local_first, actual->strings);
        }
        if (state == 0) {
            state = -1;
            if (pixel) destroy_native_pixel_shader_base_00b5e720(owner);
            else destroy_native_vertex_shader_base_00b5e7e0(owner);
        }
    }
};
NativeD3d9ShaderStorage* construct_shared(void* storage, void* com, ConstructionExecution& e) {
    e.owner = ::new (storage) NativeD3d9ShaderStorage;
    if (e.retained) e.retained->owner = e.owner;
    put(e.owner, 0, 0xceb130);
    e.owner->references_04.store(1, std::memory_order_relaxed);
    e.state = 0;
    if (e.retained) e.retained->base_published = true;
    put(e.owner, 0, e.pixel ? 0xd62a60 : 0xd62a70);
    put(e.owner, 0xc, 0); put(e.owner, 8, 0);
    if (com) {
        put(e.owner, 8, reinterpret_cast<U>(com));
        e.site(0xb5fa08, 0xb5fb48);
        if (e.retained) e.retained->com_addref_entered = true;
        using AddRef = U(__stdcall*)(void*);
        const auto target = reinterpret_cast<AddRef>(word(pointer(com), 4));
        (void)target(com);
        if (e.retained) e.retained->com_addref_returned = true;
    }
    // Native EDI is literal zero across the ABI-preserving AddRef. The old-COM
    // Release sites B5FA14/B5FB54 are unreachable, including COM self-rebinding.
    if (!e.pixel) e.register_owner();
    auto& first = e.first(); auto& second = e.second();
    put(&first, 0, 0); put(&first, 4, 0);
    if (e.retained) e.retained->first_live = true;
    e.site(0xb5fa26, 0xb5fb72);
    e.resize(first, e.pixel ? 11u : 12u);
    void* first_data = pointer(&first, 4);
    e.first_length = word(&first);
    if (e.retained) e.retained->first_length = e.first_length;
    if (first_data) std::memmove(first_data, e.pixel ? "PixelShader" : "VertexShader", e.first_length + 1u);
    void* const com_snapshot = pointer(e.owner, 8);
    e.state = 1; // CC1228/CC1258: first is armed AFTER resize and copy.
    if (e.retained) e.retained->support_com_snapshot = com_snapshot;
    else e.local_second.borrowed_com = com_snapshot;
    put(&second, 0, 0); put(&second, 4, 0);
    if (e.retained) e.retained->second_live = true;
    e.site(0xb5fa64, 0xb5fbb0);
    e.resize(second, e.first_length);
    e.second_data = static_cast<char*>(pointer(&second, 4));
    if (e.retained) {
        e.retained->second_data = e.second_data;
        e.retained->second_captured = true;
    }
    if (e.first_length) std::memmove(e.second_data, pointer(&first, 4), word(&second));
    e.state = 2; // CC1230/CC1260: support temporary armed AFTER second copy.
    e.site(0xb5fa89, 0xb5fbd5); e.support();
    e.state = 1; // Disarm before returning captured second data/current size.
    e.site(0xb5faa9, 0xb5fbf5);
    if (e.second_data) e.return_string(e.second_data, word(&second) + 1u);
    if (e.retained) e.retained->second_live = false;
    void* const current_first = pointer(&first, 4);
    e.state = 0; // Disarm before returning current first data/captured size.
    e.site(0xb5fac8, 0xb5fc14);
    if (current_first) e.return_string(current_first, e.first_length + 1u);
    if (e.retained) e.retained->first_live = false;
    if (e.pixel) e.register_owner();
    e.state = -1;
    return e.owner;
}
} // namespace

void __fastcall destroy_native_pixel_shader_base_00b5e720(void* p) noexcept {
    destroy_native_ref_counted_base_00bd30f0(p);
}
void __fastcall destroy_native_vertex_shader_base_00b5e7e0(void* p) noexcept {
    destroy_native_ref_counted_base_00bd30f0(p);
}
void destroy_native_buffer_diagnostic_record_00b3f4c0(void* temporary, NativeStringRawPoolContext& c) {
    // Existing 41DD20 raw body has exactly this current data/length schedule.
    destroy_native_string_header_0041dd20(static_cast<char*>(temporary) + 4, c);
}
NativeD3d9ShaderStorage* construct_native_pixel_shader_00b5f9b0(void* p, void* com,
    NativeD3d9ShaderConstructionActualContext& c) {
    ConstructionExecution execution{&c, nullptr, true};
    return construct_shared(p, com, execution);
}
NativeD3d9ShaderStorage* construct_native_vertex_shader_00b5faf0(void* p, void* com,
    NativeD3d9ShaderConstructionActualContext& c) {
    ConstructionExecution execution{&c, nullptr, false};
    return construct_shared(p, com, execution);
}
namespace detail {
NativeD3d9ShaderStorage* construct_native_shader_retained(void* p, void* com,
    NativeD3d9ShaderConstructionContext& c, Op& a, bool pixel) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("shader construction operation is one-shot");
    a.function = pixel ? 0xb5f9b0 : 0xb5faf0; a.context = &c; a.argument_com = com;
    a.phase = Op::Phase::running;
    try {
        ConstructionExecution execution{nullptr, &a, pixel};
        auto* result = construct_shared(p, com, execution);
        a.phase = Op::Phase::complete; return result;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace detail
} // namespace bsp
