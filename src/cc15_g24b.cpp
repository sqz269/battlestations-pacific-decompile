// Game rows from the cc15-stlA audit (packet cc15-g24b). Evidence and ABI:
// include/bsp/cc15_g24b.hpp and docs/CC15_G24B.md.
#include "bsp/cc15_g24b.hpp"

namespace bsp::cc15_g24b {
namespace {

Address address_of(const void* pointer) {
    return reinterpret_cast<Address>(pointer);
}

// The manager lock shared by 0048CA60, 0048CB30 and 0048CBB0.
void lock_manager(Address manager, Manager21D0TreeHost& host) {
    const Address section = host.read_u32(manager + 0x4);
    host.enter_critical_section_00ce2218(section);
    host.write_u32(section + 0x18, host.read_u32(section + 0x18) + 1u);
}

void unlock_manager(Address manager, Manager21D0TreeHost& host) {
    const Address section = host.read_u32(manager + 0x4);  // re-read
    host.write_u32(section + 0x18, host.read_u32(section + 0x18) - 1u);
    host.leave_critical_section_00ce2210(section);
}

// The find-then-compare-with-end() prefix: 00488630, then the end() head read
// and the container-match assert. Returns the iterator; *end receives the head.
TreeIterator find_entry(Address manager, const std::uint32_t* key, Address* end,
                        Manager21D0TreeHost& host) {
    const Address tree = manager + 0x28;
    TreeIterator found{};
    host.tree_find_00488630(tree, &found, key);
    *end = host.read_u32(tree + 0x4);
    if (found.container == 0 || found.container != tree) host.invalid_parameter_00bf6713();
    return found;
}

// The checked dereference: null container, then node == head.
void check_dereference(const TreeIterator& it, MemoryHost& host) {
    if (it.container == 0) host.invalid_parameter_00bf6713();
    if (it.node == host.read_u32(it.container + 0x4)) host.invalid_parameter_00bf6713();
}

// One tree assignment of 006AA1C0/006AA2D0 (006AA22F-006AA24B and the two
// copies after it): skipped when the trees are the same object.
template <typename Erase, typename Copy>
void assign_tree(Address destination, Address source, InputSettingsCopyHost& host, Erase erase,
                 Copy copy) {
    if (destination == source) return;
    const Address head = host.read_u32(destination + 0x4);
    const Address first = host.read_u32(head);
    TreeIterator erased{};
    (host.*erase)(destination, &erased, TreeIterator{destination, first},
                  TreeIterator{destination, head});
    (host.*copy)(destination, source);
}

// 006AA1C0 and 006AA2D0 share this body; to_shadow picks the direction.
void copy_device_maps(Address settings, bool to_shadow, InputSettingsCopyHost& host) {
    const Address map = settings + 0x8;
    TreeIterator it{map, host.read_u32(host.read_u32(settings + 0xC))};  // begin()
    for (;;) {
        const Address head = host.read_u32(settings + 0xC);              // end(), re-read
        if (it.container == 0 || it.container != map) host.invalid_parameter_00bf6713();
        if (it.node == head) return;
        if (it.container == 0) host.invalid_parameter_00bf6713();
        if (it.node == host.read_u32(it.container + 0x4)) {             // two asserts
            host.invalid_parameter_00bf6713();
            if (it.node == host.read_u32(it.container + 0x4)) host.invalid_parameter_00bf6713();
        }
        const Address node = it.node;
        const Address value = host.input_device_map_index_0055c110(settings + 0x6C, node + 0xC);
        struct Pair { Address shadow, primary; };
        const Pair trees[3] = {{value + 0x0C, node + 0x20},
                               {value + 0x24, node + 0x38},
                               {value + 0x6C, node + 0x80}};
        for (int index = 0; index < 3; ++index) {
            const Address destination = to_shadow ? trees[index].shadow : trees[index].primary;
            const Address source = to_shadow ? trees[index].primary : trees[index].shadow;
            if (index == 0) {
                assign_tree(destination, source, host, &InputSettingsCopyHost::binding_tree_erase_005599c0,
                            &InputSettingsCopyHost::binding_tree_copy_0055a160);
            } else if (index == 1) {
                assign_tree(destination, source, host, &InputSettingsCopyHost::scalar_tree_erase_00444b10,
                            &InputSettingsCopyHost::scalar_tree_copy_00499680);
            } else {
                assign_tree(destination, source, host,
                            &InputSettingsCopyHost::bit_vector_tree_erase_00559280,
                            &InputSettingsCopyHost::bit_vector_tree_copy_0055a890);
            }
        }
        host.device_iterator_next_005540c0(&it);
    }
}

} // namespace

// 004881F0-00488284. EDI holds the iterator's container ([manager+8]+0Ch,
// read once); the end() side re-reads [manager+8] on every pass.
void manager21d0_delete_kind1_and_2_objects_004881f0(void* manager_pointer,
                                                     DeleteKindObjectsHost& host) {
    const Address manager = address_of(manager_pointer);
    const Address sub = host.read_u32(manager + 0x8);
    const Address container = sub + 0xC;
    Address next = host.read_u32(host.read_u32(sub + 0x10));             // begin()
    for (;;) {
        const Address end_sub = host.read_u32(manager + 0x8);             // 00488205
        const Address head = host.read_u32(end_sub + 0x10);
        if (container == 0 || container != end_sub + 0xC) host.invalid_parameter_00bf6713();
        if (next == head) return;
        const Address node = next;
        // Post-increment: checks, then node->next (0048821F-00488236).
        if (container == 0) host.invalid_parameter_00bf6713();
        if (node == host.read_u32(container + 0x4)) host.invalid_parameter_00bf6713();
        // Dereference for the kind test (00488234-00488249); MOV EBX,[EBX] sits
        // between the null test and its assert.
        next = host.read_u32(node);
        if (container == 0) host.invalid_parameter_00bf6713();
        if (node == host.read_u32(container + 0x4)) host.invalid_parameter_00bf6713();
        bool remove = host.read_u32(host.read_u32(node + 0x8) + 0x4) == 2u;
        if (!remove) {
            if (node == host.read_u32(container + 0x4)) host.invalid_parameter_00bf6713();
            remove = host.read_u32(host.read_u32(node + 0x8) + 0x4) == 1u;
        }
        if (!remove) continue;
        if (node == host.read_u32(container + 0x4)) host.invalid_parameter_00bf6713();  // 00488265
        const Address object = host.read_u32(node + 0x8);
        if (object != 0) host.object_scalar_deleting_destructor(object, 1u);  // 0048827C
    }
}

// 0048CA60-0048CB29. The derived key overwrites the record argument slot and
// 00488630 receives that slot's address.
void manager21d0_count_entry_event_0048ca60(void* manager_pointer, void* record_pointer,
                                            std::int32_t flag, Manager21D0TreeHost& host) {
    const Address manager = address_of(manager_pointer);
    const Address record = address_of(record_pointer);
    lock_manager(manager, host);
    Address unit = host.read_u32(record);
    if (unit == 0) unit = host.read_u32(record + 0x8);
    const std::uint32_t key = unit != 0 ? static_cast<std::uint32_t>(unit + 0x1E4) : 0u;
    Address end = 0;
    const TreeIterator found = find_entry(manager, &key, &end, host);
    if (found.node != end) {
        check_dereference(found, host);
        const Address entry = host.read_u32(found.node + 0x10);
        const Address counters = host.read_u32(entry + 0xD8);
        const Address counter = counters + ((flag & 0xFF) != 0 ? 0x10 : 0x14);  // CMP byte
        host.write_u32(counter, host.read_u32(counter) + 1u);
    }
    unlock_manager(manager, host);
}

// 0048CB30-0048CBAC.
void manager21d0_clear_entry_active_0048cb30(void* manager_pointer, void* key_pointer,
                                             Manager21D0TreeHost& host) {
    const Address manager = address_of(manager_pointer);
    const std::uint32_t key = static_cast<std::uint32_t>(address_of(key_pointer));
    lock_manager(manager, host);
    Address end = 0;
    const TreeIterator found = find_entry(manager, &key, &end, host);
    if (found.node != end) {
        check_dereference(found, host);
        host.write_u32(host.read_u32(found.node + 0x10) + 0xDC, 0u);
    }
    unlock_manager(manager, host);
}

// 0048CBB0-0048CC4E. The result byte starts at 0 ([ESP+13h], 0048CBD6). The
// second dereference check (0048CC1B) omits the null-container test.
bool manager21d0_is_entry_active_and_set_0048cbb0(void* manager_pointer, void* key_pointer,
                                                  Manager21D0TreeHost& host) {
    const Address manager = address_of(manager_pointer);
    const std::uint32_t key = static_cast<std::uint32_t>(address_of(key_pointer));
    lock_manager(manager, host);
    bool result = false;
    Address end = 0;
    const TreeIterator found = find_entry(manager, &key, &end, host);
    if (found.node != end) {
        check_dereference(found, host);
        if (host.read_u32(host.read_u32(found.node + 0x10) + 0xDC) != 0) {
            if (found.node == host.read_u32(found.container + 0x4)) host.invalid_parameter_00bf6713();
            result = host.read_u32(host.read_u32(found.node + 0x10) + 0xBC) != 0;
        }
    }
    unlock_manager(manager, host);
    return result;
}

// 00687860-006878C2. The vector begin/end are re-read on every pass.
bool menu_manager_has_screen_00687860(void* manager_pointer, std::int32_t id, std::int32_t flag,
                                      MemoryHost& host) {
    const Address manager = address_of(manager_pointer);
    const std::uint32_t wanted = static_cast<std::uint32_t>(id);
    if ((flag & 0xFF) == 0 && host.read_u32(manager + 0x20) == wanted) return true;
    for (std::uint32_t index = 0;; ++index) {
        const Address begin = host.read_u32(manager + 0x5C);
        if (begin == 0) return false;
        const auto size = [&](Address first) {
            return static_cast<std::uint32_t>(
                static_cast<std::int32_t>(host.read_u32(manager + 0x60) - first) >> 2);  // SAR
        };
        if (index >= size(begin)) return false;
        // 00687893: the same begin register, end re-read.
        if (begin == 0 || index >= size(begin)) host.invalid_parameter_00bf6713();
        if (host.read_u32(host.read_u32(manager + 0x5C) + index * 4u) == wanted) return true;
    }
}

// 006A9EA0-006A9F61. The iterator's container is cached in the argument slot
// ([ESP+1Ch]) and the end() head is re-read from it on every pass.
bool input_settings_device_has_binding_kind2_006a9ea0(void* settings, const void* key,
                                                      DeviceBindingKindHost& host) {
    const Address value = host.input_device_map_index_0055c110(address_of(settings) + 0x8,
                                                               address_of(key));
    const Address tree = value + 0xC;
    TreeIterator it{tree, host.read_u32(host.read_u32(value + 0x10))};   // begin()
    for (;;) {
        const Address head = host.read_u32(tree + 0x4);
        if (it.container == 0 || it.container != tree) host.invalid_parameter_00bf6713();
        if (it.node == head) return false;
        for (std::uint32_t index = 0, offset = 0; offset < 0x28; offset += 0x14, ++index) {
            check_dereference(it, host);
            const Address begin = host.read_u32(it.node + 0x18);
            bool in_range = false;
            if (begin != 0) {
                // IMUL 66666667h / SAR 3 / sign fix: signed byte span / 14h.
                const std::int32_t span =
                    static_cast<std::int32_t>(host.read_u32(it.node + 0x1C) - begin);
                in_range = index < static_cast<std::uint32_t>(span / 0x14);
            }
            if (!in_range) host.invalid_parameter_00bf6713();
            if (host.read_u32(host.read_u32(it.node + 0x18) + offset) == 2u) return true;
        }
        host.tree_iterator_increment_00552db0(&it);                       // 006A9F39
    }
}

// 006AA1C0-006AA2C2: +8h node trees -> +6Ch value trees.
void input_settings_copy_primary_to_shadow_006aa1c0(void* settings, InputSettingsCopyHost& host) {
    copy_device_maps(address_of(settings), true, host);
}

// 006AA2D0-006AA3D2: +6Ch value trees -> +8h node trees.
void input_settings_copy_shadow_to_primary_006aa2d0(void* settings, InputSettingsCopyHost& host) {
    copy_device_maps(address_of(settings), false, host);
}

} // namespace bsp::cc15_g24b
