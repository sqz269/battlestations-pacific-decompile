#include "bsp/native_fileblock_identifier.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_byte_append.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(long) == 4,
    "FileBlock identifier storage and atol are Win32.");
static_assert(sizeof(NativeString) == 8 && sizeof(NativeStringVectorStorage) == 12);

template<class T> T load(const void* p, std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const char*>(p) + offset, sizeof(result));
    return result;
}
std::int32_t signed_word(std::uint32_t word) noexcept {
    std::int32_t result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}
char* offset_pointer(char* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
const char* offset_pointer(const char* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
NativeMeshWeightNameStorage* token_at(NativeStringVectorStorage& vector,
    std::uint32_t index) noexcept {
    return reinterpret_cast<NativeMeshWeightNameStorage*>(
        reinterpret_cast<std::uintptr_t>(vector.data_00) + index * 8u);
}
bool numeric_token(NativeStringVectorStorage& vector, std::uint32_t index) {
    const auto* data = token_at(vector, index)->data_04;
    // BF8417 delegates to strtol(text,nullptr,10). 0109CEF0 is the empty
    // fallback; nonzero decimal prefixes count, including a leading sign.
    return std::atol(data ? data : "") != 0;
}
void append_header_fragment(void* destination, const void* source,
    NativeStringStorage& strings) {
    const auto appended = load<std::uint32_t>(source, 0);
    if (appended == 0) return;
    const auto old_length = load<std::uint32_t>(destination, 0);
    resize_native_string_header_0041dd40(destination, strings,
        old_length + appended, true);
    const auto* input = load<const char*>(source, 4);
    auto* output = load<char*>(destination, 4);
    // BDFA98/BDFB67/BDFC46 capture length, then reload both data pointers.
    // BF7680 also implements backward-overlap copying.
    std::memmove(offset_pointer(output, old_length), input, appended);
}

} // namespace

void split_native_string_nonempty_on_byte_00bd20a0(const void* actual_source,
    std::uint8_t delimiter, NativeStringVectorStorage& destination,
    NativeStringStorage& strings) {
    auto length = load<std::uint32_t>(actual_source, 0);
    std::uint32_t begin = 0;
    while (begin < length) {
        const auto* skipped_data = load<const char*>(actual_source, 4);
        while (begin < length &&
            static_cast<std::uint8_t>(*offset_pointer(skipped_data, begin)) == delimiter)
            ++begin;
        auto end = begin;
        if (begin < length) {
            const auto* token_data = load<const char*>(actual_source, 4);
            while (end < length &&
                static_cast<std::uint8_t>(*offset_pointer(token_data, end)) != delimiter)
                ++end;
            NativeString temporary;
            construct_native_string_substring_00469840(actual_source, &temporary,
                begin, end - begin, strings);
            // DFF66C state0 -> -1 calls CC55B0/41DD20 only after substring
            // returned. The destination vector has no caller rollback here.
            try {
                append_native_string_vector_004cdc20(destination, temporary, strings);
            } catch (...) {
                destroy_native_string_header_0041dd20(&temporary, strings);
                throw;
            }
            destroy_native_string_header_0041dd20(&temporary, strings);
            begin = end;
        }
        length = load<std::uint32_t>(actual_source, 0); // BD214D, after return.
    }
}

void prepare_native_fileblock_identifier_00bdf950(void* actual_fileblock,
    NativeStringStorage& strings) {
    auto* const name = static_cast<char*>(actual_fileblock) + 0x14;
    std::uint32_t cursor = 0;
    for (;;) {
        const auto* data = load<const char*>(name, 4);
        if (data == nullptr) break;
        const auto length = load<std::uint32_t>(name, 0);
        if (length == 0) break;
        auto start = cursor;
        if (signed_word(cursor) < 0) start = 0;
        else if (cursor > length) break;
        cursor = static_cast<std::uint32_t>(std::strcspn(
            offset_pointer(data, start), " <>=?:;\"*+,./\\|")) + start;
        if (cursor == load<std::uint32_t>(name, 0) || cursor == UINT32_MAX) break;
        // Do not advance past or stop at an embedded NUL: the original stores
        // '_' at strcspn's stopping offset and scans from that offset again.
        *offset_pointer(load<char*>(actual_fileblock, 0x18), cursor) = '_';
    }
    if (load<std::uint32_t>(name, 0) <= 32u) return;

    NativeStringVectorStorage tokens{};
    NativeString first, second, combined;
    int state = 0; // E00C40 state0 owns vector at aligned-frame -1Ch.
    auto release_temporaries = [&]() noexcept {
        while (state > 0) {
            if (state == 3 || state == 6 || state == 9) {
                --state;
                destroy_native_string_header_0041dd20(&combined, strings);
            } else if (state == 2 || state == 5 || state == 8) {
                --state;
                destroy_native_string_header_0041dd20(&second, strings);
            } else { // 1, 4 or 7 -> 0; each branch's first constructed header.
                state = 0;
                destroy_native_string_header_0041dd20(&first, strings);
            }
        }
    };
    try {
        split_native_string_nonempty_on_byte_00bd20a0(name, '_', tokens, strings);
        const auto count = tokens.count_04;
        if (count < 3 || signed_word(static_cast<std::uint32_t>(count) * 3u - 1u) > 32) {
            // Construction order differs from the concat operands: (-64,64)
            // first, (0,64) second; result = second + first. No new clamp.
            construct_native_string_substring_00469840(name, &first, 0xffffffc0u, 64, strings);
            state = 1;
            construct_native_string_substring_00469840(name, &second, 0, 64, strings);
            state = 2;
            concatenate_native_string_headers_004261a0(&second, &combined, &first, strings);
            state = 3;
            copy_native_string_header_00be0a30_fragment(name, strings, &combined);
            release_temporaries();
        } else {
            assign_native_string_cstring_0041e350(name, "", strings);
            std::uint32_t index = 0;
            while (signed_word(index) <
                signed_word(static_cast<std::uint32_t>(tokens.count_04) - 1u)) {
                const bool numeric = numeric_token(tokens, index);
                construct_native_string_cstring_0041e870(&first, "_", strings);
                state = numeric ? 4 : 7;
                if (numeric) {
                    copy_construct_native_string_header_00426060(&second,
                        token_at(tokens, index), strings);
                } else {
                    const auto byte = static_cast<std::uint8_t>(*token_at(tokens, index)->data_04);
                    construct_native_string_byte_00531030(&second, byte, strings);
                }
                ++state; // 5 or 8, after construction returned.
                concatenate_native_string_headers_004261a0(&second, &combined, &first, strings);
                ++state; // 6 or 9.
                append_header_fragment(name, &combined, strings);
                release_temporaries();
                ++index;
            }
            if (numeric_token(tokens, index)) {
                append_header_fragment(name, token_at(tokens, index), strings);
            } else {
                const auto byte = static_cast<std::uint8_t>(*token_at(tokens, index)->data_04);
                append_native_string_byte_0054aa70(name, byte, strings);
            }
        }
        // BDFD53 disarms the whole vector before normal resize/free; no retry.
        state = -1;
        resize_native_string_vector_00427110(tokens, 0, strings);
        singleton_lifetime_free(tokens.data_00);
    } catch (...) {
        release_temporaries();
        if (state == 0) {
            state = -1;
            destroy_native_mesh_weight_names_00427880(tokens, strings);
        }
        throw;
    }
}

} // namespace bsp
