#include "bsp/native_scene_hidden_record_lookup.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_record_map_lookup.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
const char* text(Word value) noexcept { return reinterpret_cast<const char*>(value); }
Word read(Word base, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(base + offset);
}

void release_captured(Word data, Word bytes, NativeStringRawPoolContext& pool) {
    auto* const owner = native_string_pool_get_or_create_00419cc0(
        pool.actual_published_01090aa8, pool.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(owner, pointer(data), bytes,
        pool.actual_small_returns_disabled_01090aa4);
}
} // namespace

std::uint32_t find_native_scene_hidden_record_004691b0(
    void* actual_scene_database, const NativeSceneHiddenRecordLookupFrame& frame,
    NativeStringRawPoolContext& pool) {
    const Word name = read(address(frame.actual_parameter4));
    const Word database = address(actual_scene_database);
    void* const header = frame.actual_key_header8;
    construct_native_string_header_0041e870(header, pool, text(name));
    const Word map = database + 0x18u;
    void* const output = frame.actual_iterator8;
    (void)find_native_scene_record_map_00468cd0(pointer(map), 0, output, header);

    const Word data = read(address(header), 4);
    if (data != 0) {
        const Word bytes = read(address(header)) + 1u;
        release_captured(data, bytes, pool);
    }

    const Word owner = read(address(output));
    const Word head = read(map, 4);
    if (owner == 0 || owner != map) invoke_native_invalid_parameter_00bf6713();
    const Word node = read(address(output), 4);
    if (node == head) return 0;
    if (owner == 0) invoke_native_invalid_parameter_00bf6713();
    if (node == read(owner, 4)) invoke_native_invalid_parameter_00bf6713();
    return read(node, 0x14);
}

std::uint8_t has_native_scene_hidden_record_00469480(
    void* actual_scene_database, const NativeSceneHiddenRecordLookupFrame& frame,
    NativeStringRawPoolContext& pool) {
    const Word name = read(address(frame.actual_parameter4));
    const Word database = address(actual_scene_database);
    void* const header = frame.actual_key_header8;
    construct_native_string_header_0041e870(header, pool, text(name));
    const Word head = read(database, 0x1c);
    const Word map = database + 0x18u;
    void* const result = find_native_scene_record_map_00468cd0(
        pointer(map), 0, frame.actual_iterator8, header);
    const Word owner = read(address(result));
    if (owner == 0 || owner != map) invoke_native_invalid_parameter_00bf6713();
    const bool different = read(address(result), 4) != head;
    const Word data = read(address(header), 4);
    const std::uint8_t present = different ? 1u : 0u;
    if (data != 0) {
        const Word bytes = read(address(header)) + 1u;
        release_captured(data, bytes, pool);
    }
    return present;
}

} // namespace bsp
