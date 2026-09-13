#include "bsp/native_d3d9_shader_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeD3d9ShaderLifetimeOperation;
constexpr U pixel_profile = 0x00d62a60, vertex_profile = 0x00d62a70;
static_assert(sizeof(void*) == 4);
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + offset);
}
void put(void* p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(static_cast<char*>(p) + offset) = value;
}
void begin(Op& a, NativeD3d9ShaderStorage* owner, U function, U flags = 0) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("shader lifetime operation is one-shot");
    a.owner = owner; a.function = function; a.flags = flags; a.phase = Op::Phase::running;
}
struct BaseCleanup final {
    NativeD3d9ShaderStorage& owner;
    Op& operation;
    bool armed{true};
    void run() noexcept {
        if (!armed) return;
        destroy_native_ref_counted_base_00bd30f0(&owner);
        operation.base_destroyed = true; armed = false;
    }
    ~BaseCleanup() { run(); }
};
void support(NativeD3d9ShaderLifetimeContext& c, Op& a, U site) {
    a.native_site = site; a.support_entered = true;
    (void)resource_support_singleton_00b3e730(c.actual_support_0108fedc, c.actual_lifetime);
    a.support_returned = true;
}
void unregister(NativeD3d9ShaderStorage& owner, NativeD3d9ShaderLifetimeContext& c,
    Op& a, bool pixel) {
    a.captured_renderer = c.actual_renderer_00f8d394;
    a.native_site = pixel ? 0xb5f447 : 0xb5f4c2;
    if (pixel) (void)unregister_native_pixel_shader_00b268c0(a.captured_renderer, &owner);
    else (void)unregister_native_vertex_shader_00b268a0(a.captured_renderer, &owner);
    a.unregister_returned = true;
}
void destroy(NativeD3d9ShaderStorage& owner, NativeD3d9ShaderLifetimeContext& c,
    Op& a, bool pixel) {
    a.destructor_function = pixel ? 0xb5f410 : 0xb5f490;
    owner.native_vtable_00 = pixel ? pixel_profile : vertex_profile;
    BaseCleanup cleanup{owner, a};
    if (pixel) { support(c, a, 0xb5f43b); unregister(owner, c, a, true); }
    else { unregister(owner, c, a, false); support(c, a, 0xb5f4c7); }
    a.captured_com = reinterpret_cast<void*>(word(&owner, 8));
    if (a.captured_com) {
        const auto current_table = reinterpret_cast<const void*>(word(a.captured_com));
        const auto entry = word(current_table, 8);
        a.native_site = pixel ? 0xb5f459 : 0xb5f4d9;
        a.com_release_entered = true;
        (void)reinterpret_cast<U(__stdcall*)(void*)>(entry)(a.captured_com);
        a.com_release_returned = true;
        put(&owner, 8, 0);
    }
    a.native_site = pixel ? 0xb5f46c : 0xb5f4ec;
    cleanup.run();
}
void run_destroy(NativeD3d9ShaderStorage& owner, NativeD3d9ShaderLifetimeContext& c,
    Op& a, bool pixel) {
    begin(a, &owner, pixel ? 0xb5f410 : 0xb5f490);
    try { destroy(owner, c, a, pixel); a.phase = Op::Phase::complete; }
    catch (...) { a.phase = Op::Phase::failed; throw; }
}
NativeD3d9ShaderStorage* run_delete(NativeD3d9ShaderStorage* owner, U flags,
    NativeD3d9ShaderLifetimeContext& c, Op& a, bool pixel) {
    begin(a, owner, pixel ? 0xb5f6e0 : 0xb5f700, flags);
    try {
        destroy(*owner, c, a, pixel);
        if (flags & 1u) {
            a.native_site = pixel ? 0xb5f6f0 : 0xb5f710; a.free_entered = true;
            singleton_lifetime_free(owner);
            a.free_returned = true;
        }
        a.phase = Op::Phase::complete;
        return owner;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace
NativeD3d9ShaderLifetimeOperation::~NativeD3d9ShaderLifetimeOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeD3d9ShaderLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running) std::terminate();
    if (phase == Phase::failed) phase = Phase::diagnostic_retired;
}
bool unregister_native_vertex_shader_00b268a0(void* renderer, void* owner) noexcept {
    return remove_native_vertex_shader_registry_00b253e0(static_cast<char*>(renderer) + 0x1ac4, &owner);
}
bool unregister_native_pixel_shader_00b268c0(void* renderer, void* owner) noexcept {
    return remove_native_pixel_shader_registry_00b25450(static_cast<char*>(renderer) + 0x1ad0, &owner);
}
void destroy_native_pixel_shader_00b5f410(NativeD3d9ShaderStorage& owner,
    NativeD3d9ShaderLifetimeContext& c, Op& a) { run_destroy(owner, c, a, true); }
void destroy_native_vertex_shader_00b5f490(NativeD3d9ShaderStorage& owner,
    NativeD3d9ShaderLifetimeContext& c, Op& a) { run_destroy(owner, c, a, false); }
NativeD3d9ShaderStorage* delete_native_pixel_shader_00b5f6e0(NativeD3d9ShaderStorage* owner,
    U flags, NativeD3d9ShaderLifetimeContext& c, Op& a) { return run_delete(owner, flags, c, a, true); }
NativeD3d9ShaderStorage* delete_native_vertex_shader_00b5f700(NativeD3d9ShaderStorage* owner,
    U flags, NativeD3d9ShaderLifetimeContext& c, Op& a) { return run_delete(owner, flags, c, a, false); }

NativeD3d9ShaderReference::NativeD3d9ShaderReference(NativeD3d9ShaderStorage& owner,
    NativeD3d9ShaderLifetimeContext& context, NativeD3d9ShaderCompanionDisposal disposal)
    : RenderCommandReference(owner.references_04), storage_(owner), context_(context), disposal_(disposal) {
    if (!disposal_.retire || reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("shader companion requires live raw+04 and explicit retirement");
    require_virtual0();
}
NativeD3d9ShaderReference::~NativeD3d9ShaderReference() {
    if (phase_ != Phase::retired) std::terminate();
}
const volatile U* NativeD3d9ShaderReference::table(U profile) const noexcept {
    const volatile U* result = nullptr;
    if (profile == pixel_profile) result = context_.actual_pixel_profile_00d62a60;
    else if (profile == vertex_profile) result = context_.actual_vertex_profile_00d62a70;
    if (!result) std::terminate();
    return result;
}
void NativeD3d9ShaderReference::require_virtual0() const noexcept {
    if (table(storage_.native_vtable_00)[0] != 0x00bd30e0) std::terminate();
}
void NativeD3d9ShaderReference::delete_vslot04(void* owner, U profile, U flags) {
    if (owner != &storage_ || phase_ != Phase::destroying) std::terminate();
    const U entry = table(profile)[1];
    if (entry == 0x00b5f6e0) (void)delete_native_pixel_shader_00b5f6e0(&storage_, flags, context_, terminal_);
    else if (entry == 0x00b5f700) (void)delete_native_vertex_shader_00b5f700(&storage_, flags, context_, terminal_);
    else std::terminate();
}
void NativeD3d9ShaderReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_virtual0();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    invoke_native_ref_counted_delete_00bd30e0(&storage_, *this);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
