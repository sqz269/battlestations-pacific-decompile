#include "bsp/native_checked_string_storage.hpp"
#include "bsp/global_config.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(const void* p, Word offset) noexcept {
    Word value;
    std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, 4);
    return value;
}
void write(void* p, Word offset, Word value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, 4);
}
Word distance(Word first, Word last) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> 3);
}
void construct_string(void* destination, const void* source, NativeStringStorage& strings) {
    if (!destination) return;
    const bool same = destination == source;
    write(destination, 0, 0); write(destination, 4, 0);
    if (!same) copy_native_string_header_00be0a30_fragment(destination, strings, source);
}
struct TemporaryString {
    NativeString value;
    NativeStringStorage& strings;
    ~TemporaryString() noexcept { destroy_native_string_header_0041dd20(&value, strings); }
};
} // namespace

void* copy_native_string_range_storage(const void* first, const void* last,
    void* destination, NativeStringStorage& strings) {
    Word current = address(first), output = address(destination);
    const Word end = address(last);
    for (; current != end; current += 8, output += 8)
        copy_native_string_header_00be0a30_fragment(pointer(output), strings, pointer(current));
    return pointer(output);
}

void append_checked_native_string_storage(void* header, const void* source,
    NativeStringStorage& strings) {
    Word first = read(header, 4);
    const Word size = first ? distance(first, read(header, 8)) : 0;
    if (first && size < distance(first, read(header, 12))) {
        const Word end = read(header, 8);
        construct_string(pointer(end), source, strings);
        write(header, 8, end + 8);
        return;
    }
    if (read(header, 8) < first) _invalid_parameter_noinfo();
    // The slow native end-insertion driver copies the source before reading
    // growth inputs or invalidating any existing element, including aliases.
    TemporaryString saved{{}, strings};
    copy_native_string_header_00be0a30_fragment(&saved.value, strings, source);
    first = read(header, 4);
    const Word old_size = first ? distance(first, read(header, 8)) : 0;
    const Word capacity = first ? distance(first, read(header, 12)) : 0;
    constexpr Word maximum = 0x1fffffffu;
    if (old_size >= maximum) native_singleton_length_error_00bd0590();
    Word grown = capacity > maximum - (capacity >> 1) ? 0 : capacity + (capacity >> 1);
    if (grown < old_size + 1) grown = old_size + 1;
    const Word bytes = grown * 8;
    void* const replacement = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    Word completed = address(replacement);
    try {
        const Word old_end = read(header, 8);
        for (Word current = read(header, 4); current != old_end; current += 8) {
            construct_string(pointer(completed), pointer(current), strings);
            completed += 8;
        }
        construct_string(pointer(completed), &saved.value, strings);
        completed += 8;
    } catch (...) {
        destroy_global_config_name_range_00432050(replacement, pointer(completed), strings);
        singleton_lifetime_free(replacement);
        throw;
    }
    const Word old_first = read(header, 4);
    const Word final_size = (old_first ? distance(old_first, read(header, 8)) : 0) + 1;
    if (old_first) {
        destroy_global_config_name_range_00432050(pointer(old_first), pointer(read(header, 8)), strings);
        singleton_lifetime_free(pointer(read(header, 4)));
    }
    write(header, 12, address(replacement) + bytes);
    write(header, 8, address(replacement) + final_size * 8);
    write(header, 4, address(replacement));
}

void* erase_checked_native_string_storage(void* header, void* output,
    const void* first_owner, void* first, const void* last_owner, void* last,
    NativeStringStorage& strings) {
    if (!first_owner || first_owner != last_owner) _invalid_parameter_noinfo();
    if (first != last) {
        const Word old_end = read(header, 8);
        copy_native_string_range_storage(last, pointer(old_end), first, strings);
        const Word destruction_end = read(header, 8);
        const Word new_end = address(first) + distance(address(last), old_end) * 8;
        destroy_global_config_name_range_00432050(pointer(new_end), pointer(destruction_end), strings);
        write(header, 8, new_end);
    }
    write(output, 0, address(first_owner));
    write(output, 4, address(first));
    return output;
}
} // namespace bsp
