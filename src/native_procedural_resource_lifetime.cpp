#include "bsp/native_procedural_resource_lifetime.hpp"
#include "bsp/native_cube_texture_owner_array_reserve.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native procedural resource lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeProceduralResourceLifetimeOperation;
static_assert(sizeof(void*) == 4 && sizeof(std::atomic<std::int32_t>) == 4);
void* at(const void* p, U offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p) + offset);
}
volatile U& word(void* p, U offset = 0) noexcept {
    return *static_cast<volatile U*>(at(p, offset));
}
void* pointer(void* p, U offset = 0) noexcept { return reinterpret_cast<void*>(word(p, offset)); }
std::int32_t signed_bits(U bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, 4);
    return result;
}
std::atomic<std::int32_t>& references(void* p) noexcept {
    return *std::launder(static_cast<std::atomic<std::int32_t>*>(at(p, 4)));
}
void begin(Op& op, U function, void* owner) {
    if (op.phase != Op::Phase::fresh) throw std::logic_error("procedural lifetime operation is one-shot");
    op.phase = Op::Phase::running;
    op.function = function;
    op.owner = owner;
}
template<class F> void invoke(Op& op, F body) {
    try { body(); op.phase = Op::Phase::complete; }
    catch (...) { op.phase = Op::Phase::failed; throw; }
}
void destroy_array(void* owner, Op& op, bool unwind) {
    void* const header = at(owner, 0x10);
    op.native_site = unwind ? 0x00737bf5 : 0x00c3051c;
    resize_native_procedural_pointer_array_00737390(header, 0);
    op.array_allocation = pointer(header);
    op.native_site = unwind ? 0x00737bfd : 0x00c30524;
    singleton_lifetime_free(op.array_allocation);
    op.array_free_returned = true;
}
void destroy_body(void* owner, NativeRenderActualOwners& owners, Op& op) {
    word(owner) = 0x00d79b54;
    bool array_armed = true;
    try {
        while (word(owner, 0x14) != 0) {
            const U count = word(owner, 0x14);
            void* const data = pointer(owner, 0x10);
            void* const child = pointer(at(data, count * 4u - 4u));
            op.child = child;
            op.child_release_started = true;
            op.child_release_returned = false;
            op.native_site = 0x00c304ee;
            release_native_render_actual_owner(owners, child);
            op.child_release_returned = true;
            const U current_count = word(owner, 0x14);
            if (current_count != 0) word(owner, 0x14) = current_count - 1u;
        }
        // Native state1 -> state0 precedes resize/free. No repeated array
        // cleanup if resize throws; state0 still stamps the base.
        array_armed = false;
        destroy_array(owner, op, false);
    } catch (...) {
        if (array_armed) {
            // CC7EB8 ->737BF0. A second cleanup exception cannot resume the
            // original unwind; match the source C++ destructor boundary.
            try { destroy_array(owner, op, true); }
            catch (...) { std::terminate(); }
        }
        op.native_site = 0x00cc7eb3; // C30260 tail-forwards B19750.
        destroy_native_procedural_resource_base_00b19750(owner);
        op.base_destroyed = true;
        throw;
    }
    op.native_site = 0x00c30536;
    destroy_native_procedural_resource_base_00b19750(owner);
    op.base_destroyed = true;
}
void* delete_body(void* owner, U flags, NativeRenderActualOwners& owners, Op& op, U function) {
    begin(op, function, owner);
    invoke(op, [&] {
        destroy_body(owner, owners, op);
        if (flags & 1u) {
            op.native_site = function == 0x00bbc6d0 ? 0x00bbc6e0 : 0x00bbc800;
            singleton_lifetime_free(owner);
            op.owner_free_returned = true;
        }
    });
    return owner;
}
const volatile U* current_table(U profile, NativeProceduralResourceLifetimeContext& context) {
    switch (profile) {
    case 0x00d64478: return context.actual_profile_00d64478;
    case 0x00d644b4: return context.actual_profile_00d644b4;
    default: throw std::invalid_argument("unimplemented procedural resource profile");
    }
}
class TerminalCalls final : public NativeRefCountedDeleteCalls {
public:
    explicit TerminalCalls(NativeProceduralResourceLifetimeContext& context) : context_(context) {}
    void delete_vslot04(void* owner, U profile, U flags) override {
        const auto* const table = current_table(profile, context_);
        if (!table) throw std::invalid_argument("missing actual procedural resource profile");
        const U scalar = table[1];
        Op op;
        if (profile == 0x00d64478 && scalar == 0x00bbc6d0) {
            delete_native_caustics_resource_00bbc6d0(owner, flags, context_.owners, op);
        } else if (profile == 0x00d644b4 && scalar == 0x00bbc7f0) {
            delete_native_shore_wave_resource_00bbc7f0(owner, flags, context_.owners, op);
        } else {
            throw std::invalid_argument("unimplemented current procedural resource slot04");
        }
    }
private:
    NativeProceduralResourceLifetimeContext& context_;
};
} // namespace

NativeProceduralResourceLifetimeOperation::~NativeProceduralResourceLifetimeOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeProceduralResourceLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed) std::terminate();
    phase = Phase::diagnostic_retired;
}

void resize_native_procedural_pointer_array_00737390(void* header, std::int32_t requested) {
    if (requested > signed_bits(word(header, 8)))
        reserve_native_cube_texture_owner_array_00735ff0(header, requested);
    U index = word(header, 4);
    while (signed_bits(index) < requested) {
        void* const slot = at(pointer(header), index * 4u);
        if (slot) word(slot) = 0;
        ++index;
    }
    while (requested < signed_bits(word(header, 4))) word(header, 4) = word(header, 4) - 1u;
    word(header, 4) = static_cast<U>(requested);
}
void destroy_native_procedural_resource_base_00b19750(void* owner) noexcept {
    word(owner) = 0x00d5c104;
    destroy_native_ref_counted_base_00bd30f0(owner);
}
void destroy_native_procedural_resource_00c304a0(void* owner, NativeRenderActualOwners& owners, Op& op) {
    begin(op, 0x00c304a0, owner);
    invoke(op, [&] { destroy_body(owner, owners, op); });
}
void* delete_native_caustics_resource_00bbc6d0(void* owner, U flags, NativeRenderActualOwners& owners, Op& op) {
    return delete_body(owner, flags, owners, op, 0x00bbc6d0);
}
void* delete_native_shore_wave_resource_00bbc7f0(void* owner, U flags, NativeRenderActualOwners& owners, Op& op) {
    return delete_body(owner, flags, owners, op, 0x00bbc7f0);
}

NativeProceduralResourceReference::NativeProceduralResourceReference(void* owner,
    NativeProceduralResourceLifetimeContext& context, NativeProceduralResourceCompanionDisposal disposal)
    : RenderCommandReference(references(owner)), actual_resource_(owner), context_(context), disposal_(disposal) {
    if (!disposal.retire || reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("procedural reference requires live actual storage and retirement");
    const U profile = word(owner);
    const auto* const table = current_table(profile, context_);
    if (!table || table[0] != 0x00bd30e0 ||
        table[1] != (profile == 0x00d64478 ? 0x00bbc6d0u : 0x00bbc7f0u))
        throw std::invalid_argument("procedural reference requires actual recovered virtual0/04");
}
NativeProceduralResourceReference::~NativeProceduralResourceReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeProceduralResourceReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0) std::terminate();
    const auto* const table = current_table(word(actual_resource_), context_);
    if (!table || table[0] != 0x00bd30e0) std::terminate();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    TerminalCalls calls(context_);
    invoke_native_ref_counted_delete_00bd30e0(actual_resource_, calls);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
