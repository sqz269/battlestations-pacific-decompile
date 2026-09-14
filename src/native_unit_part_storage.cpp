#include "bsp/native_unit_part_storage.hpp"
#include "bsp/native_unit_part_groups.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(address(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address(p) + offset) = value;
}
void* buy_sentinel() {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x30, 0x30});
    const Word next = address(node);
    if (next) write(node, 0, next);
    const Word previous = next + 4u;
    if (previous) write(pointer(previous), 0, next);
    return node;
}
void destroy_list(void* list) noexcept {
    void* head = pointer(read(list, 4));
    Word node = read(head);
    write(head, 0, address(head));
    head = pointer(read(list, 4));
    write(head, 4, address(head));
    bool finished = node == read(list, 4);
    write(list, 8, 0);
    while (!finished) {
        const Word next = read(pointer(node));
        singleton_lifetime_free(pointer(node));
        finished = next == read(list, 4);
        node = next;
    }
    singleton_lifetime_free(pointer(read(list, 4)));
    write(list, 4, 0);
}
} // namespace

void initialize_native_collision_node_fields_004e6480(void* node,
    const volatile Word& minimum_seed, const volatile Word& maximum_seed) {
    const Word minimum = minimum_seed;
    write(node, 0x4c, 0);
    write(node, 0x10c, minimum); write(node, 0x110, minimum); write(node, 0x114, minimum);
    const Word maximum = maximum_seed;
    write(node, 0x118, maximum); write(node, 0x11c, maximum); write(node, 0x120, maximum);
    write(node, 0x154, 0xffffffffu);
    write(node, 0x108, 0);
    *reinterpret_cast<volatile unsigned char*>(address(node) + 0x158) = 0;
    write(node, 0xf8, 0); write(node, 0x100, 0); write(node, 0x104, 2);
    void* const children = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, 8, 8});
    write(node, 0xfc, address(children));
    write(node, 4, 0xffffffffu);
    write(node, 0x10, 0); write(node, 0x0c, 0);
    write(node, 0x1c, 0); write(node, 0x18, 0);
    write(node, 0x28, 0); write(node, 0x24, 0);
    write(node, 0x34, 0); write(node, 0x30, 0);
    write(node, 0x14, address(node)); write(node, 0x20, address(node));
    write(node, 0x2c, address(node)); write(node, 0x38, address(node));
    write(node, 0x40, 0); write(node, 0x48, 0); write(node, 0x44, 0);
}
void destroy_native_collision_node_base_004e6570(void* node) noexcept {
    write(node, 0, 0x00ce89e8);
    const Word children = read(node, 0xfc);
    if (children) singleton_lifetime_free(pointer(children));
}
void* buy_native_unit_part_list_sentinel_007103a0() { return buy_sentinel(); }
void* buy_native_unit_part_shape_sentinel_007103c0() { return buy_sentinel(); }
void destroy_native_unit_part_list_00710870(void* list) noexcept { destroy_list(list); }
void destroy_native_unit_part_shape_list_007108e0(void* list) noexcept { destroy_list(list); }
void destroy_native_unit_part_group_rows_00712b40(void* header) noexcept {
    const Word first = read(header, 4);
    if (first) {
        destroy_native_part_group_row_range_00711f70(pointer(first), pointer(read(header, 8)));
        singleton_lifetime_free(pointer(read(header, 4)));
    }
    write(header, 4, 0); write(header, 8, 0); write(header, 12, 0);
}
NativeUnitPartStorageBindings::NativeUnitPartStorageBindings(
    NativeUnitPartCollisionGlobals globals, NativeUnitPartCollisionCallbacks callbacks) noexcept
    : globals_(globals), callbacks_(callbacks) {}
void NativeUnitPartStorageBindings::call_004e6480(void* model) {
    initialize_native_collision_node_fields_004e6480(model,
        globals_.minimum_seed_00d7a248, globals_.maximum_seed_00d7a244);
}
void NativeUnitPartStorageBindings::call_00712440(void* model) {
    (void)build_native_unit_part_collision_00712440(model, globals_, callbacks_);
}
} // namespace bsp
