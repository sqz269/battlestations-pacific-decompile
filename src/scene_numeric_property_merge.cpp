#include "bsp/scene_numeric_property_merge.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_scene_property_enum_add.hpp"
#include "bsp/native_scene_property_bag_merge.hpp"
#include "bsp/native_scene_property_bag_assign_existing.hpp"
#include "bsp/native_scene_property_bag_reset.hpp"
#include "bsp/native_scene_property_bag_storage.hpp"
#include "bsp/native_scene_property_array_assignment.hpp"
#include "bsp/native_scene_property_bool_add.hpp"
#include "bsp/native_scene_property_float_add.hpp"
#include "bsp/native_scene_property_integer_add.hpp"
#include "bsp/native_scene_property_hash_iterator.hpp"
#include "bsp/native_scene_property_reference_subbag_add.hpp"
#include "bsp/native_scene_property_reference_set.hpp"
#include "bsp/native_scene_property_string_add.hpp"
#include "bsp/native_scene_typed_array_vector_providers.hpp"
#include "bsp/native_scene_typed_property_providers.hpp"
#include "bsp/native_string.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/cc13_seg59b.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <map>
#include <new>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace bsp::game {

// A bounded projection of the scene host's owning property trees. Only explicit
// I/F/B/S/V3 (and, under chain 311, closed nonempty IA and one-quoted-token
// reference letters, and under chain 321 closed nonempty BIN) values are admitted;
// their real parser-owned payloads feed the original
// insertion producers and the whole 008F54F0 merge. Unsupported keys stay in the
// host tree. Unique scalar keys are independent of the omitted keys/children;
// scalar/child collisions and differing tags are deliberately left to the host.
// Chain 332 also projects complete supported child subtrees, including empty
// children, through the owning 008F3B60 producer and recursive whole merge.
// Unique parent names and compatible complete trees gate that separate path.
// No original DATA phase word is called as a Source vtable.
struct SceneNumericPropertyMerge::Impl final {
public:
    Impl(const ScenePropertyBlock& destination,
        const ScenePropertyBlock& source, bool overwrite, SceneNativeEnumSource* enums)
        : context_(game_native_string_process(), property_process()),
          destination_(context_.insertion()), source_(context_.insertion()),
          selected_(source.values.size(), false), enums_(enums), overwrite_(overwrite) {
        bool any = false;
        bool source_has_only_existing_keys = true;
        for (std::size_t i = 0; i < source.values.size(); ++i) {
            const auto& value = source.values[i];
            if (!admitted(value) || occurrences(source, value.key) != 1
                || occurrences(destination, value.key) > 1
                || has_child(source, value.key) || has_child(destination, value.key)) continue;
            const auto prior = std::find_if(destination.values.begin(), destination.values.end(),
                [&](const SceneProperty& p) { return same_key(p.key, value.key); });
            if (prior != destination.values.end()) {
                if (!admitted(*prior) || prior->type_letter != value.type_letter) continue;
                insert_into(destination_, *prior);
                if (overwrite && same_key(value.type_letter, "FA")) {
                    destination_.replace_float_array(value);
                    selected_[i] = true;
                    continue;
                }
                if (overwrite && (same_key(value.type_letter, "V3A")
                        || same_key(value.type_letter, "BIN") || value.type_letter == "IA")) {
                    // The admitted same-tag overwrite has a complete,
                    // sole-owned type-8/10/11 record. Replace its array once;
                    // do not also put this key through 008F0700/008F54F0.
                    destination_.replace_owned_array(value);
                    selected_[i] = true;
                    continue;
                }
            } else {
                source_has_only_existing_keys = false;
            }
            if (prior != destination.values.end() && overwrite
                    && scene_reference_kind(value.type_letter) >= 0) {
                // The same-tag admitted destination was inserted above.
                // Replace its owned reference once, then publish that
                // record rather than also assigning it through the merge.
                destination_.replace_reference(value);
                selected_[i] = true;
                continue;
            }
            insert_into(source_, value);
            selected_[i] = true;
            any = true;
        }
        if (any) {
            // bsp.py show 008F23E0 / 008F54F0: both hit arms assign
            // through 008F0700. For this scalar-only projection, every
            // selected key already existing and overwrite=true exclude
            // the merge's clone/miss and keep branches. The raw61 owners
            // and their payloads come from the same admitted parser data
            // and canonical insertion context above. Assign exactly once;
            // publication below still consumes the resulting native bag.
            if (overwrite && source_has_only_existing_keys) {
                assign_existing_native_scene_property_bag_008f23e0(
                    destination_.data(), source_.data(), context_.insertion());
            } else {
                copy_or_merge(destination_, source_, overwrite);
            }
        }
    }

    bool handles(std::size_t source_index) const { return selected_[source_index]; }

    SceneProperty result(const SceneProperty& authored_metadata) {
        // The dependent cache parameter lets the OFF instantiation have no
        // map storage, constructor or destructor. ON constructs the optional's
        // index only when the first handled property actually needs it.
        return [&](auto& records) -> SceneProperty {
        // The constructor has finished the whole native merge. Enumerate
        // its stable destination once; every result consumes the mapped
        // native record below, with authored metadata supplying only the
        // spelling/tokens. Neither bag is mutated during publication.
        if (!merged_records_collected_) {
            records.emplace();
            records->collect(destination_.data());
            merged_records_collected_ = true;
        }
        return read_value(authored_metadata, records->find(authored_metadata.key));
        }(merged_records_);
    }

    bool merge_child(const std::string& key, const ScenePropertyBlock* destination,
        const ScenePropertyBlock& source, ScenePropertyBlock& result) {
        if (!closed_key(key) || !closed_tree(source)
            || (destination != nullptr
                && (!closed_tree(*destination) || !compatible_trees(*destination, source)))) {
            return false;
        }

        Bag native_destination(context_.insertion());
        Bag native_source(context_.insertion());
        if (destination != nullptr) attach_tree(native_destination, key, *destination);
        attach_tree(native_source, key, source);
        copy_or_merge(native_destination, native_source, overwrite_);
        result = read_tree(native_destination.child(key), destination, source);
        return true;
    }

    void* make_owned_complete_bag(const ScenePropertyBlock& source) {
        if (!closed_tree(source)) return nullptr;
        Bag root(context_.insertion(), true);
        for (const auto& value : source.values) insert_into(root, value);
        for (const auto& child : source.blocks) attach_tree(root, child.first, child.second);
        return root.release_owned();
    }

    ScenePropertyBlock read_complete_bag(void* actual_bag,
        const ScenePropertyBlock& metadata) {
        return read_tree(actual_bag, nullptr, metadata);
    }

private:
    class MergedRecords final {
    public:
        void collect(void* actual_bag) {
            // bsp.py show 008EEB10/008EEB60 --asm: raw12 words are
            // (map, current node, bucket), map+4 is count, map+8 the 64
            // heads; Next follows node+0Ch before scanning later buckets.
            // Borrow the genuine raw61 bag's +4 map and its canonical pooled
            // nodes. Initialize only table: Begin establishes the remaining
            // fields (index is deliberately untouched for an empty map).
            static_assert(sizeof(void*) == 4);
            std::array<std::uint32_t, 3> iterator;
            iterator[0] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual_bag) + 4u);
            begin_native_scene_property_hash_008eeb10(iterator.data());
            while (iterator[1] != 0) {
                const auto* node = reinterpret_cast<const std::byte*>(
                    static_cast<std::uintptr_t>(iterator[1]));
                // 00484D20 reads the NativeString DATA at node+4 (length
                // is +0) and substitutes E186ED for null. Mapped CProperty*
                // is +8; +10 is allocator metadata, not next.
                const char* key;
                void* record;
                std::memcpy(&key, node + 4, sizeof(key));
                std::memcpy(&record, node + 8, sizeof(record));
                records_.emplace(key != nullptr ? key
                    : game_native_string_process().enum_empty_00e186ed(), record);
                next_native_scene_property_hash_008eeb60(iterator.data());
            }
        }

        void* find(const std::string& key) const {
            const auto found = records_.find(key);
            if (found == records_.end()) throw std::logic_error("missing merged numeric property");
            return found->second;
        }

    private:
        struct KeyLess {
            bool operator()(const std::string& a, const std::string& b) const {
                // The admission gates permit ASCII keys only. Match the
                // existing same_key comparison when retaining a differently
                // cased destination key through an overwrite/keep merge.
                return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                    [](unsigned char x, unsigned char y) {
                        return std::tolower(x) < std::tolower(y);
                    });
            }
        };
        // Keys are owned here; record pointers borrow destination_, which
        // outlives this index and owns every payload read by result().
        std::map<std::string, void*, KeyLess> records_;
    };

    static SceneProperty read_value(const SceneProperty& authored_metadata,
        const void* actual_record) {
        // Keep source spelling/tokens (or the retained destination's when keep
        // is set) for diagnostics. Gameplay payload bytes come ONLY from the
        // merged native record, before publishing back into the owning tree.
        SceneProperty value;
        value.key = authored_metadata.key;
        value.type_letter = authored_metadata.type_letter;
        value.values = authored_metadata.values;
        const auto* record = static_cast<const std::byte*>(actual_record);
        std::uint32_t tag;
        std::memcpy(&tag, record + 4, sizeof(tag));
        switch (tag) {
        case 0:
            std::memcpy(&value.integer_value, record + 0x0c, sizeof(value.integer_value));
            value.has_integer = true;
            break;
        case 1:
            std::memcpy(&value.float_value, record + 0x0c, sizeof(value.float_value));
            value.has_float = true;
            break;
        case 2: {
            // bsp.py show 008F0700: native assignment frees/duplicates
            // type-2 +0C, so publish the merged owner, not source tokens.
            const char* text;
            std::memcpy(&text, record + 0x0c, sizeof(text));
            value.values.assign(1, text);
            value.has_string = true;
            break;
        }
        case 3:
            std::memcpy(&value.boolean_value, record + 0x0c, sizeof(value.boolean_value));
            value.has_boolean = true;
            break;
        case 4:
            // 008F3A20 stores 0048E840's word at +0Ch and the CEnum at
            // +28h; 008F0700 (keep 0 hit) and the 008F4F60 clone (miss)
            // carry both. Consumers read the merged +0Ch, as the
            // executable's readers of a type-4 property do (006CB114).
            value.has_enum = authored_metadata.has_enum;
            std::memcpy(&value.enum_value, record + 0x0c, sizeof(value.enum_value));
            value.has_enum_value = true;
            break;
            throw std::logic_error("unexpected merged scene property tag");
        case 7:
            // 008F3690 and 008F0700 copy the three +0C/+10/+14 words.
            // Bit transport preserves signed zero without float math.
            std::memcpy(value.vector3.data(), record + 0x0c, sizeof(value.vector3));
            value.has_vector3 = true;
            break;
        case 5: {
            {
                // 008EF2B0 stores the owned duplicate at +1Ch; 008F0700's
                // type-5 arm copies the +18h/+1Ch payload through 008F0340
                // (keep 0 hit) and the 008F4F60 clone keeps it (miss). The
                // destination's +08h kind is retained; letters match here.
                const char* text;
                std::memcpy(&text, record + 0x1c, sizeof(text));
                if (text == nullptr) {
                    throw std::logic_error("unexpected merged type-5 payload");
                }
                value.values.assign(1, text);
                value.has_reference = true;
                break;
            }
            throw std::logic_error("unexpected merged scene property tag");
        }
        case 8: {
            {
                // 008EF2F0 stores bytes at +20h and count at +24h.
                // 008F0700/008F4F60 assign/clone this owning pair; publish
                // the merged record before either temporary bag retires.
                const std::uint8_t* payload;
                std::uint32_t bytes;
                std::memcpy(&payload, record + 0x20, sizeof(payload));
                std::memcpy(&bytes, record + 0x24, sizeof(bytes));
                if (payload == nullptr || bytes == 0) {
                    throw std::logic_error("unexpected merged type-8 payload");
                }
                value.byte_array.assign(payload, payload + bytes);
                value.has_byte_array = true;
            }
            break;
        }
        case 9: {
            {
                // 008EF360 owns +20h and records 4*count at +24h.
                // Publish the merged native bytes before bag retirement;
                // no decimal conversion or float arithmetic occurs here.
                const void* payload;
                std::uint32_t bytes;
                std::memcpy(&payload, record + 0x20, sizeof(payload));
                std::memcpy(&bytes, record + 0x24, sizeof(bytes));
                if (payload == nullptr || bytes == 0 || bytes % 4 != 0)
                    throw std::logic_error("unexpected merged type-9 payload");
                value.float_array.resize(bytes / 4);
                std::memcpy(value.float_array.data(), payload, bytes);
                value.has_float_array = true;
            }
            break;
        }
        case 11: {
            {
                // 008EF460 records 12*count bytes at +24h. The native
                // 008F0700 assignment / 008F4F60 clone owns the resulting
                // payload. Publish its bytes before either bag retires.
                const std::array<float, 3>* triples;
                std::uint32_t bytes;
                static_assert(sizeof(*triples) == 12);
                std::memcpy(&triples, record + 0x20, sizeof(triples));
                std::memcpy(&bytes, record + 0x24, sizeof(bytes));
                if (triples == nullptr || bytes == 0 || bytes % 12 != 0) {
                    throw std::logic_error("unexpected merged type-11 payload");
                }
                value.vector3_array.resize(bytes / 12);
                std::memcpy(value.vector3_array.data(), triples, bytes);
                value.has_vector3_array = true;
            }
            break;
        }
        case 10: {
            {
                // 008EF3E0 stores the payload at +20h and 4*count at
                // +24h; 008F0700's array arm (keep 0 hit) and the
                // 008F4F60 record clone (miss) keep that pair. Publish
                // the merged native words, not the authored tokens.
                const std::int32_t* words;
                std::uint32_t bytes;
                std::memcpy(&words, record + 0x20, sizeof(words));
                std::memcpy(&bytes, record + 0x24, sizeof(bytes));
                if (words == nullptr || bytes == 0 || bytes % 4 != 0) {
                    throw std::logic_error("unexpected merged type-10 payload");
                }
                value.integer_array.assign(words, words + bytes / 4);
                value.has_integer_array = true;
                break;
            }
            throw std::logic_error("unexpected merged scene property tag");
        }
        default:
            throw std::logic_error("unexpected merged scene property tag");
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
        if (same_key(value.type_letter, "FA")) {
            return value.has_float_array && !value.float_array.empty()
                && value.float_array.size() <= 0x3fffffffu;
        }
        if (same_key(value.type_letter, "V3A")) {
            return value.has_vector3_array && !value.vector3_array.empty()
                && value.vector3_array.size() <= 0x15555555u;
        }
        if (same_key(value.type_letter, "BIN")) {
            return value.has_byte_array && !value.byte_array.empty()
                && value.byte_array.size() <= 0x7fffffffu;
        }
        // Type 10 admits count in [1,3FFFFFFFh] (008EF3E0, 008F4F60).
        if (value.type_letter == "IA") {
            return value.has_integer_array && !value.integer_array.empty()
                && value.integer_array.size() <= 0x3fffffffu;
        }
        if (scene_reference_kind(value.type_letter) >= 0) {
            return value.has_reference && value.values.size() == 1
                && value.values.front().find('\0') == std::string::npos;
        }
        return (value.type_letter == "I" && value.has_integer)
            || (value.type_letter == "F" && value.has_float)
            || (value.type_letter == "B" && value.has_boolean)
            || (value.type_letter == "V3" && value.has_vector3)
            || (value.type_letter == "S" && value.has_string
                && value.values.size() == 1
                && value.values.front().find('\0') == std::string::npos);
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
        explicit Bag(NativeScenePropertyInsertionContext& context, bool owning_child = false)
            : context_(context), owning_child_(owning_child),
              root_(owning_child
                  ? static_cast<std::byte*>(singleton_lifetime_allocate(
                        {SingletonAllocationKind::object, native_scene_property_bag_storage_bytes,
                            native_scene_property_bag_storage_bytes}))
                  : storage_.data()) {
            if (root_ == nullptr) throw std::bad_alloc();
            // Real raw61 producer, fresh aligned root, no borrowed owner.
            initialize_native_scene_property_bag_storage_008f41a0(root_, 0, 0);
        }
        ~Bag() {
            // Stack root survives; nested records, pooled keys and physical
            // nodes retire in their SAME canonical processes before the context.
            if (root_ != nullptr) {
                if (owning_child_) {
                    scalar_delete_native_scene_property_bag_008f59e0(
                        root_, 1u, context_.lifetimes());
                } else {
                    destroy_native_scene_property_bag_008f5410(root_, context_.lifetimes());
                }
            }
        }
        Bag(const Bag&) = delete;
        Bag& operator=(const Bag&) = delete;
        void* data() { return root_; }
        void* release_owned() {
            if (!owning_child_) throw std::logic_error("cannot transfer a stack scene bag");
            void* result = root_;
            root_ = nullptr;
            return result;
        }
        bool empty() const {
            // The genuine raw61 map starts at bag+4; its live count is +4.
            std::uint32_t count;
            std::memcpy(&count, root_ + 8, sizeof(count));
            return count == 0;
        }
        void adopt_child(const std::string& key, Bag& child) {
            // bsp.py show 008F3B60: the whole producer writes child+110h and
            // adopts the canonical 114h allocation. It is neither a borrowed
            // std::array nor a second owner. Parent retirement frees it once.
            add_native_scene_property_subbag_008f3b60(data(), key.c_str(), child.data(), context_);
            child.root_ = nullptr;
        }
        void* child(const std::string& key) {
            return child_in(data(), key, context_);
        }
        static void* child_in(void* bag, const std::string& key,
            NativeScenePropertyInsertionContext& context) {
            const auto* record = static_cast<const std::byte*>(find_in(bag, key, context));
            std::uint32_t tag;
            std::memcpy(&tag, record + 4, sizeof(tag));
            if (tag != 6u) throw std::logic_error("unexpected merged scene child tag");
            void* child;
            std::memcpy(&child, record + 0x0c, sizeof(child));
            return child;
        }
        void insert(const SceneProperty& value) {
            if (same_key(value.type_letter, "FA")) {
                insert_float_array(value);
                return;
            }
            if (same_key(value.type_letter, "V3A")) {
                insert_vector3_array(value);
                return;
            }
            if (same_key(value.type_letter, "BIN")) {
                insert_byte_array(value);
                return;
            }
            {
                if (value.type_letter == "IA") {
                    insert_integer_array(value);
                    return;
                }
                const int kind = scene_reference_kind(value.type_letter);
                if (kind >= 0) {
                    // bsp.py show 008F5A00 / disasm 008F6231..008F6259: a new
                    // key's reference arm pushes word08 = the letter's kind
                    // (EDI), word18 = 0 and the quoted 008D9980 token, then
                    // 008F3AC0 duplicates that text into the type-5 +1Ch.
                    add_native_scene_property_reference_008f3ac0(data(), value.key.c_str(),
                        value.values.front().c_str(), 0u, static_cast<std::uint32_t>(kind),
                        context_);
                    return;
                }
            }
            if (value.type_letter == "I") {
                std::uint32_t bits;
                std::memcpy(&bits, &value.integer_value, sizeof(bits));
                add_native_scene_property_integer_word_008f3710(data(), value.key.c_str(), bits, context_);
            } else if (value.type_letter == "F") {
                std::uint32_t bits;
                std::memcpy(&bits, &value.float_value, sizeof(bits));
                add_native_scene_property_float_bits_008f3770(data(), value.key.c_str(), bits, context_);
            } else if (value.type_letter == "B") {
                add_native_scene_property_bool_byte_008f3940(
                    data(), value.key.c_str(), value.boolean_value, context_);
            } else if (value.type_letter == "S") {
                // bsp.py show 008F38A0: the whole producer duplicates
                // this borrowed decoded value into its owning type-2 root.
                add_native_scene_property_string_008f38a0(
                    data(), value.key.c_str(), value.values.front().c_str(), context_);
            } else {
                // Source API reuse of 008F37E0's by-value XYZ entry:
                // these are the same real parser-owned V3 lanes that
                // the pointer entry below consumed. Its result owns
                // the one record in this actual bag; merge/clone and
                // subsequent class readers consume that record.
                std::uint32_t bits[3];
                std::memcpy(bits, value.vector3.data(), sizeof(bits));
                cc13_seg59b::scene_property_bag_add_vec3_008f37e0(
                    context_, data(), value.key.c_str(), bits[0], bits[1], bits[2]);
            }
        }
        // Chain 319. bsp.py show 008F5A00 --asm, 008F5F94..008F5FCF: a new
        // key's E arm passes (key, EDI = 0048E960's CEnum, the 008D9980 symbol)
        // to 008F3A20, which resolves the symbol through 0048E840 into +0Ch.
        void insert_enum(const SceneProperty& value, const void* enum_owner) {
            if (value.has_enum_value) {
                // bsp.py show 008F39B0: +28h is the real declaration and
                // +0Ch is the supplied word. A previously merged property
                // already owns that word: preserve it if a later library
                // declaration has changed the symbol's current mapping.
                std::uint32_t bits;
                std::memcpy(&bits, &value.enum_value, sizeof(bits));
                add_native_scene_property_enum_word_008f39b0(data(), value.key.c_str(),
                    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(enum_owner)),
                    bits, context_);
            } else {
                add_native_scene_property_enum_symbol_008f3a20(data(), value.key.c_str(), enum_owner,
                    value.values[2].c_str(), game_native_string_process(), context_);
            }
        }
        void* find(const std::string& key) {
            return find_in(data(), key, context_);
        }
        static void* find_in(void* bag, const std::string& key,
            NativeScenePropertyInsertionContext& context) {
            NativeString query;
            construct_native_string_header_0041e870(&query, context.lifetimes().keys(), key.c_str());
            std::uint32_t bucket;
            const auto* node = static_cast<const std::byte*>(find_native_property_map_node_0043b8b0(
                static_cast<std::byte*>(bag) + 4, &query, bucket,
                context.property_empty(), context.query_empty()));
            void* record = nullptr;
            if (node != nullptr) std::memcpy(&record, node + 8, sizeof(record));
            destroy_native_string_header_0041dd20(&query, context.lifetimes().keys());
            if (record == nullptr) throw std::logic_error("missing merged numeric property");
            return record;
        }
        void replace_reference(const SceneProperty& value) {
            // bsp.py show 008F0420 --asm: free/duplicate owned +1Ch,
            // then store the full scalar +18h and reference kind +08h.
            // Its declared-reference parser call at 008F60C1 supplies the
            // authored target, scalar0 and the letter's kind, just as the
            // existing 008F3AC0 producer does for an absent declaration.
            // Here that producer made the actual type-5 record and sole-owned
            // current-heap name. The separate parser-owned token survives the
            // setter's free, and result() reads the replacement before bag
            // retirement. Same-letter admission preserves the merge's kind.
            set_native_scene_property_reference_008f0420(find(value.key), nullptr,
                value.values.front().c_str(), 0u,
                static_cast<std::uint32_t>(scene_reference_kind(value.type_letter)));
        }
        void replace_float_array(const SceneProperty& value) {
            // bsp.py show 008F5A00 --asm 008F647E..008F64A0 frees old +20h
            // and adopts the new 4*count buffer. Reuse the existing complete
            // 008F04C0 flags-0 body for these identical stores (Source reuse,
            // not a recovered CALL at this inlined parser site). The actual
            // type-9 record was built by 008F3510/008EF360, and 008F03F0
            // retires the sole-owned replacement. Do not also merge this key.
            void* const record = find(value.key);
            const std::size_t bytes = value.float_array.size() * sizeof(float);
            void* const payload = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, bytes, bytes});
            if (payload == nullptr) throw std::bad_alloc();
            std::memcpy(payload, value.float_array.data(), bytes);
            reassign_native_scene_property_four_byte_groups_008f04c0(record,
                nullptr, static_cast<std::uint32_t>(value.float_array.size()), payload, 0u);
        }
        void replace_owned_array(const SceneProperty& value) {
            // bsp.py show 008F0460 / 008F0520 / 008F0580: replace +20h/+24h,
            // freeing the old allocation before adopting the new flags-0 one.
            // The insert_*_array paths built these actual type8/10/11 raw56
            // records with unique current-heap +20h allocations. For V3A,
            // 008F5A00 --asm 008F6676..008F6684 calls 008F0580 directly;
            // for BIN/IA, the parser inlines the same free/size/store steps
            // (008F63A0..008F63C9 / 008F654A..008F6570). 008F0520 is explicit
            // Source reuse of the opaque count*4 body, not a recovered IA call.
            // Each setter frees that allocation and adopts this separate one;
            // the bag's existing 008F03F0 retirement releases the replacement.
            // Publication reads this record's +20h/+24h, including changed
            // counts and the exact authored bits (no float math).
            static_assert(sizeof(std::array<float, 3>) == 12);
            static_assert(sizeof(std::int32_t) == 4);
            void* const record = find(value.key);
            const bool vector3 = same_key(value.type_letter, "V3A");
            const bool integers = value.type_letter == "IA";
            const std::size_t count = vector3 ? value.vector3_array.size()
                : integers ? value.integer_array.size() : value.byte_array.size();
            const std::size_t bytes = count * (vector3 ? 12u : integers ? 4u : 1u);
            const void* const input = vector3 ? static_cast<const void*>(value.vector3_array.data())
                : integers ? static_cast<const void*>(value.integer_array.data())
                : static_cast<const void*>(value.byte_array.data());
            void* const payload = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, bytes, bytes});
            if (payload == nullptr) throw std::bad_alloc();
            std::memcpy(payload, input, bytes);
            if (vector3) {
                reassign_native_scene_property_twelve_byte_groups_008f0580(record,
                    nullptr, static_cast<std::uint32_t>(count), payload, 0u);
            } else if (integers) {
                reassign_native_scene_property_four_byte_groups_008f0520(record,
                    nullptr, static_cast<std::uint32_t>(count), payload, 0u);
            } else {
                reassign_native_scene_property_raw_bytes_008f0460(record,
                    nullptr, payload, static_cast<std::uint32_t>(bytes), 0u);
            }
        }
    private:
        void insert_float_array(const SceneProperty& value) {
            // 008F644C allocates 4*count and 008F64B9 transfers that buffer
            // to 008F3510 with flags 0. Give its type-9 owner a canonical
            // allocation, never std::vector's borrowed backing storage.
            static_assert(sizeof(float) == 4);
            const std::size_t bytes = value.float_array.size() * sizeof(float);
            void* const payload = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, bytes, bytes});
            if (payload == nullptr) throw std::bad_alloc();
            std::memcpy(payload, value.float_array.data(), bytes);
            add_native_scene_property_four_byte_array_008f3510(data(), value.key.c_str(), payload,
                static_cast<std::uint32_t>(value.float_array.size()), 0u, context_);
        }
        void insert_vector3_array(const SceneProperty& value) {
            // 008F65DE..008F669A builds an operator-new buffer of 12*count
            // and transfers it to 008F3610 with flags 0. The type-11 root
            // retains that sole-owned allocation, later freed by 008F03F0;
            // never transfer std::vector's borrowed payload to this owner.
            static_assert(sizeof(std::array<float, 3>) == 12);
            const std::size_t bytes = value.vector3_array.size() * 12;
            void* const payload = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, bytes, bytes});
            if (payload == nullptr) throw std::bad_alloc();
            std::memcpy(payload, value.vector3_array.data(), bytes);
            add_native_scene_property_type11_array_008f3610(data(), value.key.c_str(), payload,
                static_cast<std::uint32_t>(value.vector3_array.size()), 0u, context_);
        }

        void insert_byte_array(const SceneProperty& value) {
            // bsp.py show 008F5A00 --asm, 008F6375/008F63CE..008F63DF:
            // operator-new(count) bytes pass to 008F3490 with flags 0. Its
            // 008EF2F0 constructor retains that allocation, so transfer an
            // actual canonical allocation, not the vector's borrowed storage.
            const std::size_t bytes = value.byte_array.size();
            void* const payload = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, bytes, bytes});
            if (payload == nullptr) throw std::bad_alloc();
            std::memcpy(payload, value.byte_array.data(), bytes);
            add_native_scene_property_byte_array_008f3490(data(), value.key.c_str(), payload,
                static_cast<std::uint32_t>(bytes), 0u, context_);
        }

        void insert_integer_array(const SceneProperty& value) {
            // bsp.py show 008F5A00 / disasm 008F650C..008F658A: the parser's
            // 008F6524 operator-new buffer of 4*count holds the 008D9AD0 words
            // and 008F6585 passes (key, data, count, flags 0) to 008F3590, so
            // 008EF3E0 RETAINS that exact allocation at +20h. Mirror it: one
            // canonical current allocation (the provider 008F03F0 frees) owns
            // a copy of the parser's words and transfers with the root.
            const std::size_t bytes = value.integer_array.size() * sizeof(std::int32_t);
            void* const payload = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, bytes, bytes});
            if (payload == nullptr) throw std::bad_alloc();
            std::memcpy(payload, value.integer_array.data(), bytes);
            add_native_scene_property_type10_array_008f3590(data(), value.key.c_str(), payload,
                static_cast<std::uint32_t>(value.integer_array.size()), 0u, context_);
        }

        NativeScenePropertyInsertionContext& context_;
        alignas(std::uint32_t) std::array<std::byte, native_scene_property_bag_storage_bytes> storage_;
        bool owning_child_;
        std::byte* root_;
    };

    void copy_or_merge(Bag& destination, Bag& source, bool overwrite) {
        if (destination.empty()) {
            // bsp.py show 008F5470 --asm: 008F5474 clears the OUTER bag's
            // +4 map; 008F54B0..008F54C3 clones and inserts each source record.
            // An empty destination has no collision, so keep/overwrite agree.
            // 008F4F60 also copies +34h (the merge's extra ordinal store is
            // redundant here). Both bags are actual raw61 owners, disjoint,
            // and use this same canonical clone/insertion/lifetime context.
            reset_native_scene_property_bag_008f5470(destination.data(), source.data(), context_);
        } else {
            // Populated destinations retain the original recursive merge and
            // its keep flag; they must never be cleared by the copy routine.
            merge_native_scene_property_bag_008f54f0(destination.data(), source.data(),
                overwrite ? 0u : 1u, context_);
        }
    }

    // Chain 319: the genuine CEnum for a closed E value, or null when there is
    // no registry, the value is not the closed form, or the registry does not
    // hold both its table and its symbol.
    const void* enum_owner(const SceneProperty& value) const {
        if (enums_ == nullptr || value.type_letter != "E" || !value.has_enum
            || value.values.size() != 3) return nullptr;
        for (const std::string& token : value.values) {
            if (std::any_of(token.begin(), token.end(),
                    [](unsigned char c) { return c == 0 || c >= 0x80; })) return nullptr;
        }
        return enums_->known_enum_owner(value.values[0], value.values[2]);
    }
    // supported() plus, under chain 319, the closed known E form.
    bool admitted(const SceneProperty& value) const {
        if (value.type_letter == "E") {
            return !std::any_of(value.key.begin(), value.key.end(),
                       [](unsigned char c) { return c == 0 || c >= 0x80; })
                && enum_owner(value) != nullptr;
        }
        return supported(value);
    }
    void insert_into(Bag& bag, const SceneProperty& value) {
        if (value.type_letter == "E") {
            bag.insert_enum(value, enum_owner(value));
            return;
        }
        bag.insert(value);
    }

    static bool closed_key(const std::string& key) {
        return key.size() < 0x7fffffffu
            && !std::any_of(key.begin(), key.end(),
                [](unsigned char c) { return c == 0 || c >= 0x80; });
    }

    static const SceneProperty* named_value(const ScenePropertyBlock& block,
        const std::string& key) {
        const auto found = std::find_if(block.values.begin(), block.values.end(),
            [&](const SceneProperty& value) { return same_key(value.key, key); });
        return found == block.values.end() ? nullptr : &*found;
    }

    static const ScenePropertyBlock* named_child(const ScenePropertyBlock& block,
        const std::string& key) {
        const auto found = std::find_if(block.blocks.begin(), block.blocks.end(),
            [&](const auto& child) { return same_key(child.first, key); });
        return found == block.blocks.end() ? nullptr : &found->second;
    }

    bool closed_tree(const ScenePropertyBlock& block) const {
        for (const auto& value : block.values) {
            if (!admitted(value) || occurrences(block, value.key) != 1
                || has_child(block, value.key)) return false;
        }
        for (const auto& child : block.blocks) {
            if (!closed_key(child.first) || occurrences(block, child.first) != 0
                || std::count_if(block.blocks.begin(), block.blocks.end(),
                    [&](const auto& other) { return same_key(child.first, other.first); }) != 1
                || !closed_tree(child.second)) return false;
        }
        return true;
    }

    static bool compatible_trees(const ScenePropertyBlock& destination,
        const ScenePropertyBlock& source) {
        for (const auto& value : source.values) {
            const SceneProperty* prior = named_value(destination, value.key);
            if (has_child(destination, value.key)
                || (prior != nullptr && prior->type_letter != value.type_letter)) return false;
        }
        for (const auto& child : source.blocks) {
            const ScenePropertyBlock* prior = named_child(destination, child.first);
            if (named_value(destination, child.first) != nullptr
                || (prior != nullptr && !compatible_trees(*prior, child.second))) return false;
        }
        return true;
    }

    void attach_tree(Bag& parent, const std::string& key, const ScenePropertyBlock& block) {
        // Parser evidence: 008F66B9..008F6747 constructs a fresh raw61 child,
        // attaches it through 008F3B60, then recurses. This projection already
        // has the closed parser-owned subtree, so fills it before attachment;
        // it supplies no parser allocation/observation timing equivalence.
        // Attachment transfers the same heap root with its initial backlink0.
        Bag child(context_.insertion(), true);
        for (const auto& value : block.values) insert_into(child, value);
        for (const auto& nested : block.blocks) attach_tree(child, nested.first, nested.second);
        parent.adopt_child(key, child);
    }

    ScenePropertyBlock read_tree(void* actual_bag, const ScenePropertyBlock* destination,
        const ScenePropertyBlock& source) {
        ScenePropertyBlock result = destination != nullptr ? *destination : ScenePropertyBlock{};
        for (const auto& value : source.values) {
            const auto found = std::find_if(result.values.begin(), result.values.end(),
                [&](const SceneProperty& prior) { return same_key(prior.key, value.key); });
            const SceneProperty& metadata = found != result.values.end() && !overwrite_
                ? *found : value;
            SceneProperty merged = read_value(metadata,
                Bag::find_in(actual_bag, value.key, context_.insertion()));
            if (found == result.values.end()) result.values.push_back(std::move(merged));
            else *found = std::move(merged);
        }
        for (const auto& child : source.blocks) {
            ScenePropertyBlock merged = read_tree(
                Bag::child_in(actual_bag, child.first, context_.insertion()),
                destination != nullptr ? named_child(*destination, child.first) : nullptr,
                child.second);
            const auto found = std::find_if(result.blocks.begin(), result.blocks.end(),
                [&](const auto& prior) { return same_key(prior.first, child.first); });
            if (found == result.blocks.end()) result.blocks.emplace_back(child.first, std::move(merged));
            else found->second = std::move(merged);
        }
        return result;
    }

    NativeScenePropertyCloneContext context_;
    Bag destination_;
    Bag source_;
    std::vector<bool> selected_;
    SceneNativeEnumSource* enums_;
    bool overwrite_;
    std::optional<MergedRecords> merged_records_{};
    bool merged_records_collected_{};
};

SceneNumericPropertyMerge::SceneNumericPropertyMerge(const ScenePropertyBlock& destination,
    const ScenePropertyBlock& source, bool overwrite, SceneNativeEnumSource* enums)
    : impl_(std::make_unique<Impl>(destination, source, overwrite, enums)) {}

SceneNumericPropertyMerge::~SceneNumericPropertyMerge() = default;

bool SceneNumericPropertyMerge::handles(std::size_t source_index) const {
    return impl_->handles(source_index);
}

SceneProperty SceneNumericPropertyMerge::result(const SceneProperty& authored_metadata) {
    return impl_->result(authored_metadata);
}

bool SceneNumericPropertyMerge::merge_child(const std::string& key,
    const ScenePropertyBlock* destination, const ScenePropertyBlock& source,
    ScenePropertyBlock& result) {
    return impl_->merge_child(key, destination, source, result);
}

void* SceneNumericPropertyMerge::make_owned_complete_bag(const ScenePropertyBlock& source) {
    return impl_->make_owned_complete_bag(source);
}

ScenePropertyBlock SceneNumericPropertyMerge::read_complete_bag(void* actual_bag,
    const ScenePropertyBlock& metadata) {
    return impl_->read_complete_bag(actual_bag, metadata);
}

} // namespace bsp::game
