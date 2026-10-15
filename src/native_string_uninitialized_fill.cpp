#include "bsp/native_string_uninitialized_fill.hpp"

#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_string_uninitialized_copy.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* pointer) noexcept { return reinterpret_cast<Word>(pointer); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
} // namespace

void fill_native_string_uninitialized_0044ce20(void* destination,
    std::uint32_t count, const void* value, void* owner,
    NativeStringRawPoolContext& strings) {
    const Word destination_begin = bits(destination);
    Word completed_end = destination_begin;
    try {
        while (count != 0u) {
            // Native EBX holds the same source-header address throughout;
            // the existing constructor rereads its fields on each call.
            construct_native_render_alias_string_0044bcb0(
                pointer(completed_end), value, strings);
            --count;
            completed_end += 8u;
        }
    } catch (...) {
        //0044CE6D captures the original begin and completed end. Owner ECX
        // is ignored by00431F40. The failed current slot is not destroyed.
        Word current = destination_begin;
        while (current != completed_end) {
            destroy_native_string_storage_argument_00431f40(
                owner, pointer(current), strings);
            current += 8u;
        }
        throw; // Source-domain counterpart of __CxxThrowException(0,0).
    }
}

void* fill_native_string_uninitialized_n_dispatch_0044db60(void* owner,
    void* destination, std::uint32_t count, const void* value,
    NativeStringRawPoolContext& strings) {
    // EDI/ESI preserve these scalars across the native core call. Do not
    // derive the result by rereading an owner/vector header after callbacks.
    const Word destination_begin = bits(destination);
    const Word original_count = count;
    fill_native_string_uninitialized_0044ce20(
        destination, count, value, owner, strings);
    return pointer(destination_begin + original_count * 8u);
}
} // namespace bsp
