#include "bsp/native_mpak_container_storage.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }
void* pointer(U p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, U offset) noexcept { return pointer(address(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }
U count(const void* vector, U stride) noexcept {
    const U begin = word(vector, 4);
    return begin ? (word(vector, 8) - begin) / stride : 0;
}
U capacity(const void* vector, U stride) noexcept {
    const U begin = word(vector, 4);
    return begin ? (word(vector, 12) - begin) / stride : 0;
}
U grown(U old_capacity, U needed, U limit) noexcept {
    U next = old_capacity;
    if (limit - (old_capacity >> 1) < old_capacity) next = 0;
    else next += old_capacity >> 1;
    return next < needed ? needed : next;
}
}

NativeMpakContainerStorage::NativeMpakContainerStorage(NativeStringStorage& strings,
    NativeAdoptedSubstreamDispatch& streams,
    NativePathCanonicalizerServices& allocation_and_pool,
    NativeMpakOffsetVectorCopyLibrary& offsets) noexcept
    : strings_(strings), streams_(streams), allocation_and_pool_(allocation_and_pool),
      offsets_(offsets) {}

NativeMpakDirectoryContext NativeMpakContainerStorage::directory_context() noexcept {
    return {strings_, streams_, allocation_and_pool_, *this};
}
void NativeMpakContainerStorage::invalid_parameter_00bf6713() {
    _invalid_parameter_noinfo();
}

void NativeMpakContainerStorage::destroy_file_range_00bb6220(void* begin,
    void* end, void*, void*) {
    auto context = directory_context();
    for (U cursor = address(begin); cursor != address(end); cursor += 0x24)
        destroy_native_mpak_file_00bb5430(pointer(cursor), context);
}
void NativeMpakContainerStorage::destroy_file_vector_00bb6e60(void* vector) {
    const U begin = word(vector, 4);
    if (begin) {
        destroy_file_range_00bb6220(pointer(begin), pointer(word(vector, 8)), vector, vector);
        allocation_and_pool_.free_scratch_00bf65ac(pointer(word(vector, 4)));
    }
    put(vector, 4, 0); put(vector, 8, 0); put(vector, 12, 0);
}
void NativeMpakContainerStorage::destroy_directory_vector_00bb71f0(void* vector) {
    const U begin = word(vector, 4);
    if (begin) {
        auto context = directory_context();
        for (U cursor = begin; cursor != word(vector, 8); cursor += 0x14)
            destroy_native_mpak_directory_00bb6500(pointer(cursor), context);
        allocation_and_pool_.free_scratch_00bf65ac(pointer(word(vector, 4)));
    }
    put(vector, 4, 0); put(vector, 8, 0); put(vector, 12, 0);
}

void NativeMpakContainerStorage::append_file_00bb7a20(void* vector, const void* record) {
    const U begin = word(vector, 4), end = word(vector, 8);
    const U used = count(vector, 0x24), available = capacity(vector, 0x24);
    if (begin && used < available) {
        copy_construct_native_mpak_file_00bb65a0(pointer(end), record, strings_, offsets_);
        put(vector, 8, end + 0x24);
        return;
    }
    if (end < begin) invalid_parameter_00bf6713();
    if (used >= 0x071c71c7u) throw std::length_error("MPAK file vector length");
    // BB7240 aliases the source through a temporary before any reallocation.
    alignas(4) std::byte temporary[0x24];
    copy_construct_native_mpak_file_00bb65a0(temporary, record, strings_, offsets_);
    auto context = directory_context();
    try {
        const U next = grown(available, used + 1, 0x071c71c7u);
        void* const fresh = allocation_and_pool_.allocate_scratch_00bf55be(next * 0x24);
        U constructed = 0;
        try {
            for (; constructed < used; ++constructed)
                copy_construct_native_mpak_file_00bb65a0(at(fresh, constructed * 0x24),
                    pointer(begin + constructed * 0x24), strings_, offsets_);
            copy_construct_native_mpak_file_00bb65a0(at(fresh, constructed * 0x24),
                temporary, strings_, offsets_);
            ++constructed;
        } catch (...) {
            destroy_file_range_00bb6220(fresh, at(fresh, constructed * 0x24), vector, vector);
            allocation_and_pool_.free_scratch_00bf65ac(fresh);
            throw;
        }
        if (begin) {
            destroy_file_range_00bb6220(pointer(begin), pointer(end), vector, vector);
            allocation_and_pool_.free_scratch_00bf65ac(pointer(begin));
        }
        put(vector, 4, address(fresh));
        put(vector, 8, address(fresh) + (used + 1) * 0x24);
        put(vector, 12, address(fresh) + next * 0x24);
    } catch (...) {
        destroy_native_mpak_file_00bb5430(temporary, context);
        throw;
    }
    destroy_native_mpak_file_00bb5430(temporary, context);
}

void NativeMpakContainerStorage::append_directory_00bb7ba0(void* vector,
    const void* record) {
    const U begin = word(vector, 4), end = word(vector, 8);
    const U used = count(vector, 0x14), available = capacity(vector, 0x14);
    if (begin && used < available) {
        copy_construct_native_mpak_directory_00bb6630(pointer(end), record, strings_);
        put(vector, 8, end + 0x14);
        return;
    }
    if (end < begin) invalid_parameter_00bf6713();
    if (used >= 0x0cccccccu) throw std::length_error("MPAK directory vector length");
    alignas(4) std::byte temporary[0x14];
    copy_construct_native_mpak_directory_00bb6630(temporary, record, strings_);
    auto context = directory_context();
    try {
        const U next = grown(available, used + 1, 0x0cccccccu);
        void* const fresh = allocation_and_pool_.allocate_scratch_00bf55be(next * 0x14);
        U constructed = 0;
        try {
            for (; constructed < used; ++constructed)
                copy_construct_native_mpak_directory_00bb6630(at(fresh, constructed * 0x14),
                    pointer(begin + constructed * 0x14), strings_);
            copy_construct_native_mpak_directory_00bb6630(at(fresh, constructed * 0x14),
                temporary, strings_);
            ++constructed;
        } catch (...) {
            for (U index = 0; index < constructed; ++index)
                destroy_native_mpak_directory_00bb6500(at(fresh, index * 0x14), context);
            allocation_and_pool_.free_scratch_00bf65ac(fresh);
            throw;
        }
        if (begin) {
            for (U cursor = begin; cursor != end; cursor += 0x14)
                destroy_native_mpak_directory_00bb6500(pointer(cursor), context);
            allocation_and_pool_.free_scratch_00bf65ac(pointer(begin));
        }
        put(vector, 4, address(fresh));
        put(vector, 8, address(fresh) + (used + 1) * 0x14);
        put(vector, 12, address(fresh) + next * 0x14);
    } catch (...) {
        destroy_native_mpak_directory_00bb6500(temporary, context);
        throw;
    }
    destroy_native_mpak_directory_00bb6500(temporary, context);
}

void NativeMpakContainerStorage::insert_offset_00a40d60(void* vector,
    void* result_iterator, void* where_container, void* where_pointer,
    const void* value) {
    const U begin = word(vector, 4), end = word(vector, 8);
    const U used = count(vector, 4), available = capacity(vector, 4);
    U index = 0;
    if (used) {
        if (end < begin) invalid_parameter_00bf6713();
        if (!where_container || where_container != vector) invalid_parameter_00bf6713();
        index = (address(where_pointer) - begin) >> 2;
    }
    const U captured = word(value);
    if (used >= 0x3fffffffu) throw std::length_error("MPAK offset vector length");
    if (available < used + 1) {
        const U next = grown(available, used + 1, 0x3fffffffu);
        void* const fresh = allocation_and_pool_.allocate_scratch_00bf55be(next * 4);
        if (index) std::memmove(fresh, pointer(begin), index * 4);
        put(fresh, index * 4, captured);
        if (used > index) std::memmove(at(fresh, (index + 1) * 4),
            pointer(begin + index * 4), (used - index) * 4);
        if (begin) allocation_and_pool_.free_scratch_00bf65ac(pointer(begin));
        put(vector, 4, address(fresh));
        put(vector, 8, address(fresh) + (used + 1) * 4);
        put(vector, 12, address(fresh) + next * 4);
    } else {
        std::memmove(pointer(begin + (index + 1) * 4), pointer(begin + index * 4),
            (used - index) * 4);
        put(pointer(begin + index * 4), 0, captured);
        put(vector, 8, end + 4);
    }
    const U current_begin = word(vector, 4);
    if (word(vector, 8) < current_begin) invalid_parameter_00bf6713();
    const U result = current_begin + index * 4;
    if (word(vector, 8) < result || result < current_begin)
        invalid_parameter_00bf6713();
    put(result_iterator, 4, result);
    put(result_iterator, 0, address(vector));
}
} // namespace bsp
