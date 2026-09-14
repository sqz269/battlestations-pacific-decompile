#include "bsp/native_mpkg_enumeration.hpp"

#include "bsp/native_mpak_enumeration.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
void release(void* data, U bytes, NativeMpkgDirectoryContext& context) {
    auto* pool = context.services.string_pool_00419cc0();
    context.services.return_string_00bd1510(pool, data, bytes, 1);
}
void copy_name(NativeString& output, const void* source,
    NativeMpkgDirectoryContext& context) {
    resize_native_string_header_0041dd40(&output, context.strings, word(source), true);
    if (word(source)) std::memmove(pointer(word(&output, 4)),
        pointer(word(source, 4)), word(&output));
}
} // namespace

void enumerate_native_mpkg_names_00bb98f0(void* provider,
    const void* directory, const void* extension, U flags,
    NativeStringVectorStorage& output, NativeMpkgDirectoryContext& context,
    const char* null_pattern) {
    // BB98F0 replaces ECX with provider+14h and jumps into BB97B0.
    void* const archive = pointer(word(provider, 0x14));
    NativeString prefix{};
    NativeString slash{};
    int state = -1;
    try {
        copy_name(prefix, directory, context);
        state = 0;
        const U length = prefix.length();
        // The native byte read at data[length-1] is unconditional. An empty
        // directory without a valid preimage is outside this source contract.
        if (*reinterpret_cast<const volatile char*>(
            static_cast<std::uintptr_t>(word(&prefix, 4) + length - 1u)) != '/') {
            slash.assign_0041e870(context.strings, "/");
            state = 1;
            const U slash_length = slash.length();
            void* const slash_data = pointer(word(&slash, 4));
            if (slash_length) {
                resize_native_string_header_0041dd40(&prefix, context.strings,
                    slash_length + length, true);
                std::memmove(pointer(word(&prefix, 4) + length),
                    slash_data, slash_length);
            }
            state = 0;
            if (slash_data) release(slash_data, slash_length + 1u, context);
        }
        U index = 0;
        U offset = 0;
        while (static_cast<std::int32_t>(index) <
            static_cast<std::int32_t>(word(archive, 0x2c))) {
            const void* row = pointer(word(archive, 0x28) + offset);
            // BB97B0 filters against the original directory, not prefix.
            if (matches_native_provider_enumeration_00bee340(directory,
                extension, flags, row, null_pattern)) {
                append_native_string_vector_004cdc20(output,
                    *static_cast<const NativeString*>(row), context.strings);
            }
            ++index;
            offset += 0x24;
        }
        void* const data = pointer(word(&prefix, 4));
        state = -1;
        if (data) release(data, word(&prefix) + 1u, context);
    } catch (...) {
        if (state >= 1) destroy_native_string_header_0041dd20(&slash, context.strings);
        if (state >= 0) destroy_native_string_header_0041dd20(&prefix, context.strings);
        throw;
    }
}
} // namespace bsp
