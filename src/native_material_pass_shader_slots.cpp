#include "bsp/native_material_pass_shader_slots.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeMaterialPassShaderSlotsOperation;
static_assert(sizeof(void*) == 4);
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + offset);
}
void put(void* p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(static_cast<char*>(p) + offset) = value;
}
void* pointer(const void* p, U offset = 0) noexcept { return reinterpret_cast<void*>(word(p, offset)); }
std::int32_t signed_word(U bits) noexcept {
    std::int32_t result; std::memcpy(&result, &bits, 4); return result;
}
void begin(Op& a, NativeMaterialPassBaseStorage& pass, U function) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("pass shader-slot operation is one-shot");
    a.pass = &pass; a.function = function; a.phase = Op::Phase::running;
}
void assign(NativeMaterialPassBaseStorage& pass, void* incoming, NativeRenderActualOwners& owners,
    Op& a, U function, U offset, U increment_site, U release_site) {
    begin(a, pass, function); a.owners = &owners; a.incoming_identity = incoming;
    a.old_identity = pointer(&pass, offset);
    try {
        if (a.old_identity != incoming) {
            put(&pass, offset, reinterpret_cast<U>(incoming)); a.slot_published = true;
            if (incoming) {
                a.native_site = increment_site;
                auto* count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
                    static_cast<char*>(incoming) + 4));
                count->fetch_add(1, std::memory_order_seq_cst); a.incoming_incremented = true;
            }
            if (a.old_identity) {
                a.native_site = release_site; a.old_release_entered = true;
                // Shared helper decrements SAME actual +04 first and resolves
                // only on zero. It checks the canonical companion's atomic
                // identity and invokes its nonthrowing current-profile terminal.
                release_native_render_actual_owner(owners, a.old_identity);
                a.old_release_returned = true;
            }
        }
        a.phase = Op::Phase::complete;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace
NativeMaterialPassShaderSlotsOperation::~NativeMaterialPassShaderSlotsOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeMaterialPassShaderSlotsOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running) std::terminate();
    if (phase == Phase::failed) phase = Phase::diagnostic_retired;
}
void remove_native_material_pass_sampler_slot_00b5eff0(NativeMaterialPassBaseStorage& pass,
    U slot, Op& a) {
    begin(a, pass, 0xb5eff0); a.slot = slot;
    try {
        for (;;) {
            a.sampled_sampler = static_cast<NativeMaterialStateOwnerStorage*>(pointer(&pass, 0x20));
            a.count = word(a.sampled_sampler, 0xc); a.index = 0;
            if (!a.count) break;
            auto* scan = static_cast<const char*>(pointer(a.sampled_sampler, 8));
            while (word(scan) != slot) {
                ++a.index; scan += 12;
                if (a.index == a.count) { a.phase = Op::Phase::complete; return; }
            }
            if (a.count - 1u != a.index) {
                const U current_count = word(a.sampled_sampler, 0xc);
                const auto last = reinterpret_cast<U>(pointer(a.sampled_sampler, 8)) + current_count * 12u - 12u;
                const auto destination = reinterpret_cast<U>(pointer(a.sampled_sampler, 8)) + a.index * 12u;
                // Native copies three DWORDs in order, without clearing tail.
                put(reinterpret_cast<void*>(destination), 0, word(reinterpret_cast<void*>(last)));
                put(reinterpret_cast<void*>(destination), 4, word(reinterpret_cast<void*>(last), 4));
                put(reinterpret_cast<void*>(destination), 8, word(reinterpret_cast<void*>(last), 8));
            }
            auto* current = pointer(&pass, 0x20);
            a.target_count = word(current, 0xc) - 1u;
            a.resize_rows = reinterpret_cast<NativeMaterialStateArray*>(static_cast<char*>(current) + 8);
            if (signed_word(a.target_count) > signed_word(word(current, 0x10))) {
                a.native_site = 0xb5f068;
                reserve_native_material_sampler_states_00b40be0(*a.resize_rows, signed_word(a.target_count));
            }
            while (signed_word(a.target_count) < signed_word(word(a.resize_rows, 4)))
                put(a.resize_rows, 4, word(a.resize_rows, 4) - 1u);
            put(a.resize_rows, 4, a.target_count); ++a.removed_rows;
        }
        a.phase = Op::Phase::complete;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
void set_native_material_pass_pixel_shader_00b5f080(NativeMaterialPassBaseStorage& pass,
    void* incoming, NativeRenderActualOwners& owners, Op& a) {
    assign(pass, incoming, owners, a, 0xb5f080, 0x58, 0xb5f097, 0xb5f0a5);
}
void set_native_material_pass_vertex_shader_00b5f0c0(NativeMaterialPassBaseStorage& pass,
    void* incoming, NativeRenderActualOwners& owners, Op& a) {
    assign(pass, incoming, owners, a, 0xb5f0c0, 0x54, 0xb5f0d7, 0xb5f0e5);
}
} // namespace bsp
