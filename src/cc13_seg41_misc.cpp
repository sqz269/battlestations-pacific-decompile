// Segment 41 containers, reference pointers and singleton lifetime helpers
// (cc13-seg41). Evidence and ABI: include/bsp/cc13_seg41.hpp, docs/CC13_SEG41.md.
#include "bsp/cc13_seg41.hpp"

#include "bsp/native_locale_map_node_recycling.hpp"

namespace bsp::cc13_seg41 {
namespace {

// The shared template body of the four reserve instances (listing 00735F90).
void reserve_body(DwordVector& v, std::int32_t n, VectorMemory& memory) {
    if (n < 1) n = 1;                                        // 00735F96 CMP EDI,1 / JGE
    if (v.capacity_08 < n) {                                 // 00735FA2
        auto* fresh = static_cast<std::uint32_t*>(memory.operator_new_00bf55be(
            static_cast<std::size_t>(n) * 4u));               // 00735FB0
        std::uint32_t* out = fresh;
        for (std::int32_t i = 0; i < v.size_04; ++i, ++out) {
            if (out != nullptr) *out = v.data_00[i];
        }
        memory.free_00bf6989(v.data_00);                     // 00735FDC
        v.data_00 = fresh;
        v.capacity_08 = n;
    }
}

template <class Reserve>
void resize_body(DwordVector& v, std::int32_t n, Reserve&& reserve) {
    if (v.capacity_08 < n) reserve(n);
    for (std::int32_t i = v.size_04; i < n; ++i) {
        std::uint32_t* slot = v.data_00 + i;
        if (slot != nullptr) *slot = 0;
    }
    // The element destructor is trivial; the image still counts size down.
    while (n < v.size_04) --v.size_04;
    v.size_04 = n;
}

template <class Reserve>
void push_back_body(DwordVector& v, const std::uint32_t* value, Reserve&& reserve) {
    if (v.size_04 == v.capacity_08) {
        std::int32_t grown = v.capacity_08 * 2;
        if (grown < 2) grown = 1;
        reserve(grown);
    }
    std::uint32_t* slot = v.data_00 + v.size_04;
    if (slot != nullptr) *slot = *value;
    ++v.size_04;
}

RefCounted** release_ref_body(RefCounted** slot, RefCountedHost& host) {
    RefCounted* object = *slot;
    if (object != nullptr) {
        if (host.interlocked_decrement(&object->refs_04) == 0) host.release_slot0(object);
        *slot = nullptr;
    }
    return slot;
}

}  // namespace

void reserve_00735f90(DwordVector& v, std::int32_t n, VectorMemory& memory) { reserve_body(v, n, memory); }
void reserve_0073e8a0(DwordVector& v, std::int32_t n, VectorMemory& memory) { reserve_body(v, n, memory); }
void reserve_0073e940(DwordVector& v, std::int32_t n, VectorMemory& memory) { reserve_body(v, n, memory); }
void reserve_0073eac0(DwordVector& v, std::int32_t n, VectorMemory& memory) { reserve_body(v, n, memory); }

void resize_00737340(DwordVector& v, std::int32_t n, VectorMemory& memory) {
    resize_body(v, n, [&](std::int32_t k) { reserve_00735f90(v, k, memory); });
}
void resize_0073ea40(DwordVector& v, std::int32_t n, VectorMemory& memory) {
    resize_body(v, n, [&](std::int32_t k) { reserve_0073e940(v, k, memory); });
}
void resize_0073fc70(DwordVector& v, std::int32_t n, VectorMemory& memory) {
    resize_body(v, n, [&](std::int32_t k) { reserve_0073e8a0(v, k, memory); });
}
void resize_0073fda0(DwordVector& v, std::int32_t n, VectorMemory& memory) {
    resize_body(v, n, [&](std::int32_t k) { reserve_0073eac0(v, k, memory); });
}
void resize_0073fd00(DwordVector& v, std::int32_t n, DecalRecordReserve& reserve) {
    resize_body(v, n, [&](std::int32_t k) { reserve.reserve_0073e9a0(v, k); });
}

void push_back_00736460(DwordVector& v, const std::uint32_t* value, AppPointerReserve& reserve) {
    push_back_body(v, value, [&](std::int32_t k) { reserve.reserve_00735ec0(v, k); });
}
void push_back_0073fa30(DwordVector& v, const std::uint32_t* value, VectorMemory& memory) {
    push_back_body(v, value, [&](std::int32_t k) { reserve_0073e8a0(v, k, memory); });
}
void push_back_0073fbb0(DwordVector& v, const std::uint32_t* value, VectorMemory& memory) {
    push_back_body(v, value, [&](std::int32_t k) { reserve_0073e940(v, k, memory); });
}
void push_back_0073fbf0(DwordVector& v, const std::uint32_t* value, DecalRecordReserve& reserve) {
    push_back_body(v, value, [&](std::int32_t k) { reserve.reserve_0073e9a0(v, k); });
}

void erase_swap_back_0073fcc0(DwordVector& v, std::int32_t index) {
    if (index == -1) return;
    if (index != v.size_04 - 1) v.data_00[index] = v.data_00[v.size_04 - 1];
    --v.size_04;
}

void destroy_00737bd0(DwordVector& v, VectorMemory& memory) {
    resize_00737340(v, 0, memory);
    memory.free_00bf6989(v.data_00);
}
void destroy_007402d0(DwordVector& v, VectorMemory& memory) {
    resize_0073fc70(v, 0, memory);
    memory.free_00bf6989(v.data_00);
}
void destroy_00740310(DwordVector& v, VectorMemory& memory) {
    resize_0073fda0(v, 0, memory);
    memory.free_00bf6989(v.data_00);
}
void destroy_007403f0(DwordVector& v, VectorMemory& memory) {
    resize_0073fda0(v, 0, memory);
    memory.free_00bf6989(v.data_00);
}
void destroy_007400a0(DwordVector& v, VectorMemory& memory) {
    resize_0073ea40(v, 0, memory);  // 007400A5
    memory.free_00bf6989(v.data_00);
}
void destroy_00737810(DwordVector& v, AppPointerResize& resize, VectorMemory& memory) {
    resize.resize_00735f30(v, 0);       // 00737815
    memory.free_00bf6989(v.data_00);    // 0073781D
}
void destroy_007402f0(DwordVector& v, DecalRecordReserve& reserve, VectorMemory& memory) {
    resize_0073fd00(v, 0, reserve);
    memory.free_00bf6989(v.data_00);
}

RefCounted** release_ref_00736050(RefCounted** slot, RefCountedHost& host) { return release_ref_body(slot, host); }
void release_ref_void_007457e0(RefCounted** slot, RefCountedHost& host) { (void)release_ref_body(slot, host); }

std::uint32_t* singleton_register(std::uint32_t* self, std::uint32_t vtable, void*& global,
                                  SingletonLifetimeHost& host) {
    *self = vtable;
    void* section = host.lock();
    global = self;
    host.register_00bd0c30(global);  // the argument is the re-read global
    host.unlock(section);
    return self;
}
std::uint32_t* singleton_register_00736100(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    return singleton_register(self, 0x00CFEA54u, g, host);
}
std::uint32_t* singleton_register_007364a0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    return singleton_register(self, 0x00CFEA58u, g, host);
}
std::uint32_t* singleton_register_00736540(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    return singleton_register(self, 0x00CFEA5Cu, g, host);
}
std::uint32_t* singleton_register_007366b0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    return singleton_register(self, 0x00CFEA60u, g, host);
}
std::uint32_t* singleton_register_007367f0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    return singleton_register(self, 0x00CFEA64u, g, host);
}

void singleton_unregister(std::uint32_t* self, std::uint32_t vtable, void*& global, SingletonLifetimeHost& host) {
    *self = vtable;
    void* section = host.lock();
    host.unregister_00bcfca0(global);
    global = nullptr;
    host.unlock(section);
    *self = kSingletonBaseVtable_00ce3818;
}
void singleton_unregister_007365e0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_unregister(self, 0x00CFEA5Cu, g, host);
}
void singleton_unregister_00736750(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_unregister(self, 0x00CFEA60u, g, host);
}
void singleton_unregister_00736890(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_unregister(self, 0x00CFEA64u, g, host);
}
void singleton_unregister_00736f70(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_unregister(self, 0x00CFEA68u, g, host);
}
void singleton_unregister_00737400(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_unregister(self, 0x00CFEA54u, g, host);
}
void singleton_unregister_007375e0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_unregister(self, 0x00CFEA58u, g, host);
}

void singleton_destroy_007361a0(void*& g, LazySingletonDestroyHost& host) { lazy_singleton_destroy(g, host); }
void singleton_destroy_00736300(void*& g, LazySingletonDestroyHost& host) { lazy_singleton_destroy(g, host); }
void singleton_destroy_007369e0(void*& g, LazySingletonDestroyHost& host) { lazy_singleton_destroy(g, host); }

const LazySingletonGetterRow kMsarProviderFactoryGetter_00736d00{
    0x00736D00u, 0x010904F8u, 8u, 0u, 4u, 0x00CFEA04u, 0x00CFEA2Cu, 0x00CFEA28u};
void* msar_provider_factory_get_00736d00(void*& g, LazySingletonHost& host) {
    return lazy_singleton_get(kMsarProviderFactoryGetter_00736d00, g, host);
}

std::uint32_t* holder_construct_007376e0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_register_007366b0(self, g, host);
    *self = 0x00CFEA70u;
    return self;
}
std::uint32_t* holder_construct_007377b0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_register_007367f0(self, g, host);
    *self = 0x00CFEA80u;
    return self;
}
std::uint32_t* holder_construct_00737a00(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_register_007367f0(self, g, host);
    *self = 0x00CFEAC8u;
    return self;
}
std::uint32_t* holder_construct_00737ee0(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_register_007364a0(self, g, host);
    *self = 0x00CFEAE8u;
    for (int i = 1; i <= 6; ++i) self[i] = 0;
    return self;
}

std::uint32_t* application_lifetime_construct_00737e20(std::uint32_t* self, void*& global_0109cf00,
                                                       const char* empty_literal_00ce3a0c,
                                                       SingletonLifetimeHost& lifetime, TempStringHost& strings) {
    singleton_register_00736100(self, global_0109cf00, lifetime);       // 00737E41
    std::int32_t len = 0;
    char* data = nullptr;
    strings.native_string_resize_0041dd40(&len, &data, 0, 1);          // 00737E5B
    if (data != nullptr) {
        const std::size_t bytes = static_cast<std::size_t>(len) + 1u;
        strings.memcpy_00bf7680(data, empty_literal_00ce3a0c, bytes);   // 00737E76
        void* pool = strings.sized_pool_00419cc0();                      // 00737E82
        strings.sized_pool_return_00bd1510(pool, data, bytes, 1);        // 00737E89
    }
    *self = kApplicationLifetimeVtable_00cfeae4;                         // 00737E93
    return self;
}

// ---- decal manager -------------------------------------------------------

namespace {
void release_field(RefCounted*& field, RefCountedHost& host) {
    RefCounted* object = field;
    if (object != nullptr) {
        if (host.interlocked_decrement(&object->refs_04) == 0) host.release_slot0(object);
        field = nullptr;
    }
}
void release_value(RefCounted* object, RefCountedHost& host) {
    if (object != nullptr && host.interlocked_decrement(&object->refs_04) == 0) host.release_slot0(object);
}
}  // namespace

void decal_system_destruct_0073fb10(std::uint32_t* self, void*& g, SingletonLifetimeHost& host) {
    singleton_unregister(self, 0x00CFF218u, g, host);
}

std::uint32_t named_key_index_of_0073f9c0(const NamedKeyArray& table, const std::int32_t* key, NamedKeyHost& host) {
    for (std::uint32_t i = 0; i < table.count_14; ++i) {
        const std::int32_t* entry = table.entries_10[i];
        std::int32_t entry_len = entry[0];
        const std::int32_t key_len = key[0];
        if (entry_len != key_len) continue;
        int order;
        if (entry_len == 0) {
            order = -static_cast<int>(key_len != 0);
        } else if (key_len == 0) {
            order = 1;
        } else {
            order = host.stricmp(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(entry[1])),
                                 reinterpret_cast<const char*>(static_cast<std::uintptr_t>(key[1])));  // 0073F9FB
        }
        if (order == 0) return i;
    }
    return 0xFFFFFFFFu;
}

std::uint32_t decal_batch_ensure_vertex_capacity_0073ff50(DecalRecord& record, std::uint32_t count, DecalHost& host) {
    void* section = record.draw_section_1c;
    const auto* sec = static_cast<const std::uint8_t*>(section);
    const auto format = [&] {
        // [[definition+14h]+8]: the texture's vertex format dword.
        const auto* texture = reinterpret_cast<const std::uint8_t*>(record.definition_18->vertex_format_14);
        return *reinterpret_cast<const std::uint32_t*>(texture + 8);
    };
    if (*reinterpret_cast<const std::uint32_t*>(sec + 0x4C) == 0) {  // 0073FF5C: no stream yet
        RefCounted* buffer = host.device_create_vertex_buffer_5c(record.vertex_count_08 + count, 1, format());
        host.draw_section_append_stream_00b85b80(section, buffer);      // 00740058
        record.vertex_capacity_0c = host.buffer_capacity_20(buffer);    // 00740064
        release_value(buffer, host);
        return 0;
    }
    const std::uint32_t base = record.vertex_count_08;
    if (record.vertex_capacity_0c >= base + count) return base;        // 0073FF6E JNC
    auto* old_buffer = *reinterpret_cast<RefCounted* const*>(sec + 0x3C);
    RefCounted* buffer = host.device_create_vertex_buffer_5c(base + count, 1, format());  // 0073FF97
    void* destination = host.buffer_lock_10(buffer, 0, 0, 0);          // 0073FFA8
    void* source = host.buffer_lock_10(old_buffer, 0, 0, 0);           // 0073FFB9
    const std::uint32_t stride = host.buffer_stride_24_cc(old_buffer);  // 0073FFC6
    host.memcpy_00bf7680(destination, source, static_cast<std::size_t>(stride * record.vertex_count_08));
    host.buffer_unlock_14(old_buffer);                                  // 0073FFE8
    host.buffer_unlock_14(buffer);                                      // 0073FFF1
    host.draw_section_clear_streams_00b86550(section);
    host.draw_section_append_stream_00b85b80(section, buffer);
    record.vertex_capacity_0c = host.buffer_capacity_20(buffer);
    release_value(buffer, host);
    return base;
}

bool erase_value_007400c0(DwordVector& v, const std::uint32_t* value) {
    std::uint32_t* const first = v.data_00;
    const auto count = static_cast<std::uint32_t>(v.size_04);
    std::int32_t index = -1;
    if (first < first + count) {
        std::uint32_t* p = first;
        while (*p != *value) {
            ++p;
            if (first + count <= p) return false;
        }
        index = static_cast<std::int32_t>(p - first);
        if (index != -1) {
            if (static_cast<std::uint32_t>(index) != count - 1) first[index] = first[count - 1];
            --v.size_04;
        }
    }
    return index != -1;
}

void list_clear_00740140(NodeList& list, DecalHost& host) {
    void** head = list.head_04;
    auto* node = static_cast<void**>(head[0]);
    head[0] = head;
    list.head_04[1] = list.head_04;
    list.size_08 = 0;
    if (node != reinterpret_cast<void**>(list.head_04)) {
        void** next;
        do {
            next = static_cast<void**>(node[0]);
            host.free_00bf65ac(node);
            node = next;
        } while (next != reinterpret_cast<void**>(list.head_04));
    }
    host.free_00bf65ac(list.head_04);
    list.head_04 = nullptr;
}

void decal_definition_destruct_007401b0(DecalDefinition& d, DecalHost& host) {
    release_field(d.texture_18, host);
    release_field(d.effect_1c, host);
    release_field(d.vertex_format_14, host);
    if (d.name_data_04 != nullptr) {
        host.native_string_release_00419cc0_00bd1510(d.name_data_04, static_cast<std::size_t>(d.name_len_00) + 1u);
    }
}

void life_ref_release_00740270(DwordVector& owner_04, RefCounted* value, DecalHost& host) {
    const auto as_dword = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
    (void)erase_value_007400c0(owner_04, &as_dword);  // 00740279, ECX = owner+4
    release_value(value, host);
}

NodeList* list_construct_007402a0(NodeList& list, DecalHost& host) {
    list.head_04 = static_cast<void**>(host.buy_list_head_node_0073fc50());
    list.size_08 = 0;
    return &list;
}

DecalRecord* decal_record_construct_00740350(DecalRecord& r, DecalHost& host) {
    r.rc.vtable_00 = 0x00CEB130u;
    r.rc.refs_04 = 1;
    r.rc.vtable_00 = 0x00CFF21Cu;
    r.owner_10 = nullptr;
    r.definition_18 = nullptr;
    r.draw_section_1c = nullptr;
    r.ref_20 = nullptr;
    r.list_head_28 = host.buy_list_head_node_0073fc50();
    r.list_size_2c = 0;
    return &r;
}

void decal_record_destruct_007403c0(DecalRecord& r, DecalHost& host) {
    list_clear_00740140(*reinterpret_cast<NodeList*>(&r.list_alloc_24), host);
    host.destroy_refcounted_base_00bd30f0(&r);
}

DecalDefinition* decal_definition_construct_00740410(DecalDefinition& d, const char* literal_00cff224,
                                                     DecalHost& host) {
    d.name_len_00 = 0;
    d.name_data_04 = nullptr;
    d.vertex_format_14 = nullptr;
    d.texture_18 = nullptr;
    d.effect_1c = nullptr;
    std::int32_t name[2] = {0, 0};  // {len, data}
    host.native_string_resize_0041dd40(name, 10, 1);                    // 00740454
    auto* data = reinterpret_cast<char*>(static_cast<std::uintptr_t>(name[1]));
    if (data != nullptr) host.memcpy_00bf7680(data, literal_00cff224, static_cast<std::size_t>(name[0]) + 1u);
    d.vertex_format_14 = host.device_load_texture_34(name);                   // 0074048C
    data = reinterpret_cast<char*>(static_cast<std::uintptr_t>(name[1]));
    if (data != nullptr) host.native_string_release_00419cc0_00bd1510(data, static_cast<std::size_t>(name[0]) + 1u);
    return &d;
}

void decal_list_destruct_007404d0(DecalList& list, DecalHost& host, VectorMemory& memory) {
    list.rc.vtable_00 = 0x00CFF230u;
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(list.records_08.size_04); ++i) {
        auto* record = reinterpret_cast<DecalRecord*>(static_cast<std::uintptr_t>(list.records_08.data_00[i]));
        host.ref_chain_unlink_00b85b30(record->ref_20,
                                       static_cast<RefCounted*>(record->draw_section_1c));  // 00740527
        record = reinterpret_cast<DecalRecord*>(static_cast<std::uintptr_t>(list.records_08.data_00[i]));
        release_field(record->ref_20, host);
        record = reinterpret_cast<DecalRecord*>(static_cast<std::uintptr_t>(list.records_08.data_00[i]));
        release_field(record->owner_10, host);
        record = reinterpret_cast<DecalRecord*>(static_cast<std::uintptr_t>(list.records_08.data_00[i]));
        auto* section = static_cast<RefCounted*>(record->draw_section_1c);
        release_field(section, host);
        record->draw_section_1c = section;
        auto* slot = reinterpret_cast<RefCounted**>(&list.records_08.data_00[i]);
        release_field(*slot, host);
    }
    resize_0073fc70(list.records_08, 0, memory);  // 007405F7
    resize_0073fc70(list.records_08, 0, memory);  // 00740605 (the inlined vector destructor)
    memory.free_00bf6989(list.records_08.data_00);
    host.destroy_refcounted_base_00bd30f0(&list);  // 0074061F
}

DecalRecord* decal_list_create_record_00740660(DecalList& list, RefCounted* owner, std::uint32_t field_14,
                                               DecalDefinition* definition, DecalHost& host, VectorMemory& memory) {
    void* memory_30 = host.operator_new_00bf681b(0x30);                 // 00740680
    DecalRecord* r = memory_30 != nullptr
        ? decal_record_construct_00740350(*static_cast<DecalRecord*>(memory_30), host)
        : nullptr;  // the image goes on with ESI = 0 and faults on +10h
    RefCounted* previous = r->owner_10;
    if (previous != owner) {
        r->owner_10 = owner;
        if (owner != nullptr) host.interlocked_increment(&owner->refs_04);  // 007406C3
        release_value(previous, host);
    }
    r->field_14 = field_14;
    r->definition_18 = definition;
    void* section = host.draw_section_create_00533fa0();               // 007406F2
    r->draw_section_1c = section;
    *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(section) + 8) = 4;
    host.draw_section_set_indexed_00b85600(r->draw_section_1c, 0);
    void* slot = host.material_alloc_00b18780(0x110);                  // 00740710, ECX = 110h
    RefCounted* material = slot != nullptr ? host.material_construct_00b18900(slot, definition->effect_1c) : nullptr;
    host.material_set_texture_00b189f0(material, 0, definition->texture_18);  // 00740747
    host.draw_section_set_material_00b864c0(r->draw_section_1c, material);
    release_value(material, host);
    r->vertex_count_08 = 0;
    r->vertex_capacity_0c = 0;
    const auto value = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(r));
    push_back_0073fa30(list.records_08, &value, memory);  // inlined: growth through 0073E8A0
    return r;
}

void decal_definition_delete_00740eb0(DecalDefinition** slot, DecalHost& host) {
    DecalDefinition* d = *slot;
    if (d != nullptr) {
        decal_definition_destruct_007401b0(*d, host);
        host.free_00bf65ac(d);
        *slot = nullptr;
    }
}

void decal_manager_destruct_00740f10(DecalManager& m, void*& global_00e1aea0, DecalHost& host,
                                     VectorMemory& memory, SingletonLifetimeHost& lifetime) {
    m.vtable_00 = 0x00CFF2A0u;
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m.definitions_10.size_04); ++i) {
        auto* slot = reinterpret_cast<DecalDefinition**>(&m.definitions_10.data_00[i]);
        if (*slot != nullptr) {
            decal_definition_destruct_007401b0(**slot, host);  // 00740F56
            host.free_00bf65ac(*slot);                          // 00740F5C
            *slot = nullptr;
        }
    }
    resize_0073fd00(m.definitions_10, 0, host);  // 00740F7B
    resize_0073fd00(m.definitions_10, 0, host);  // 00740F89
    memory.free_00bf6989(m.definitions_10.data_00);
    resize_0073ea40(m.lists_04, 0, memory);          // 00740FA5
    memory.free_00bf6989(m.lists_04.data_00);
    decal_system_destruct_0073fb10(&m.vtable_00, global_00e1aea0, lifetime);  // 00740FBF
}

DecalList* decal_manager_register_owner_00740fe0(DecalManager& m, DecalHost& host, VectorMemory& memory) {
    auto* list = static_cast<DecalList*>(host.operator_new_00bf681b(0x14));
    if (list != nullptr) {
        list->rc.vtable_00 = 0x00CEB130u;
        list->rc.refs_04 = 1;
        list->rc.vtable_00 = 0x00CFF230u;
        list->records_08 = DwordVector{nullptr, 0, 0};
    }
    const auto value = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(list));
    push_back_0073fbb0(m.lists_04, &value, memory);  // inlined: growth through 0073E940
    return list;
}

// ---- locale table manager -------------------------------------------------

void locale_table_manager_destroy_0073c9f0(std::uint8_t* self, void*& global_00f8bc4c, LocaleManagerHost& host,
                                           SingletonLifetimeHost& lifetime) {
    auto dword = [self](std::uint32_t offset) -> std::uint32_t& {
        return *reinterpret_cast<std::uint32_t*>(self + offset);
    };
    auto pointer = [](std::uint32_t value) { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value)); };
    dword(0) = 0x00CFF140u;
    if (dword(0x4034) != 0) host.free_00bf65ac(pointer(dword(0x4034)));
    dword(0x4034) = 0;
    dword(0x4038) = 0;
    dword(0x403C) = 0;
    if (dword(0x4024) != 0) host.free_00bf65ac(pointer(dword(0x4024)));
    dword(0x4024) = 0;
    dword(0x4028) = 0;
    dword(0x402C) = 0;
    if (dword(0x401C) != 0) host.native_string_release_00419cc0_00bd1510(pointer(dword(0x401C)), dword(0x4018) + 1u);
    host.string_map_clear_0073c240(self + 0x14);                          // 0073CA94
    if (dword(0x8) != 0) {
        host.destroy_string_range_00432050(pointer(dword(0x8)), pointer(dword(0xC)), self + 4, self);  // 0073CAB0
        host.free_00bf65ac(pointer(dword(0x8)));
    }
    dword(0x8) = 0;
    dword(0xC) = 0;
    dword(0x10) = 0;
    singleton_unregister_007365e0(reinterpret_cast<std::uint32_t*>(self), global_00f8bc4c, lifetime);  // 0073CAD4
}

void operator_delete_0073e6b0(void* p, std::uint32_t, DecalHost& host) { host.free_00bf65ac(p); }

void locale_node_return_slot_thunk_007373f0(void* slot, void* locale_pool_00e1768c) {
    return_native_locale_node_slot_00736090(locale_pool_00e1768c, slot);
}
void frame_hook_slot04_noop_00737850() {}
void frame_hook_slot14_noop_00737890() {}

void decal_list_add_decal_007407c0(std::uint8_t& guard_00e1aea8, std::uint32_t& scope_00e1aea4,
                                   const char* name_00cff238, ProfileScopeHost& host) {
    if ((guard_00e1aea8 & 1) != 0) return;
    guard_00e1aea8 |= 1;
    std::int32_t name[2];  // {len, data}, initialised by 0041E870
    host.native_string_assign_0041e870(name, name_00cff238);
    scope_00e1aea4 = 0;
    auto* data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(name[1]));
    if (data != nullptr) host.native_string_release_00419cc0_00bd1510(data, static_cast<std::size_t>(name[0]) + 1u);
}

namespace {
// {len, data} built from a C string: 0041DD40(&s, strlen, 1), memcpy len+1.
void build_string(std::int32_t (&s)[2], const char* text, ContentPackageHost& host) {
    s[0] = 0;
    s[1] = 0;
    std::uint32_t length = 0;
    while (text[length] != '\0') ++length;
    host.native_string_resize_0041dd40(s, length, 1);
    auto* data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(s[1]));
    if (data != nullptr) host.memcpy_00bf7680(data, text, static_cast<std::size_t>(s[0]) + 1u);
}
void release_string(const std::int32_t (&s)[2], ContentPackageHost& host) {
    auto* data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(s[1]));
    if (data != nullptr) host.native_string_release_00419cc0_00bd1510(data, static_cast<std::size_t>(s[0]) + 1u);
}
}  // namespace

void mount_content_packages_0073bc40(void* const volatile& vfs_0109ceec, ContentPackageHost& host) {
    void* const vfs_cached = vfs_0109ceec;                                // 0073BC64
    host.vfs_register_factory_00be0660(vfs_cached, host.msar_factory_get_00736d00());
    std::int32_t n = 1;
    std::int32_t priority = -11;
    do {
        char suffix[260];
        char path[256];
        host.format_int_004b8490(suffix, "%08x", n);                       // 00CFF0B0
        host.format_text_004b8490(path, "ContentPackage/%s.msar", suffix);  // 00CFF098
        std::int32_t probe[2];
        build_string(probe, path, host);
        const bool found = host.vfs_find_slot08(vfs_0109ceec, probe);     // 0073BD1B
        release_string(probe, host);
        if (found) {
            std::int32_t check[2];
            build_string(check, path, host);
            const bool valid = host.msar_validate_00bbbb90(check);       // 0073BD9F
            release_string(check, host);
            if (valid) {
                std::int32_t root[2];
                build_string(root, ".", host);                            // 00CE3A70, length 1
                std::int32_t mount[2];
                build_string(mount, path, host);
                host.vfs_mount_00be1890(vfs_0109ceec, mount, root, priority, 0, -1);  // 0073BE73
                release_string(mount, host);
                release_string(root, host);
                std::int32_t name[2];
                build_string(name, suffix, host);
                host.vfs_register_suffix_00bdef80(vfs_0109ceec, name);  // 0073BF22
                release_string(name, host);
            }
        }
        ++n;
        --priority;
    } while (-26 < priority);
}

}  // namespace bsp::cc13_seg41
