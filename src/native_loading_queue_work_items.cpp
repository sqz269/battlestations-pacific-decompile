#include "bsp/native_loading_queue_work_items.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

namespace bsp {
namespace {

volatile std::uint32_t& word(void* storage, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(storage) + offset);
}
std::uint32_t word(const void* storage, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const unsigned char*>(storage) + offset);
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void* address(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
std::uint32_t bits(const void* value) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
void* offset(void* storage, std::uint32_t bytes) noexcept {
    return address(bits(storage) + bytes);
}

void copy_name(void* destination, NativeStringRawPoolContext& strings,
    const void* source) {
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    // Source length is the condition; the current DESTINATION length is the
    // memcpy count. Reload both headers after the potentially reentrant resize.
    if (word(source) != 0) {
        const auto count = word(destination);
        if (count != 0)
            std::memmove(address(word(destination, 4)), address(word(source, 4)), count);
    }
}

void destroy_work_vector(void* vector, NativeStringRawPoolContext& strings) {
    // 00504E10, reached by the job constructor/destructor unwind funclets.
    resize_native_loading_work_items_005019d0(vector, strings, 0);
    singleton_lifetime_free(address(word(vector)));
}

struct WorkVectorUnwind {
    void* vector;
    NativeStringRawPoolContext& strings;
    bool active = true;
    ~WorkVectorUnwind() noexcept(false) {
        if (active) destroy_work_vector(vector, strings);
    }
};

struct JobAllocationUnwind {
    void* allocation;
    bool active = true;
    ~JobAllocationUnwind() noexcept {
        if (active) singleton_lifetime_free(allocation);
    }
};

} // namespace

void reserve_native_loading_job_pointers_004fb310(void* vector,
    std::int32_t requested_capacity) {
    if (requested_capacity < 1) requested_capacity = 1;
    if (signed_word(word(vector, 8)) >= requested_capacity) return;
    const auto bytes = static_cast<std::uint32_t>(requested_capacity) * 4u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    auto destination = bits(replacement);
    std::uint32_t index = 0;
    while (signed_word(index) < signed_word(word(vector, 4))) {
        if (destination != 0)
            word(address(destination)) = word(address(word(vector) + index * 4u));
        ++index;
        destination += 4u;
    }
    singleton_lifetime_free(address(word(vector)));
    // Decoded returning-free continuation004FB361..004FB369.
    word(vector) = bits(replacement);
    word(vector, 8) = static_cast<std::uint32_t>(requested_capacity);
}

void resize_native_loading_job_pointers_004fb4b0(void* vector,
    std::int32_t requested_count) {
    if (requested_count > signed_word(word(vector, 8)))
        reserve_native_loading_job_pointers_004fb310(vector, requested_count);
    auto index = word(vector, 4);
    while (signed_word(index) < requested_count) {
        void* const slot = address(word(vector) + index * 4u);
        if (slot != nullptr) word(slot) = 0;
        ++index;
    }
    while (requested_count < signed_word(word(vector, 4)))
        word(vector, 4) = word(vector, 4) - 1u;
    word(vector, 4) = static_cast<std::uint32_t>(requested_count);
}

void remove_native_loading_work_item_00501670(void* vector,
    NativeStringRawPoolContext& strings, std::int32_t index) {
    auto current = static_cast<std::uint32_t>(index);
    auto displacement = current * 0x10u;
    while (signed_word(current) < signed_word(word(vector, 4) - 1u)) {
        void* const destination = address(word(vector) + displacement);
        void* const source = offset(destination, 0x10);
        copy_name(destination, strings, source);
        word(destination, 8) = word(source, 8);
        word(destination, 0x0c) = word(source, 0x0c);
        ++current;
        displacement += 0x10u;
    }
    void* const last = address(word(vector) + word(vector, 4) * 0x10u - 0x10u);
    destroy_native_string_header_0041dd20(last, strings);
    word(vector, 4) = word(vector, 4) - 1u;
}

void reserve_native_loading_work_items_005018a0(void* vector,
    NativeStringRawPoolContext& strings, std::int32_t requested_capacity) {
    if (requested_capacity < 1) requested_capacity = 1;
    if (signed_word(word(vector, 8)) >= requested_capacity) return;
    const auto bytes = static_cast<std::uint32_t>(requested_capacity) * 0x10u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
    std::uint32_t index = 0;
    while (signed_word(index) < signed_word(word(vector, 4))) {
        void* const destination = address(bits(replacement) + index * 0x10u);
        // Native state0 calls00401130 (RET) on failure. It does not destroy
        // this name, earlier copies, or the replacement array. No RAII rollback.
        if (destination != nullptr) {
            const void* const source = address(word(vector) + index * 0x10u);
            word(destination) = 0;
            word(destination, 4) = 0;
            copy_name(destination, strings, source);
            word(destination, 8) = word(source, 8);
            word(destination, 0x0c) = word(source, 0x0c);
        }
        ++index;
    }
    index = 0;
    while (signed_word(index) < signed_word(word(vector, 4))) {
        destroy_native_string_header_0041dd20(
            address(word(vector) + index * 0x10u), strings);
        ++index;
    }
    singleton_lifetime_free(address(word(vector)));
    // Decoded returning-free continuation005019A4..005019B2.
    word(vector) = bits(replacement);
    word(vector, 8) = static_cast<std::uint32_t>(requested_capacity);
}

void resize_native_loading_work_items_005019d0(void* vector,
    NativeStringRawPoolContext& strings, std::int32_t requested_count) {
    if (requested_count > signed_word(word(vector, 8)))
        reserve_native_loading_work_items_005018a0(vector, strings, requested_count);
    const auto initial_count = word(vector, 4);
    if (signed_word(initial_count) < requested_count) {
        auto displacement = initial_count * 0x10u;
        auto remaining = static_cast<std::uint32_t>(requested_count) - initial_count;
        do {
            void* const item = address(word(vector) + displacement);
            if (item != nullptr) {
                word(item) = 0;
                word(item, 4) = 0;
                word(item, 8) = 0;
                word(item, 0x0c) = 0;
            }
            displacement += 0x10u;
            --remaining;
        } while (remaining != 0);
    }
    while (requested_count < signed_word(word(vector, 4))) {
        // Unlike remove00501670, shrink publishes the smaller count BEFORE
        // resolving the current pool/releasing that tail string.
        word(vector, 4) = word(vector, 4) - 1u;
        destroy_native_string_header_0041dd20(
            address(word(vector) + word(vector, 4) * 0x10u), strings);
    }
    word(vector, 4) = static_cast<std::uint32_t>(requested_count);
}

void destroy_native_loading_job_storage_005051a0(void* job,
    NativeStringRawPoolContext& strings) {
    void* const vector = offset(job, 8);
    WorkVectorUnwind unwind{vector, strings};
    destroy_native_string_header_0041dd20(offset(job, 0x14), strings);
    unwind.active = false; // Native state -1 precedes normal resize(work,0).
    destroy_work_vector(vector, strings);
}

void* construct_native_loading_job_00505530(void* job,
    NativeStringRawPoolContext& strings, const void* package_name) {
    word(job) = 0;
    *static_cast<volatile unsigned char*>(offset(job, 4)) = 0;
    word(job, 8) = 0;
    word(job, 0x0c) = 0;
    word(job, 0x10) = 0;
    WorkVectorUnwind unwind{offset(job, 8), strings};
    void* const name = offset(job, 0x14);
    word(name) = 0;
    word(name, 4) = 0;
    copy_name(name, strings, package_name);
    word(job, 0x1c) = 0;
    word(job, 0x20) = 0;
    unwind.active = false;
    return job;
}

void enqueue_native_loading_job_005055c0(void* loader,
    NativeStringRawPoolContext& strings, const void* package_name) {
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x24, 0x24});
    JobAllocationUnwind unwind{allocation};
    void* const job = allocation == nullptr ? nullptr :
        construct_native_loading_job_00505530(allocation, strings, package_name);
    void* const vector = offset(loader, 0x10);
    const auto capacity = word(vector, 8);
    const bool full = word(vector, 4) == capacity;
    unwind.active = false; // Growth failure must not destroy/free the new job.
    if (full) {
        auto doubled = signed_word(capacity + capacity);
        if (doubled <= 1) doubled = 1;
        reserve_native_loading_job_pointers_004fb310(vector, doubled);
    }
    void* const slot = address(word(vector) + word(vector, 4) * 4u);
    if (slot != nullptr) word(slot) = bits(job);
    word(vector, 4) = word(vector, 4) + 1u;
}

} // namespace bsp
