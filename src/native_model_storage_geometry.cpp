#include "bsp/native_model_storage_geometry.hpp"
#include "bsp/native_model_resource_storage.hpp"
#include <atomic>
#include <cstddef>
#include <cstring>
#include <exception>
#include <intrin.h>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Model geometry association requires MSVC Win32 and its x86 inline assembler.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeModelStorageGeometryPhase;
using Scalars = NativeModelStorageGeometryScalars;
static_assert(sizeof(void*) == 4);
static_assert(std::is_standard_layout_v<Scalars>);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(reinterpret_cast<Word>(p) + offset), 4);
    _ReadWriteBarrier();
    return value;
}
void write(void* p, Word offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(at(p, offset), &value, 4);
    _ReadWriteBarrier();
}
void require_model(void* model, NativeModelStorageContext& c) {
    if (&c.node.trees.owners != static_cast<NativeRenderActualOwners*>(&c.node.scenes.owners) ||
        &c.node.scenes.decrement_00ce2220 != &c.node.trees.decrement_00ce2220)
        throw std::invalid_argument("raw Model geometry requires SAME actual owners and decrement cell");
    if (!model || reinterpret_cast<Word>(model) % 4u != 0)
        throw std::invalid_argument("raw Model geometry requires an aligned actual Model");
    auto* const reference = c.node.scenes.owners.find(model);
    auto* const typed = dynamic_cast<NativeModelStorageReference*>(reference);
    if (!typed || typed->retired() || static_cast<void*>(&typed->reference_count) != at(model, 4) ||
        typed->reference_count.load(std::memory_order_relaxed) <= 0 || read(model) != 0x00d62de8)
        throw std::invalid_argument("raw Model geometry requires its live canonical raw Model");
    // 481 keeps this helper private. Repeat only the checked public pool view;
    // no new pool, slot state, count or owner is created here.
    const auto& pool = c.pool_01090054.storage();
    const Word slab_index = NativeModelPool::live_slab_index(model);
    if (!pool.slabs_28 || slab_index >= pool.slab_count_2c)
        throw std::invalid_argument("raw Model geometry requires SAME allocated pool slab");
    const auto* const slab = pool.slabs_28[slab_index];
    const Word offset = reinterpret_cast<Word>(model) - reinterpret_cast<Word>(slab);
    if (!slab || offset >= NativeModelPool::slot_bytes * NativeModelPool::slots_per_slab ||
        offset % NativeModelPool::slot_bytes != 0)
        throw std::invalid_argument("raw Model geometry requires exact SAME pool slot");
    const Word count = *reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3140);
    if (count > NativeModelPool::slots_per_slab)
        throw std::invalid_argument("raw Model geometry pool has invalid free-count");
    const auto* const free_slots = reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3100);
    for (Word index = 0; index < count; ++index) {
        if (free_slots[index] >= NativeModelPool::slots_per_slab ||
            free_slots[index] == offset / NativeModelPool::slot_bytes)
            throw std::invalid_argument("raw Model geometry cannot use invalid/returned pool storage");
    }
}
void require_target(NativeTextureSurfaceReferenceIncrement target) {
    if (!target) throw std::logic_error("raw Model geometry requires the genuine reached import");
}
void terminal(void* captured, NativeNodeBaseDestructionContext& node) {
    auto& reference = node.scenes.owners.resolve_actual(captured);
    if (static_cast<void*>(&reference.reference_count) != at(captured, 4) ||
        reference.reference_count.load(std::memory_order_relaxed) != 0)
        throw std::logic_error("raw Model geometry terminal requires SAME observed-zero actual count");
    // The existing actual-owner contract supplies its genuine CURRENT0 family.
    // No Model/node extent or scene-null test is imposed on opaque geometry.
    reference.release_zero_references();
}

enum ScalarOffsets : int {
    geom_first_bits = static_cast<int>(offsetof(Scalars, first_argument_bits)),
    geom_sentinel_bits = static_cast<int>(offsetof(Scalars, sentinel_bits)),
    geom_second_bits = static_cast<int>(offsetof(Scalars, second_argument_bits)),
    geom_first_read = static_cast<int>(offsetof(Scalars, first_read)),
    geom_sentinel_read = static_cast<int>(offsetof(Scalars, sentinel_read)),
    geom_first_stored = static_cast<int>(offsetof(Scalars, first_stored)),
    geom_second_read = static_cast<int>(offsetof(Scalars, second_read)),
    geom_second_stored = static_cast<int>(offsetof(Scalars, second_stored))
};

// No C++ float argument/value or floating conversion crosses this boundary.
// Added MOV/MOVSS diagnostic stores use disjoint metadata, have no callback and
// do not change MXCSR or the flags used by the two native compare/branch pairs.
__declspec(noinline) void scalar_tail(void* model, const volatile Word* first_argument,
    const volatile Word* sentinel, const volatile Word* second_argument,
    Scalars* output, Word* active_site) {
    __asm {
        mov edi, model
        mov ebx, output
        mov ecx, active_site
        mov edx, first_argument
        mov dword ptr [ecx], 00b751b0h
        movss xmm1, dword ptr [edx]
        movss dword ptr [ebx + geom_first_bits], xmm1
        mov byte ptr [ebx + geom_first_read], 1
        mov edx, sentinel
        mov dword ptr [ecx], 00b751b6h
        movss xmm0, dword ptr [edx]
        movss dword ptr [ebx + geom_sentinel_bits], xmm0
        mov byte ptr [ebx + geom_sentinel_read], 1
        mov dword ptr [ecx], 00b751beh
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jnp geom_skip_first_store
        mov dword ptr [ecx], 00b751c7h
        movss dword ptr [edi + 178h], xmm1
        mov byte ptr [ebx + geom_first_stored], 1
    geom_skip_first_store:
        mov edx, second_argument
        mov dword ptr [ecx], 00b751cfh
        movss xmm1, dword ptr [edx]
        movss dword ptr [ebx + geom_second_bits], xmm1
        mov byte ptr [ebx + geom_second_read], 1
        mov dword ptr [ecx], 00b751d5h
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jnp geom_skip_second_store
        mov dword ptr [ecx], 00b751deh
        movss dword ptr [edi + 17ch], xmm1
        mov byte ptr [ebx + geom_second_stored], 1
    geom_skip_second_store:
    }
}
} // namespace

NativeModelStorageGeometryFrame::NativeModelStorageGeometryFrame(
    const volatile Word& unused, const volatile Word& geometry,
    const volatile Word& first, const volatile Word& second) noexcept
    : unused_argument_(&unused), geometry_argument_(&geometry),
      scalar_178_argument_(&first), scalar_17c_argument_(&second) {}
NativeModelStorageGeometryFrame::~NativeModelStorageGeometryFrame() noexcept {
    if (requires_process_retention()) std::terminate();
}
NativeModelStorageGeometryPhase NativeModelStorageGeometryFrame::phase() const noexcept { return phase_; }
bool NativeModelStorageGeometryFrame::requires_process_retention() const noexcept {
    return phase_ == Phase::entered || phase_ == Phase::failed;
}

void set_native_model_geometry_storage_00b75170(void* model,
    NativeModelStorageGeometryFrame& frame, NativeModelStorageGeometryContext& c,
    NativeModelStorageGeometryAcquired& a) {
    NativeModelGeometryKernelAccess::require_fresh(frame, a);
    require_model(model, c.storage); // qualified metadata/pool admission, no argument/import reads.
    NativeModelGeometryKernelAccess::execute(model, frame, c.storage.node,
        c.unchanged_00d7a260, a, &c, &c.storage, &c, &c);
}
void NativeModelGeometryKernelAccess::require_fresh(
    NativeModelStorageGeometryFrame& frame, NativeModelStorageGeometryAcquired& a) {
    if (frame.phase_ != Phase::fresh || frame.poisoned_ || a.phase != Phase::fresh ||
        a.frame || a.context || a.actual_model || a.issuer || a.context_identity ||
        a.service_identity || a.sentinel_home || a.node_context) {
        // A rejected same-frame callback poisons the owning invocation even if
        // its caller catches the rejection. Diagnostics cannot clear this fact.
        if (frame.phase_ == Phase::entered) frame.poisoned_ = true;
        throw std::invalid_argument("raw Model geometry requires fresh retained frame/acquisition");
    }
}
void NativeModelGeometryKernelAccess::execute(void* model,
    NativeModelStorageGeometryFrame& frame, NativeNodeBaseDestructionContext& node,
    const volatile Word& unchanged, NativeModelStorageGeometryAcquired& a,
    const void* issuer, const void* context_identity, const void* service_identity,
    NativeModelStorageGeometryContext* legacy_context) {
    frame.phase_ = a.phase = Phase::entered;
    a.actual_model = model;
    a.context = legacy_context;
    a.frame = &frame;
    a.issuer = issuer;
    a.context_identity = context_identity;
    a.service_identity = service_identity;
    a.sentinel_home = &unchanged;
    a.node_context = &node;
    try {
        a.active_call_site = 0x00b75170;
        const volatile Word* const geometry_home = frame.geometry_argument_;
        Word requested_bits;
        // FIRST actual operand capture. One qualified x86 4B representation
        // transfer; no typed Word evaluation over a pointer/float alias.
        __asm {
            mov eax, geometry_home
            mov eax, dword ptr [eax]
            mov requested_bits, eax
        }
        void* const requested = reinterpret_cast<void*>(requested_bits);
        a.captured_requested = requested;
        a.requested_captured = true;
        a.active_call_site = 0x00b75178;
        void* const previous = reinterpret_cast<void*>(read(model, 0x180));
        a.captured_previous = previous;
        a.previous_captured = true;
        a.same_geometry = previous == requested;
        if (previous != requested) {
            write(model, 0x180, requested_bits);
            a.geometry_published = true;
            if (requested) {
                // Current target is sampled only AFTER publication, at B75190.
                const auto increment = node.scenes.increment_00ce221c;
                a.captured_increment = increment;
                a.active_call_site = 0x00b75190;
                require_target(increment);
                (void)increment(static_cast<volatile long*>(at(requested, 4)));
                a.increment_completed = true;
            }
            if (previous) {
                // A preceding increment callback may replace this current cell.
                const auto decrement = node.scenes.decrement_00ce2220;
                a.captured_decrement = decrement;
                a.active_call_site = 0x00b7519e;
                require_target(decrement);
                const long count = decrement(static_cast<volatile long*>(at(previous, 4)));
                a.decrement_completed = true;
                if (count == 0) {
                    a.active_call_site = 0x00b751ae;
                    terminal(previous, node);
                    a.terminal_completed = true;
                }
            }
        }
        // No further read or store of Model180. Even identity reaches scalars.
        // Passing addresses here reads neither scalar nor the sentinel value.
        scalar_tail(model, frame.scalar_178_argument_, &unchanged,
            frame.scalar_17c_argument_, &a.scalars, &a.active_call_site);
        a.native_returned = true;
        if (frame.poisoned_) {
            a.source_poisoned = true;
            throw std::logic_error("raw Model geometry owning invocation was poisoned");
        }
        frame.phase_ = a.phase = Phase::complete;
    } catch (...) {
        frame.phase_ = a.phase = Phase::failed;
        throw; // No unwind action/rollback exists in B75170.
    }
}
void NativeModelGeometryKernelAccess::reject_returned(
    NativeModelStorageGeometryFrame& frame, NativeModelStorageGeometryAcquired& a) {
    frame.poisoned_ = true;
    frame.phase_ = a.phase = Phase::failed;
    a.source_poisoned = true;
    throw std::logic_error("raw Model geometry issuer failed during the completed Native body");
}
} // namespace bsp
