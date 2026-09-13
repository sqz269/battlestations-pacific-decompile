#include "bsp/native_renderer_begin_frame.hpp"
#include "bsp/native_renderer_reset_process.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer BeginFrame requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRendererRecordIterator) == 8);
constexpr Word stride = 0x12c;

Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
Word byte(const volatile void* base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { movzx eax, byte ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word bits(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* base, Word offset) noexcept { return pointer(bits(base) + offset); }

void release_captured(void* data, Word size, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, size,
        strings.actual_small_returns_disabled_01090aa4);
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_first_header(void* header, NativeStringRawPoolContext& strings) noexcept {
    __try { destroy_native_string_header_0041dd20(header, strings); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct FirstHeaderCleanup {
    void* header;
    NativeStringRawPoolContext& strings;
    bool armed = true;
    ~FirstHeaderCleanup() noexcept { if (armed) unwind_first_header(header, strings); }
};
void assign_header(void* destination, const void* source, NativeStringRawPoolContext& strings) {
    if (destination == source) return;
    const Word requested = word(source);
    resize_native_string_header_0041dd40(destination, strings, requested, true);
    if (word(source) != 0) {
        const Word copied = word(destination);
        const void* const current_source = pointer(word(source, 4));
        void* const current_destination = pointer(word(destination, 4));
        // BF7680 selects a backward path for overlap. Omit the zero-byte call
        // as in the existing actual-header provider; all field reads remain.
        if (copied != 0) std::memmove(current_destination, current_source, copied);
    }
}
} // namespace

void* assign_native_renderer_record_00b13180(void* destination,
    const void* source, NativeStringRawPoolContext& strings) {
    put(destination, 0, word(source));
    // B131A0/B5/D0/E8 and B13200 are five forward groups of fourteen DWORDs.
    // These scalar accesses also preserve the native cascading overlap case.
    for (Word group = 0; group != 5; ++group) {
        const Word start = 4u + group * 0x38u;
        for (Word index = 0; index != 14; ++index) {
            const Word offset = start + index * 4u;
            put(destination, offset, word(source, offset));
        }
    }
    assign_header(at(destination, 0x11c), at(source, 0x11c), strings);
    assign_header(at(destination, 0x124), at(source, 0x124), strings);
    return destination;
}

void destroy_native_renderer_record_00b10740(void* record,
    NativeStringRawPoolContext& strings) {
    void* const second_data = pointer(word(record, 0x128));
    FirstHeaderCleanup cleanup{at(record, 0x11c), strings}; // Native state0.
    if (second_data != nullptr)
        release_captured(second_data, word(record, 0x124) + 1u, strings);
    void* const first_data = pointer(word(record, 0x120));
    cleanup.armed = false; // B10790, before first length read or getter.
    if (first_data != nullptr)
        release_captured(first_data, word(record, 0x11c) + 1u, strings);
}

void* copy_native_renderer_records_00b13280(const void* first, const void* last,
    void* output, NativeStringRawPoolContext& strings) {
    Word source = bits(first);
    const Word end = bits(last);
    Word destination = bits(output);
    while (source != end) {
        assign_native_renderer_record_00b13180(pointer(destination), pointer(source), strings);
        source += stride;
        destination += stride;
    }
    return pointer(destination);
}

void* copy_native_renderer_record_tail_00b13630(const void* first, const void* last,
    void* output, NativeStringRawPoolContext& strings) {
    copy_native_renderer_records_00b13280(first, last, output, strings);
    const Word difference = bits(last) - bits(first);
    std::int32_t signed_difference;
    std::memcpy(&signed_difference, &difference, sizeof(difference));
    const auto quotient = signed_difference / static_cast<std::int32_t>(stride);
    return pointer(bits(output) + static_cast<Word>(quotient) * stride);
}

void* erase_native_renderer_records_00b14480(void* vector, void* output_iterator,
    NativeRendererRecordIterator first, NativeRendererRecordIterator last,
    NativeStringRawPoolContext& strings) {
    void* const first_owner = first.owner;
    if (first_owner == nullptr || first_owner != last.owner) _invalid_parameter_noinfo();
    if (first.record != last.record) {
        const void* const captured_end = pointer(word(vector, 8));
        void* const new_end = copy_native_renderer_record_tail_00b13630(
            last.record, captured_end, first.record, strings);
        const Word current_end = word(vector, 8); // Fresh AFTER copy.
        Word current = bits(new_end);
        while (current != current_end) {
            destroy_native_renderer_record_00b10740(pointer(current), strings);
            current += stride;
        }
        put(vector, 8, bits(new_end)); // No publication if copy/destruction throws.
    }
    put(output_iterator, 0, bits(first_owner));
    put(output_iterator, 4, bits(first.record));
    return output_iterator;
}

void clear_native_renderer_records_00b15090(void* service,
    NativeStringRawPoolContext& strings) {
    void* const vector = at(service, 0x68c);
    const Word captured_end = word(vector, 8);
    if (word(vector, 4) > captured_end) _invalid_parameter_noinfo();
    const Word current_begin = word(vector, 4);
    if (current_begin > word(vector, 8)) _invalid_parameter_noinfo();
    NativeRendererRecordIterator output;
    erase_native_renderer_records_00b14480(vector, &output,
        {vector, pointer(current_begin)}, {vector, pointer(captured_end)}, strings);
}

std::uint8_t begin_native_renderer_frame_00b2b200(void* renderer,
    NativeRendererBeginFrameContext& context) {
    if (word(renderer, 0x1998) == 0) {
        const Word inhibited = word(renderer, 0x1d90);
        put(renderer, 0x1998, 1);
        if (inhibited != 0) put(renderer, 0x1d90, inhibited - 1u);
        process_native_renderer_device_reset_00b2abd0(renderer, context.reset);
        if (word(renderer, 0x1d90) == 0 && byte(renderer, 0x1d8a) == 0) {
            void* const device = pointer(word(renderer, 0x1a10));
            const void* const table = pointer(word(device));
            using BeginScene = HRESULT (WINAPI*)(void*);
            const auto begin_scene = reinterpret_cast<BeginScene>(word(table, 0xa4));
            begin_scene(device);
        }
        void* const service = context.actual_service_00f8d39c;
        if (service != nullptr) clear_native_renderer_records_00b15090(service, context.strings);
    }
    return 1; // Only AL is specified in the original ABI.
}
} // namespace bsp
