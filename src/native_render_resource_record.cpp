#include "bsp/native_render_resource_record.hpp"

#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer resource records require MSVC Win32 pointer widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceAliasNode) == 0x10);
static_assert(offsetof(NativeRenderResourceAliasNode, next_00) == 0x00);
static_assert(offsetof(NativeRenderResourceAliasNode, previous_04) == 0x04);
static_assert(offsetof(NativeRenderResourceAliasNode, string_length_08) == 0x08);
static_assert(offsetof(NativeRenderResourceAliasNode, string_data_0c) == 0x0c);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
static_assert(offsetof(NativeRenderResourceRecord, name_length_00) == 0x00);
static_assert(offsetof(NativeRenderResourceRecord, name_data_04) == 0x04);
static_assert(offsetof(NativeRenderResourceRecord, unknown_08) == 0x08);
static_assert(offsetof(NativeRenderResourceRecord, sentinel_0c) == 0x0c);
static_assert(offsetof(NativeRenderResourceRecord, alias_count_10) == 0x10);
static_assert(offsetof(NativeRenderResourceRecord, payload_14_24) == 0x14);
static_assert(offsetof(NativeRenderResourceRecord, resource_28) == 0x28);

namespace {
// FuncInfo 00DF6100 state 0 -> 00CBD840 -> 0041DD20. Unlike
// NativeString::release_to, native destruction retains the raw name fields.
struct RecordNameUnwind {
    NativeRenderResourceRecord& record;
    SizedStoragePool& pool;
    bool active = true;

    ~RecordNameUnwind() noexcept {
        if (active) {
            auto* const data = record.name_data_04;
            if (data) {
                pool.release_00bd1510(data, record.name_length_00 + 1u);
            }
        }
    }
};
}

void destroy_native_render_resource_record_00b2f990(
    NativeRenderResourceRecord& record, SizedStoragePool& actual_string_pool) {
    RecordNameUnwind unwind{record, actual_string_pool};

    // 004D05E0 receives the embedded list at record+8. Capture its first
    // node before resetting the real sentinel and count, then retain next
    // across each pool release/free. Compare with the current sentinel after
    // those calls; a cached end pointer would change allocator reentry behavior.
    auto* cursor = record.sentinel_0c->next_00;
    record.sentinel_0c->next_00 = record.sentinel_0c;
    record.sentinel_0c->previous_04 = record.sentinel_0c;
    record.alias_count_10 = 0;
    while (cursor != record.sentinel_0c) {
        auto* const data = cursor->string_data_0c;
        auto* const next = cursor->next_00;
        if (data) {
            actual_string_pool.release_00bd1510(data, cursor->string_length_08 + 1u);
        }
        singleton_lifetime_free(cursor);
        cursor = next;
    }

    // 00B2F9C0..00B2F9D0: reload sentinel, free it, then clear that field.
    singleton_lifetime_free(record.sentinel_0c);
    record.sentinel_0c = nullptr;
    auto* const name = record.name_data_04;
    unwind.active = false; // 00B2F9D8: state -1, before normal name release.
    if (name) {
        actual_string_pool.release_00bd1510(name, record.name_length_00 + 1u);
    }
}

}
