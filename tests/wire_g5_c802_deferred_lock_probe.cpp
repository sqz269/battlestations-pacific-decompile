#include "bsp/native_deferred_entity_lock.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/cc13_seg49.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <new>

// One lock-lifecycle regression: real lazy publication/recursive acquisition,
// exception release and actual registered-owner drain; then the constructor's
// concrete failure risk (base reset, allocation cleanup, manager unlock).
namespace {
struct ThrowingConstruction final
    : bsp::entity_mission_seg60_argon::DeferredEventLockAccess,
      bsp::cc13_seg49::DeferredEventLockLifetimeHost {
    alignas(4) unsigned char manager[0x14]{};
    bsp::TrackedCriticalSection section{};
    void* publication{};
    unsigned released{};
    ThrowingConstruction() {
        InitializeCriticalSection(&section.native);
        *reinterpret_cast<void**>(manager + 0x10) = &section;
    }
    ~ThrowingConstruction() { DeleteCriticalSection(&section.native); }
    std::uint32_t& global_u32(std::uint32_t address) override {
        assert(address == 0x00f899ec);
        return reinterpret_cast<std::uint32_t&>(publication);
    }
    void*& global_pointer(std::uint32_t address) override {
        assert(address == 0x00f899ec);
        return publication;
    }
    void* singleton_lifetime_get_manager_00415350() override { return manager; }
    void enter_critical_section_00ce2218(void* cs) override {
        EnterCriticalSection(static_cast<CRITICAL_SECTION*>(cs));
    }
    void leave_critical_section_00ce2210(void* cs) override {
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(cs));
    }
    void* operator_new_00bf681b(std::uint32_t bytes) override {
        assert(bytes == 8);
        void* const storage = std::malloc(bytes);
        if (!storage) throw std::bad_alloc{};
        return storage;
    }
    void free_00bf6989(void* storage) override {
        assert(*static_cast<std::uint32_t*>(storage) == 0x00ce3818);
        ++released;
        std::free(storage);
    }
    void free_00bf65ac(void*) override { std::abort(); }
    void* deferred_entity_event_lock_construct_00924200(void* storage) override {
        return bsp::cc13_seg49::deferred_entity_event_lock_construct_00924200(
            *this, {static_cast<std::uint8_t*>(storage)});
    }
    void singleton_lifetime_register_00bd0c30(void*, void*) override { std::abort(); }
    void* critical_section_create_00bd1860() override { throw std::bad_alloc{}; }
    void critical_section_destroy_owned_0041cc80(void*) override { std::abort(); }
};
}

int main() {
    auto& publication = bsp::process_native_deferred_entity_lock_publication();
    auto& manager = bsp::game::game_native_string_process().manager_01090aa0();
    assert(publication == nullptr && manager == nullptr);
    bsp::TrackedCriticalSection* section{};
    try {
        bsp::NativeDeferredEntityEventQueueGuard outer;
        void* const owner = publication;
        assert(owner && *static_cast<std::uint32_t*>(owner) == 0x00d190c8);
        section = *reinterpret_cast<bsp::TrackedCriticalSection**>(
            static_cast<unsigned char*>(owner) + 4);
        assert(section && section->depth == 1);
        {
            bsp::NativeDeferredEntityEventQueueGuard inner;
            assert(publication == owner && section->depth == 2);
        }
        assert(section->depth == 1);
        auto** const begin = *reinterpret_cast<void***>(static_cast<unsigned char*>(manager) + 4);
        auto** const end = *reinterpret_cast<void***>(static_cast<unsigned char*>(manager) + 8);
        assert(end - begin == 1 && begin[0] == owner);
        throw std::bad_alloc{};
    } catch (const std::bad_alloc&) {}
    assert(section->depth == 0 && section->native.RecursionCount == 0);
    bsp::NativeSingletonDeletionBindings bindings{};
    bsp::destroy_native_singleton_manager_00bd0400(manager, bindings);
    assert(publication == nullptr);
    bsp::singleton_lifetime_free(manager);
    manager = nullptr;

    ThrowingConstruction failure;
    try {
        bsp::entity_mission_seg60_argon::deferred_entity_event_lock_get_00924990(failure);
        std::abort();
    } catch (const std::bad_alloc&) {}
    assert(!failure.publication && failure.released == 1);
    assert(failure.section.depth == 0 && failure.section.native.RecursionCount == 0);
    std::puts("c802 deferred lock lifecycle: passed");
}
