#include "bsp/scene_numeric_property_merge.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_scene_property_bag_merge.hpp"
#include "bsp/native_scene_property_bag_storage.hpp"
#include "bsp/native_scene_property_bool_add.hpp"
#include "bsp/native_scene_property_float_add.hpp"
#include "bsp/native_scene_property_integer_add.hpp"
#include "bsp/native_string.hpp"
#include "bsp/scene_file.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace bsp::game {

// A bounded projection of the scene host's owning property trees. Only explicit
// I/F/B values are admitted; their real parser-owned payloads feed the original
// insertion producers and the whole 008F54F0 merge. Unsupported keys stay in the
// host tree. Unique scalar keys are independent of the omitted keys/children;
// scalar/child collisions and differing tags are deliberately left to the host.
// No original DATA phase word is called as a Source vtable.
struct SceneNumericPropertyMerge::Impl final {
public:
    Impl(const ScenePropertyBlock& destination,
        const ScenePropertyBlock& source, bool overwrite)
        : context_(game_native_string_process(), property_process()),
          destination_(context_.insertion()), source_(context_.insertion()),
          selected_(source.values.size(), false) {
        bool any = false;
        for (std::size_t i = 0; i < source.values.size(); ++i) {
            const auto& value = source.values[i];
            if (!supported(value) || occurrences(source, value.key) != 1
                || occurrences(destination, value.key) > 1
                || has_child(source, value.key) || has_child(destination, value.key)) continue;
            const auto prior = std::find_if(destination.values.begin(), destination.values.end(),
                [&](const SceneProperty& p) { return same_key(p.key, value.key); });
            if (prior != destination.values.end()) {
                if (!supported(*prior) || prior->type_letter != value.type_letter) continue;
                destination_.insert(*prior);
            }
            source_.insert(value);
            selected_[i] = true;
            any = true;
        }
        if (any) {
            // 008F55FD..008F560D: keep byte zero assigns an existing scalar;
            // 008F5614..008F563B: a miss clones/inserts the actual source record.
            // The flag is the host's existing group/default precedence.
            merge_native_scene_property_bag_008f54f0(destination_.data(), source_.data(),
                overwrite ? 0u : 1u, context_);
        }
    }

    bool handles(std::size_t source_index) const { return selected_[source_index]; }

    SceneProperty result(const SceneProperty& authored_metadata) {
        // Keep source spelling/tokens (or the retained destination's when keep
        // is set) for diagnostics. Gameplay payload bytes come ONLY from the
        // merged native record, before publishing back into the owning tree.
        SceneProperty value;
        value.key = authored_metadata.key;
        value.type_letter = authored_metadata.type_letter;
        value.values = authored_metadata.values;
        const auto* record = static_cast<const std::byte*>(destination_.find(value.key));
        std::uint32_t tag;
        std::memcpy(&tag, record + 4, sizeof(tag));
        switch (tag) {
        case 0:
            std::memcpy(&value.integer_value, record + 0x0c, sizeof(value.integer_value));
            value.has_integer = true;
            break;
        case 1:
            // Bit transport only: no extra float operation/rounding is added.
            std::memcpy(&value.float_value, record + 0x0c, sizeof(value.float_value));
            value.has_float = true;
            break;
        case 3:
            std::memcpy(&value.boolean_value, record + 0x0c, sizeof(value.boolean_value));
            value.has_boolean = true;
            break;
        default:
            throw std::logic_error("unexpected merged numeric property tag");
        }
        return value;
    }

private:
    static GameNativePhysicalPoolProcess& property_process() {
        auto& pools = game_native_physical_pool_process();
        pools.initialize_property_node_pool_once_00cc8a30();
        return pools;
    }
    static bool same_key(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i]))
                != std::tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    }
    static bool supported(const SceneProperty& value) {
        if (std::any_of(value.key.begin(), value.key.end(),
                [](unsigned char c) { return c == 0 || c >= 0x80; })) return false;
        return (value.type_letter == "I" && value.has_integer)
            || (value.type_letter == "F" && value.has_float)
            || (value.type_letter == "B" && value.has_boolean);
    }
    static std::size_t occurrences(const ScenePropertyBlock& bag, const std::string& key) {
        return static_cast<std::size_t>(std::count_if(bag.values.begin(), bag.values.end(),
            [&](const SceneProperty& p) { return same_key(p.key, key); }));
    }
    static bool has_child(const ScenePropertyBlock& bag, const std::string& key) {
        return std::any_of(bag.blocks.begin(), bag.blocks.end(),
            [&](const auto& child) { return same_key(child.first, key); });
    }

    class Bag final {
    public:
        explicit Bag(NativeScenePropertyInsertionContext& context) : context_(context) {
            // Real raw61 producer, fresh aligned root, no borrowed owner.
            initialize_native_scene_property_bag_storage_008f41a0(storage_.data(), 0, 0);
        }
        ~Bag() {
            // Stack root survives; nested records, pooled keys and physical
            // nodes retire in their SAME canonical processes before the context.
            destroy_native_scene_property_bag_008f5410(storage_.data(), context_.lifetimes());
        }
        Bag(const Bag&) = delete;
        Bag& operator=(const Bag&) = delete;
        void* data() { return storage_.data(); }
        void insert(const SceneProperty& value) {
            if (value.type_letter == "I") {
                std::uint32_t bits;
                std::memcpy(&bits, &value.integer_value, sizeof(bits));
                add_native_scene_property_integer_word_008f3710(data(), value.key.c_str(), bits, context_);
            } else if (value.type_letter == "F") {
                std::uint32_t bits;
                std::memcpy(&bits, &value.float_value, sizeof(bits));
                add_native_scene_property_float_bits_008f3770(data(), value.key.c_str(), bits, context_);
            } else {
                add_native_scene_property_bool_byte_008f3940(
                    data(), value.key.c_str(), value.boolean_value, context_);
            }
        }
        void* find(const std::string& key) {
            NativeString query;
            construct_native_string_header_0041e870(&query, context_.lifetimes().keys(), key.c_str());
            std::uint32_t bucket;
            const auto* node = static_cast<const std::byte*>(find_native_property_map_node_0043b8b0(
                storage_.data() + 4, &query, bucket, context_.property_empty(), context_.query_empty()));
            void* record = nullptr;
            if (node != nullptr) std::memcpy(&record, node + 8, sizeof(record));
            destroy_native_string_header_0041dd20(&query, context_.lifetimes().keys());
            if (record == nullptr) throw std::logic_error("missing merged numeric property");
            return record;
        }
    private:
        NativeScenePropertyInsertionContext& context_;
        alignas(std::uint32_t) std::array<std::byte, native_scene_property_bag_storage_bytes> storage_;
    };

    NativeScenePropertyCloneContext context_;
    Bag destination_;
    Bag source_;
    std::vector<bool> selected_;
};

SceneNumericPropertyMerge::SceneNumericPropertyMerge(const ScenePropertyBlock& destination,
    const ScenePropertyBlock& source, bool overwrite)
    : impl_(std::make_unique<Impl>(destination, source, overwrite)) {}

SceneNumericPropertyMerge::~SceneNumericPropertyMerge() = default;

bool SceneNumericPropertyMerge::handles(std::size_t source_index) const {
    return impl_->handles(source_index);
}

SceneProperty SceneNumericPropertyMerge::result(const SceneProperty& authored_metadata) {
    return impl_->result(authored_metadata);
}

} // namespace bsp::game
