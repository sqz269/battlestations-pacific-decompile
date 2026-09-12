#include "bsp/native_path_canonicalizer.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
std::uint32_t address(const void* pointer) noexcept {
    return reinterpret_cast<std::uint32_t>(pointer);
}
std::uint32_t word(const void* pointer, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(address(pointer) + offset);
}
std::uint8_t byte(std::uint32_t pointer) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(pointer);
}
void put_byte(std::uint32_t pointer, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(pointer) = value;
}
bool slash(std::uint8_t value) noexcept {
    return value == '/' || value == '\\';
}

// BDB970 inlines this capture/getter/return sequence twice. Its CC60D0/CC60D8
// unwind actions use 41DD20, whose body performs the same captures and calls.
// Keep the getter outside NativeStringStorage's noexcept release interface so
// failure while recreating the actual pool follows the recovered EH schedule.
void release_temporary(void* header, NativePathCanonicalizerServices& services) {
    void* const captured_data = reinterpret_cast<void*>(word(header, 4));
    if (!captured_data) return;
    const auto bytes = word(header) + 1u;
    auto* const pool = services.string_pool_00419cc0();
    services.return_string_00bd1510(pool, captured_data, bytes, 1);
}
struct TemporaryCleanup {
    void* source;
    void* output;
    NativePathCanonicalizerServices& services;
    int state = 0;
    ~TemporaryCleanup() noexcept(false) {
        if (state == 0) release_temporary(source, services);
        else if (state == 2) release_temporary(output, services);
    }
};
} // namespace

NativePathCanonicalizerRuntimeServices::NativePathCanonicalizerRuntimeServices(
    NativeStringPoolStorage* volatile& publication,
    volatile std::uint32_t& disabled, SingletonLifetimeDomain& lifetime,
    Lowercase lower) noexcept
    : publication_(publication), returns_disabled_(disabled), lifetime_(&lifetime), lower_(lower) {}
NativePathCanonicalizerRuntimeServices::NativePathCanonicalizerRuntimeServices(
    NativeStringPoolStorage* volatile& publication,
    volatile std::uint32_t& disabled, void* volatile& actual_manager, Lowercase lower) noexcept
    : publication_(publication), returns_disabled_(disabled), actual_manager_(&actual_manager), lower_(lower) {}
void* NativePathCanonicalizerRuntimeServices::allocate_scratch_00bf55be(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
int NativePathCanonicalizerRuntimeServices::lowercase_00bf9611(int value) {
    return lower_(value);
}
void NativePathCanonicalizerRuntimeServices::free_scratch_00bf65ac(void* value) noexcept {
    singleton_lifetime_free(value);
}
NativeStringPoolStorage* NativePathCanonicalizerRuntimeServices::string_pool_00419cc0() {
    if(actual_manager_)
        return native_string_pool_get_or_create_00419cc0(publication_, *actual_manager_);
    return native_string_pool_get_or_create_00419cc0(publication_, *lifetime_);
}
void NativePathCanonicalizerRuntimeServices::return_string_00bd1510(
    NativeStringPoolStorage* pool, void* block, std::uint32_t bytes,
    std::uint32_t unused) noexcept {
    (void)unused;
    return_native_string_pool_00bd1510(pool, block, bytes, returns_disabled_);
}

void* canonicalize_native_path_00bee390(void* output, const void* input,
    NativePathCanonicalizerContext& context) {
    char stack_scratch[260];
    const auto initial_length = word(input);
    void* const storage = initial_length < 256u
        ? static_cast<void*>(stack_scratch)
        : context.services.allocate_scratch_00bf55be(initial_length + 1u);
    const auto base = address(storage);
    auto input_data = word(input, 4);
    if (!input_data) input_data = address(context.null_source_0109db91);
    auto copied = base;
    std::uint8_t value;
    do {
        value = byte(input_data);
        put_byte(copied, value);
        ++input_data;
        ++copied;
    } while (value != 0);
    // BEE3DF reloads the count after allocation and C-string copy. Later calls
    // do not change the captured end; both cursors use native DWORD arithmetic.
    const auto end = base + word(input);
    auto current = base;
    auto result = base;
    auto parent_barrier = base;
    if (slash(byte(base))) {
        current = base + 1u;
        result = current;
        put_byte(base, '/');
        parent_barrier = current;
    }
    bool separator = true;
    while (current != end) {
        value = byte(current);
        if (slash(value)) {
            ++current;
            if (!separator) {
                put_byte(result, '/');
                ++result;
                separator = true;
            }
            continue;
        }
        if (separator && value == '.') {
            const auto remaining = end - current;
            if (remaining == 1u) {
                ++current;
                if (result > base) --result;
                separator = false;
                continue;
            }
            const auto next = byte(current + 1u);
            if (slash(next)) {
                current += 2u;
                continue;
            }
            if (next == '.' && (remaining == 2u || slash(byte(current + 2u)))) {
                if (result == parent_barrier) {
                    put_byte(result++, '.');
                    put_byte(result++, '.');
                    put_byte(result++, '/');
                    parent_barrier = result;
                } else {
                    --result;
                    while (result != base && byte(result - 1u) != '/') --result;
                }
                current += 2u;
                separator = true;
                continue;
            }
        }
        // MOVSX at BEE4C4. Do not change negative byte values into 128..255.
        const int signed_value = value < 128u ? value : static_cast<int>(value) - 256;
        separator = false;
        const int lowered = context.services.lowercase_00bf9611(signed_value);
        put_byte(result, static_cast<std::uint8_t>(lowered));
        ++result;
        ++current;
    }
    put_byte(result, 0);
    construct_native_string_cstring_0041e870(output,
        static_cast<const char*>(storage), context.strings);
    if (storage != stack_scratch) context.services.free_scratch_00bf65ac(storage);
    return output;
}

void canonicalize_discard_native_path_00bdb970(const char* source,
    NativePathCanonicalizerContext& context) {
    std::uint32_t source_header[2];
    std::uint32_t output_header[2];
    construct_native_string_cstring_0041e870(source_header, source, context.strings);
    TemporaryCleanup cleanup{source_header, output_header, context.services};
    canonicalize_native_path_00bee390(output_header, source_header, context);
    cleanup.state = 2;
    release_temporary(source_header, context.services);
    cleanup.state = -1;
    release_temporary(output_header, context.services);
}
} // namespace bsp
