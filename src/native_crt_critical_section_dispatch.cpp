#include "bsp/native_crt_critical_section_dispatch.hpp"

#include "bsp/native_crt_canonical_failure.hpp"
#include "bsp/native_crt_critical_section_primitives.hpp"
#include "bsp/native_crt_osplatform_getter.hpp"

#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32 raw DWORD access and Windows SEH.
#endif

namespace bsp {
namespace {

// These Source helpers admit raw, potentially unaligned/uninitialized DWORD
// bytes. They add no noexcept barrier to the actual Windows exception paths.
std::uint32_t read_raw_word(const void* address) {
    std::uint32_t value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}

void write_raw_word(void* address, std::uint32_t value) {
    __asm {
        mov eax, address
        mov ecx, value
        mov dword ptr [eax], ecx
    }
}

void and_zero_raw_word(void* address) {
    __asm {
        mov eax, address
        and dword ptr [eax], 0
    }
}

void* word_address(void* base, std::uint32_t offset) {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}

const void* word_address(const void* base, std::uint32_t offset) {
    return reinterpret_cast<const void*>(
        reinterpret_cast<std::uintptr_t>(base) + offset);
}

int filter_no_memory(EXCEPTION_POINTERS* exception_pointers, void* code_word) {
    // C176D9..C176EF: actual pointer -> record -> captured code, then publish.
    // Comparison uses that captured value even if the exposed store aliases
    // other mutable storage. The handler's later comparison is a fresh read.
    const auto record = read_raw_word(exception_pointers);
    const auto captured_code = read_raw_word(reinterpret_cast<const void*>(record));
    write_raw_word(code_word, captured_code);
    return captured_code == 0xc0000017u ? 1 : 0;
}

using CriticalSectionInitializer = std::int32_t (__stdcall*)(void*, std::uint32_t);

} // namespace

std::int32_t initialize_native_crt_critical_section_dispatch_00c17653(
    const void* actual_parameters8,
    void* actual_private12,
    volatile std::uint32_t& actual_encoded_cache_0109e454,
    const NativeCrtPointerDecodeContext& decode,
    const NativeCrtPointerEncodeContext& encode,
    const NativeCrtPointerDecodeSupportContext& support) {
    auto* const code_word = word_address(actual_private12, 0);
    auto* const result_word = word_address(actual_private12, 4);
    auto* const platform_word = word_address(actual_private12, 8);

    // C17661 is reached even when the cache subsequently decodes to a hit.
    write_raw_word(platform_word, 0);
    auto* selected = native_crt_decode_pointer_00c04fde(
        reinterpret_cast<void*>(actual_encoded_cache_0109e454), decode);

    if (selected == nullptr) {
        const auto status = get_native_crt_osplatform_00bfbab2(
            static_cast<std::uint32_t*>(platform_word), support);
        if (status != 0) {
            invoke_native_crt_canonical_watson_00bf65bb(nullptr, nullptr, nullptr, 0, 0);
        }

        // This is a current cell read after any returning Watson path.
        if (read_raw_word(platform_word) == 1) {
            selected = reinterpret_cast<void*>(
                &initialize_native_crt_critical_section_without_spin_00c17643);
        } else {
            const auto module = ::GetModuleHandleA("kernel32.dll");
            if (module != nullptr) {
                selected = reinterpret_cast<void*>(
                    ::GetProcAddress(module, "InitializeCriticalSectionAndSpinCount"));
            }
            if (selected == nullptr) {
                selected = reinterpret_cast<void*>(
                    &initialize_native_crt_critical_section_without_spin_00c17643);
            }
        }

        const auto encoded = native_crt_encode_pointer_00c04f67(selected, encode);
        actual_encoded_cache_0109e454 = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(encoded));
    }

    // C176C9 arms after all preparation/cache publication. Do not move any of
    // that work, or the common final result read, into this protected region.
    __try {
        const auto spin = read_raw_word(word_address(actual_parameters8, 4));
        const auto critical_section = read_raw_word(word_address(actual_parameters8, 0));
        const auto result = reinterpret_cast<CriticalSectionInitializer>(selected)(
            reinterpret_cast<void*>(critical_section), spin);
        write_raw_word(result_word, static_cast<std::uint32_t>(result));
    } __except (filter_no_memory(GetExceptionInformation(), code_word)) {
        // C176F3 rereads code after unwind; C17704 is one raw x86 RMW.
        if (read_raw_word(code_word) == 0xc0000017u) {
            ::SetLastError(8);
        }
        and_zero_raw_word(result_word);
    }

    return static_cast<std::int32_t>(read_raw_word(result_word));
}

} // namespace bsp
