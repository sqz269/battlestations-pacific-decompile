// Segment 74 leaves with a ledger name and no reconstruction record (cc13-sound1).
// Reconstructed and build-tested only: not ABI-compatible, not fixture- or game-validated, and not
// wired into any game host. See docs/CC13_SOUND_1.md and the address notes in bsp/sound_cc13.hpp.

#include "bsp/sound_cc13.hpp"

#include "bsp/native_input_configuration_parse.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_dialog_table.hpp"
#include "bsp/sound_sample.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstring>

namespace bsp::sound_cc13 {

static_assert(sizeof(void*) == 4, "native layouts are Win32");
static_assert(sizeof(NativePointerArray) == 12, "native pointer-array header is 0Ch bytes");
static_assert(sizeof(NativeSoundTreeNode) == 0x1C, "native sound tree node is 1Ch bytes");

namespace {

template<class T> T get(const void* p, std::size_t n) noexcept {
    T v;
    std::memcpy(&v, static_cast<const unsigned char*>(p) + n, sizeof v);
    return v;
}

// The native count arithmetic is 32-bit two's complement.
std::int32_t add32(std::int32_t n, std::int32_t k) noexcept {
    const auto bits = static_cast<std::uint32_t>(n) + static_cast<std::uint32_t>(k);
    std::int32_t r;
    std::memcpy(&r, &bits, sizeof r);
    return r;
}

std::uint32_t* allocate_slots(std::int32_t count) {
    const auto bytes = static_cast<std::size_t>(static_cast<std::uint32_t>(count) * 4u);
    return static_cast<std::uint32_t*>(
        singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes}));
}

// 00A7A850 / 00A7A8B0 shared body; only the minimum differs (CMP EDI,8 or 10h; JGE).
void reserve_with_minimum(NativePointerArray& array, std::int32_t requested, std::int32_t minimum) {
    if (requested < minimum) {
        requested = minimum;
    }
    if (array.capacity >= requested) {
        return;
    }
    std::uint32_t* const fresh = allocate_slots(requested);
    for (std::int32_t i = 0; i < array.count; i = add32(i, 1)) {
        // The listing tests the destination slot for null before each store.
        if (fresh + i != nullptr) {
            fresh[i] = array.data[i];
        }
    }
    singleton_lifetime_free(array.data);
    array.data = fresh;
    array.capacity = requested;
}

using Reserve = void (*)(NativePointerArray&, std::int32_t);

// 00A7A930 / 00A7A980 shared body.
void resize_with(NativePointerArray& array, std::int32_t count, Reserve reserve) {
    if (count > array.capacity) {
        reserve(array, count);
    }
    for (std::int32_t i = array.count; i < count; i = add32(i, 1)) {
        if (array.data + i != nullptr) {
            array.data[i] = 0;
        }
    }
    while (count < array.count) {
        array.count = add32(array.count, -1);
    }
    array.count = count;
}

void reserve_sound_system_dsps(NativePointerArray& array, std::int32_t requested) {
    reserve_sound_system_dsp_pointers_00a7a8b0(array, requested);
}
void reserve_sound_group_effects(NativePointerArray& array, std::int32_t requested) {
    reserve_sound_group_effect_pointers_00a7a850(array, requested);
}
void reserve_sound_samples(NativePointerArray& array, std::int32_t requested) {
    reserve_sound_sample_pointers_00a7a9e0(&array, requested);
}

// Growth when full: capacity + step with a floor, or doubling with a floor of 1.
std::int32_t grown_capacity_additive(std::int32_t capacity, std::int32_t step, std::int32_t floor) noexcept {
    const std::int32_t grown = add32(capacity, step);
    return grown > floor ? grown : floor;
}
std::int32_t grown_capacity_doubling(std::int32_t capacity) noexcept {
    const std::int32_t grown = add32(capacity, capacity);
    return grown > 1 ? grown : 1;
}

void store_back(NativePointerArray& array, std::uint32_t value) noexcept {
    std::uint32_t* const slot = array.data + array.count;
    if (slot != nullptr) {
        *slot = value;
    }
    array.count = add32(array.count, 1);
}

void destroy_with(NativePointerArray& array, void (*resize)(NativePointerArray&, std::int32_t)) {
    resize(array, 0);
    singleton_lifetime_free(array.data);
}

void resize_sound_samples(NativePointerArray& array, std::int32_t count) {
    resize_sound_sample_pointers_00a7b3d0(&array, count);
}
void resize_sound_system_dsps(NativePointerArray& array, std::int32_t count) {
    // 00A7A980: the 00A7A930 body with the 00A7A8B0 reserve (its own record is a fragment in
    // src/sound_configuration_shutdown.cpp).
    resize_with(array, count, &reserve_sound_system_dsps);
}

void release_object(void* object) {
    auto* const count = reinterpret_cast<volatile LONG*>(static_cast<unsigned char*>(object) + 4);
    if (InterlockedDecrement(count) == 0) {
        using Slot0 = void(__thiscall*)(void*);
        get<Slot0>(get<void*>(object, 0), 0)(object);
    }
}

void delete_owned(void** slot, std::size_t vtable_offset) {
    if (*slot != nullptr) {
        using Deleting = void*(__thiscall*)(void*, std::uint32_t);
        get<Deleting>(get<void*>(*slot, 0), vtable_offset)(*slot, 1);
        *slot = nullptr;
    }
}

constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818u; // BSP_SingletonBase_ScalarDelete table

void singleton_base_teardown(void* object, std::uint32_t& published) noexcept {
    published = 0;
    std::memcpy(object, &kSingletonBaseVtable, sizeof kSingletonBaseVtable);
}

void* singleton_base_scalar_delete(void* object, std::uint32_t flags, std::uint32_t& published) noexcept {
    singleton_base_teardown(object, published);
    if ((flags & 1u) != 0) {
        singleton_lifetime_free(object);
    }
    return object;
}

void init_tree_head(NativeSoundTree& tree) {
    tree.head = allocate_sound_tree_black_node_00a88270();
    tree.head->isnil = 1;
    tree.head->parent = tree.head;
    tree.head->left = tree.head;
    tree.head->right = tree.head;
    tree.size = 0;
}

} // namespace

void reserve_sound_group_effect_pointers_00a7a850(NativePointerArray& array, std::int32_t requested) {
    reserve_with_minimum(array, requested, 8);
}

void reserve_sound_system_dsp_pointers_00a7a8b0(NativePointerArray& array, std::int32_t requested) {
    reserve_with_minimum(array, requested, 16);
}

void resize_sound_group_effect_pointers_00a7a930(NativePointerArray& array, std::int32_t count) {
    resize_with(array, count, &reserve_sound_group_effects);
}

void append_sound_group_effect_pointer_00a7b2d0(NativePointerArray& array, const std::uint32_t& value) {
    if (array.count == array.capacity) {
        reserve_sound_group_effect_pointers_00a7a850(array, grown_capacity_additive(array.capacity, 4, 8));
    }
    store_back(array, value);
}

void append_sound_system_dsp_pointer_00a7b310(NativePointerArray& array, const std::uint32_t& value) {
    if (array.count == array.capacity) {
        reserve_sound_system_dsp_pointers_00a7a8b0(array, grown_capacity_additive(array.capacity, 16, 16));
    }
    store_back(array, value);
}

void append_sound_sample_pointer_00a7b390(NativePointerArray& array, const std::uint32_t& value) {
    if (array.count == array.capacity) {
        reserve_sound_samples(array, grown_capacity_doubling(array.capacity));
    }
    store_back(array, value);
}

void destroy_sound_group_effect_pointers_00a7b7a0(NativePointerArray& array) {
    destroy_with(array, &resize_sound_group_effect_pointers_00a7a930);
}

void destroy_sound_system_dsps_00a7b830(NativePointerArray& array) {
    destroy_with(array, &resize_sound_system_dsps);
}

void destroy_sound_sample_pointers_00a7ba80(NativePointerArray& array) {
    destroy_with(array, &resize_sound_samples);
}

void destroy_sound_sample_pointers_00a7bb20(NativePointerArray& array) {
    destroy_with(array, &resize_sound_samples);
}

void destroy_sound_sample_pointers_00a7bf20(NativePointerArray& array) {
    destroy_with(array, &resize_sound_samples);
}

NativePointerArray& assign_sound_group_effect_pointers_00a7b7c0(NativePointerArray& self, const NativePointerArray& source) {
    resize_sound_group_effect_pointers_00a7a930(self, 0);
    reserve_sound_group_effect_pointers_00a7a850(self, source.count);
    for (std::int32_t i = 0; i < source.count; i = add32(i, 1)) {
        const std::uint32_t* const element = source.data + i; // LEA EBP before the growth test
        if (self.count == self.capacity) {
            reserve_sound_group_effect_pointers_00a7a850(self, grown_capacity_additive(self.capacity, 4, 8));
        }
        store_back(self, *element);
    }
    return self;
}

NativePointerArray& assign_sound_sample_pointers_00a7baa0(NativePointerArray& self, const NativePointerArray& source) {
    resize_sound_samples(self, 0);
    reserve_sound_samples(self, source.count);
    for (std::int32_t i = 0; i < source.count; i = add32(i, 1)) {
        const std::uint32_t* const element = source.data + i;
        if (self.count == self.capacity) {
            reserve_sound_samples(self, grown_capacity_doubling(self.capacity));
        }
        store_back(self, *element);
    }
    return self;
}

void** reset_ref_slot_return_self(void** slot) {
    release_ref_slot(slot);
    return slot;
}

void release_ref_slot(void** slot) {
    if (*slot != nullptr) {
        release_object(*slot);
        *slot = nullptr;
    }
}

void** assign_ref_slot_no_addref_00a92b00(void** slot, void* object) {
    release_ref_slot(slot);
    *slot = object;
    return slot;
}

void reset_ref_slot_at_2c_00a92c00(void* owner) {
    auto** const slot = reinterpret_cast<void**>(static_cast<unsigned char*>(owner) + 0x2C);
    release_ref_slot(slot);
    *slot = nullptr; // 00A92C1D stores 0 on the null path too
}

void delete_owned_slot4_00a81c20(void** slot) { delete_owned(slot, 4); }
void delete_owned_slot4_00a81cf0(void** slot) { delete_owned(slot, 4); }
void delete_owned_slot0_00a87cb0(void** slot) { delete_owned(slot, 0); }

NativeSoundTreeNode* sound_tree_max_00a81960(NativeSoundTreeNode* node) noexcept {
    while (node->right->isnil == 0) {
        node = node->right;
    }
    return node;
}

NativeSoundTreeNode* sound_tree_min_00a81980(NativeSoundTreeNode* node) noexcept {
    while (node->left->isnil == 0) {
        node = node->left;
    }
    return node;
}

void sound_tree_rotate_left_00a822e0(NativeSoundTree& tree, NativeSoundTreeNode* where) noexcept {
    NativeSoundTreeNode* const pivot = where->right;
    where->right = pivot->left;
    if (pivot->left->isnil == 0) {
        pivot->left->parent = where;
    }
    pivot->parent = where->parent;
    if (where == tree.head->parent) {
        tree.head->parent = pivot;
    } else if (where == where->parent->left) {
        where->parent->left = pivot;
    } else {
        where->parent->right = pivot;
    }
    pivot->left = where;
    where->parent = pivot;
}

void sound_tree_rotate_right_00a819c0(NativeSoundTree& tree, NativeSoundTreeNode* where) noexcept {
    NativeSoundTreeNode* const pivot = where->left;
    where->left = pivot->right;
    if (pivot->right->isnil == 0) {
        pivot->right->parent = where;
    }
    pivot->parent = where->parent;
    if (where == tree.head->parent) {
        tree.head->parent = pivot;
    } else if (where == where->parent->right) {
        where->parent->right = pivot;
    } else {
        where->parent->left = pivot;
    }
    pivot->right = where;
    where->parent = pivot;
}

NativeSoundTreeNode* allocate_sound_tree_black_node_00a88270() {
    auto* const node = static_cast<NativeSoundTreeNode*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, sizeof(NativeSoundTreeNode), sizeof(NativeSoundTreeNode)}));
    node->left = nullptr;
    node->parent = nullptr;
    node->right = nullptr;
    node->color = 1;
    node->isnil = 0;
    return node;
}

void init_sound_name_tree_00a88370(NativeSoundTree& tree) { init_tree_head(tree); }

NativeSoundTree& construct_sound_name_tree_with_allocator_00a883d0(NativeSoundTree& tree) {
    init_tree_head(tree);
    return tree;
}

NativeSoundTree& construct_sound_name_tree_00a88620(NativeSoundTree& tree) {
    init_tree_head(tree);
    return tree;
}

void* scalar_delete_sound_event_query_lock_base_00a894b0(void* object, std::uint32_t flags,
                                                         std::uint32_t& published_00f8bbdc) {
    return singleton_base_scalar_delete(object, flags, published_00f8bbdc);
}

void* scalar_delete_shadow_job_base_00a8dd60(void* object, std::uint32_t flags, std::uint32_t& published_00e18ad4) {
    return singleton_base_scalar_delete(object, flags, published_00e18ad4);
}

void destruct_input_manager_base_00a92180(void* object, std::uint32_t& published_00f8bbf8) {
    singleton_base_teardown(object, published_00f8bbf8);
}

void* scalar_delete_input_manager_base_00a92230(void* object, std::uint32_t flags, std::uint32_t& published_00f8bbf8) {
    return singleton_base_scalar_delete(object, flags, published_00f8bbf8);
}

void __stdcall fmod_free_callback_00a87bd0(void* memory, std::uint32_t) {
    // 004254B0("- FMOD 0x%X ", memory) is an empty body (trivial_body tag); then CRT free.
    singleton_lifetime_free(memory);
}

void fill_n_dword_00a90840(std::uint32_t* first, std::uint32_t count, const std::uint32_t* value) noexcept {
    for (; count != 0; --count) {
        *first++ = *value;
    }
}

bool input_device_table_any_button_down_00a90180(const void* table) {
    for (int i = 0; i < 8; ++i) {
        void* const device = get<void*>(table, 0x44 + 4 * static_cast<std::size_t>(i));
        if (device != nullptr) {
            using Query = bool(__thiscall*)(void*);
            if (get<Query>(get<void*>(device, 0), 0x2C)(device)) {
                return true;
            }
        }
    }
    return false;
}

void input_device_clear_states_00a93ec0(void* device) noexcept {
    std::memset(static_cast<unsigned char*>(device) + 0x0C, 0, 0x100);
    std::memset(static_cast<unsigned char*>(device) + 0x10C, 0, 0x100);
}

char key_char_map_translate_00a96450(const void* map, char c, const char* null_data_00f8bc02) noexcept {
    const auto size = get<std::uint32_t>(map, 8);
    std::uint32_t i = 0;
    if (size != 0) {
        do {
            const char* from = get<const char*>(map, 0x0C); // reloaded each pass
            if (from == nullptr) {
                from = null_data_00f8bc02;
            }
            if (from[i] == c) {
                break;
            }
            ++i;
        } while (i < size);
    }
    if (i < size) {
        return get<const char*>(map, 0x14)[i];
    }
    return c;
}

void* assign_input_binding_00a931a0(void* dst, const void* src) {
    auto* const d = static_cast<unsigned char*>(dst);
    const auto* const s = static_cast<const unsigned char*>(src);
    d[0] = s[0];
    d[1] = s[1];
    std::memcpy(d + 0x04, s + 0x04, 0x14); // DWORDs +4h..+14h, in order
    assign_native_input_modifiers_00a92ee0(d + 0x18, s + 0x18);
    assign_native_input_modifiers_00a92ee0(d + 0x24, s + 0x24);
    const float scale = get<float>(s, 0x30); // FLD/FSTP
    std::memcpy(d + 0x30, &scale, sizeof scale);
    return dst;
}

void swap_remove_input_binding_00a934b0(void* header, void* const* where) {
    auto* const base = get<unsigned char*>(header, 0);
    const auto count = get<std::int32_t>(header, 4);
    unsigned char* const last = base + static_cast<std::ptrdiff_t>(add32(count, -1)) * 0x34;
    if (*where != last) {
        assign_input_binding_00a931a0(*where, last);
    }
    destroy_native_input_binding_00a93070(last);
    const std::int32_t reduced = add32(get<std::int32_t>(header, 4), -1);
    std::memcpy(static_cast<unsigned char*>(header) + 4, &reduced, sizeof reduced);
}

void destroy_input_bindings_00a938e0(void* header, const NativeInputBindingStorageContext& context) {
    resize_native_input_bindings_00a93500(header, 0, context);
    singleton_lifetime_free(get<void*>(header, 0));
}

void destroy_native_input_actions_00a93d80(void* header, NativeInputActionRecordsContext& context) {
    resize_native_input_actions_00a93c10(header, 0, context);
    singleton_lifetime_free(get<void*>(header, 0));
}

void clear_input_action_map_vectors_00a93d60(void* map, NativeInputActionRecordsContext& context) {
    resize_native_input_actions_00a93c10(static_cast<unsigned char*>(map) + 4, 0, context);
    resize_native_input_dwords_0086a430(static_cast<unsigned char*>(map) + 0x10, 0);
}

namespace {

// The shared prologue of the record copies: zero the header, then (dst != src) 0041DD40 and BF7680.
void copy_pooled_string(void* dst, const void* src, NativeStringStorage& strings) {
    std::memset(dst, 0, 8);
    if (dst == src) {
        return;
    }
    const auto length = get<std::uint32_t>(src, 0);
    resize_native_string_header_0041dd40(dst, strings, length, true);
    if (get<std::uint32_t>(src, 0) != 0) {
        // memcpy 00BF7680 with the CURRENT destination length; it tolerates overlap.
        std::memmove(get<char*>(dst, 4), get<const char*>(src, 4), get<std::uint32_t>(dst, 0));
    }
}

unsigned char* bytes(void* p, std::size_t offset = 0) noexcept { return static_cast<unsigned char*>(p) + offset; }
const unsigned char* bytes(const void* p, std::size_t offset = 0) noexcept {
    return static_cast<const unsigned char*>(p) + offset;
}

NativePointerArray& array_at(void* record, std::size_t offset) noexcept {
    return *reinterpret_cast<NativePointerArray*>(bytes(record, offset));
}
const NativePointerArray& array_at(const void* record, std::size_t offset) noexcept {
    return *reinterpret_cast<const NativePointerArray*>(bytes(record, offset));
}

using RecordCopy = void* (*)(void*, const void*, NativeStringStorage&);
using RecordDestroy = void (*)(void*, NativeStringStorage&);

// 00A7FBD0 / 00A7F160 shared body; stride and minimum differ.
void reserve_records(NativePointerArray& header, std::int32_t requested, std::int32_t minimum, std::uint32_t stride,
                     RecordCopy copy, RecordDestroy destroy, NativeStringStorage& strings) {
    if (requested < minimum) {
        requested = minimum;
    }
    if (header.capacity >= requested) {
        return;
    }
    const auto size = static_cast<std::size_t>(static_cast<std::uint32_t>(requested) * stride);
    void* const fresh = singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
    auto* const old = reinterpret_cast<unsigned char*>(header.data);
    for (std::int32_t i = 0; i < header.count; i = add32(i, 1)) {
        void* const slot = bytes(fresh, static_cast<std::size_t>(i) * stride);
        if (slot != nullptr) {
            copy(slot, old + static_cast<std::size_t>(i) * stride, strings);
        }
    }
    for (std::int32_t i = 0; i < header.count; i = add32(i, 1)) {
        destroy(old + static_cast<std::size_t>(i) * stride, strings);
    }
    singleton_lifetime_free(header.data);
    header.data = static_cast<std::uint32_t*>(fresh);
    header.capacity = requested;
}

} // namespace

void* copy_named_value_00a82ff0(void* dst, const void* name, const std::uint32_t* value, NativeStringStorage& strings) {
    copy_pooled_string(dst, name, strings);
    std::memcpy(bytes(dst, 8), value, 4);
    return dst;
}

void* copy_named_value_00a83040(void* dst, const void* src, NativeStringStorage& strings) {
    copy_pooled_string(dst, src, strings);
    std::memcpy(bytes(dst, 8), bytes(src, 8), 4);
    return dst;
}

void* copy_named_value_00a83090(void* dst, const void* src, NativeStringStorage& strings) {
    return copy_named_value_00a83040(dst, src, strings);
}

void* copy_sound_configured_type_00a7fb50(void* dst, const void* src, NativeStringStorage& strings) {
    copy_pooled_string(dst, src, strings);
    std::memset(bytes(dst, 8), 0, 12);
    try {
        assign_sound_sample_pointers_00a7baa0(array_at(dst, 8), array_at(src, 8));
    } catch (...) {
        destroy_native_string_header_0041dd20(dst, strings); // EH state 0
        throw;
    }
    return dst;
}

void* copy_sound_configured_group_00a7d880(void* dst, const void* src, NativeStringStorage& strings) {
    copy_pooled_string(dst, src, strings);
    std::memcpy(bytes(dst, 8), bytes(src, 8), 4);
    std::memset(bytes(dst, 0x0C), 0, 12);
    try {
        assign_sound_group_effect_pointers_00a7b7c0(array_at(dst, 0x0C), array_at(src, 0x0C));
    } catch (...) {
        destroy_native_string_header_0041dd20(dst, strings);
        throw;
    }
    return dst;
}

void destroy_sound_configured_type_raw_00a7c6c0(void* record, NativeStringStorage& strings) {
    NativePointerArray& samples = array_at(record, 8);
    resize_sound_samples(samples, 0);
    singleton_lifetime_free(samples.data);
    destroy_native_string_header_0041dd20(record, strings);
}

void destroy_sound_configured_group_raw_00a7bfe0(void* record, NativeStringStorage& strings) {
    NativePointerArray& effects = array_at(record, 0x0C);
    resize_sound_group_effect_pointers_00a7a930(effects, 0);
    singleton_lifetime_free(effects.data);
    destroy_native_string_header_0041dd20(record, strings);
}

void reserve_sound_configured_types_00a7fbd0(NativePointerArray& header, std::int32_t requested, NativeStringStorage& strings) {
    reserve_records(header, requested, 1, 0x14, &copy_sound_configured_type_00a7fb50,
                    &destroy_sound_configured_type_raw_00a7c6c0, strings);
}

void reserve_sound_configured_groups_00a7f160(NativePointerArray& header, std::int32_t requested, NativeStringStorage& strings) {
    reserve_records(header, requested, 16, 0x18, &copy_sound_configured_group_00a7d880,
                    &destroy_sound_configured_group_raw_00a7bfe0, strings);
}

void append_sound_configured_type_00a7feb0(NativePointerArray& header, const void* type, NativeStringStorage& strings) {
    if (header.count == header.capacity) {
        reserve_sound_configured_types_00a7fbd0(header, grown_capacity_doubling(header.capacity), strings);
    }
    void* const slot = bytes(header.data, static_cast<std::size_t>(header.count) * 0x14);
    if (slot != nullptr) {
        copy_sound_configured_type_00a7fb50(slot, type, strings);
    }
    header.count = add32(header.count, 1);
}

void* append_default_sound_configured_group_00a7faf0(NativePointerArray& header, NativeStringStorage& strings) {
    if (header.count == header.capacity) {
        reserve_sound_configured_groups_00a7f160(header, grown_capacity_additive(header.capacity, 16, 16), strings);
    }
    void* const slot = bytes(header.data, static_cast<std::size_t>(header.count) * 0x18);
    if (slot != nullptr) {
        std::memset(slot, 0, 0x18);
    }
    header.count = add32(header.count, 1);
    return bytes(header.data, static_cast<std::size_t>(header.count) * 0x18 - 0x18);
}

void pop_back_sound_configured_group_00a7f110(NativePointerArray& header, NativeStringStorage& strings) {
    if (header.count != 0) {
        destroy_sound_configured_group_raw_00a7bfe0(
            bytes(header.data, static_cast<std::size_t>(header.count) * 0x18 - 0x18), strings);
        header.count = add32(header.count, -1);
    }
}

namespace {

using RecordReserve = void (*)(NativePointerArray&, std::int32_t, NativeStringStorage&);

void resize_records(NativePointerArray& header, std::int32_t count, std::uint32_t stride, RecordReserve reserve,
                    RecordDestroy destroy, NativeStringStorage& strings) {
    if (count > header.capacity) {
        reserve(header, count, strings);
    }
    for (std::int32_t i = header.count; i < count; i = add32(i, 1)) {
        void* const slot = bytes(header.data, static_cast<std::size_t>(i) * stride);
        if (slot != nullptr) {
            std::memset(slot, 0, stride);
        }
    }
    while (count < header.count) {
        header.count = add32(header.count, -1);
        destroy(bytes(header.data, static_cast<std::size_t>(header.count) * stride), strings);
    }
    header.count = count;
}

} // namespace

void resize_sound_configured_types_raw_00a7fcc0(NativePointerArray& header, std::int32_t count, NativeStringStorage& strings) {
    resize_records(header, count, 0x14, &reserve_sound_configured_types_00a7fbd0,
                   &destroy_sound_configured_type_raw_00a7c6c0, strings);
}

void resize_sound_configured_groups_raw_00a7f240(NativePointerArray& header, std::int32_t count, NativeStringStorage& strings) {
    resize_records(header, count, 0x18, &reserve_sound_configured_groups_00a7f160,
                   &destroy_sound_configured_group_raw_00a7bfe0, strings);
}

void destroy_sound_configured_types_00a81460(NativePointerArray& header, NativeStringStorage& strings) {
    resize_sound_configured_types_raw_00a7fcc0(header, 0, strings);
    singleton_lifetime_free(header.data);
}

void destroy_sound_configured_groups_00a7ff30(NativePointerArray& header, NativeStringStorage& strings) {
    resize_sound_configured_groups_raw_00a7f240(header, 0, strings);
    singleton_lifetime_free(header.data);
}

void append_dialog_channel_record_00a86fb0(void* header, const void* record, NativeStringStorage& strings,
                                           const std::array<std::uint32_t, 4>& format_counts) {
    if (get<std::int32_t>(header, 4) == get<std::int32_t>(header, 8)) {
        reserve_sound_dialog_records_00a78dc0(header, grown_capacity_doubling(get<std::int32_t>(header, 8)), strings,
                                              format_counts);
    }
    void* const slot = get<unsigned char*>(header, 0) + static_cast<std::size_t>(get<std::int32_t>(header, 4)) * 0x14;
    if (slot != nullptr) {
        copy_sound_dialog_record_00a77ef0(slot, record, strings, format_counts);
    }
    const std::int32_t count = add32(get<std::int32_t>(header, 4), 1);
    std::memcpy(bytes(header, 4), &count, sizeof count);
}

void* copy_construct_input_binding_vector_00a93900(void* dst, const void* src,
                                                   const NativeInputBindingStorageContext& context) {
    std::memset(dst, 0, 12);
    assign_native_input_bindings_00a937e0(dst, src, context);
    return dst;
}

void erase_sound_name_subtree_00a88400(NativeSoundTree& tree, NativeSoundTreeNode* node, NativeStringStorage& strings) {
    while (node->isnil == 0) {
        erase_sound_name_subtree_00a88400(tree, node->right, strings);
        NativeSoundTreeNode* const left = node->left;
        destroy_native_string_header_0041dd20(node->value, strings); // length +0Ch, data +10h
        singleton_lifetime_free(node);
        node = left;
    }
}

void clear_sound_name_tree_00a88460(NativeSoundTree& tree, NativeStringStorage& strings) {
    erase_sound_name_subtree_00a88400(tree, tree.head->parent, strings);
    tree.head->parent = tree.head;
    tree.size = 0;
    tree.head->left = tree.head;
    tree.head->right = tree.head;
}

void sound_instance_default_set_parameter_00a7be70(void*, std::uint32_t, std::uint32_t) noexcept {}
void sound_instance_default_slot34_00a7be80(void*, const std::uint32_t*) noexcept {}
void sound_event_refresh_ended_noop_00a89c50() noexcept {}
void shadow_target_extents_vslot1_noop_00a8a9b0(void*, std::uint32_t) noexcept {}
void shadow_target_extents_vslot2_noop_00a8a9c0(void*, std::uint32_t) noexcept {}

bool input_backend_default_slot08_00a90ec0(void*) noexcept { return false; }
bool text_listener_base_slot00_00a97220(void*, std::uint32_t, std::uint32_t) noexcept { return false; }

std::uint8_t native_input_backend_byte_f4_00a97b80(const void* backend) noexcept {
    return get<std::uint8_t>(backend, 0xF4);
}

std::uint32_t input_manager_context_level_00a92290(const void* manager, std::int32_t context) noexcept {
    const auto* const levels = get<const std::uint32_t*>(manager, 0x10);
    return levels[context];
}

bool sound_sample_has_resource_word08_00a818d0(const void* sample) noexcept {
    return get<std::uint32_t>(get<const void*>(sample, 0x78), 8) != 0;
}

} // namespace bsp::sound_cc13
