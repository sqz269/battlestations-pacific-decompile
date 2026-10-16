#include "bsp/native_dword_vector_transfer_range.hpp"

#include "bsp/native_dword_vector_copy_construct.hpp"
#include "bsp/native_word_vector_tidy.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word), "requires Win32 raw addresses");

Word address(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
} // namespace

__declspec(noinline) void* __fastcall transfer_native_dword_vector_range_006980e0(
    void* first, void* last, void* output, Word unused_owner_bits,
    Word /*unused_output_bits*/, Word /*unused_tag*/) {
    // Native captures output even when first==last. Only these three private
    // header words are initialized; opaque +0 has no ownership or value use.
    const Word initial_output = address(output);
    Word current_output = initial_output;
    Word current_input = address(first);
    const Word captured_last = address(last);
    Word completed_output = initial_output;
    alignas(4) Word empty_header[4];
    empty_header[1] = 0;
    empty_header[2] = 0;
    empty_header[3] = 0;
    bool temporary_cleanup_armed = true;

    try {
        try {
            while (current_input != captured_last) {
                // Native null output skips only construction. The following
                // raw field accesses still occur, including on address zero.
                if (current_output != 0) {
                    construct_native_checked_dword_vector_copy_00557590(
                        pointer(current_output), 0, empty_header);
                }

                // Exact public alias schedule. Each pair is loaded/stored
                // before the next pair. Advance output before source +C store;
                // never snapshot a header or use typed-array pointer arithmetic.
                __asm {
                    mov edi, current_input
                    mov esi, current_output
                    mov edx, dword ptr [edi + 04h]
                    mov eax, dword ptr [esi + 04h]
                    mov dword ptr [esi + 04h], edx
                    mov dword ptr [edi + 04h], eax
                    mov edx, dword ptr [edi + 08h]
                    mov eax, dword ptr [esi + 08h]
                    mov dword ptr [esi + 08h], edx
                    mov dword ptr [edi + 08h], eax
                    mov edx, dword ptr [edi + 0ch]
                    mov eax, dword ptr [esi + 0ch]
                    mov dword ptr [esi + 0ch], edx
                    add esi, 10h
                    mov dword ptr [edi + 0ch], eax
                    mov current_output, esi
                }
                completed_output = current_output;
                current_input += 0x10u;
            }

            // 698198..6981A5 remains state1 inside the native prefix try.
            // Preserve literal captured-pointer free, not normal Tidy/reset.
            // The accepted current provider is noexcept; it does not establish
            // native free-origin C++/SEH exception propagation.
            const Word captured_empty_begin = empty_header[1];
            if (captured_empty_begin != 0) {
                singleton_lifetime_free(pointer(captured_empty_begin));
            }
            temporary_cleanup_armed = false; // only after normal free returns
        } catch (...) {
            Word cleanup = initial_output;
            const Word captured_completed_end = completed_output;
            if (cleanup != captured_completed_end) {
                // Native reads this physical stack slot only for a nonempty
                // prefix. Actual 004D29D0 ignores it; no synthetic owner exists.
                Word captured_owner;
                __asm {
                    mov eax, unused_owner_bits
                    mov captured_owner, eax
                }
                do {
                    tidy_native_word_vector_range_element_004d29d0(
                        pointer(captured_owner), 0, pointer(cleanup));
                    cleanup += 0x10u;
                } while (cleanup != captured_completed_end);
            }
            throw;
        }
    } catch (...) {
        // C7EC30 selects actual CURRENT private-header Tidy, after the lexical
        // completed-prefix catch. C7EC38 is effect-elided only under the proved
        // valid-private-frame/literal-RET CurrentCpp qualification.
        if (temporary_cleanup_armed) {
            tidy_native_word_vector_storage_004cdce0(empty_header);
        }
        throw;
    }
    return pointer(current_output);
}

__declspec(noinline) void* __stdcall
transfer_native_dword_vector_range_dispatch_00698350(
    void* first, void* last, void* output) {
    void* const captured_first = first;
    void* const captured_last = last;
    void* const captured_output = output;
    return transfer_native_dword_vector_range_006980e0(
        captured_first, captured_last, captured_output, 0,
        address(captured_output), 0);
}

} // namespace bsp
