// Packet cc13-seg61: segment 61 leaves (0092E250..00951940). See
// include/bsp/support_seg61_cc13.hpp and docs/CC13_SEG61.md. Each body carries
// its address, the original ABI and every simplification next to it.

#include "bsp/support_seg61_cc13.hpp"

#include <cstdlib>
#include <cstring>

#include "bsp/entity_identity.hpp"
#include "bsp/sound_sample.hpp"

namespace bsp::support_seg61_cc13 {
namespace {

// 00F89F80: the shared 256-byte scratch buffer (.bss) the writer formats into
// and fills with indent characters.
constexpr std::uint32_t kScratch = 0x00f89f80;
// 00D19B70, "\r\n": the line terminator (written with length 2).
constexpr char kNewline[] = "\r\n";
// 00CF393C "%u" and 00D19B78 "%.4f".
constexpr char kFormatUnsigned[] = "%u";
constexpr char kFormatFloat[] = "%.4f";
// Vtables (image addresses).
constexpr std::uint32_t kSceneTextChunkVtable = 0x00d19b74;  // slot 0 = 00950380
constexpr std::uint32_t kBlockBufferVtable = 0x00d19b64;     // slot 0 = 0094F810
constexpr std::uint32_t kSingletonBaseVtable = 0x00ce3818;
// CreateFileA arguments of 009501B0.
constexpr std::uint32_t kGenericWrite = 0x40000000;
constexpr std::uint32_t kCreateAlways = 2;
constexpr std::uint32_t kAttributeNormal = 0x80;
constexpr std::uint32_t kInvalidHandle = 0xffffffff;
// 00CE3D08, float 100.0: the pair 0093F790 returns when no key matches.
constexpr float kDefaultPairValue = 100.0f;
// 00F89B75: a zero byte in .bss that stands in for a null string data
// pointer. Only its contents (empty) are read, so a local empty string is used.
const char kEmptyString[1] = {0};

constexpr std::uint32_t kChunkCapacity = 0x800;  // data bytes per chunk (+4..+803h)
constexpr std::size_t kChunkUsed = 0x804;
constexpr std::size_t kChunkNext = 0x808;

char* scratch(Seg61Host& host) { return static_cast<char*>(host.image_address(kScratch)); }

Raw as_raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }

// The `count * 4` operator-new argument: MUL by 4 with SETO/NEG/OR saturating
// to FFFFFFFFh on overflow.
std::uint32_t saturating_times4(std::uint32_t count) noexcept {
    const std::uint64_t bytes = static_cast<std::uint64_t>(count) * 4u;
    return (bytes >> 32) != 0 ? 0xffffffffu : static_cast<std::uint32_t>(bytes);
}

// The string data pointer of a {length, data} record, or the empty fallback.
const char* string_data(const void* string) noexcept {
    const char* data = static_cast<const char*>(as_raw(const_cast<void*>(string)).ptr(0x4));
    return data != nullptr ? data : kEmptyString;
}

// The writer's "indent, then write text when non-empty" step shared by
// 0094FF10, 0094FF50, 00950000, 00950050 and 00950130. Length is strlen.
void write_text_after_indent(Raw writer, const char* text, Seg61Host& host) {
    scene_text_writer_write_indent_0094fdc0(writer, host);
    if (text != nullptr && *text != '\0') {
        host.writer_write_slot0(writer, text, static_cast<std::uint32_t>(std::strlen(text)));
    }
}

// The new-chunk body inlined in 009501F0, 00950240 and 009502E0: operator
// new 80Ch, and when it succeeds vtable 00D19B74, used = 0, next = 0.
void* new_chunk(Seg61Host& host) {
    void* const chunk = host.operator_new_00bf681b(0x80c);
    if (chunk != nullptr) {
        Raw c = as_raw(chunk);
        c.at<std::uint32_t>(0) = kSceneTextChunkVtable;
        c.at<std::uint32_t>(kChunkUsed) = 0;
        c.ptr(kChunkNext) = nullptr;
    }
    return chunk;
}

// The pooled release shared by 0094F3A0 and 0094F3E0's loop: when the data
// pointer is set, (data, length + 1, 1) back to the sized pool; then operator
// delete (00BF65AC) of the 8-byte record.
void delete_pooled_string(void* string, Seg61Host& host) {
    Raw s = as_raw(string);
    void* const data = s.ptr(0x4);
    if (data != nullptr) {
        const std::uint32_t bytes = s.at<std::uint32_t>(0x0) + 1;
        void* const pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, data, bytes, 1);
    }
    host.operator_delete_00bf65ac(string);
}

// 0094F270 / 0094F2F0 shared head: grow when full, then, unless `adopt`, a
// copy of `string` in a new 8-byte record (null when operator new fails).
void* prepare_list_entry(Raw list, void* string, bool adopt, Seg61Host& host) {
    if (list.at<std::int32_t>(0x4) == list.at<std::int32_t>(0x8)) string_list_grow_0094f010(list, host);
    if (adopt) return string;
    void* const memory = host.operator_new_00bf681b(8);
    if (memory == nullptr) return nullptr;
    return host.native_string_copy_construct_00426060(memory, string);
}

void singleton_base_destruct(Raw object, std::uint32_t global, Seg61Host& host) {
    host.global_u32(global) = 0;
    object.at<std::uint32_t>(0) = kSingletonBaseVtable;
}

}  // namespace

// 0094FDC0, __thiscall(writer), RET. Unless the line is already indented
// (byte +4) or the depth (+8) is not positive: memset the scratch buffer with
// the fill char (+10h, sign-extended) for depth * width (+0Ch) bytes and write
// it through slot 0. Byte +4 = 1 in every case. The scratch buffer is 256
// bytes; the native does not bound the memset either.
void scene_text_writer_write_indent_0094fdc0(Raw writer, Seg61Host& host) {
    if (writer.at<std::uint8_t>(0x4) == 0) {
        const std::int32_t depth = writer.at<std::int32_t>(0x8);
        if (depth > 0) {
            const std::uint32_t bytes = writer.at<std::uint32_t>(0xc) * static_cast<std::uint32_t>(depth);
            char* const buffer = scratch(host);
            std::memset(buffer, static_cast<int>(writer.at<std::int8_t>(0x10)), bytes);
            host.writer_write_slot0(writer, buffer, bytes);
        }
    }
    writer.at<std::uint8_t>(0x4) = 1;
}

// 0094FF10, __thiscall(writer, const char*), RET 4, EAX = writer.
Raw scene_text_writer_write_string_0094ff10(Raw writer, const char* text, Seg61Host& host) {
    write_text_after_indent(writer, text, host);
    return writer;
}

// 0094FF50, __thiscall(writer, const char*), RET 4, EAX = writer: as 0094FF10,
// then "\r\n" (length 2) and byte +4 = 0.
Raw scene_text_writer_write_line_0094ff50(Raw writer, const char* text, Seg61Host& host) {
    write_text_after_indent(writer, text, host);
    host.writer_write_slot0(writer, kNewline, 2);
    writer.at<std::uint8_t>(0x4) = 0;
    return writer;
}

// 0094FFA0, __thiscall(writer, int code), RET 4, EAX = writer. 0: "\r\n" and
// byte +4 = 0. 1: one indent unit (width +0Ch of the fill char) through the
// scratch buffer; byte +4 is left alone. Other codes do nothing.
Raw scene_text_writer_manipulator_0094ffa0(Raw writer, std::int32_t code, Seg61Host& host) {
    if (code == 0) {
        host.writer_write_slot0(writer, kNewline, 2);
        writer.at<std::uint8_t>(0x4) = 0;
    } else if (code == 1) {
        const std::uint32_t bytes = writer.at<std::uint32_t>(0xc);
        char* const buffer = scratch(host);
        std::memset(buffer, static_cast<int>(writer.at<std::int8_t>(0x10)), bytes);
        host.writer_write_slot0(writer, buffer, bytes);
    }
    return writer;
}

// 00950000, __thiscall(writer, int), RET 4, EAX = writer: _itoa(value,
// scratch, 10) (00BF9010) before the indent, then the scratch text.
Raw scene_text_writer_write_int_00950000(Raw writer, std::int32_t value, Seg61Host& host) {
    char* const buffer = scratch(host);
    host.itoa_00bf9010(value, buffer, 10);
    write_text_after_indent(writer, buffer, host);
    return writer;
}

// 00950050, __thiscall(writer, unsigned), RET 4, EAX = writer: 00439AC0("%u",
// value) before the indent, then that text. (The ledger name says
// "WriteFormatted"; the format is the fixed 00CF393C "%u".)
Raw scene_text_writer_write_unsigned_00950050(Raw writer, std::uint32_t value, Seg61Host& host) {
    const char* const text = host.format_static_00439ac0(kFormatUnsigned, value);
    write_text_after_indent(writer, text, host);
    return writer;
}

// 00950130, __thiscall(writer, float), RET 4, EAX = writer: 004B8490(scratch,
// "%.4f", (double)value) before the indent, then the scratch text.
Raw scene_text_writer_write_float_00950130(Raw writer, float value, Seg61Host& host) {
    char* const buffer = scratch(host);
    host.format_string256_004b8490(buffer, kFormatFloat, static_cast<double>(value));
    write_text_after_indent(writer, buffer, host);
    return writer;
}

// 009500A0, __thiscall(writer), RET: operator new (00BF55BE) of total + 1,
// copy each chunk's used bytes from head along the next links, NUL-terminate.
// No null test on the allocation.
char* scene_text_chunk_list_join_009500a0(Raw writer, Seg61Host& host) {
    char* const joined = static_cast<char*>(host.operator_new_00bf55be(writer.at<std::uint32_t>(0x1c) + 1));
    char* out = joined;
    for (void* chunk = writer.ptr(0x14); chunk != nullptr; chunk = as_raw(chunk).ptr(kChunkNext)) {
        Raw c = as_raw(chunk);
        std::memcpy(out, c.p + 4, c.at<std::uint32_t>(kChunkUsed));
        out += c.at<std::uint32_t>(kChunkUsed);
    }
    *out = '\0';
    return joined;
}

// 00950190, __thiscall(writer, data, length), RET 8: WriteFile(+14h, data,
// length, &length, null); the written count lands in the argument slot.
void win32_file_writer_write_00950190(Raw writer, const void* data, std::uint32_t length, Seg61Host& host) {
    host.write_file_ce2290(writer.at<std::uint32_t>(0x14), data, length, &length, nullptr);
}

// 009501B0, __thiscall(writer, name), RET 4: +14h = CreateFileA(name,
// GENERIC_WRITE, 0, null, CREATE_ALWAYS, NORMAL, null); on INVALID_HANDLE_VALUE
// 004531F0(name). Returns +14h != -1.
bool win32_file_writer_open_009501b0(Raw writer, const char* name, Seg61Host& host) {
    writer.at<std::uint32_t>(0x14) =
        host.create_file_a_ce2260(name, kGenericWrite, 0, nullptr, kCreateAlways, kAttributeNormal, nullptr);
    if (writer.at<std::uint32_t>(0x14) == kInvalidHandle) host.show_last_system_error_box_004531f0(name);
    return writer.at<std::uint32_t>(0x14) != kInvalidHandle;
}

// 009501F0, __thiscall(writer), RET: tail(+18h)->next = a new chunk (null when
// operator new failed), tail = it.
void scene_text_chunk_list_append_chunk_009501f0(Raw writer, Seg61Host& host) {
    void* const chunk = new_chunk(host);
    as_raw(writer.ptr(0x18)).ptr(kChunkNext) = chunk;
    writer.ptr(0x18) = chunk;
}

// 00950240, __thiscall(writer, data, length), RET 8 (the chunk writer's slot
// 0): total (+1Ch) += length, then copy in pieces, appending a chunk whenever
// the tail's used count exceeds 7FFh.
void scene_text_chunk_list_write_00950240(Raw writer, const void* data, std::uint32_t length, Seg61Host& host) {
    writer.at<std::uint32_t>(0x1c) += length;
    const auto* source = static_cast<const std::uint8_t*>(data);
    while (length != 0) {
        if (as_raw(writer.ptr(0x18)).at<std::uint32_t>(kChunkUsed) > kChunkCapacity - 1) {
            void* const chunk = new_chunk(host);
            as_raw(writer.ptr(0x18)).ptr(kChunkNext) = chunk;
            writer.ptr(0x18) = chunk;
        }
        Raw tail = as_raw(writer.ptr(0x18));
        const std::uint32_t used = tail.at<std::uint32_t>(kChunkUsed);
        std::uint32_t piece = kChunkCapacity - used;
        if (length < piece) piece = length;
        std::memcpy(tail.p + 4 + used, source, piece);
        tail.at<std::uint32_t>(kChunkUsed) += piece;
        source += piece;
        length -= piece;
    }
}

// 009502E0, __thiscall(writer), RET: head = tail = a new chunk (both null when
// operator new failed), total = 0.
void scene_text_chunk_list_reset_009502e0(Raw writer, Seg61Host& host) {
    void* const chunk = new_chunk(host);
    writer.ptr(0x18) = chunk;
    writer.ptr(0x14) = chunk;
    writer.at<std::uint32_t>(0x1c) = 0;
}

// 00950320, __thiscall(writer), RET: the head chunk's slot 0 (1) deletes the
// chain (00950350 deletes its next), then head, tail and total are cleared.
void scene_text_chunk_list_clear_00950320(Raw writer, Seg61Host& host) {
    if (writer.ptr(0x14) != nullptr) {
        host.object_delete_slot0(writer.ptr(0x14));
        writer.ptr(0x14) = nullptr;
    }
    writer.ptr(0x18) = nullptr;
    writer.ptr(0x14) = nullptr;
    writer.at<std::uint32_t>(0x1c) = 0;
}

// 00950350, __thiscall(chunk), RET: vtable 00D19B74; the next chunk's slot
// 0 (1) when set, and next cleared.
void scene_text_chunk_destruct_00950350(Raw chunk, Seg61Host& host) {
    chunk.at<std::uint32_t>(0) = kSceneTextChunkVtable;
    if (chunk.ptr(kChunkNext) != nullptr) {
        host.object_delete_slot0(chunk.ptr(kChunkNext));
        chunk.ptr(kChunkNext) = nullptr;
    }
}

// 0094EFB0, __thiscall(list), RET, EAX = list: capacity 40h, array = operator
// new (00BF55BE) of 40h * 4 (saturating multiply), count 0.
Raw string_list_construct_0094efb0(Raw list, Seg61Host& host) {
    list.at<std::uint32_t>(0x8) = 0x40;
    list.ptr(0x0) = host.operator_new_00bf55be(saturating_times4(0x40));
    list.at<std::uint32_t>(0x4) = 0;
    return list;
}

// 0094F010, __thiscall(list), RET: capacity += 40h, new array (00BF55BE,
// saturating count * 4), copy count * 4 bytes, _free the old array when set
// (and clear it), store the new one.
void string_list_grow_0094f010(Raw list, Seg61Host& host) {
    list.at<std::uint32_t>(0x8) += 0x40;
    void* const array = host.operator_new_00bf55be(saturating_times4(list.at<std::uint32_t>(0x8)));
    std::memcpy(array, list.ptr(0x0), list.at<std::uint32_t>(0x4) * 4);
    if (list.ptr(0x0) != nullptr) {
        host.free_00bf6989(list.ptr(0x0));
        list.ptr(0x0) = nullptr;
    }
    list.ptr(0x0) = array;
}

// 0094F090, __thiscall(list, const char* needle, bool), RET 8: the first
// index whose string equals the needle, else -1. Flag set: 00438E10(needle,
// item) case-insensitive; clear: an inline strcmp(needle, item).
std::int32_t string_list_find_0094f090(Raw list, const char* needle, bool case_insensitive) {
    for (std::int32_t i = 0; i < list.at<std::int32_t>(0x4); ++i) {
        const void* const string = static_cast<void**>(list.ptr(0x0))[i];
        const char* const item = string_data(string);
        const int order = case_insensitive ? compare_insensitive_00438e10(needle, item) : std::strcmp(needle, item);
        if (order == 0) return i;
    }
    return -1;
}

// 0094F130 (a qsort comparator inside 0094F1B0's body, no Ghidra function):
// strcmp of the two records' data (null data reads as "").
int string_list_compare_0094f130(const void* left, const void* right) {
    const char* const a = string_data(*static_cast<void* const*>(left));
    const char* const b = string_data(*static_cast<void* const*>(right));
    const int order = std::strcmp(a, b);
    return order < 0 ? -1 : (order > 0 ? 1 : 0);
}

// 0094F180: the same with a tail jump to 00438E10(ECX = left, EDX = right).
int string_list_compare_insensitive_0094f180(const void* left, const void* right) {
    const char* const a = string_data(*static_cast<void* const*>(left));
    const char* const b = string_data(*static_cast<void* const*>(right));
    return compare_insensitive_00438e10(a, b);
}

// 0094F1B0, __thiscall(list, bool), RET 4: qsort (00BFC350) of the pointer
// array, element size 4. Flag set: 0094F130 (case-sensitive); clear:
// 0094F180. The flag sense is the opposite of 0094F090's.
void string_list_sort_0094f1b0(Raw list, bool case_sensitive) {
    std::qsort(list.ptr(0x0), list.at<std::uint32_t>(0x4), 4,
               case_sensitive ? &string_list_compare_0094f130 : &string_list_compare_insensitive_0094f180);
}

// 0094F1E0, __thiscall(list), RET: one buffer (00BF55BE) of sum(lengths) +
// count - 1 bytes (at least 1) plus the terminator; the strings joined with
// '\n' (0Ah) and NUL-terminated.
char* string_list_join_lines_0094f1e0(Raw list, Seg61Host& host) {
    const std::int32_t count = list.at<std::int32_t>(0x4);
    void** const items = static_cast<void**>(list.ptr(0x0));
    std::int32_t total = 0;
    for (std::int32_t i = 0; i < count; ++i) total += as_raw(items[i]).at<std::int32_t>(0x0);
    std::int32_t bytes = total - 1 + count;
    if (bytes < 1) bytes = 1;
    char* const joined = static_cast<char*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(bytes + 1)));
    char* out = joined;
    for (std::int32_t i = 0; i < list.at<std::int32_t>(0x4); ++i) {
        if (i != 0) *out++ = '\n';
        void* const string = static_cast<void**>(list.ptr(0x0))[i];
        std::memcpy(out, string_data(string), as_raw(string).at<std::uint32_t>(0x0));
        out += as_raw(static_cast<void**>(list.ptr(0x0))[i]).at<std::int32_t>(0x0);
    }
    *out = '\0';
    return joined;
}

// 0094F270, __thiscall(list, string, bool adopt), RET 8, EAX = new count:
// grow when full; adopt stores `string` itself, otherwise a copy (operator
// new 8 + 00426060; null when the allocation fails); append.
std::int32_t string_list_append_0094f270(Raw list, void* string, bool adopt, Seg61Host& host) {
    void* const entry = prepare_list_entry(list, string, adopt, host);
    static_cast<void**>(list.ptr(0x0))[list.at<std::int32_t>(0x4)] = entry;
    list.at<std::int32_t>(0x4) += 1;
    return list.at<std::int32_t>(0x4);
}

// 0094F2F0, __thiscall(list, string, int index, bool adopt), RET 0Ch: the
// same entry, inserted at index clamped to [0, count], shifting the tail up.
void string_list_insert_0094f2f0(Raw list, void* string, std::int32_t index, bool adopt, Seg61Host& host) {
    void* const entry = prepare_list_entry(list, string, adopt, host);
    if (index < 0) {
        index = 0;
    } else if (list.at<std::int32_t>(0x4) < index) {
        index = list.at<std::int32_t>(0x4);
    }
    void** const items = static_cast<void**>(list.ptr(0x0));
    for (std::int32_t i = list.at<std::int32_t>(0x4); index < i; --i) items[i] = items[i - 1];
    items[index] = entry;
    list.at<std::int32_t>(0x4) += 1;
}

// 0094F3A0, __thiscall(slot), RET: when *slot is set, release its data to the
// pool, operator delete the record and clear the slot.
void string_list_delete_string_0094f3a0(Raw slot, Seg61Host& host) {
    void* const string = slot.ptr(0x0);
    if (string != nullptr) {
        delete_pooled_string(string, host);
        slot.ptr(0x0) = nullptr;
    }
}

// 0094F3E0, __thiscall(list), RET: delete every non-null entry (count re-read
// each iteration) and clear its slot, then count = 0.
void string_list_clear_0094f3e0(Raw list, Seg61Host& host) {
    for (std::int32_t i = 0; i < list.at<std::int32_t>(0x4); ++i) {
        void** const slot = static_cast<void**>(list.ptr(0x0)) + i;
        if (*slot != nullptr) {
            delete_pooled_string(*slot, host);
            *slot = nullptr;
        }
    }
    list.at<std::int32_t>(0x4) = 0;
}

// 0094F540, __thiscall(list), RET: 0094F3E0, then _free the array when set
// and clear it.
void string_list_destroy_0094f540(Raw list, Seg61Host& host) {
    string_list_clear_0094f3e0(list, host);
    if (list.ptr(0x0) != nullptr) {
        host.free_00bf6989(list.ptr(0x0));
        list.ptr(0x0) = nullptr;
    }
}

// 0094F560, __thiscall(buffer, block size), RET 4, EAX = buffer: vtable
// 00D19B64, +4 = size, +8 = operator new(4) (00BF55BE), [+8][0] = operator
// new(size), +0Ch..+1Ch = 0 (stores in the order +10h, +0Ch, +14h, +18h, +1Ch).
Raw block_buffer_construct_0094f560(Raw buffer, std::uint32_t block_size, Seg61Host& host) {
    buffer.at<std::uint32_t>(0) = kBlockBufferVtable;
    buffer.at<std::uint32_t>(0x4) = block_size;
    buffer.ptr(0x8) = host.operator_new_00bf55be(4);
    void* const block = host.operator_new_00bf55be(buffer.at<std::uint32_t>(0x4));
    static_cast<void**>(buffer.ptr(0x8))[0] = block;
    buffer.at<std::uint32_t>(0x10) = 0;
    buffer.at<std::uint32_t>(0xc) = 0;
    buffer.at<std::uint32_t>(0x14) = 0;
    buffer.at<std::uint32_t>(0x18) = 0;
    buffer.at<std::uint32_t>(0x1c) = 0;
    return buffer;
}

// 0094F5E0, __thiscall(buffer), RET: vtable 00D19B64; unless +10h + 1 wraps
// to 0, _free each non-null block 0..+10h (bound re-read) and clear it; then
// _free the block array and the flattened copy when set, clearing each.
void block_buffer_destruct_0094f5e0(Raw buffer, Seg61Host& host) {
    buffer.at<std::uint32_t>(0) = kBlockBufferVtable;
    if (buffer.at<std::uint32_t>(0x10) + 1 != 0) {
        std::uint32_t i = 0;
        do {
            void** const slot = static_cast<void**>(buffer.ptr(0x8)) + i;
            if (*slot != nullptr) {
                host.free_00bf6989(*slot);
                *slot = nullptr;
            }
            ++i;
        } while (i < buffer.at<std::uint32_t>(0x10) + 1);
    }
    if (buffer.ptr(0x8) != nullptr) {
        host.free_00bf6989(buffer.ptr(0x8));
        buffer.ptr(0x8) = nullptr;
    }
    if (buffer.ptr(0x18) != nullptr) {
        host.free_00bf6989(buffer.ptr(0x18));
        buffer.ptr(0x18) = nullptr;
    }
}

// 0094F650, __thiscall(buffer), RET: index and used = 0; _free the flattened
// copy when set; +1Ch and +18h = 0. The blocks stay allocated for reuse.
void block_buffer_reset_0094f650(Raw buffer, Seg61Host& host) {
    buffer.at<std::uint32_t>(0xc) = 0;
    buffer.at<std::uint32_t>(0x14) = 0;
    if (buffer.ptr(0x18) != nullptr) host.free_00bf6989(buffer.ptr(0x18));
    buffer.at<std::uint32_t>(0x1c) = 0;
    buffer.ptr(0x18) = nullptr;
}

// 0094F680, __thiscall(buffer), RET: ++index; past the last allocated block,
// a new array of (index + 1) pointers (00BF55BE, saturating), copy index
// pointers, a new block (operator new of the block size) at [index], _free the
// old array (cleared first), store the array, last = index. Used = 0.
void block_buffer_next_block_0094f680(Raw buffer, Seg61Host& host) {
    buffer.at<std::uint32_t>(0xc) += 1;
    if (buffer.at<std::uint32_t>(0x10) < buffer.at<std::uint32_t>(0xc)) {
        void** const array = static_cast<void**>(
            host.operator_new_00bf55be(saturating_times4(buffer.at<std::uint32_t>(0xc) + 1)));
        std::memcpy(array, buffer.ptr(0x8), buffer.at<std::uint32_t>(0xc) * 4);
        array[buffer.at<std::uint32_t>(0xc)] = host.operator_new_00bf55be(buffer.at<std::uint32_t>(0x4));
        if (buffer.ptr(0x8) != nullptr) {
            host.free_00bf6989(buffer.ptr(0x8));
            buffer.ptr(0x8) = nullptr;
        }
        buffer.ptr(0x8) = array;
        buffer.at<std::uint32_t>(0x10) = buffer.at<std::uint32_t>(0xc);
    }
    buffer.at<std::uint32_t>(0x14) = 0;
}

// 0094F770, __thiscall(buffer), RET: the flattened copy. When +18h is set and
// +1Ch == index * size + used, the old copy is returned. Otherwise a new copy
// (00BF55BE, size * index + used + 1) of the full blocks and the used part of
// the current one, NUL-terminated, is stored at +18h and returned. The native
// neither frees a previous copy nor updates +1Ch here (both kept).
char* block_buffer_flatten_0094f770(Raw buffer, Seg61Host& host) {
    if (buffer.ptr(0x18) != nullptr &&
        buffer.at<std::uint32_t>(0x1c) ==
            buffer.at<std::uint32_t>(0xc) * buffer.at<std::uint32_t>(0x4) + buffer.at<std::uint32_t>(0x14)) {
        return static_cast<char*>(buffer.ptr(0x18));
    }
    buffer.ptr(0x18) = host.operator_new_00bf55be(
        buffer.at<std::uint32_t>(0x4) * buffer.at<std::uint32_t>(0xc) + buffer.at<std::uint32_t>(0x14) + 1);
    std::uint32_t offset = 0;
    for (std::uint32_t i = 0; i < buffer.at<std::uint32_t>(0xc); ++i) {
        std::memcpy(static_cast<char*>(buffer.ptr(0x18)) + offset, static_cast<void**>(buffer.ptr(0x8))[i],
                    buffer.at<std::uint32_t>(0x4));
        offset += buffer.at<std::uint32_t>(0x4);
    }
    if (buffer.at<std::uint32_t>(0x14) != 0) {
        std::memcpy(static_cast<char*>(buffer.ptr(0x18)) + offset,
                    static_cast<void**>(buffer.ptr(0x8))[buffer.at<std::uint32_t>(0xc)],
                    buffer.at<std::uint32_t>(0x14));
    }
    static_cast<char*>(buffer.ptr(0x18))[buffer.at<std::uint32_t>(0x14) + offset] = '\0';
    return static_cast<char*>(buffer.ptr(0x18));
}

// 0093F520 / 00940D80, __thiscall(object), RET: clear the singleton global
// (00F89B24 / 00F89B34) and store the singleton base vtable 00CE3818.
void singleton_base_destruct_0093f520(Raw object, Seg61Host& host) { singleton_base_destruct(object, 0x00f89b24, host); }
void singleton_base_destruct_00940d80(Raw object, Seg61Host& host) { singleton_base_destruct(object, 0x00f89b34, host); }

// --- the engine's own dynamic arrays: {data +0, count +4, capacity +8} ---
// Element kinds: 4-byte (0093F610/0093F670/0093F8A0), 12-byte {float, float,
// int key} (0093F590/0093F840/0093F7F0/0093F790), 16-byte (0093F900).
// The `if (slot != null)` tests the native makes on placement targets are kept.

// 0093F590, __thiscall(array, int n), RET 4: n clamped to at least 1; when
// capacity < n, a new 0Ch * n block (00BF55BE, plain multiply), copy count
// elements (two FLD/FSTP floats and a dword), _free the old block, store,
// capacity = n.
void array12_reserve_0093f590(Raw array, std::int32_t n, Seg61Host& host) {
    if (n < 1) n = 1;
    if (array.at<std::int32_t>(0x8) < n) {
        auto* const block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(n) * 0xc));
        std::uint8_t* out = block;
        for (std::int32_t i = 0; i < array.at<std::int32_t>(0x4); ++i, out += 0xc) {
            if (out != nullptr) {
                const Raw in{static_cast<std::uint8_t*>(array.ptr(0x0)) + i * 0xc};
                Raw o{out};
                o.at<float>(0x0) = in.at<float>(0x0);
                o.at<float>(0x4) = in.at<float>(0x4);
                o.at<std::uint32_t>(0x8) = in.at<std::uint32_t>(0x8);
            }
        }
        host.free_00bf6989(array.ptr(0x0));
        array.ptr(0x0) = block;
        array.at<std::int32_t>(0x8) = n;
    }
}

// 0093F610, __thiscall(array, int n), RET 4: the 4-byte version (n * 4 by LEA,
// no overflow test).
void array4_reserve_0093f610(Raw array, std::int32_t n, Seg61Host& host) {
    if (n < 1) n = 1;
    if (array.at<std::int32_t>(0x8) < n) {
        auto* const block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(n) * 4));
        std::uint32_t* out = block;
        for (std::int32_t i = 0; i < array.at<std::int32_t>(0x4); ++i, ++out) {
            if (out != nullptr) *out = static_cast<std::uint32_t*>(array.ptr(0x0))[i];
        }
        host.free_00bf6989(array.ptr(0x0));
        array.ptr(0x0) = block;
        array.at<std::int32_t>(0x8) = n;
    }
}

// 0093F670, __thiscall(array, int n), RET 4: reserve when n > capacity, zero
// the new 4-byte slots [count, n), count = n (the native first steps count
// down one by one while it exceeds n, then stores n; same result).
void array4_resize_0093f670(Raw array, std::int32_t n, Seg61Host& host) {
    if (n > array.at<std::int32_t>(0x8)) array4_reserve_0093f610(array, n, host);
    for (std::int32_t i = array.at<std::int32_t>(0x4); i < n; ++i) {
        std::uint32_t* const slot = static_cast<std::uint32_t*>(array.ptr(0x0)) + i;
        if (slot != nullptr) *slot = 0;
    }
    array.at<std::int32_t>(0x4) = n;
}

// 0093F840, __thiscall(array, int n), RET 4: the 12-byte version; new
// elements get only their key (+8) zeroed.
void array12_resize_0093f840(Raw array, std::int32_t n, Seg61Host& host) {
    if (array.at<std::int32_t>(0x8) < n) array12_reserve_0093f590(array, n, host);
    for (std::int32_t i = array.at<std::int32_t>(0x4); i < n; ++i) {
        std::uint8_t* const element = static_cast<std::uint8_t*>(array.ptr(0x0)) + i * 0xc;
        if (element != nullptr) Raw{element}.at<std::uint32_t>(0x8) = 0;
    }
    array.at<std::int32_t>(0x4) = n;
}

// 0093F7F0, __thiscall(array, const element*), RET 4: full -> reserve
// capacity * 2 (values below 2 become 1); copy the element (two FLD/FSTP
// floats and a dword) to [count]; ++count.
void array12_push_0093f7f0(Raw array, const void* element, Seg61Host& host) {
    if (array.at<std::int32_t>(0x4) == array.at<std::int32_t>(0x8)) {
        std::int32_t n = array.at<std::int32_t>(0x8) * 2;
        if (n < 2) n = 1;
        array12_reserve_0093f590(array, n, host);
    }
    std::uint8_t* const slot = static_cast<std::uint8_t*>(array.ptr(0x0)) + array.at<std::int32_t>(0x4) * 0xc;
    if (slot != nullptr) {
        const Raw in{static_cast<std::uint8_t*>(const_cast<void*>(element))};
        Raw o{slot};
        o.at<float>(0x0) = in.at<float>(0x0);
        o.at<float>(0x4) = in.at<float>(0x4);
        o.at<std::uint32_t>(0x8) = in.at<std::uint32_t>(0x8);
    }
    array.at<std::int32_t>(0x4) += 1;
}

// 0093F8A0, __thiscall(array, const dword*), RET 4: the 4-byte push.
void array4_push_0093f8a0(Raw array, const std::uint32_t* element, Seg61Host& host) {
    if (array.at<std::int32_t>(0x4) == array.at<std::int32_t>(0x8)) {
        std::int32_t n = array.at<std::int32_t>(0x8) * 2;
        if (n < 2) n = 1;
        array4_reserve_0093f610(array, n, host);
    }
    std::uint32_t* const slot = static_cast<std::uint32_t*>(array.ptr(0x0)) + array.at<std::int32_t>(0x4);
    if (slot != nullptr) *slot = *element;
    array.at<std::int32_t>(0x4) += 1;
}

// 0093F900, __thiscall(array, const element*), RET 4: the 16-byte push; full
// -> 0093F6E0 (reserve_sound_sample_parameters_0093f6e0, src/sound_sample.cpp)
// with capacity * 2 (at most 1 becomes 1); four dword copies to [count];
// ++count.
void array16_push_0093f900(Raw array, const void* element) {
    if (array.at<std::int32_t>(0x4) == array.at<std::int32_t>(0x8)) {
        std::int32_t n = array.at<std::int32_t>(0x8) * 2;
        if (n <= 1) n = 1;
        reserve_sound_sample_parameters_0093f6e0(array.p, n);
    }
    auto* const slot = reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(array.ptr(0x0)) +
                                                        array.at<std::int32_t>(0x4) * 0x10);
    if (slot != nullptr) {
        const auto* const in = static_cast<const std::uint32_t*>(element);
        slot[0] = in[0];
        slot[1] = in[1];
        slot[2] = in[2];
        slot[3] = in[3];
    }
    array.at<std::int32_t>(0x4) += 1;
}

// 0093F790, __thiscall(owner, float out[2], int key), RET 8: the array at
// owner+4; the first element whose key (+8) equals `key` gives out = its two
// floats (FLD/FSTP); none gives out = {100.0f, 100.0f} (00CE3D08). The
// decompiler drops the early return; the listing returns at 0093F7E4.
void array12_find_pair_0093f790(Raw owner, float out[2], std::int32_t key) noexcept {
    const std::int32_t count = owner.at<std::int32_t>(0x8);
    for (std::int32_t i = 0; i < count; ++i) {
        const Raw element{static_cast<std::uint8_t*>(owner.ptr(0x4)) + i * 0xc};
        if (element.at<std::int32_t>(0x8) == key) {
            out[0] = element.at<float>(0x0);
            out[1] = element.at<float>(0x4);
            return;
        }
    }
    out[0] = kDefaultPairValue;
    out[1] = kDefaultPairValue;
}

// 0093FC10 / 0093FC30, __thiscall(array), RET: resize to 0 (0093F840 /
// 0093F670), then _free the data pointer.
void vector_destroy_0093fc10(Raw vector, Seg61Host& host) {
    array12_resize_0093f840(vector, 0, host);
    host.free_00bf6989(vector.ptr(0x0));
}
void vector_destroy_0093fc30(Raw vector, Seg61Host& host) {
    array4_resize_0093f670(vector, 0, host);
    host.free_00bf6989(vector.ptr(0x0));
}

// 0093FC80, __thiscall(owner), RET: 0093F840(owner+4, 0), _free(+4 data),
// global 00F89B24 = 0, vtable 00CE3818 (the derived destructor of the
// 0093F520 singleton). SEH frame 00CA7CD8 only for unwinding.
void singleton_owner_destruct_0093fc80(Raw owner, Seg61Host& host) {
    array12_resize_0093f840(Raw{owner.p + 0x4}, 0, host);
    host.free_00bf6989(owner.ptr(0x4));
    singleton_base_destruct(owner, 0x00f89b24, host);
}

// 009318D0, __thiscall(owner), RET: zero two three-dword vectors at +4h and
// +14h (an SEH frame 00CA7558 is set up and torn down with no other effect).
void two_vectors_clear_009318d0(Raw owner) noexcept {
    owner.at<std::uint32_t>(0x4) = 0;
    owner.at<std::uint32_t>(0x8) = 0;
    owner.at<std::uint32_t>(0xc) = 0;
    owner.at<std::uint32_t>(0x14) = 0;
    owner.at<std::uint32_t>(0x18) = 0;
    owner.at<std::uint32_t>(0x1c) = 0;
}

// 0092E580, __thiscall(owner), RET: the entity at +1Ch gets 00414DB0 when
// its byte +C8h is 0, then a tail jump to 006FA6D0 with ECX = owner+2Ch's
// pointer and EDX = entity+0CCh (its world matrix).
void dyn_world_apply_entity_transform_0092e580(Raw owner, Seg61Host& host) {
    const Raw entity{static_cast<std::uint8_t*>(owner.ptr(0x1c))};
    if (entity.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_00414db0(entity);
    host.dyn_world_for_each_body_transform_006fa6d0(owner.ptr(0x2c), entity.p + 0xcc);
}

}  // namespace bsp::support_seg61_cc13
