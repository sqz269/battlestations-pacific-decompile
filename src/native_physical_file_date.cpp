#include "bsp/native_physical_file_date.hpp"

#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <Windows.h>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical file dates require MSVC Win32.
#endif

namespace bsp {
namespace {
const void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(p, offset));
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(p, offset)) = value;
}
void* pointer(const void* p, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
unsigned char byte(const void* p, std::uint32_t offset) noexcept {
    return *static_cast<const volatile unsigned char*>(at(p, offset));
}
void invalid(const SingletonLifetimeCallbacks& crt) {
    crt.invalid_parameter(crt.context);
}
void require_slot(void* provider, std::uint32_t offset, std::uint32_t expected) {
    if (word(pointer(provider), offset) != expected)
        throw std::invalid_argument("Unimplemented current physical provider slot");
}
void copy_current_header_data(void* destination, const void* source) {
    const auto count = word(destination);
    const auto* input = pointer(source, 4);
    auto* output = pointer(destination, 4);
    // Same zero-byte-copy boundary as the existing actual string layer.
    if (count != 0) std::memcpy(output, input, count);
}
void clear_header(void* p) noexcept { put(p, 0, 0); put(p, 4, 0); }
const char* cstring(const void* header) noexcept {
    auto* data = static_cast<const char*>(pointer(header, 4));
    return data ? data : ""; // 0109DBEC empty-byte fallback.
}
std::uint32_t cstring_length(const char* source) noexcept {
    std::uint32_t count = 0;
    while (byte(source, count) != 0) ++count; // Original inline byte scan.
    return count;
}
void* dispatch_build(void* provider, void* output, const void* suffix,
    NativePhysicalFileDateContext& context) {
    require_slot(provider, 0x1c, 0x00bf3970);
    return build_native_physical_path_00bf3970(provider, output, suffix, context);
}
} // namespace

bool equal_native_string_headers_00435c40(const void* left, const void* right) {
    const auto left_length = word(left);
    const auto right_length = word(right);
    if (left_length != right_length) return false;
    if (left_length == 0) return true;
    const auto* right_data = static_cast<const char*>(pointer(right, 4));
    const auto* left_data = static_cast<const char*>(pointer(left, 4));
    return _stricmp(left_data, right_data) == 0;
}

bool unequal_native_string_headers_00449af0(const void* left, const void* right) {
    if (word(left) == 0) return word(right) != 0;
    if (word(right) == 0) return true;
    const auto* right_data = static_cast<const char*>(pointer(right, 4));
    const auto* left_data = static_cast<const char*>(pointer(left, 4));
    return _stricmp(left_data, right_data) != 0;
}

std::int32_t reverse_find_native_string_header_00467cf0(
    const void* source, const void* needle, std::uint32_t limit) {
    if (!pointer(source, 4) || !pointer(needle, 4)) return -1;
    const auto length = word(source);
    if (limit > length) limit = length;
    auto position = limit - word(needle);
    while (static_cast<std::int32_t>(position) > 0) {
        const auto count = word(needle);
        const auto* data = static_cast<const char*>(pointer(source, 4));
        const auto* pattern = static_cast<const char*>(pointer(needle, 4));
        if (std::strncmp(static_cast<const char*>(at(data, position)), pattern, count) == 0)
            return static_cast<std::int32_t>(position);
        --position;
    }
    return -1;
}

void* assign_native_string_header_00425f40(void* destination, const void* source,
    NativeStringStorage& strings) {
    if (destination != source) {
        resize_native_string_header_0041dd40(destination, strings, word(source), true);
        if (word(source) != 0) copy_current_header_data(destination, source);
    }
    return destination;
}

void* concatenate_native_string_headers_004261a0(const void* left, void* output,
    const void* right, NativeStringStorage& strings) {
    const bool identical = output == left;
    clear_header(output);
    if (!identical) {
        resize_native_string_header_0041dd40(output, strings, word(left), true);
        if (word(left) != 0) copy_current_header_data(output, left);
    }
    // C5E830's ownership bit is still clear throughout that initial copy.
    const auto appended = word(right);
    try {
        if (appended != 0) {
            const auto old_length = word(output);
            resize_native_string_header_0041dd40(output, strings, old_length + appended, true);
            const auto* input = pointer(right, 4);
            auto* destination = pointer(output, 4);
            std::memcpy(at(destination, old_length), input, appended);
        }
    } catch (...) {
        destroy_native_string_header_0041dd20(output, strings);
        throw;
    }
    return output;
}

void* assign_native_string_cstring_0041e350(void* destination, const char* source,
    NativeStringStorage& strings) {
    const auto length = source ? cstring_length(source) : 0;
    resize_native_string_header_0041dd40(destination, strings, length, false);
    auto* data = pointer(destination, 4);
    if (data) {
        const auto count = word(destination);
        if (count != 0) std::memcpy(data, source, count);
    }
    return destination;
}

void* construct_native_string_cstring_0041e870(void* destination, const char* source,
    NativeStringStorage& strings) {
    clear_header(destination);
    const auto length = cstring_length(source);
    resize_native_string_header_0041dd40(destination, strings, length, true);
    auto* data = pointer(destination, 4);
    if (data) {
        const auto count = word(destination) + 1u;
        // BF7680 at41E8B4 also implements backward overlap (BF769A -> BF7844).
        if (count != 0) std::memmove(data, source, count);
    }
    return destination;
}

void* lower_bound_native_physical_index_00bda260(void* tree, const void* key) {
    auto* candidate = pointer(tree, 4);
    auto* node = pointer(candidate, 4);
    while (byte(node, 0x1d) == 0) {
        if (less_native_string_headers_00443d00(at(node, 0x0c), key)) {
            node = pointer(node, 8);
        } else {
            candidate = node;
            node = pointer(node);
        }
    }
    return candidate;
}

void* find_native_physical_index_00bf36d0(void* tree, void* output,
    const void* key, const SingletonLifetimeCallbacks& crt) {
    auto* node = lower_bound_native_physical_index_00bda260(tree, key);
    if (!tree) invalid(crt);
    if (node == pointer(tree, 4) || less_native_string_headers_00443d00(key, at(node, 0x0c)))
        node = pointer(tree, 4);
    const auto owner_word = reinterpret_cast<std::uint32_t>(tree);
    const auto node_word = reinterpret_cast<std::uint32_t>(node);
    put(output, 0, owner_word);
    put(output, 4, node_word);
    return output;
}

bool equal_native_physical_iterators_00bd92c0(const void* left, const void* right,
    const SingletonLifetimeCallbacks& crt) {
    auto* owner = pointer(left);
    if (!owner || owner != pointer(right)) invalid(crt);
    const auto left_node = word(left, 4);
    return left_node == word(right, 4);
}

void advance_native_physical_index_00bd9860(void* iterator,
    const SingletonLifetimeCallbacks& crt) {
    if (!pointer(iterator)) invalid(crt);
    auto* node = pointer(iterator, 4);
    if (byte(node, 0x1d) != 0) {
        invalid(crt); // Native tail call: a returning handler returns to caller.
        return;
    }
    auto* child = pointer(node, 8);
    if (byte(child, 0x1d) == 0) {
        auto* next = pointer(child);
        while (byte(next, 0x1d) == 0) {
            child = next;
            next = pointer(child);
        }
        put(iterator, 4, reinterpret_cast<std::uint32_t>(child));
    } else {
        auto* parent = pointer(node, 4);
        while (byte(parent, 0x1d) == 0) {
            if (pointer(iterator, 4) != pointer(parent, 8)) break;
            put(iterator, 4, reinterpret_cast<std::uint32_t>(parent));
            parent = pointer(parent, 4);
        }
        put(iterator, 4, reinterpret_cast<std::uint32_t>(parent));
    }
}

void* build_native_physical_path_00bf3970(void* provider, void* output,
    const void* suffix, NativePhysicalFileDateContext& context) {
    auto* root = at(provider, 8);
    concatenate_native_string_headers_004261a0(root, output, suffix, context.strings);
    auto offset = word(root);
    while (offset < word(output)) {
        auto* data = pointer(output, 4);
        auto* current = static_cast<volatile unsigned char*>(at(data, offset));
        if (*current == '/') *current = '\\';
        ++offset;
    }
    return output;
}

bool replace_native_physical_path_00bf39c0(void* provider, void* name,
    NativePhysicalFileDateContext& context) {
    require_slot(provider, 0x10, 0x00bf3f70);
    if (!exists_native_physical_path_00bf3f70(provider, name, context)) return false;
    std::uint32_t temporary[2];
    auto* returned = dispatch_build(provider, temporary, name, context);
    try {
        assign_native_string_header_00425f40(name, returned, context.strings);
    } catch (...) {
        destroy_native_string_header_0041dd20(temporary, context.strings);
        throw;
    }
    destroy_native_string_header_0041dd20(temporary, context.strings);
    return true;
}

bool exists_native_physical_path_00bf3f70(void* provider, const void* suffix,
    NativePhysicalFileDateContext& context) {
    if (word(suffix) == 0) return false;
    auto* cache = at(provider, 0x20);
    if (equal_native_string_headers_00435c40(suffix, cache)) return true;
    if (byte(provider, 0x28) != 0) return true;
    std::uint32_t path[2];
    dispatch_build(provider, path, suffix, context); // No owner until it returns.
    bool path_armed = true;
    std::uint32_t basename[2];
    bool basename_armed = false;
    try {
        if (byte(provider, 0x28) != 0 || word(provider, 0x34) == 0) {
            const auto attributes = GetFileAttributesA(cstring(path));
            const bool found = attributes != INVALID_FILE_ATTRIBUTES;
            if (found) assign_native_string_header_00425f40(cache, suffix, context.strings);
            else assign_native_string_cstring_0041e350(cache, "", context.strings);
            path_armed = false;
            destroy_native_string_header_0041dd20(path, context.strings);
            return found;
        }
        std::uint32_t slash[2];
        construct_native_string_cstring_0041e870(slash, "/", context.strings);
        const auto position = reverse_find_native_string_header_00467cf0(suffix, slash, 0x7fffffffu);
        destroy_native_string_header_0041dd20(slash, context.strings);
        construct_native_string_substring_00469840(suffix, basename,
            static_cast<std::uint32_t>(position) + 1u, 0x7fffffffu, context.strings);
        basename_armed = true;
        auto* tree = at(provider, 0x2c);
        std::uint32_t iterator[2];
        find_native_physical_index_00bf36d0(tree, iterator, basename, context.invalid_parameters);
        std::uint32_t end[2];
        put(end, 4, word(tree, 4));
        put(end, 0, reinterpret_cast<std::uint32_t>(tree));
        bool found = false;
        if (!equal_native_physical_iterators_00bd92c0(iterator, end, context.invalid_parameters)) {
            auto* node = pointer(iterator, 4);
            auto* owner = pointer(iterator);
            for (;;) {
                if (!owner) invalid(context.invalid_parameters);
                if (node == pointer(owner, 4)) invalid(context.invalid_parameters);
                if (equal_native_string_headers_00435c40(at(node, 0x14), suffix)) {
                    found = true;
                    break;
                }
                advance_native_physical_index_00bd9860(iterator, context.invalid_parameters);
                owner = pointer(iterator);
                if (!owner || owner != tree) invalid(context.invalid_parameters);
                node = pointer(iterator, 4);
                if (node == pointer(end, 4)) break;
                if (!owner) invalid(context.invalid_parameters);
                if (node == pointer(owner, 4)) invalid(context.invalid_parameters);
                if (unequal_native_string_headers_00449af0(at(node, 0x0c), basename)) break;
            }
        }
        basename_armed = false;
        destroy_native_string_header_0041dd20(basename, context.strings);
        path_armed = false;
        destroy_native_string_header_0041dd20(path, context.strings);
        return found;
    } catch (...) {
        if (basename_armed) destroy_native_string_header_0041dd20(basename, context.strings);
        if (path_armed) destroy_native_string_header_0041dd20(path, context.strings);
        throw;
    }
}

void* query_native_physical_file_date_00bf3a80(void* provider, void* output,
    const void* name, NativePhysicalFileDateContext& context) {
    auto* manager = context.manager_0109ceec;
    if (byte(manager, 0x78) != 0) {
        put(output, 0x10, 0); put(output, 0x0c, 0); put(output, 8, 0);
        put(output, 4, 0); put(output, 0, 0);
        return output;
    }
    std::uint32_t path[2] = {0, 0};
    std::uint32_t year = 0, month = 0, day = 0, seconds = 0, milliseconds = 0;
    // Native ESP+28 clears seconds/milliseconds; hours/minutes stay unspecified.
    // ESP+2C is the separate zero millisecond result used on attributes failure.
    SYSTEMTIME time;
    time.wSecond = 0; time.wMilliseconds = 0;
    assign_native_string_header_00425f40(path, name, context.strings);
    void* captured_path_data;
    try {
        require_slot(provider, 0x18, 0x00bf39c0);
        (void)replace_native_physical_path_00bf39c0(provider, path, context);
        WIN32_FILE_ATTRIBUTE_DATA attributes;
        if (GetFileAttributesExA(cstring(path), GetFileExInfoStandard, &attributes)) {
            (void)FileTimeToSystemTime(&attributes.ftLastWriteTime, &time);
            year = time.wYear;
            month = time.wMonth;
            day = time.wDay;
            seconds = time.wSecond + (time.wMinute + time.wHour * 60u) * 60u;
            milliseconds = time.wMilliseconds;
        } else {
            // Native captures original-name data before GetLastError; 4254B0
            // is a complete RET-only variadic diagnostic (no formatting).
            (void)cstring(name);
            (void)GetLastError();
        }
        put(output, 0, year); put(output, 4, month); put(output, 8, day);
        put(output, 0x0c, seconds);
        captured_path_data = pointer(path, 4); // BF3BCE precedes the fifth store.
        put(output, 0x10, milliseconds);
    } catch (...) {
        destroy_native_string_header_0041dd20(path, context.strings);
        throw;
    }
    if (captured_path_data)
        context.strings.release(static_cast<char*>(captured_path_data), word(path) + 1u);
    return output;
}
} // namespace bsp
