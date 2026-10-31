#include "bsp/native_model_resource_storage.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_matrix_storage_lifetime.hpp"
#include <cstddef>
#include <cstring>
#include <intrin.h>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Model resource transport requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(Word) == 4);
static_assert(sizeof(std::uint16_t) == 2);
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(offsetof(NativeNodeStorage, references_04) == 4);
static_assert(offsetof(NativeNodeStorage, scene_170) == 0x170);
static_assert(sizeof(decltype(NativeNodeStorage::scene_170)) == 4);
static_assert(sizeof(NativeModelTailStorage) == 0x10);
static_assert(offsetof(NativeModelTailStorage, scalar_178) + 0x174 == 0x178);
static_assert(offsetof(NativeModelTailStorage, scalar_17c) + 0x174 == 0x17c);
static_assert(offsetof(NativeModelTailStorage, geometry_180) + 0x174 == 0x180);
std::uint16_t read_pool_half(const std::byte* address) noexcept {
    // The actual producer begins SlabBytes and copies metadata bytes. No
    // uint16 scalar/array object at this backing address is assumed or begun.
    std::uint16_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, address, sizeof value);
    _ReadWriteBarrier();
    return value;
}
}

NativeMatrixStorageGeometryContext::NativeMatrixStorageGeometryContext(
    const NativeMatrixStorageContext& matrix,
    const NativeModelStorageGeometryContext& roles)
    : matrix_(&matrix), roles_(&roles) {
    // Compare genuine existing service/cell identities only. The distinct
    // Matrix pool is never passed off as the Model188 pool.
    auto& prefix = matrix.prefix();
    if (&roles.storage.node != &prefix.node() ||
        &roles.storage.constants != &prefix.constants() ||
        &roles.storage.negative_zero_00d7a208 != &prefix.negative_zero_00d7a208())
        throw std::invalid_argument("Matrix geometry requires SAME genuine node/constant services");
}

void NativeModelGeometryKernelAccess::require_live_model_slot(void* model,
    NativeModelPool& owner) {
    if (!model || reinterpret_cast<Word>(model) % 4u != 0)
        throw std::invalid_argument("checked Model requires aligned actual188 storage");
    const auto& pool = owner.storage();
    const Word slab_index = NativeModelPool::live_slab_index(model);
    if (!pool.slabs_28 || slab_index >= pool.slab_count_2c)
        throw std::invalid_argument("checked Model requires its current allocated slab");
    const auto* const slab = pool.slabs_28[slab_index];
    const Word offset = reinterpret_cast<Word>(model) - reinterpret_cast<Word>(slab);
    if (!slab || offset >= NativeModelPool::slot_bytes * NativeModelPool::slots_per_slab ||
        offset % NativeModelPool::slot_bytes != 0)
        throw std::invalid_argument("checked Model requires its exact SAME pool slot");
    const Word count = read_pool_half(slab + 0x3140);
    if (count > NativeModelPool::slots_per_slab)
        throw std::invalid_argument("checked Model pool has invalid free-count");
    for (Word index = 0; index < count; ++index) {
        if (read_pool_half(slab + 0x3100 + index * 2u) >= NativeModelPool::slots_per_slab ||
            read_pool_half(slab + 0x3100 + index * 2u) == offset / NativeModelPool::slot_bytes)
            throw std::invalid_argument("checked Model cannot borrow returned/invalid pool storage");
    }
}

void NativeModelReference::set_raw_geometry_00b75170(
    NativeModelStorageGeometryFrame& frame, NativeModelStorageGeometryAcquired& acquired) {
    NativeModelGeometryKernelAccess::require_fresh(frame, acquired);
    auto& raw = require_checked_raw_live();
    try {
        NativeModelGeometryKernelAccess::execute(&owner_.storage.node, frame,
            raw.storage.node, owner_.environment.constants.unchanged_00d7a260,
            acquired, this, &raw.storage, &owner_.environment, nullptr);
        // A nested distinct-frame invocation may have failed and been caught
        // by a callback. Check only retained private metadata AFTER Native work;
        // never re-read payload/count/import/input values here.
        if (owner_.raw_facts_.failed)
            NativeModelGeometryKernelAccess::reject_returned(frame, acquired);
    } catch (...) {
        if (frame.phase() == NativeModelStorageGeometryPhase::failed)
            record_checked_raw_failure(acquired.active_call_site);
        throw; // no Native cleanup or credit adjustment
    }
}
} // namespace bsp
