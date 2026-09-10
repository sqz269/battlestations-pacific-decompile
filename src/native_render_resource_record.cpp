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
template<class T>
volatile T& list_field(void* actual_owner, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(
        static_cast<unsigned char*>(actual_owner) + offset);
}

NativeRenderResourceAliasNode* sentinel(void* actual_owner) {
    return list_field<NativeRenderResourceAliasNode*>(actual_owner, 4);
}

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

void clear_native_render_resource_aliases_004d05e0(
    void* actual_list_owner, SizedStoragePool& actual_string_pool) {
    // Volatile accesses preserve reloads even when actual owner/sentinel
    // storage overlaps. The initial equality test precedes the count store.
    auto* first_sentinel = sentinel(actual_list_owner);
    volatile auto* node = first_sentinel;
    auto* cursor = node->next_00;
    node->next_00 = first_sentinel;
    auto* second_sentinel = sentinel(actual_list_owner);
    node = second_sentinel;
    node->previous_04 = second_sentinel;
    const bool initially_empty = cursor == sentinel(actual_list_owner);
    list_field<std::uint32_t>(actual_list_owner, 8) = 0;
    if (initially_empty) {
        return;
    }
    do {
        node = cursor;
        auto* const data = node->string_data_0c;
        auto* const next = node->next_00;
        if (data) {
            actual_string_pool.release_00bd1510(data, node->string_length_08 + 1u);
        }
        singleton_lifetime_free(cursor);
        cursor = next;
    } while (cursor != sentinel(actual_list_owner));
}

void destroy_native_render_resource_record_00b2f990(
    NativeRenderResourceRecord& record, SizedStoragePool& actual_string_pool) {
    RecordNameUnwind unwind{record, actual_string_pool};
    clear_native_render_resource_aliases_004d05e0(
        reinterpret_cast<unsigned char*>(&record) + 8, actual_string_pool);

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
