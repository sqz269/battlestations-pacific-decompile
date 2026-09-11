#pragma once

#include "bsp/game_entry.hpp"
#include "bsp/system_time_constants.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Explicit host allocation boundary: native object sizes are evidence, while
// host_bytes allocates the reconstructed C++ type. The returned bytes are raw.
enum class SingletonAllocationKind { manager, pointer_slots, critical_section, object };
struct SingletonAllocationRequest {
    SingletonAllocationKind kind;
    std::size_t native_bytes;
    std::size_t host_bytes;
};
void* singleton_lifetime_allocate(const SingletonAllocationRequest&);
void singleton_lifetime_free(void*) noexcept;

struct SingletonLifetimeCallbacks {
    void* context;
    // The actual registered pointer is the owner. Dispatch its deleting
    // destructor, including its unregister/free behavior, with native flag 1.
    void (*destroy_registered)(void* context, void* owner, std::uint32_t flags) noexcept;
    // 00BF6713 can return. Validation sites continue after this call returns.
    void (*invalid_parameter)(void* context);
};

// Retained pointer-container storage corresponding to native +4/+8/+C.
// Mutable access supports owners whose validation callback repairs the storage.
struct SingletonPointerSlots {
    void** begin;
    void** end;
    void** capacity_end;
};

// These perform only the OS operation. ParticleClockLifetimeAccess adjusts the
// projected recursion_18 itself; using manager.lock() there would double count.
void singleton_enter_critical_section(SystemSingletonCriticalSection&);
void singleton_leave_critical_section(SystemSingletonCriticalSection&) noexcept;

class SingletonLifetimeDomain;
class ConcreteSingletonLifetimeManager final : public SingletonLifetimeManager {
public:
    ~ConcreteSingletonLifetimeManager() override;
    ConcreteSingletonLifetimeManager(const ConcreteSingletonLifetimeManager&) = delete;
    ConcreteSingletonLifetimeManager& operator=(const ConcreteSingletonLifetimeManager&) = delete;

    void lock() override;
    void unlock() override;
    void register_object(void* object) override; // 00BD0C30, null ignored after validation
    void unregister_object(void* object);       // 00BCFCA0, zero first match, keep hole
    // 00BD0D70, thiscall(manager, object, after), RET8. Remove the first object
    // match, then insert it after the first remaining after match. Both searches
    // must succeed; native validation may return and does not add recovery.
    void move_object_after_00bd0d70(void* object, void* after);
    void append_pointer_00bd0bc0(void* const* value);
    std::uint32_t count_00bcf910() const noexcept;
    std::uint32_t capacity() const noexcept;
    SingletonPointerSlots& pointer_slots() noexcept { return slots_; }
    SystemSingletonLifetimeOwner& system_owner() noexcept { return owner_; }

private:
    friend class SingletonLifetimeDomain;
    explicit ConcreteSingletonLifetimeManager(SingletonLifetimeCallbacks);
    void destroy_00bd0400();
    void destroy_owned_section_0041cc80() noexcept;
    void invalid_parameter();
    void insert_pointer_at_checked_00bd08d0(void** position, void* const* value);

    SingletonLifetimeCallbacks callbacks_;
    SingletonPointerSlots slots_{};
    struct OwnedCriticalSection;
    OwnedCriticalSection* owned_section_ = nullptr;
    SystemSingletonCriticalSection* section_10_ = nullptr;
    SystemSingletonLifetimeOwner owner_{this, section_10_};
};

// One instance is one native global 01090AA0 lifetime domain. Startup and all
// singleton adapters must share it. The native getter has no publication lock.
class SingletonLifetimeDomain final {
public:
    explicit SingletonLifetimeDomain(SingletonLifetimeCallbacks);
    ~SingletonLifetimeDomain();
    SingletonLifetimeDomain(const SingletonLifetimeDomain&) = delete;
    SingletonLifetimeDomain& operator=(const SingletonLifetimeDomain&) = delete;

    ConcreteSingletonLifetimeManager* get_manager_00415350();
    ConcreteSingletonLifetimeManager* published_manager() const noexcept { return published_; }
    // 008F8449..008F8463: destroy, free, then clear published pointer. Registered
    // destructors may call get_manager/register/unregister while it is draining.
    void shutdown();

private:
    SingletonLifetimeCallbacks callbacks_;
    ConcreteSingletonLifetimeManager* volatile published_ = nullptr;
};

} // namespace bsp
