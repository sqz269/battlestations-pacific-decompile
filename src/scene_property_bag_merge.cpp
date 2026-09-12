// Scene property bag merge and per-type value copy.
// Addresses: 008F54F0, 008F23E0, 008F4F60, 008F0700, 008F0340, 008F03B0,
// 008F03F0, 008F0640, 008F0420, 008F3AC0, 008EF2B0, 008EF2F0, 008EF360,
// 008EF7F0. See docs/SCENE_PROPERTY_BAG_MERGE.md for the evidence behind every
// rule below. The names are hypotheses, not recovered symbols.
#include "bsp/scene_property_bag_merge.hpp"

#include <cctype>
#include <cstddef>

namespace bsp {
namespace {

// 0043B8B0 accepts a pointer-equal key or __stricmp == 0 after the hash match,
// so every lookup in this file is case-insensitive. Same rule as the lookup in
// scene_property_bag.cpp; kept file-local rather than exported twice.
bool equals_no_case_merge(const std::string& left, const std::string& right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
        const int a = std::tolower(static_cast<unsigned char>(left[i]));
        const int b = std::tolower(static_cast<unsigned char>(right[i]));
        if (a != b) {
            return false;
        }
    }
    return true;
}

ScenePropertyEntry* find_entry(ScenePropertyBagModel& model,
                               int bag_index,
                               const std::string& key) noexcept {
    if (bag_index < 0 || static_cast<std::size_t>(bag_index) >= model.bags.size()) {
        return nullptr;
    }
    for (auto& entry : model.bags[static_cast<std::size_t>(bag_index)].entries) {
        if (equals_no_case_merge(entry.key, key)) {
            return &entry;
        }
    }
    return nullptr;
}

bool is_array_type(ScenePropertyType type) noexcept {
    return type == ScenePropertyType::ByteArray || type == ScenePropertyType::FloatArray ||
           type == ScenePropertyType::IntArray || type == ScenePropertyType::Vector3Array;
}

} // namespace

ScenePropertyCopySpec scene_property_clone_spec(ScenePropertyType type) noexcept {
    ScenePropertyCopySpec spec;
    spec.handled = true;
    spec.carries_ordinal = true; // every arm ends with clone+34h = source+34h
    switch (type) {
    case ScenePropertyType::Int:    // 008EF140 via the case-0 arm
    case ScenePropertyType::Float:  // 008EF170
        spec.inline_dword = true;
        break;
    case ScenePropertyType::String: // 008EF1B0, which duplicates +0Ch
        spec.owned_string = true;
        break;
    case ScenePropertyType::Bool:   // 008EF1F0 takes the byte at +0Ch
        spec.inline_byte = true;
        break;
    case ScenePropertyType::Enum:   // 008EF230(decl = +28h, value = +0Ch)
        spec.inline_dword = true;
        spec.shared_decl = true;
        break;
    case ScenePropertyType::Reference: // 008EF2B0(+1Ch, +18h, +08h)
        spec.reference_payload = true;
        spec.reference_kind = true;
        break;
    case ScenePropertyType::SubBag: // 008F41F0 then 008EF780
        spec.nested_bag = true;
        break;
    case ScenePropertyType::Vector3: // 008EF270(&record+0Ch), three floats
        spec.vector3 = true;
        break;
    case ScenePropertyType::ByteArray:    // 008EF2F0(+20h, +24h, copy = 1)
    case ScenePropertyType::FloatArray:   // 008EF360(count, +20h, copy = 1)
    case ScenePropertyType::IntArray:     // 008EF3E0, same shape
    case ScenePropertyType::Vector3Array: // 008EF460, same shape
        spec.owned_block = true;
        break;
    default:
        spec.handled = false;
        spec.carries_ordinal = false;
        break;
    }
    return spec;
}

ScenePropertyCopySpec scene_property_assign_spec(ScenePropertyType type) noexcept {
    ScenePropertyCopySpec spec;
    spec.handled = true;
    switch (type) {
    case ScenePropertyType::Int:
    case ScenePropertyType::Float:
    case ScenePropertyType::Enum:
        // 008F071A, 008F0726 and 008F0790: one dword from other+0Ch. The type-4
        // arm does NOT touch +28h, so the destination keeps its own declaration.
        spec.inline_dword = true;
        break;
    case ScenePropertyType::String:
        // 008F0747: free(dest+0Ch) then 00438E40 on other+0Ch.
        spec.owned_string = true;
        break;
    case ScenePropertyType::Bool:
        spec.inline_byte = true;
        break;
    case ScenePropertyType::Reference:
        // 008F076E: 008F0340 with ECX = dest+18h and other+18h on the stack. It
        // moves exactly two dwords, +18h and +1Ch, so the destination keeps its
        // own kind at +08h. That asymmetry with the clone arm is real.
        spec.reference_payload = true;
        break;
    case ScenePropertyType::Vector3:
        spec.vector3 = true;
        break;
    case ScenePropertyType::ByteArray:
    case ScenePropertyType::FloatArray:
    case ScenePropertyType::IntArray:
    case ScenePropertyType::Vector3Array:
        // 008F07B3: 008F03B0 with ECX = dest+20h. One arm for all four codes,
        // and it copies the source's raw block with no stride check.
        spec.owned_block = true;
        break;
    case ScenePropertyType::SubBag:
    default:
        // Code 6 has no arm at all; the switch falls through to the tail.
        spec.handled = false;
        break;
    }
    return spec;
}

std::uint32_t scene_property_array_elements(ScenePropertyType type,
                                            std::uint32_t byte_size) noexcept {
    // 008EF7F0: __fastcall(record /*ECX*/), reading record+04h and record+24h.
    switch (type) {
    case ScenePropertyType::FloatArray:
    case ScenePropertyType::IntArray:
        return byte_size >> 2;
    case ScenePropertyType::Vector3Array:
        return byte_size / 0x0C;
    default:
        return 0;
    }
}

bool scene_property_assign_value_from(ScenePropertyValue& dest,
                                      const ScenePropertyValue& source) noexcept {
    const ScenePropertyCopySpec spec = scene_property_assign_spec(dest.type);
    if (!spec.handled) {
        return false;
    }
    // The native arm copies raw storage and never compares the two type codes;
    // a mismatched pair reinterprets the source's dword. This projection refuses
    // instead of inventing a value, so a false return here is a divergence from
    // the native behaviour rather than a native no-op. Only the SubBag case
    // above matches the native fall-through.
    switch (dest.type) {
    case ScenePropertyType::Int:
    case ScenePropertyType::Enum:
        if (source.type != dest.type) {
            return false;
        }
        dest.int_value = source.int_value;
        return true;
    case ScenePropertyType::Float:
        if (source.type != ScenePropertyType::Float) {
            return false;
        }
        dest.float_value = source.float_value;
        return true;
    case ScenePropertyType::Bool:
        if (source.type != ScenePropertyType::Bool) {
            return false;
        }
        dest.int_value = source.int_value != 0 ? 1 : 0;
        return true;
    case ScenePropertyType::String:
        if (source.type != ScenePropertyType::String) {
            return false;
        }
        dest.text = source.text;
        return true;
    case ScenePropertyType::Reference:
        if (source.type != ScenePropertyType::Reference) {
            return false;
        }
        // +1Ch only. dest.reference_kind is +08h and stays as it was.
        dest.text = source.text;
        return true;
    case ScenePropertyType::Vector3:
        if (source.type != ScenePropertyType::Vector3) {
            return false;
        }
        dest.vector[0] = source.vector[0];
        dest.vector[1] = source.vector[1];
        dest.vector[2] = source.vector[2];
        return true;
    case ScenePropertyType::ByteArray:
    case ScenePropertyType::FloatArray:
    case ScenePropertyType::IntArray:
    case ScenePropertyType::Vector3Array:
        if (!is_array_type(source.type)) {
            // The native still runs 008F03B0, which reads the source's +20h and
            // +24h; on a non-array record both are zero, so the destination ends
            // up with an empty block.
            dest.byte_array.clear();
            dest.int_array.clear();
            dest.float_array.clear();
            return false;
        }
        dest.byte_array = source.byte_array;
        dest.int_array = source.int_array;
        dest.float_array = source.float_array;
        return true;
    default:
        return false;
    }
}

void scene_property_bag_assign_existing(ScenePropertyBagModel& model,
                                        int dest_bag,
                                        const ScenePropertyBagModel& source_model,
                                        int source_bag) noexcept {
    if (dest_bag < 0 || static_cast<std::size_t>(dest_bag) >= model.bags.size()) {
        return;
    }
    if (source_bag < 0 ||
        static_cast<std::size_t>(source_bag) >= source_model.bags.size()) {
        return;
    }
    // Copied out first: the walk never inserts, but `model` and `source_model`
    // may be the same object at a different index.
    const auto entries =
        source_model.bags[static_cast<std::size_t>(source_bag)].entries;
    for (const auto& source : entries) {
        ScenePropertyEntry* target = find_entry(model, dest_bag, source.key);
        if (target == nullptr) {
            // 008F24A2 / 008F24D4: BL stays 0 and the whole body is skipped.
            continue;
        }
        if (target->value.type == ScenePropertyType::SubBag) {
            // 008F24D8 tests the DESTINATION's code, then 008F24EB takes both
            // sides' +0Ch.
            scene_property_bag_assign_existing(model, target->sub_bag, source_model,
                                               source.sub_bag);
            continue;
        }
        scene_property_assign_value_from(target->value, source.value);
    }
}

void scene_property_bag_merge_native(ScenePropertyMergeHost& host,
                                     void* dest_bag,
                                     void* source_bag,
                                     bool keep_existing) {
    void* iterator = host.iterator_open(source_bag);
    if (iterator == nullptr) {
        // 008F5511 leaves the pointer null and 008F553F dereferences it anyway.
        // Refusing is a deliberate divergence from that fault.
        return;
    }
    while (host.iterator_valid(iterator)) {
        const std::string key = host.iterator_key(iterator);
        void* const source_record = host.iterator_record(iterator);
        void* const dest_record = host.find_in_dest(dest_bag, key);
        if (dest_record == nullptr) {
            void* const clone = host.clone_record(source_record);
            host.set_record_ordinal(clone, host.record_ordinal(source_record));
            host.insert_record(dest_bag, key, clone);
        } else if (host.record_type(dest_record) == ScenePropertyType::SubBag) {
            scene_property_bag_merge_native(host, host.record_sub_bag(dest_record),
                                            host.record_sub_bag(source_record),
                                            keep_existing);
        } else if (!keep_existing) {
            host.assign_value(dest_record, source_record);
        }
        host.iterator_advance(iterator);
    }
    host.iterator_close(iterator);
}

void scene_property_bag_assign_existing_native(ScenePropertyMergeHost& host,
                                               void* dest_bag,
                                               void* source_bag) {
    void* iterator = host.iterator_open(source_bag);
    if (iterator == nullptr) {
        return;
    }
    while (host.iterator_valid(iterator)) {
        const std::string key = host.iterator_key(iterator);
        void* const source_record = host.iterator_record(iterator);
        void* const dest_record = host.find_in_dest(dest_bag, key);
        if (dest_record != nullptr) {
            if (host.record_type(dest_record) == ScenePropertyType::SubBag) {
                scene_property_bag_assign_existing_native(
                    host, host.record_sub_bag(dest_record),
                    host.record_sub_bag(source_record));
            } else {
                host.assign_value(dest_record, source_record);
            }
        }
        host.iterator_advance(iterator);
    }
    host.iterator_close(iterator);
}

} // namespace bsp
