#pragma once

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"

namespace bsp {

// Complete BD1120. Native ECX pool, stack (size, unused), RET8; EAX block.
// size>=150 calls current CRT malloc (nullable), without dereferencing pool.
// Small requests require the constructed actual owner; exact-size ring or
// unchecked DWORD bump into embedded arena. Zero size neither advances nor
// aligns the bump. No arena bounds or ring-capacity policy is added.
void* allocate_native_string_pool_00bd1120(NativeStringPoolStorage* actual_pool,
    std::uint32_t size);

// Complete BD12A0. Native ECX ring, stack (address of block, size_class), RET8.
// Caller guarantees class<150 and valid native ring indices. Capture *block
// before the first ring write, including when block aliases a slot. No lock.
void return_native_string_pool_small_00bd12a0(NativeStringPoolRingStorage&,
    void* const* block, std::uint32_t size_class) noexcept;

// Complete BD1510. Native ECX pool, stack (block, size, unused), RET0C.
// Large requests free without reading pool or the real01090AA4 gate. Small
// returns read that live gate first; a nonzero value does not touch the owner.
void return_native_string_pool_00bd1510(NativeStringPoolStorage* actual_pool,
    void* block, std::uint32_t size,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4) noexcept;

// Borrow the application's one publication slot, real shutdown gate and
// canonical lifetime domain. The latter must use NativeStringPoolLifetimeBinding
// (composed with all other owners). EVERY operation calls00419CC0, even large
// allocations/frees and disabled small returns: do not cache the pool here.
// NativeStringStorage::release is noexcept: its returning-getter domain is
// covered; C++ failure while lazily recreating a pool terminates under that
// pre-existing interface. This bridge does not claim native EH/SEH ABI parity.
class ActualNativeStringPoolStorage final : public NativeStringStorage {
public:
    ActualNativeStringPoolStorage(NativeStringPoolStorage* volatile& publication,
        volatile std::uint32_t& returns_disabled,
        SingletonLifetimeDomain& lifetime) noexcept;
    char* allocate(std::uint32_t size) override;
    void release(char* block, std::uint32_t size) noexcept override;

private:
    NativeStringPoolStorage* volatile& publication_;
    volatile std::uint32_t& returns_disabled_;
    SingletonLifetimeDomain& lifetime_;
};

} // namespace bsp
