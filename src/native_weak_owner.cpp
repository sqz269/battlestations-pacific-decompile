#include "bsp/native_weak_owner.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Native weak owners target MSVC Win32.");
static_assert(sizeof(NativeWeakHandle) == 0x0c);
static_assert(offsetof(NativeWeakHandle, references_04) == 4);
static_assert(offsetof(NativeWeakHandle, target_08) == 8);
static_assert(sizeof(NativeWeakMutexOwner) == 8);
static_assert(sizeof(NativeWeakPoolStorage) == 0x38);
static_assert(offsetof(NativeWeakPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NativeWeakPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeWeakPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(NativeWeakPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NativeWeakPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NativeWeakPoolStorage, first_free_slab_34) == 0x34);
static_assert(offsetof(NativeGuiSceneStorage, references_04) == 4);
static_assert(offsetof(NativeGuiSceneStorage, weak_handle_08) == 8);
constexpr std::uint32_t reference_table = 0x00ceb130;
constexpr std::uint32_t weak_owner_table = 0x00d190f4;
constexpr std::uint32_t weak_handle_table = 0x00d190b8;
constexpr std::uint32_t mutex_table = 0x00d190b4;
constexpr std::uint32_t simple_owner_table = 0x00ce3818;
constexpr std::uint32_t no_free_slab = 0xffffffffu;
constexpr std::size_t free_indices = 0x800;
constexpr std::size_t free_count = 0x900;
struct SlabBytes { std::byte bytes[NativeWeakHandlePool::slab_bytes]; };

template<class T> T read(const std::byte* address) noexcept {
    T value;
    std::memcpy(&value, address, sizeof(value));
    return value;
}
template<class T> void write(std::byte* address, T value) noexcept {
    std::memcpy(address, &value, sizeof(value));
}
void add_depth(std::int32_t& depth, std::uint32_t amount) noexcept {
    const auto bits = static_cast<std::uint32_t>(depth) + amount;
    std::memcpy(&depth, &bits, sizeof(bits));
}
CRITICAL_SECTION* section(NativeWeakPoolStorage& storage) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(storage.critical_section_0c);
}
void enter(TrackedCriticalSection* lock) {
    if (lock) {
        EnterCriticalSection(&lock->native);
        add_depth(lock->depth, 1);
    }
}
void leave(TrackedCriticalSection* lock) noexcept {
    if (lock) {
        add_depth(lock->depth, 0xffffffffu);
        LeaveCriticalSection(&lock->native);
    }
}
class ManagerGuard final {
public:
    explicit ManagerGuard(SystemSingletonCriticalSection* lock) : lock_(lock) {
        if (lock_) {
            singleton_enter_critical_section(*lock_);
            ++lock_->recursion_18;
        }
    }
    ~ManagerGuard() {
        if (lock_) {
            --lock_->recursion_18;
            singleton_leave_critical_section(*lock_);
        }
    }
private:
    SystemSingletonCriticalSection* lock_;
};
std::byte** allocate_table(std::uint32_t capacity) {
    const std::uint32_t bytes = capacity * 4u;
    return static_cast<std::byte**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes}));
}
std::byte* initialize_slab_009236a0(void* raw, std::uint32_t index) noexcept {
    auto* bytes = (::new (raw) SlabBytes)->bytes;
    write<std::uint16_t>(bytes + free_count, 128);
    for (std::uint32_t i = 0; i < 128; ++i) {
        write<std::uint16_t>(bytes + free_indices + i * 2u,
            static_cast<std::uint16_t>(127u - i));
        write<std::uint32_t>(bytes + i * 0x10u + 0x0c, index);
    }
    // Each object's 0Ch prefix and padding at slab +902 remain untouched.
    return bytes;
}
NativeWeakHandlePool* static_pool_0109ce94;
NativeWeakHandlePool& require_static_pool() {
    if (!static_pool_0109ce94)
        throw std::logic_error("native 0109CE94 weak pool has no actual storage binding");
    return *static_pool_0109ce94;
}
bool is_actual_prefix(NativeWeakOwnerView owner) noexcept {
    const auto identity = reinterpret_cast<std::uintptr_t>(owner.identity);
    return reinterpret_cast<std::uintptr_t>(&owner.vtable_00) == identity
        && reinterpret_cast<std::uintptr_t>(&owner.references_04) == identity + 4u
        && reinterpret_cast<std::uintptr_t>(&owner.weak_handle_08) == identity + 8u;
}
} // namespace

NativeWeakHandlePool::NativeWeakHandlePool(AllocatorListDomain& list,
    NativeWeakPoolStorage& storage) : list_(list), storage_(storage) {
    list_.bind_virtual0(storage_.allocator_00,
        {native_vtable, native_virtual0, this, &invoke_trim});
}
void NativeWeakHandlePool::initialize_00b004b0() {
    list_.prepend_base_element(storage_.allocator_00);
    storage_.allocator_00.native_vtable_00 = native_vtable;
    unsigned unwind_state = 0;
    __try {
        ::new (storage_.critical_section_0c) CRITICAL_SECTION;
        InitializeCriticalSection(section(storage_));
        storage_.recursion_24 = 0;
        unwind_state = 1;
        storage_.slabs_28 = nullptr;
        storage_.slab_count_2c = 0;
        storage_.table_capacity_30 = 0;
        storage_.first_free_slab_34 = no_free_slab;
        unwind_state = 2;
        if (storage_.table_capacity_30 < 32) {
            storage_.table_capacity_30 = 32;
            auto** replacement = allocate_table(32);
            for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
                ::new (replacement + i) std::byte*(storage_.slabs_28[i]);
            if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
            storage_.slabs_28 = replacement;
        }
    } __finally {
        if (AbnormalTermination()) {
            // CBB3B3 -> AFFA20, CBB3A8 -> 402F70, CBB3A0 -> 403970.
            if (unwind_state >= 2 && storage_.slabs_28)
                singleton_lifetime_free(storage_.slabs_28);
            if (unwind_state >= 1) destroy_section_00402f70();
            list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
}
void* NativeWeakHandlePool::allocate_raw_slot_009242f0() {
    EnterCriticalSection(section(storage_));
    add_depth(storage_.recursion_24, 1);
    // This function has no EH guard: allocation failure keeps native pool
    // recursion and publication state. The outer weak-owner guard is separate.
    if (storage_.first_free_slab_34 == no_free_slab) {
        storage_.first_free_slab_34 = storage_.slab_count_2c;
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object,
            slab_bytes, slab_bytes});
        auto* slab = raw ? initialize_slab_009236a0(raw, storage_.first_free_slab_34) : nullptr;
        if (storage_.slab_count_2c == storage_.table_capacity_30) {
            storage_.table_capacity_30 = storage_.table_capacity_30 * 2u + 2u;
            auto** replacement = allocate_table(storage_.table_capacity_30);
            for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i) {
                if (replacement + i)
                    ::new (replacement + i) std::byte*(storage_.slabs_28[i]);
            }
            if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
            // 924386 ADD ESP,4; 924389 publishes EDI after returning free.
            storage_.slabs_28 = replacement;
        }
        auto** destination = storage_.slabs_28 + storage_.slab_count_2c;
        if (destination) ::new (destination) std::byte*(slab);
        ++storage_.slab_count_2c;
    }
    auto* slab = storage_.slabs_28[storage_.first_free_slab_34];
    const auto count = static_cast<std::uint16_t>(read<std::uint16_t>(slab + free_count) - 1u);
    write<std::uint16_t>(slab + free_count, count);
    auto* slot = slab + read<std::uint16_t>(slab + free_indices + count * 2u) * slot_bytes;
    if (count == 0) {
        auto index = storage_.first_free_slab_34 + 1u;
        storage_.first_free_slab_34 = no_free_slab;
        for (; index < storage_.slab_count_2c; ++index) {
            if (read<std::uint16_t>(storage_.slabs_28[index] + free_count) != 0) {
                storage_.first_free_slab_34 = index;
                break;
            }
        }
    }
    add_depth(storage_.recursion_24, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
    return slot;
}
std::uint32_t NativeWeakHandlePool::live_slab_index(const void* slot) noexcept {
    return read<std::uint32_t>(static_cast<const std::byte*>(slot) + 0x0c);
}
void NativeWeakHandlePool::return_raw_slot_00924420(void* slot) {
    EnterCriticalSection(section(storage_));
    add_depth(storage_.recursion_24, 1);
    const auto index = live_slab_index(slot);
    auto* slab = storage_.slabs_28[index];
    const auto count = read<std::uint16_t>(slab + free_count);
    const auto slot_index = (static_cast<std::byte*>(slot) - slab) / 0x10;
    write<std::uint16_t>(slab + free_indices + count * 2u, static_cast<std::uint16_t>(slot_index));
    write<std::uint16_t>(slab + free_count, static_cast<std::uint16_t>(count + 1u));
    if (index < storage_.first_free_slab_34) storage_.first_free_slab_34 = index;
    add_depth(storage_.recursion_24, 0xffffffffu);
    LeaveCriticalSection(section(storage_));
}
void NativeWeakHandlePool::trim_empty_slabs_00b003a0() {
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i) {
        auto* slab = storage_.slabs_28[i];
        if (read<std::uint16_t>(slab + free_count) == 128) {
            singleton_lifetime_free(slab);
            storage_.slabs_28[i] = storage_.slabs_28[storage_.slab_count_2c - 1u];
            --storage_.slab_count_2c;
            if (i < storage_.slab_count_2c) {
                auto* moved = storage_.slabs_28[i];
                for (std::uint32_t j = 0; j < 128; ++j)
                    write<std::uint32_t>(moved + j * 0x10u + 0x0c, i);
            }
            --i; // native SUB EDI,1 causes the moved slab to be rechecked
        }
    }
    storage_.first_free_slab_34 = no_free_slab;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i) {
        if (read<std::uint16_t>(storage_.slabs_28[i] + free_count) != 0) {
            storage_.first_free_slab_34 = i;
            break;
        }
    }
}
void NativeWeakHandlePool::invoke_trim(void* context) {
    static_cast<NativeWeakHandlePool*>(context)->trim_empty_slabs_00b003a0();
}
void NativeWeakHandlePool::destroy_section_00402f70() noexcept {
    while (storage_.recursion_24 > 0) {
        add_depth(storage_.recursion_24, 0xffffffffu);
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}
void NativeWeakHandlePool::destroy_00b002c0() {
    storage_.allocator_00.native_vtable_00 = native_vtable;
    for (std::uint32_t i = 0; i < storage_.slab_count_2c; ++i)
        singleton_lifetime_free(storage_.slabs_28[i]);
    if (storage_.slabs_28) singleton_lifetime_free(storage_.slabs_28);
    destroy_section_00402f70();
    list_.unlink_base_element_00403970(storage_.allocator_00);
    // Native leaves the freed table/count/index fields and old list links.
}
void bind_static_native_weak_pool_0109ce94(NativeWeakHandlePool& pool) {
    if (static_pool_0109ce94 && static_pool_0109ce94 != &pool)
        throw std::logic_error("native 0109CE94 weak pool is already bound");
    static_pool_0109ce94 = &pool;
}
int initialize_static_native_weak_pool_00cd8a60(NativeWeakPoolAtexit register_atexit) {
    require_static_pool().initialize_00b004b0();
    return register_atexit(&destroy_static_native_weak_pool_00ce1040);
}
void destroy_static_native_weak_pool_00ce1040() {
    require_static_pool().destroy_00b002c0();
}

NativeWeakOwnerDomain::NativeWeakOwnerDomain(SingletonLifetimeDomain& lifetime,
    NativeWeakMutexOwner* volatile& publication, NativeWeakHandlePool& pool) noexcept
    : lifetime_(lifetime), global_0109ce90_(publication), pool_(pool) {}
NativeWeakMutexOwner* NativeWeakOwnerDomain::initialize_lock_owner_00924050(void* raw) {
    auto* owner = ::new (raw) NativeWeakMutexOwner;
    owner->vtable_00 = mutex_table;
    __try {
        owner->section_04 = critical_section_create_00bd1860();
    } __finally {
        if (AbnormalTermination()) {
            // CA6AA0 -> 923620 base unwind, not the complete destructor.
            global_0109ce90_ = nullptr;
            owner->vtable_00 = simple_owner_table;
        }
    }
    return owner;
}
NativeWeakMutexOwner* NativeWeakOwnerDomain::lock_owner_00924480() {
    if (auto* current = global_0109ce90_) return current;
    {
        ManagerGuard guard(lifetime_.get_manager_00415350()->system_owner().section_10);
        if (!global_0109ce90_) {
            void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 8, 8});
            NativeWeakMutexOwner* owner = nullptr;
            try {
                if (raw) owner = initialize_lock_owner_00924050(raw);
            } catch (...) {
                singleton_lifetime_free(raw);
                throw;
            }
            global_0109ce90_ = owner;
            lifetime_.get_manager_00415350()->register_object(global_0109ce90_);
        }
    }
    return global_0109ce90_; // native reload occurs after the captured lock exits
}
NativeWeakMutexOwner* NativeWeakOwnerDomain::delete_lock_owner_00925430(
    NativeWeakMutexOwner* owner, std::uint32_t flags) noexcept {
    owner->vtable_00 = mutex_table;
    critical_section_destroy_owned_0041cc80(owner->section_04);
    global_0109ce90_ = nullptr;
    owner->vtable_00 = simple_owner_table;
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
NativeWeakHandle* NativeWeakOwnerDomain::delete_handle_00925470(
    NativeWeakHandle* handle, std::uint32_t flags) noexcept {
    handle->vtable_00 = reference_table; // BD30F0 base destructor
    if (flags & 1u) pool_.return_raw_slot_00924420(handle);
    return handle;
}
void NativeWeakOwnerDomain::retain_handle(NativeWeakHandle& handle) noexcept {
    handle.references_04.fetch_add(1);
}
void NativeWeakOwnerDomain::release_handle(NativeWeakHandle& handle) noexcept {
    if (handle.references_04.fetch_sub(1) == 1) delete_handle_00925470(&handle, 1);
}
void NativeWeakOwnerDomain::construct_00925490(NativeWeakOwnerView owner) {
    if (!is_actual_prefix(owner))
        throw std::invalid_argument("weak owner view must borrow actual +00/+04/+08 fields");
    owner.vtable_00 = reference_table;
    owner.references_04.store(1, std::memory_order_relaxed);
    owner.vtable_00 = weak_owner_table;
    TrackedCriticalSection* lock = nullptr;
    bool entered = false;
    __try {
        lock = lock_owner_00924480()->section_04;
        enter(lock);
        entered = true;
        auto* raw = pool_.allocate_raw_slot_009242f0();
        NativeWeakHandle* handle = nullptr;
        if (raw) {
            handle = ::new (raw) NativeWeakHandle;
            handle->vtable_00 = reference_table;
            handle->references_04.store(1, std::memory_order_relaxed);
            handle->vtable_00 = weak_handle_table;
        }
        owner.weak_handle_08 = handle;
        handle->target_08 = owner.identity; // native null allocation also faults here
    } __finally {
        if (entered) leave(lock);
        if (AbnormalTermination()) owner.vtable_00 = reference_table;
    }
}
void NativeWeakOwnerDomain::destroy_00925540(NativeWeakOwnerView owner) noexcept {
    if (!is_actual_prefix(owner)) std::terminate();
    owner.vtable_00 = weak_owner_table;
    TrackedCriticalSection* lock = nullptr;
    bool entered = false;
    __try {
        lock = lock_owner_00924480()->section_04;
        enter(lock);
        entered = true;
        auto* handle = static_cast<NativeWeakHandle*>(owner.weak_handle_08);
        handle->target_08 = nullptr;
        release_handle(*handle);
    } __finally {
        if (entered) leave(lock);
        owner.vtable_00 = reference_table;
    }
    // +04 and +08 remain unchanged. External weak references retain the SAME
    // now-invalidated handle until their own terminal release returns its slot.
}
void NativeWeakOwnerDomain::construct_00925490(NativeGuiSceneStorage& scene) {
    construct_00925490({&scene, scene.vtable_00, scene.references_04, scene.weak_handle_08});
}
void NativeWeakOwnerDomain::destroy_00925540(NativeGuiSceneStorage& scene) noexcept {
    destroy_00925540({&scene, scene.vtable_00, scene.references_04, scene.weak_handle_08});
}

} // namespace bsp
