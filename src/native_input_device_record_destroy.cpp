#include "bsp/native_input_device_record_destroy.hpp"
#include "bsp/native_input_bit_sensitivity_map_full_cleanup.hpp"
#include "bsp/native_input_integer_map_full_cleanup.hpp"
#include "bsp/native_input_scalar_map_full_cleanup.hpp"
#include "bsp/native_input_vector_map_full_cleanup.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_outer_string_vector_cleanup_wrappers.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Actual input-device storage requires Win32");

void* member(void* receiver, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(
        reinterpret_cast<std::uintptr_t>(receiver) + offset);
}

struct RecordUnwind {
    void* record;
    NativeStringRawPoolContext& strings;
    volatile std::int32_t state = -1;

    ~RecordUnwind() noexcept {
        // D986EC: state is advanced before the corresponding native action.
        // These calls occur only during unwind: normal completion reaches -1.
        switch (state) {
        case 8:
            state = 7;
            unwind_native_input_device_record_00c6dc85(record, strings);
            [[fallthrough]];
        case 7:
            state = 6;
            unwind_native_input_device_record_00c6dc7a(record, strings);
            [[fallthrough]];
        case 6:
            state = 5;
            unwind_native_input_device_record_00c6dc6f(record, strings);
            [[fallthrough]];
        case 5:
            state = 4;
            unwind_native_input_device_record_00c6dc64(record, strings);
            [[fallthrough]];
        case 4:
            state = 3;
            unwind_native_input_device_record_00c6dc59(record, strings);
            [[fallthrough]];
        case 3:
            state = 2;
            unwind_native_input_device_record_00c6dc4e(record, strings);
            [[fallthrough]];
        case 2:
            state = 1;
            unwind_native_input_device_record_00c6dc43(record, strings);
            [[fallthrough]];
        case 1:
            state = 0;
            unwind_native_input_device_record_00c6dc38(record, strings);
            [[fallthrough]];
        case 0:
            state = -1;
            unwind_native_input_device_record_00c6dc30(record, strings);
            break;
        }
    }
};

struct PairUnwind {
    void* pair;
    NativeStringRawPoolContext& strings;
    volatile std::int32_t state = -1;

    ~PairUnwind() noexcept {
        if (state == 0) {
            state = -1;
            unwind_native_input_device_pair_00c6dca0(pair, strings);
        }
    }
};
} // namespace

void unwind_native_input_device_record_00c6dc30(void* record,
    NativeStringRawPoolContext& strings) {
    destroy_native_input_word_vector_map_0055a3f0(record, 0, strings);
}
void unwind_native_input_device_record_00c6dc38(void* record,
    NativeStringRawPoolContext& strings) {
    destroy_native_input_descriptor_vector_map_0055aab0(member(record, 0x0c), 0, strings);
}
void unwind_native_input_device_record_00c6dc43(void* record,
    NativeStringRawPoolContext& strings) {
    destroy_native_input_sensitivity_map_0055aaf0(member(record, 0x18), 0, strings);
}
void unwind_native_input_device_record_00c6dc4e(void* record,
    NativeStringRawPoolContext& strings) {
    destroy_native_input_scalar_map_00445900(member(record, 0x24), 0, strings);
}
void unwind_native_input_device_record_00c6dc59(void* record,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_header_vector_004324a0(member(record, 0x30), strings);
}
void unwind_native_input_device_record_00c6dc64(void* record,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_header_vector_004324a0(member(record, 0x40), strings);
}
void unwind_native_input_device_record_00c6dc6f(void* record,
    NativeStringRawPoolContext&) {
    destroy_native_input_nested_int_map_0055b490(member(record, 0x50), 0);
}
void unwind_native_input_device_record_00c6dc7a(void* record,
    NativeStringRawPoolContext& strings) {
    tidy_native_outer_string_vector_0055b110(member(record, 0x5c), strings);
}
void unwind_native_input_device_record_00c6dc85(void* record,
    NativeStringRawPoolContext& strings) {
    destroy_native_input_bit_vector_map_0055a430(member(record, 0x6c), 0, strings);
}
void unwind_native_input_device_pair_00c6dca0(void* pair,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(pair, strings);
}

void __fastcall destroy_native_input_device_record_0055b690(
    void* record, std::uint32_t, NativeStringRawPoolContext& strings) {
    RecordUnwind unwind{record, strings};
    std::uint32_t private_output[2];
    void* header;
    void* head;
    void* first;
    void* begin;
    void* end;
    void* current;
    std::uint32_t nonnull;

    // 78: captures precede the initial DWORD state8 store at B6C4.
    __asm {
        mov esi, record
        mov eax, dword ptr [esi + 0x7c]
        mov head, eax
        mov ecx, dword ptr [eax]
        mov first, ecx
        lea eax, [esi + 0x78]
        mov header, eax
    }
    unwind.state = 8;
    clear_native_input_int_only_tree_full_prefix_004cef40(
        header, 0, private_output, header, first, header, head);
    __asm {
        mov ecx, header
        mov eax, dword ptr [ecx + 4]
        mov current, eax
    }
    singleton_lifetime_free(current);
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov esi, record
        mov eax, dword ptr [esi + 0x70]
        mov head, eax
        mov edx, dword ptr [eax]
        mov first, edx
        lea eax, [esi + 0x6c]
        mov header, eax
    }
    unwind.state = 7;
    clear_native_input_bit_vector_tree_full_prefix_00559280(
        header, 0, private_output, header, first, header, head, strings);
    __asm {
        mov ecx, header
        mov eax, dword ptr [ecx + 4]
        mov current, eax
    }
    singleton_lifetime_free(current);
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov esi, record
        mov ecx, dword ptr [esi + 0x60]
        mov begin, ecx
        lea edx, [esi + 0x5c]
        mov header, edx
        xor eax, eax
        test ecx, ecx
        setnz al
        mov nonnull, eax
    }
    unwind.state = 6;
    if (nonnull != 0) {
        __asm {
            mov ecx, header
            mov eax, dword ptr [ecx + 8]
            mov end, eax
        }
        destroy_native_outer_string_vector_range_0055a3a0(begin, end,
            header, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(record)), strings);
        __asm {
            mov ecx, header
            mov eax, dword ptr [ecx + 4]
            mov current, eax
        }
        singleton_lifetime_free(current);
    }
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov dword ptr [ecx + 0x0c], 0
        mov esi, record
        mov eax, dword ptr [esi + 0x54]
        mov head, eax
        mov edx, dword ptr [eax]
        mov first, edx
        lea eax, [esi + 0x50]
        mov header, eax
    }
    unwind.state = 5;
    clear_native_input_nested_int_tree_full_prefix_0055b230(
        header, 0, private_output, header, first, header, head);
    __asm {
        mov ecx, header
        mov eax, dword ptr [ecx + 4]
        mov current, eax
    }
    singleton_lifetime_free(current);
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov esi, record
        mov ecx, dword ptr [esi + 0x44]
        mov begin, ecx
        lea edx, [esi + 0x40]
        mov header, edx
        xor eax, eax
        test ecx, ecx
        setnz al
        mov nonnull, eax
    }
    unwind.state = 4;
    if (nonnull != 0) {
        __asm {
            mov ecx, header
            mov eax, dword ptr [ecx + 8]
            mov end, eax
        }
        destroy_native_string_header_range_00432050(begin, end, strings);
        __asm {
            mov ecx, header
            mov eax, dword ptr [ecx + 4]
            mov current, eax
        }
        singleton_lifetime_free(current);
    }
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov dword ptr [ecx + 0x0c], 0
        mov esi, record
        mov ecx, dword ptr [esi + 0x34]
        mov begin, ecx
        xor eax, eax
        test ecx, ecx
        setnz al
        mov nonnull, eax
        lea edx, [esi + 0x30]
        mov header, edx
    }
    unwind.state = 3;
    if (nonnull != 0) {
        __asm {
            mov ecx, header
            mov eax, dword ptr [ecx + 8]
            mov end, eax
        }
        destroy_native_string_header_range_00432050(begin, end, strings);
        __asm {
            mov ecx, header
            mov eax, dword ptr [ecx + 4]
            mov current, eax
        }
        singleton_lifetime_free(current);
    }
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov dword ptr [ecx + 0x0c], 0
        mov esi, record
        mov eax, dword ptr [esi + 0x28]
        mov head, eax
        mov edx, dword ptr [eax]
        mov first, edx
        lea eax, [esi + 0x24]
        mov header, eax
    }
    unwind.state = 2;
    clear_native_input_scalar_tree_full_prefix_00444b10(
        header, 0, private_output, header, first, header, head, strings);
    __asm {
        mov ecx, header
        mov eax, dword ptr [ecx + 4]
        mov current, eax
    }
    singleton_lifetime_free(current);
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov esi, record
        mov eax, dword ptr [esi + 0x1c]
        mov head, eax
        mov edx, dword ptr [eax]
        mov first, edx
        lea eax, [esi + 0x18]
        mov header, eax
    }
    unwind.state = 1;
    clear_native_input_sensitivity_tree_full_prefix_00559a90(
        header, 0, private_output, header, first, header, head, strings);
    __asm {
        mov ecx, header
        mov eax, dword ptr [ecx + 4]
        mov current, eax
    }
    singleton_lifetime_free(current);
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov esi, record
        mov eax, dword ptr [esi + 0x10]
        mov head, eax
        mov edx, dword ptr [eax]
        mov first, edx
        lea eax, [esi + 0x0c]
        mov header, eax
    }
    unwind.state = 0;
    clear_native_input_descriptor_vector_tree_full_prefix_005599c0(
        header, 0, private_output, header, first, header, head, strings);
    __asm {
        mov ecx, header
        mov eax, dword ptr [ecx + 4]
        mov current, eax
    }
    singleton_lifetime_free(current);
    __asm {
        mov ecx, header
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        mov ecx, record
        mov eax, dword ptr [ecx + 4]
        mov head, eax
        mov edx, dword ptr [eax]
        mov first, edx
    }
    unwind.state = -1;
    clear_native_input_word_vector_tree_full_prefix_00559110(
        record, 0, private_output, record, first, record, head, strings);
    __asm {
        mov ecx, record
        mov eax, dword ptr [ecx + 4]
        mov current, eax
    }
    singleton_lifetime_free(current);
    __asm {
        mov ecx, record
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
    }
}

void __fastcall destroy_native_input_device_pair_0055b8b0(
    void* pair, std::uint32_t, NativeStringRawPoolContext& strings) {
    PairUnwind unwind{pair, strings};
    unwind.state = 0;
    destroy_native_input_device_record_0055b690(member(pair, 8), 0, strings);
    void* data;
    std::uint32_t nonnull;
    __asm {
        mov ecx, pair
        mov edx, dword ptr [ecx + 4]
        mov data, edx
        xor eax, eax
        test edx, edx
        setnz al
        mov nonnull, eax
    }
    unwind.state = -1;
    if (nonnull != 0) {
        std::uint32_t captured_size;
        __asm {
            mov ecx, pair
            mov eax, dword ptr [ecx]
            add eax, 1
            mov captured_size, eax
        }
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, data, captured_size,
            strings.actual_small_returns_disabled_01090aa4);
    }
}

void* __fastcall scalar_delete_native_input_device_pair_node_006a6eb0(
    void* node, std::uint32_t, std::uint32_t flags,
    NativeStringRawPoolContext& strings) {
    destroy_native_input_device_pair_0055b8b0(member(node, 0x0c), 0, strings);
    std::uint32_t release;
    __asm {
        xor eax, eax
        test byte ptr flags, 1
        setnz al
        mov release, eax
    }
    if (release != 0) singleton_lifetime_free(node);
    return node;
}
} // namespace bsp
