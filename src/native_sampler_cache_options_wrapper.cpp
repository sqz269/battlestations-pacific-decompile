#include "bsp/native_sampler_cache_options_wrapper.hpp"

#include "bsp/native_string.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native sampler options wrapper requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Operation = NativeSamplerOptionsLoadOperation;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(NativeSamplerLoaderSingletonStorage) == 0x1c);

template <class T>
T read_current(const void* address, Word offset = 0) noexcept {
    const auto* bytes = static_cast<const char*>(address);
    return *reinterpret_cast<const volatile T*>(bytes + offset);
}

bool unresolved(const std::optional<NativeSamplerCacheEntryOperation>& child) noexcept {
    return child && (child->phase == NativeSamplerCacheEntryOperation::Phase::running ||
                     child->phase == NativeSamplerCacheEntryOperation::Phase::failed);
}

// These pointers address the public slots of the naked adapter. Volatile
// prevents an early or cached options read across the lowercase provider.
__declspec(noinline) void* __cdecl load_options_body(
    NativeSamplerLoaderSingletonStorage* receiver,
    NativeSamplerOptionsWrapperContext* context,
    const void* const volatile* original_name_slot,
    const void* const volatile* original_options_slot) {
    Operation& operation = context->operation;
    if (operation.phase != Operation::Phase::fresh)
        throw std::logic_error("sampler options load is one-shot");
    operation.phase = Operation::Phase::running;
    operation.native_site = 0x00b1b41b;
    const void* const name = *original_name_slot;
    operation.captured_name = name;
    char* captured_data = nullptr;
    operation.captured_requested_data = nullptr;
    // Volatile comparison result and volatile header stores keep the native
    // address guard ahead of BOTH zero stores in generated Win32 code.
    volatile bool same_header = static_cast<const void*>(operation.requested) == name;
    operation.receiver = receiver;
    auto* const temporary_words =
        reinterpret_cast<volatile Word*>(operation.requested);
    temporary_words[0] = 0;
    temporary_words[1] = 0;
    operation.temporary_obligation = true;

    try {
        if (!same_header) {
            operation.native_site = 0x00b1b440;
            resize_native_string_header_0041dd40(operation.requested,
                context->cache.cache.strings, read_current<Word>(name), true);
            // Native tests the CURRENT source length before loading temporary
            // data into ESI. Keep that captured pointer through both calls.
            const bool source_nonempty = read_current<Word>(name) != 0;
            captured_data = read_current<char*>(operation.requested, 4);
            operation.captured_requested_data = captured_data;
            if (source_nonempty) {
                const Word current_length = read_current<Word>(operation.requested);
                const void* const source_data = read_current<void*>(name, 4);
                operation.native_site = 0x00b1b457;
                std::memmove(captured_data, source_data, current_length);
            }
        }

        // The native state0 arm is after the complete initial copy. Neither
        // resize nor memcpy failure cleans this partially built local header.
        operation.cleanup_armed = true;
        operation.native_site = 0x00b1b46b;
        lowercase_native_string_header_004bcc00(operation.requested);
        const void* const options = *original_options_slot;
        operation.late_options = options;
        operation.cache_child.emplace();
        operation.native_site = 0x00b1b481;
        void* const result = load_native_sampler_cache_00b1a4f0(
            reinterpret_cast<char*>(receiver) + 4, operation.requested,
            options, 1, 1, context->cache, *operation.cache_child);
        operation.result = result;

        // Disarm BEFORE the current pool return. ESI remains the captured
        // pointer, while length is freshly read from the completed header.
        operation.cleanup_armed = false;
        if (captured_data) {
            const Word current_length = read_current<Word>(operation.requested);
            operation.native_site = 0x00b1b49f;
            context->cache.cache.strings.release(captured_data, current_length + 1u);
        }
        operation.temporary_obligation = false;
        operation.phase = Operation::Phase::complete;
        return result;
    } catch (...) {
        if (operation.cleanup_armed) {
            operation.cleanup_armed = false;
            operation.native_site = 0x00cbc730;
            // Native state0 tail-calls 41DD20 on the CURRENT temporary header.
            // This actual owning-pool source overload is noexcept; an inner
            // pool/getter failure follows its established terminate boundary.
            destroy_native_string_header_0041dd20(operation.requested,
                context->cache.cache.strings);
            operation.temporary_obligation = false;
        }
        operation.phase = Operation::Phase::failed;
        throw;
    }
}
} // namespace

NativeSamplerOptionsLoadOperation::~NativeSamplerOptionsLoadOperation() {
    if (phase == Phase::running || phase == Phase::failed ||
        temporary_obligation || unresolved(cache_child)) std::terminate();
}

void NativeSamplerOptionsLoadOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed || temporary_obligation || cleanup_armed ||
        unresolved(cache_child)) std::terminate();
    phase = Phase::diagnostic_retired;
}

__declspec(naked) void* __fastcall load_native_sampler_with_options_00b1b400(
    NativeSamplerLoaderSingletonStorage*, NativeSamplerOptionsWrapperContext&,
    const void*, const void*) {
    __asm {
        lea eax, [esp + 8]  // original options slot
        push eax
        lea eax, [esp + 8]  // original name slot after the first push
        push eax
        push edx             // stable new source context
        push ecx             // original complete owner
        call load_options_body
        add esp, 16
        ret 8
    }
}

} // namespace bsp
