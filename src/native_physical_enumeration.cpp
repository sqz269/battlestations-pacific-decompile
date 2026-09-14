#include "bsp/native_physical_enumeration.hpp"

#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_append.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_string_pool_storage.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical enumeration requires MSVC Win32 storage.
#endif

namespace bsp {
namespace {
static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(NativeStringVectorStorage) == 12);

struct OwnedString {
    explicit OwnedString(NativeStringStorage& storage) : storage(storage) {}
    ~OwnedString() { value.release_to(storage); }
    NativeString value{};
    NativeStringStorage& storage;
};
struct FindOwner {
    HANDLE handle{INVALID_HANDLE_VALUE};
    ~FindOwner() { if (handle != INVALID_HANDLE_VALUE) FindClose(handle); }
};

std::uint32_t slot(void* provider, std::uint32_t offset) {
    auto* const table = *static_cast<std::uint32_t**>(provider);
    return table[offset / 4];
}
void require_slot(void* provider, std::uint32_t offset, std::uint32_t expected) {
    if (slot(provider, offset) != expected)
        throw std::invalid_argument("unsupported physical provider method");
}
void append_character(NativeString& destination, char character,
    NativeStringStorage& strings) {
    char one[2]{character, 0};
    OwnedString temporary(strings);
    construct_native_string_cstring_0041e870(&temporary.value, one, strings);
    append_native_string_00425e10(destination, temporary.value, strings);
}
} // namespace

void* join_native_physical_enumerated_name_00bee520(void* output,
    const void* directory, const void* child,
    NativePhysicalEnumerationContext& context) {
    auto& strings = context.physical.strings;
    auto* const parent = static_cast<const NativeString*>(directory);
    auto* const name = static_cast<const NativeString*>(child);
    if (parent->length() == 0)
        return canonicalize_native_path_00bee390(output, child, context.canonicalizer);
    if (name->length() == 0)
        return canonicalize_native_path_00bee390(output, directory, context.canonicalizer);

    OwnedString slash(strings), left(strings), joined(strings), canonical(strings);
    construct_native_string_cstring_0041e870(&slash.value, "/", strings);
    concatenate_native_string_headers_004261a0(directory, &left.value, &slash.value, strings);
    concatenate_native_string_headers_004261a0(&left.value, &joined.value, child, strings);
    canonicalize_native_path_00bee390(&canonical.value, &joined.value,
        context.canonicalizer);
    joined.value.release_to(strings);
    left.value.release_to(strings);
    slash.value.release_to(strings);
    return copy_construct_native_string_header_00426060(output,
        &canonical.value, strings);
}

void enumerate_native_physical_names_00bf47e0(void* provider,
    const void* actual_directory, const void* actual_extension,
    std::uint32_t flags, NativeStringVectorStorage& output,
    NativePhysicalEnumerationContext& context) {
    auto& strings = context.physical.strings;
    require_slot(provider, 0x1c, 0x00bf3970);
    OwnedString physical_path(strings);
    build_native_physical_path_00bf3970(provider, &physical_path.value,
        actual_directory, context.physical);
    if (physical_path.value.length() == 0 || !physical_path.value.data())
        throw std::invalid_argument("empty physical enumeration path");
    if (physical_path.value.data()[physical_path.value.length() - 1] != '\\')
        append_character(physical_path.value, '\\', strings);
    OwnedString pattern(strings);
    copy_construct_native_string_header_00426060(&pattern.value,
        &physical_path.value, strings);
    append_character(pattern.value, '*', strings);

    WIN32_FIND_DATAA found{};
    FindOwner search{FindFirstFileA(pattern.value.data(), &found)};
    if (search.handle == INVALID_HANDLE_VALUE) return;
    auto* const extension = static_cast<const NativeString*>(actual_extension);
    do {
        const char* const filename = found.cFileName;
        if (filename[0] == '.') continue;
        if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            const auto length = static_cast<std::uint32_t>(std::strlen(filename));
            if (length <= extension->length() ||
                !equal_native_string_header_00425850(actual_extension,
                    filename + length - extension->length()) ||
                _stricmp(filename, "MidwayDL_Content") == 0) continue;
            OwnedString child(strings), logical(strings);
            construct_native_string_cstring_0041e870(&child.value, filename, strings);
            join_native_physical_enumerated_name_00bee520(&logical.value,
                actual_directory, &child.value, context);
            append_native_string_vector_004cdc20(output, logical.value, strings);
        } else if (static_cast<std::uint8_t>(flags) != 0) {
            OwnedString child(strings), logical(strings);
            construct_native_string_cstring_0041e870(&child.value, filename, strings);
            join_native_physical_enumerated_name_00bee520(&logical.value,
                actual_directory, &child.value, context);
            require_slot(provider, 0x14, 0x00bf47e0);
            enumerate_native_physical_names_00bf47e0(provider, &logical.value,
                actual_extension, flags, output, context);
        }
    } while (FindNextFileA(search.handle, &found));
}
} // namespace bsp
