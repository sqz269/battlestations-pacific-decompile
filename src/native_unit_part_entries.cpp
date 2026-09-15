#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "bsp/native_unit_part_entries.hpp"
#include "bsp/native_unit_part_groups.hpp"
#include "bsp/native_node_construction.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Sbo = NativeLegacySboStringStorage;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(address(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address(p) + offset) = value;
}
void initialize_empty(Sbo& s) noexcept {
    write(&s, 0x18, 15); write(&s, 0x14, 0);
    *reinterpret_cast<volatile char*>(address(&s) + 4) = 0;
}
const char* data(const Sbo& s) noexcept {
    return static_cast<const char*>(pointer(read(&s, 0x18) < 16 ? address(&s) + 4 : read(&s, 4)));
}
void release_temporary(Sbo& s) noexcept {
    if (read(&s, 0x18) >= 16) singleton_lifetime_free(pointer(read(&s, 4)));
}
struct NameGuard {
    Sbo& value;
    bool active = true;
    ~NameGuard() noexcept { if (active) native_legacy_sbo_string_destroy_004072d0(value); }
};
} // namespace

void build_native_unit_part_entries_00711c60(void* part) {
    const Word selected = read(part, 0x160);
    if (!selected) return;
    void* const source_header = pointer(selected + 0x7c);
    Word cursor = read(source_header, 4);
    if (cursor > read(source_header, 8)) _invalid_parameter_noinfo();
    for (;;) {
        const Word current_set = read(part, 0x160);
        const Word end = read(pointer(current_set), 0x84);
        if (read(pointer(current_set), 0x80) > end) _invalid_parameter_noinfo();
        if (address(source_header) != current_set + 0x7c) _invalid_parameter_noinfo();
        if (cursor == end) return;
        Sbo name;
        initialize_empty(name);
        NameGuard name_guard{name};
        if (cursor >= read(source_header, 8)) _invalid_parameter_noinfo();
        // Native CMP reads the source end before capturing record4, then calls
        // validation if that saved comparison failed. Keep the captured node.
        const bool in_range = cursor < read(source_header, 8);
        void* const node = pointer(read(pointer(cursor), 4));
        if (!in_range) _invalid_parameter_noinfo();
        {
            Sbo copied;
            copy_native_part_record_name_00711c30(pointer(read(pointer(cursor))), copied);
            NameGuard copied_guard{copied};
            native_legacy_sbo_string_assign_substring_00408120(name, copied, 0, 0xffffffffu);
            copied_guard.active = false;
            release_temporary(copied);
        }
        // 70F900's actual invocation searches the one-byte set "_" from0.
        // Use the SDK byte search, retaining embedded NUL/high bytes and npos.
        const Word length = read(&name, 0x14);
        const char* const begin = length ? data(name) : nullptr;
        const void* const found = length ? std::memchr(begin, '_', length) : nullptr;
        const Word underscore = found ? address(found) - address(begin) : 0xffffffffu;
        Sbo prefix;
        construct_native_part_substring_004cdbe0(name, prefix, 0, underscore);
        NameGuard prefix_guard{prefix};
        if (underscore == 4 && std::memcmp(data(prefix), "part", 4) == 0) {
            void* const record = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x10, 0x10});
            if (record) {
                write(record, 4, 0); write(record, 8, 0); write(record, 12, 0);
                write(record, 0, address(node));
            }
            auto& entries = *static_cast<NativeRenderPointerArrayStorage*>(pointer(address(part) + 0x1a0));
            // The raw record is deliberately not guarded across this reserve:
            // native publishes it only after reserve succeeds and has no unwind
            // action for an unpublished record. Existing array semantics apply.
            append_native_instance_entry_pointer_00b1cb80(entries, &record);
        }
        prefix_guard.active = false;
        release_temporary(prefix);
        initialize_empty(prefix);
        name_guard.active = false;
        release_temporary(name);
        if (cursor >= read(source_header, 8)) _invalid_parameter_noinfo();
        cursor += 8;
    }
}

void assign_native_unit_part_node_name_00b6f960(NativeNodeStorage& node,
    const NativeString& name, NativeStringRawPoolContext& strings) {
    void* const destination = &node.name_54;
    const void* const source = &name;
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, read(source), true);
    if (read(source) != 0) {
        const Word count = read(destination);
        const void* const source_bytes = pointer(read(source, 4));
        void* const destination_bytes = pointer(read(destination, 4));
        if (count) std::memmove(destination_bytes, source_bytes, count);
    }
}

void release_native_unit_part_selected_set_00711080(void* cell,
    NativeUnitPartSelectedSetCallbacks callbacks) {
    const Word owner = read(cell);
    if (!owner) return;
    auto* references = reinterpret_cast<volatile LONG*>(owner + 4);
    if (InterlockedDecrement(references) == 0) {
        const Word table = read(pointer(owner));
        const Word entry = callbacks.read_slot_zero
            ? callbacks.read_slot_zero(callbacks.context, table) : read(pointer(table));
        callbacks.dispatch_zero(callbacks.context, entry, pointer(owner));
    }
    write(cell, 0, 0);
}
} // namespace bsp
