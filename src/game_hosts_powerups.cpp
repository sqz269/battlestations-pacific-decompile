// Packet cc13-powerup-bind: the power-up category lists and their product. See
// include/bsp/game_hosts_powerups.hpp. Re-derived from the image bytes
// (disasm-raw); the corresponding modifier copies in powerup_seg58_argon.cpp
// were a cross-check only. Chain 339 below calls its active-item collector.
// Names are hypotheses, not recovered symbols.

#include "bsp/game_hosts_powerups.hpp"

#include "bsp/cc13_powerup.hpp"
#include "bsp/cc13_sweepA.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/powerup_session_broadcast_cc13.hpp"
#include "bsp/wire_goal4_chain310_switches.hpp"
#include "bsp/powerup_seg58_argon.hpp"

#include <cstddef>
#include <cstring>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game_powerups {

namespace {

const PowerupUnitResolver* g_unit_resolver = nullptr;

// Goal 4 chain 339: read-only native views for 008E9AF0. Each node retains
// its actual inventory item; class descriptors already have the E4h layout.
// Only fields read by this collector are projected. Its item+8h binding and
// modifier list are not dereferenced and are not reconstructed here.
struct CollectionNode {
    CollectionNode* next{};
    CollectionNode* previous{};
    std::uint32_t word{};
    const PowerupClassConfig* cls{};
    void* unused_binding{};
    std::int32_t uses{};
    const cc13_powerup::PowerupItem* source{};
};
struct CollectionList {
    std::uint32_t unused{};
    CollectionNode* head{};
    std::uint32_t size{};
};
struct CollectionManager {
    std::uint8_t unused_00[0x20]{};
    std::array<CollectionList, 8> lists{};
    std::uint8_t unused_80[0xcc]{};
    std::array<std::array<std::uint8_t, 0x0c>, 8> maps{};
};
struct CollectionPlayer {
    std::uint8_t unused[0x28]{};
    std::int32_t party{};
};
struct CollectionGame {
    std::uint8_t unused[0x18cc]{};
    std::array<CollectionPlayer*, 8> players{};
    std::int32_t local_slot{};
};
struct CollectionCooldown {
    std::uint32_t notified{};
    std::uint32_t unused{};
    void* source{};
    float start{};
    float end{};
    std::uint8_t relative{};
};
static_assert(sizeof(void*) == 4);
static_assert(offsetof(CollectionNode, word) == 8);
static_assert(offsetof(CollectionNode, uses) == 0x14);
static_assert(sizeof(CollectionList) == 0x0c);
static_assert(offsetof(CollectionManager, maps) == 0x14c);
static_assert(offsetof(CollectionGame, local_slot) == 0x18ec);
static_assert(offsetof(CollectionCooldown, relative) == 0x14);
static_assert(offsetof(PowerupClassConfig, group_08) == 8);

// Live uint32_t objects occupy the native capacity, so direct stores by the
// body and append callbacks share one buffer and the same last pointer.
struct CollectionVector {
    std::uint32_t unused{};
    std::uint32_t* first{};
    std::uint32_t* last{};
    std::uint32_t* capacity{};
    std::vector<std::uint32_t> storage;

    void append(const void* value) {
        const std::size_t count = size();
        if (count == storage.size()) storage.resize(count == 0 ? 4 : count * 2);
        first = storage.data();
        last = first + count;
        capacity = first + storage.size();
        std::memcpy(last++, value, sizeof(std::uint32_t));
    }
    std::size_t size() const {
        return first == nullptr ? 0u : static_cast<std::size_t>(last - first);
    }
};
static_assert(offsetof(CollectionVector, first) == 4);
static_assert(offsetof(CollectionVector, last) == 8);
static_assert(offsetof(CollectionVector, capacity) == 0x0c);

class PowerupCollectionBinding final : public powerup_seg58_argon::PowerupCollectionHost,
    public powerup_seg58_argon::PowerupUsableInventoryHost {
public:
    explicit PowerupCollectionBinding(cc13_powerup::PowerupRuntime& runtime,
        bool include_disabled_inventory = false)
        : runtime_(runtime), enabled_(runtime.host->powerups_enabled_00e0c978() ? 1 : 0),
          clock_(runtime.host->clock_00f876a4()) {
        // The image returns after clearing its outputs when EnablePowerups is
        // clear; it does not inspect a player or an inventory on that path.
        if (enabled_ == 0 && !include_disabled_inventory) return;
        game_.local_slot = runtime.host->local_player_slot_18ec();
        if (game_.local_slot < 0 || game_.local_slot >= 8)
            throw std::logic_error("008E9AF0 local player outside 0..7");
        for (std::size_t slot = 0; slot < heads_.size(); ++slot) {
            // 004C6890's published scene parties; an unauthored Party is -1.
            players_[slot].party = static_cast<int>(slot) == game_.local_slot
                ? game::local_player_party() : game::scene_slot_party(static_cast<int>(slot), -1);
            game_.players[slot] = &players_[slot];
            CollectionNode& head = heads_[slot];
            auto& nodes = nodes_[slot];
            const auto& inventory = runtime.inventories[slot];
            nodes.resize(inventory.size());
            std::size_t index = 0;
            for (const auto& item : inventory) {
                if (item.cls == nullptr) throw std::logic_error("008E9AF0 item without class");
                CollectionNode& node = nodes[index];
                node.next = index + 1 == nodes.size() ? &head : &nodes[index + 1];
                node.previous = index == 0 ? &head : &nodes[index - 1];
                node.word = item.word_00;
                node.cls = item.cls;
                node.uses = item.use_limit;
                node.source = &item;
                ++index;
            }
            head.next = nodes.empty() ? &head : &nodes.front();
            head.previous = nodes.empty() ? &head : &nodes.back();
            manager_.lists[slot].head = &head;
            manager_.lists[slot].size = static_cast<std::uint32_t>(nodes.size());
        }
        game_word_ = reinterpret_cast<std::uint32_t>(&game_);
    }

    bool usable() {
        return powerup_seg58_argon::power_up_manager_local_player_has_usable_item_008ed9c0(
            *this, &manager_) != 0;
    }
    // bsp.py show 008ED9C0: copy list (+20h + localSlot*0Ch), then read node
    // class+8h and node+14h, finally clear nodes and free the sentinel on either
    // return. These callbacks own a distinct snapshot of the real inventory.
    void* stl_list_copy_construct_008ed660(void* dst, const void* src) override {
        const CollectionList* source = nullptr;
        for (const auto& list : manager_.lists) if (&list == src) source = &list;
        if (source == nullptr || copy_live_) invalid_parameter_noinfo_00bf6713();
        copy_nodes_.clear();
        copy_nodes_.reserve(source->size);
        for (auto* node = source->head->next; node != source->head; node = node->next)
            copy_nodes_.push_back(*node);
        for (std::size_t i = 0; i < copy_nodes_.size(); ++i) {
            copy_nodes_[i].next = i + 1 == copy_nodes_.size() ? &copy_head_ : &copy_nodes_[i + 1];
            copy_nodes_[i].previous = i == 0 ? &copy_head_ : &copy_nodes_[i - 1];
        }
        copy_head_.next = copy_nodes_.empty() ? &copy_head_ : &copy_nodes_.front();
        copy_head_.previous = copy_nodes_.empty() ? &copy_head_ : &copy_nodes_.back();
        const CollectionList copy{0, &copy_head_, static_cast<std::uint32_t>(copy_nodes_.size())};
        std::memcpy(dst, &copy, sizeof(copy));
        copy_live_ = true;
        copy_list_ = dst;
        return dst;
    }
    void stl_inst_008e7da0(void* list) override {
        if (!copy_live_ || list != copy_list_) invalid_parameter_noinfo_00bf6713();
        copy_nodes_.clear();
        copy_head_.next = copy_head_.previous = &copy_head_;
        const CollectionList empty{0, &copy_head_, 0};
        std::memcpy(list, &empty, sizeof(empty));
    }
    void free_00bf65ac(void* ptr) override {
        if (!copy_live_ || ptr != &copy_head_ || !copy_nodes_.empty())
            invalid_parameter_noinfo_00bf6713();
        copy_live_ = false;
        copy_list_ = nullptr;
    }

    void collect(HudPowerupEntries& entries) {
        entries.clock = clock_;
        // 00649A36: five output vectors followed by the image's literal 0, 1.
        powerup_seg58_argon::power_up_manager_collect_active_008e9af0(*this, &manager_,
            &vectors_[0], &vectors_[1], &vectors_[2], &vectors_[3], &vectors_[4], 0, 1);
        for (std::size_t i = 0; i < vectors_[0].size(); ++i)
            entries.untimed.push_back(source_item(vectors_[0].first[i]));
        for (std::size_t i = 0; i < vectors_[1].size(); ++i)
            entries.timed.push_back(source_item(vectors_[1].first[i]));
        for (std::size_t i = 0; i < vectors_[2].size(); ++i) {
            std::int32_t count;
            std::memcpy(&count, &vectors_[2].first[i], sizeof(count));
            entries.counts.push_back(count);
        }
        copy_floats(vectors_[3], entries.cooldown_ends);
        copy_floats(vectors_[4], entries.deadlines);
    }
    std::uint32_t& global_u32(std::uint32_t address) override {
        if (address != 0x00e188a8u) throw std::logic_error("008E9AF0 unexpected dword global");
        return game_word_;
    }
    std::uint8_t& global_u8(std::uint32_t address) override {
        if (address != 0x00e0c978u) throw std::logic_error("008E9AF0 unexpected byte global");
        return enabled_;
    }
    float& global_f32(std::uint32_t address) override {
        if (address != 0x00f876a4u) throw std::logic_error("008E9AF0 unexpected clock global");
        return clock_;
    }
    void invalid_parameter_noinfo_00bf6713() override {
        throw std::logic_error("008E9AF0 invalid checked container");
    }
    void* stl_map_subscript_00617030(void* map, const void* key) override {
        for (std::size_t slot = 0; slot < manager_.maps.size(); ++slot) {
            if (map != manager_.maps[slot].data()) continue;
            const PowerupClassConfig* cls;
            std::memcpy(&cls, key, sizeof(cls));
            const PowerUpSlot& record = cc13_powerup::slot_record_00617030(
                runtime_, static_cast<std::int32_t>(slot), cls);
            cooldown_.notified = record.notified ? 1u : 0u;
            cooldown_.source = record.source;
            cooldown_.start = record.absolute_start;
            cooldown_.end = record.absolute_expiry;
            cooldown_.relative = record.relative_deadline ? 1u : 0u;
            if (record.relative_deadline && record.source == nullptr)
                throw std::logic_error("008E9AF0 relative cooldown without source");
            return &cooldown_;
        }
        throw std::logic_error("008E9AF0 foreign cooldown map");
    }
    void* stl_vector_erase_range_004685a0(void* v, void* out, void* a, void* first, void* b, void* last) override {
        return clear(v, out, a, first, b, last);
    }
    void* stl_vector_erase_range_00488bf0(void* v, void* out, void* a, void* first, void* b, void* last) override {
        return clear(v, out, a, first, b, last);
    }
    void* stl_vector_erase_range_005a0be0(void* v, void* out, void* a, void* first, void* b, void* last) override {
        return clear(v, out, a, first, b, last);
    }
    void* stl_vector_insert_one_00441f30(void* v, void* out, void* owner, void* pos, const void* value) override {
        return insert(v, out, owner, pos, value);
    }
    void* stl_vector_insert_one_005a6580(void* v, void* out, void* owner, void* pos, const void* value) override {
        return insert(v, out, owner, pos, value);
    }
    void* stl_checked_float_vector_insert_one_0045a120(void* v, void* out, void* owner, void* pos, const void* value) override {
        return insert(v, out, owner, pos, value);
    }
    void stl_vector_push_back_00442190(void* v, const void* value) override { vector(v).append(value); }
    void stl_vector_push_back_0045afd0(void* v, const void* value) override { vector(v).append(value); }
    void stl_vector_push_back_005a7960(void* v, const void* value) override { vector(v).append(value); }

private:
    CollectionVector& vector(void* address) {
        for (auto& candidate : vectors_) if (&candidate == address) return candidate;
        throw std::logic_error("008E9AF0 foreign vector");
    }
    void* clear(void* v, void* out, void* a, void* first, void* b, void* last) {
        CollectionVector& vec = vector(v);
        if (a != v || b != v || first != vec.first || last != vec.last)
            invalid_parameter_noinfo_00bf6713();
        vec.last = vec.first;
        void* result[2]{v, vec.last};
        std::memcpy(out, result, sizeof(result));
        return out;
    }
    void* insert(void* v, void* out, void* owner, void* pos, const void* value) {
        CollectionVector& vec = vector(v);
        if (owner != v || pos != vec.last) invalid_parameter_noinfo_00bf6713();
        vec.append(value);
        void* result[2]{v, vec.last - 1};
        std::memcpy(out, result, sizeof(result));
        return out;
    }
    static const cc13_powerup::PowerupItem* source_item(std::uint32_t item) {
        return reinterpret_cast<const CollectionNode*>(item - 8u)->source;
    }
    static void copy_floats(const CollectionVector& vec, std::vector<float>& out) {
        out.resize(vec.size());
        if (!out.empty()) std::memcpy(out.data(), vec.first, out.size() * sizeof(float));
    }
    cc13_powerup::PowerupRuntime& runtime_;
    CollectionManager manager_;
    CollectionGame game_;
    std::array<CollectionPlayer, 8> players_{};
    std::array<CollectionNode, 8> heads_{};
    std::array<std::vector<CollectionNode>, 8> nodes_{};
    std::array<CollectionVector, 5> vectors_{};
    CollectionNode copy_head_{};
    std::vector<CollectionNode> copy_nodes_;
    void* copy_list_{};
    bool copy_live_{};
    CollectionCooldown cooldown_;
    std::uint32_t game_word_{};
    std::uint8_t enabled_{};
    float clock_{};
};

// [00CE6A04], 1000000.0f: what 006F58D0, slot 0 of the command-building binding
// vtable 00D16194, loads. 008E477E..008E4786 reach it as the centre unit's
// vtable[64h]() (its binding at +758h) then that object's slot 0.
// PROVISIONAL: a centre id names the binding's building, the only unit kind
// whose vtable[64h] is not the null 0042B920 of the root vtable 00CE6290.
constexpr float kBindingRadiusSquared = 1000000.0f;

}  // namespace

void collect_hud_powerups(HudPowerupEntries& entries) {
    entries = {};
    auto& runtime = cc13_powerup::powerup_runtime();
    if (runtime.manager == nullptr || runtime.host == nullptr) return;
    PowerupCollectionBinding binding(runtime);
    binding.collect(entries);
}

bool local_player_has_usable_powerup() {
    auto& runtime = cc13_powerup::powerup_runtime();
    if (runtime.manager == nullptr || runtime.host == nullptr) return false;
    // Unlike 008E9AF0, 008ED9C0 has no EnablePowerups early return.
    PowerupCollectionBinding binding(runtime, true);
    return binding.usable();
}

// [BSP reconstruction 008e4680]
// Original ABI: __thiscall(ECX = manager, const record* = node+8h, unit*), RET 8;
// returns AL.
bool powerup_target_applies_to_unit_008e4680(const PowerupModifier& record,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver) {
    // 008E4689..008E46D3: the unit-id arm, only for a live unit. 008E46B9 calls
    // vtable[140h] (0047F320 in the root vtable: MOV EAX,ECX) and compares its
    // +174h word with record+0Ch.
    bool matched = record.unit_id != 0 && unit.live && record.unit_id == unit.id;
    // 008E46D5..008E46EB: the party arm, a signed record+8h below 2 equal to
    // unit+54h (read with no liveness test).
    if (record.party < 2 && record.party == unit.party) matched = true;
    // 008E46ED..008E4708: the slot arm, 009FFD20(unit) when record+10h is not -1.
    if (record.slot != -1 && record.slot == unit.slot) matched = true;
    if (matched) return true;  // 008E470A JNE 008E47CE
    // 008E4710..008E4717: no centre, no match.
    if (record.centre_id == 0) return false;
    // 008E471D..008E4755: the id table lookup; a missing unit fails.
    PowerupUnitView centre;
    if (!resolver.view_unit_by_id(record.centre_id, centre)) return false;
    // 008E4757..008E4775: a live centre on the unit's party.
    if (!centre.live || centre.party != unit.party) return false;
    // 008E4777..008E47A9: the radius (FSTP qword, kBindingRadiusSquared above),
    // then 00427C90 (centre position minus unit position, three float stores)
    // and 00427E30, the squared length (y*y + x*x) + z*z, FSTP to float at
    // 00427E49. Per x87 operation at the main thread's 24-bit precision.
    namespace x8 = bsp::cc13x87;
    const float dx = centre.position[0] - unit.position[0];
    const float dy = centre.position[1] - unit.position[1];
    const float dz = centre.position[2] - unit.position[2];
    const float length_sq = static_cast<float>(x8::x87_add24(
        x8::x87_add24(x8::x87_mul24(dy, dy), x8::x87_mul24(dx, dx)),
        x8::x87_mul24(dz, dz)));
    // 008E47AE..008E47B6: FLD radius; FCOMI ST(1); JBE fails. A match is
    // radius > length_sq.
    return static_cast<double>(kBindingRadiusSquared) > static_cast<double>(length_sq);
}

// [BSP reconstruction 008e6430]
// Original ABI: __thiscall(ECX = manager, category, unit*), RET 8; ST0 result.
float gameplay_modifier_product_008e6430(const std::list<PowerupModifier>& list,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver) {
    // 008E6435: the product starts at 1.0f (00D7A24C); 008E6460..008E6490 walk
    // the list from the head's next in order; each accepted record's +14h
    // (node+1Ch) multiplies it, FSTP to the float slot at 008E6480.
    float product = 1.0f;
    for (const PowerupModifier& record : list) {
        if (powerup_target_applies_to_unit_008e4680(record, unit, resolver)) {
            product = record.multiplier * product;
        }
    }
    return product;
}

namespace {

// The sites' gate (00470440 shows it whole): CMP BYTE [00E0C978],0, then the
// size at [00F88C30]+88h+category*0Ch. The host has no manager when the class
// registry did not load; its lists are then empty. Null when the gate is shut.
const std::list<PowerupModifier>* gated_list(int category) {
    cc13_powerup::PowerupRuntime& runtime = cc13_powerup::powerup_runtime();
    if (runtime.manager == nullptr || runtime.host == nullptr ||
        !runtime.host->powerups_enabled_00e0c978() || category < 0 ||
        category >= kPowerupCategoryCount) {
        return nullptr;
    }
    const std::list<PowerupModifier>& list =
        runtime.category_lists[static_cast<std::size_t>(category)];
    return list.empty() ? nullptr : &list;
}

// 00470440 only reads the enable byte and manager+88h+category*0Ch. This
// read-only layout projection copies those sizes from the actual manager's
// lists; it is not a second modifier store. The lookup below uses those same
// lists, and resolves the actual queried unit only if the body accepts the gate.
// Evidence: bsp.py show 00470440, CALL/JMP target 008E6430. The original unit
// argument is only forwarded, so a typed query can carry the existing host's
// unit index or its in-construction PowerupUnitView without a fabricated unit.
struct PowerupQueryUnit {
    std::size_t index{};
    const PowerupUnitView* view{};
};

class PowerupMultiplierBinding final : public cc13_sweepA::PowerupMultiplierHost {
public:
    explicit PowerupMultiplierBinding(cc13_powerup::PowerupRuntime& runtime)
        : runtime_(runtime), enabled_(runtime.host->powerups_enabled_00e0c978() ? 1 : 0) {
        for (std::size_t category = 0; category < runtime.category_lists.size(); ++category) {
            counts_[(0x88 + category * 0x0c) / sizeof(std::uint32_t)] =
                static_cast<std::uint32_t>(runtime.category_lists[category].size());
        }
    }

    std::optional<float> product(int category, PowerupQueryUnit& unit) {
        const float value = cc13_sweepA::gameplay_modifiers_powerup_multiplier_00470440(
            cc13_sweepA::Raw{reinterpret_cast<std::uint8_t*>(counts_.data())},
            category, &unit, *this);
        // The readers' nullopt means to keep their existing 1.0f arm. Only
        // 00470440's 008E6430 arm changes that optional to a product value.
        return looked_up_ ? std::optional<float>{value} : std::nullopt;
    }

    void* global_address(std::uint32_t address) override {
        if (address != 0x00e0c978u) {
            throw std::logic_error("00470440 requested an unrelated global");
        }
        return &enabled_;
    }

    float gameplay_modifiers_lookup_008e6430(cc13_sweepA::Raw,
        std::int32_t category, void* opaque_unit) override {
        const auto& query = *static_cast<const PowerupQueryUnit*>(opaque_unit);
        PowerupUnitView resolved;
        const PowerupUnitView* unit = query.view;
        if (unit != nullptr) {
            if (g_unit_resolver == nullptr) {
                throw std::logic_error("008E6430 without a unit resolver");
            }
        } else {
            if (g_unit_resolver == nullptr || !g_unit_resolver->view_unit(query.index, resolved)) {
                throw std::logic_error("008E6430 without a unit view");
            }
            unit = &resolved;
        }
        looked_up_ = true;
        return gameplay_modifier_product_008e6430(
            runtime_.category_lists[static_cast<std::size_t>(category)], *unit, *g_unit_resolver);
    }

private:
    cc13_powerup::PowerupRuntime& runtime_;
    // Last observed size word: +88h + 15*0Ch = +13Ch.
    std::array<std::uint32_t, 0x140 / sizeof(std::uint32_t)> counts_{};
    std::uint8_t enabled_;
    bool looked_up_{};
};

std::optional<float> reconstructed_site_product(int category, PowerupQueryUnit unit) {
    cc13_powerup::PowerupRuntime& runtime = cc13_powerup::powerup_runtime();
    if (runtime.manager == nullptr || runtime.host == nullptr || category < 0 ||
        category >= kPowerupCategoryCount) {
        return std::nullopt;
    }
    PowerupMultiplierBinding binding(runtime);
    return binding.product(category, unit);
}

}  // namespace

std::optional<float> powerup_site_product(int category, std::size_t unit_index) {
    std::optional<float> result;
    if constexpr (kPowerUpSubsystemCc13Bound) {
        if constexpr (bsp::kWireGoal4Chain310Bound) {
            result = reconstructed_site_product(category, PowerupQueryUnit{unit_index, nullptr});
        } else {
            if (const std::list<PowerupModifier>* list = gated_list(category)) {
                PowerupUnitView unit;
                if (g_unit_resolver == nullptr || !g_unit_resolver->view_unit(unit_index, unit)) {
                    throw std::logic_error("008E6430 without a unit view");
                }
                result = gameplay_modifier_product_008e6430(*list, unit, *g_unit_resolver);
            }
        }
    } else {
        (void)category;
        (void)unit_index;
    }
    return result;
}

std::optional<float> powerup_site_product_for_view(int category, const PowerupUnitView& view) {
    std::optional<float> result;
    if constexpr (kPowerUpSubsystemCc13Bound) {
        if constexpr (bsp::kWireGoal4Chain310Bound) {
            result = reconstructed_site_product(category, PowerupQueryUnit{0, &view});
        } else {
            if (const std::list<PowerupModifier>* list = gated_list(category)) {
                if (g_unit_resolver == nullptr) {
                    throw std::logic_error("008E6430 without a unit resolver");
                }
                result = gameplay_modifier_product_008e6430(*list, view, *g_unit_resolver);
            }
        }
    } else {
        (void)category;
        (void)view;
    }
    return result;
}

void publish_powerup_unit_resolver(const PowerupUnitResolver* resolver) {
    g_unit_resolver = resolver;
}

// --- phase 2a ----------------------------------------------------------------
// Packet cc13-powerup-bind phase 2a: the writers a scene reaches without player
// input, re-derived from disasm-raw of each body. Only the single-player
// (session word 0) arms are modelled; each hosted-session arm throws.

namespace {

namespace cp = bsp::cc13_powerup;

// [game+1FE4h] == 1 sends message 67 or 68 at 008EA466, 008ED5AB, 008E8AC0 and
// 008E8CE6. A mission-tree or skirmish run is session 0; hosting is not modelled.
// kPowerupSessionBroadcastCc13Bound replaces each call with the send.
// CORRECTION (packet cc13-mp1): only a mission-tree run is session 0. A front-end
// skirmish is session 1. 005EC990 calls 00770E40, which is Session_SetMode(1), and
// game+218Ch is set as well (005E6F87). These sends test only +1FE4h, so a skirmish
// reaches them and this throws (MP5).
void require_unhosted_session(cp::PowerupRuntime& runtime, const char* where) {
    if (runtime.host->session_word_1fe4() == 1) {
        throw std::logic_error(std::string(where) + ": session 1 power-up messages not modelled");
    }
}

const PowerupUnitResolver& resolver_or_throw(const char* where) {
    if (g_unit_resolver == nullptr) {
        throw std::logic_error(std::string(where) + " without a unit resolver");
    }
    return *g_unit_resolver;
}

PowerupUnitView view_or_throw(std::size_t unit, const char* where) {
    PowerupUnitView view;
    if (!resolver_or_throw(where).view_unit(unit, view)) {
        throw std::logic_error(std::string(where) + " without a unit view");
    }
    return view;
}

// The list size at manager+28h + slot*0Ch is 008EB110's notify gate for the
// active slot; the host caches it, so every inventory change refreshes it.
void refresh_notify_gate(cp::PowerupRuntime& runtime, std::int32_t slot) {
    if (cp::is_active_slot(runtime, slot)) {
        runtime.host->set_active_slot_notify_gate(
            !runtime.inventories[static_cast<std::size_t>(slot)].empty());
    }
}

// 008E4660: [00E198C4] non-null and its +C0h screen's byte +5 set, then 0061B0F0.
void refresh_hud_if_shown_008e4660(cp::PowerupRuntime& runtime) {
    if (runtime.host->powerup_screen_shown_00e198c4()) {
        runtime.host->powerup_screen_rebuild_lists_0061b0f0();
        ++runtime.counts.screen_rebuilds;
    }
}

// 008E35F0 __fastcall(ECX = unit, EDX = class): the class scope for a unit.
bool class_scope_accepts_unit_008e35f0(const PowerupClassConfig& cls, bool unit_present) {
    // 008E35F0..008E3606: scopes other than PUTT_FRIENDUNIT / PUTT_ENEMYUNIT pass.
    if (cls.target_type_84 != 1 && cls.target_type_84 != 2) return true;
    if (!unit_present) return false;                 // 008E3607..008E360E
    if (cls.target_filter_88 == 0x0F) return true;   // 008E3616..008E361F
    // 008E3620..008E367C: IsKindOf 8 / 6 / 18h and 009FE0B0 select a mask bit.
    // Every class 008ED4B0 uses at once is PUT_PASSIVE, scoped RANGE or PARTY.
    throw std::logic_error("008E35F0 unit-kind mask arms not modelled");
}

// 008E8C30 __thiscall(ECX = manager, record*), RET 4.
void expire_modifier_008e8c30(cp::PowerupRuntime& runtime, PowerupModifier* record) {
    cp::PowerupItem* const item = record->item;      // 008E8C4E
    if (item != nullptr) {
        if (item->cls->group_08 == 2) {
            item->word_00 -= 1;                      // 008E8C63, an active item
        } else {
            // 008E8C69..008E8C98: the item's pointer to this record goes, and
            // record+0 is cleared with it.
            for (auto it = item->modifiers.begin(); it != item->modifiers.end(); ++it) {
                if (*it == record) {
                    record->item = nullptr;
                    item->modifiers.erase(it);
                    break;
                }
            }
        }
    }
    // 008E8C9D..008E8D57: the record's node leaves its category list (008E8370).
    std::list<PowerupModifier>& list =
        runtime.category_lists.at(static_cast<std::size_t>(record->category));
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (&*it == record) {
            if constexpr (kPowerupSessionBroadcastCc13Bound) {
                // 008E8CE6..008E8D46: 008E6FB0(record, 2) to the transport peers.
                if (runtime.host->session_word_1fe4() == 1) {
                    runtime.host->send_powerup_message68(*record, 2);
                    ++runtime.counts.session_messages;
                }
            } else {
                require_unhosted_session(runtime, "008E8C30");  // 008E8CE0, message 68
            }
            list.erase(it);
            break;
        }
    }
    // 008E8D5C..008E8D7A: an active item with no live record and no use left goes
    // through 008E87C0. No active item is used in this packet's reach.
    if (item != nullptr && item->cls->group_08 == 2 && static_cast<std::int32_t>(item->word_00) < 1 &&
        item->use_limit == 0) {
        throw std::logic_error("008E87C0 active item removal not modelled");
    }
}

// 008EA250 __thiscall(ECX = manager, category, multiplier, class, item, slot,
// unit), RET 18h.
void insert_modifier_008ea250(cp::PowerupRuntime& runtime, std::int32_t category,
    float multiplier, const PowerupClassConfig& cls, cp::PowerupItem* item,
    std::int32_t slot, const PowerupUnitView& unit) {
    // 008EA27B..008EA2FB: the template record appended to the category's list
    // (008E8480 links it before the head, so after the last record).
    std::list<PowerupModifier>& list =
        runtime.category_lists.at(static_cast<std::size_t>(category));
    list.push_back(PowerupModifier{});
    PowerupModifier* const record = &list.back();
    record->category = category;                     // 008EA353
    record->multiplier = multiplier;                 // 008EA362
    record->owner_slot = slot;                       // 008EA367
    record->cls = &cls;                              // 008EA35D..008EA38D, the name
    // 008EA390..008EA3AE: an active class ends `duration` after the clock (one
    // FADD, FSTP float); every other class at the 1.0e10 double 00D16220.
    if (cls.group_08 == 2) {
        namespace x8 = bsp::cc13x87;
        record->end_time = static_cast<float>(
            x8::x87_add24(cls.duration_7c, runtime.host->clock_00f876a4()));
    } else {
        record->end_time = static_cast<float>(1.0e10);
    }
    // 008EA3B1..008EA409: a passive item lists the record (008E5AA0 append);
    // another item counts it. record+0 names the item either way.
    if (item != nullptr) {
        if (cls.group_08 == 3) {
            item->modifiers.push_back(record);
        } else {
            item->word_00 += 1;
        }
        record->item = item;
    }
    // 008EA40B..008EA45E, by class+84h (table 008EA4C4).
    switch (cls.target_type_84) {
    case 1:
    case 2:
        record->unit_id = unit.id;                   // 008EA420, unit+174h
        break;
    case 3: {
        // 008EA434: [[record+0]+8]+4 is the item's binding's building; its +174h.
        if (item == nullptr || item->binding == cp::PowerupItem::kNoBinding) {
            throw std::logic_error("008EA434 range record without a binding");
        }
        record->centre_id = view_or_throw(item->binding, "008EA434").id;
        break;
    }
    case 4:
        record->slot = slot;                         // 008EA449
        break;
    case 5: {
        // 008EA44E: [[game+18CCh + slot*4]+28h], the slot's player's party.
        const int party = bsp::game::scene_slot_party(slot, -1);
        if (party < 0) {
            throw std::logic_error("008EA44E slot without a published party");
        }
        record->party = party;
        break;
    }
    default:
        break;                                       // 008EA417 JA
    }
    if constexpr (kPowerupSessionBroadcastCc13Bound) {
        // 008EA466..008EA4A9: 008E6FB0(record, 0) to the transport peers.
        if (runtime.host->session_word_1fe4() == 1) {
            runtime.host->send_powerup_message68(*record, 0);
            ++runtime.counts.session_messages;
        }
    } else {
        require_unhosted_session(runtime, "008EA250");   // 008EA461, message 68
    }
}

// 008EADA0 __thiscall(ECX = manager, item, slot, unit), RET 0Ch, as 008ED4B0
// reaches it: a single-player use of a just-granted item.
void use_item_008eada0(cp::PowerupRuntime& runtime, cp::PowerupItem& item,
    std::int32_t slot, const PowerupUnitView& unit) {
    cp::PowerupManagerHost& host = *runtime.host;
    if (!host.powerups_enabled_00e0c978()) return;   // 008EADB8
    if (host.session_word_1fe4() == 2) {
        throw std::logic_error("008EADDA client use not modelled");
    }
    const PowerupClassConfig& cls = *item.cls;
    // 008EAE58..008EAEA7: the slot's +14Ch record for the class (00617030 inserts
    // a zeroed one), ready unless its deadline is still ahead of the clock.
    PowerUpSlot& ready = cp::slot_record_00617030(runtime, slot, &cls);
    const float now = host.clock_00f876a4();
    float ready_at = ready.absolute_expiry;
    if (ready.relative_deadline) {
        namespace x8 = bsp::cc13x87;
        ready_at = static_cast<float>(x8::x87_add24(ready.source_offset, now));
    }
    if (ready_at > now) return;
    // 008EAEAD: the air-support arm (class+8 == 1). 008ED4B0 uses passives only.
    if (cls.group_08 == 1) {
        throw std::logic_error("008EAEC3 air-support use not modelled");
    }
    if (!class_scope_accepts_unit_008e35f0(cls, true)) return;  // 008EAF6F
    // 008EAF7E..008EAFEB: item+0 cleared, then one insert per category whose
    // multiplier is not 1.0f (UCOMISS: an unordered compare inserts too).
    const bool active = cls.group_08 == 2;
    bool consumed = false;
    item.word_00 = 0;
    for (std::int32_t category = 0; category < kPowerupCategoryCount; ++category) {
        const float multiplier = cls.multipliers_8c[static_cast<std::size_t>(category)];
        if (!(multiplier == 1.0f)) {
            insert_modifier_008ea250(runtime, category, multiplier, cls, &item, slot, unit);
            consumed = active;
        }
    }
    // 008EAFED: only an active class goes on to the cooldown and use count.
    if (consumed) {
        throw std::logic_error("008EAFF8 active use tail not modelled");
    }
}

// 008ED4B0 __thiscall(ECX = manager, binding), RET 4: the binding's class to its
// building's slot, used at once when passive.
void grant_008ed4b0(cp::PowerupRuntime& runtime, std::size_t building,
    const PowerupClassConfig* cls) {
    cp::PowerupManagerHost& host = *runtime.host;
    if (!host.powerups_enabled_00e0c978()) return;   // 008ED4C8
    if (host.session_word_1fe4() == 2) return;       // 008ED4E1
    if (cls == nullptr) return;                      // 008ED4F3, binding+8
    const PowerupUnitView view = view_or_throw(building, "008ED4B0");
    const std::int32_t slot = view.slot;             // 008ED504, 009FFD20
    if (static_cast<std::uint32_t>(slot) > 7u) return;  // 008ED50B JA
    // 008ED514..008ED554: a fresh 008E6A80 item appended to the slot's list,
    // +4h the class, +8h the binding.
    std::list<cp::PowerupItem>& inventory = runtime.inventories[static_cast<std::size_t>(slot)];
    inventory.push_back(cp::PowerupItem{});
    cp::PowerupItem& item = inventory.back();
    item.cls = cls;
    item.binding = building;
    refresh_notify_gate(runtime, slot);
    ++runtime.counts.grants;
    // 008ED557..008ED597: 009789A0(slot, item, name) on [00F8A0C4], "pup_lost"
    // (00D1627C) for a building on party 2, else "pup_gain" (00D16270).
    host.report_powerup_009789a0(slot, item, view.party == 2 ? "pup_lost" : "pup_gain");
    ++runtime.counts.pup_gain_reports;
    refresh_hud_if_shown_008e4660(runtime);          // 008ED5A0
    if constexpr (kPowerupSessionBroadcastCc13Bound) {
        // 008ED5AB..008ED5EC: 008E6E00(slot, item, 0, building unit, 0) to the
        // transport peers.
        if (host.session_word_1fe4() == 1) {
            host.send_powerup_message67(slot, item, 0);
            ++runtime.counts.session_messages;
        }
    } else {
        require_unhosted_session(runtime, "008ED4B0");   // 008ED5AB, message 67
    }
    // 008ED5F1..008ED601: a passive class (class+8 == 3) is used now.
    if (cls->group_08 == 3) use_item_008eada0(runtime, item, slot, view);
}

// 008E6230 __thiscall(ECX = item): each listed record expires (passive) or is
// released (+0 cleared), while the manager exists.
void release_item_modifiers_008e6230(cp::PowerupRuntime& runtime, cp::PowerupItem& item) {
    if (runtime.manager == nullptr) return;          // 008E6230
    for (auto it = item.modifiers.begin(); it != item.modifiers.end();) {
        PowerupModifier* const record = *it;
        ++it;                                        // 008E6275, next before the call
        if (item.cls->group_08 == 3) {
            expire_modifier_008e8c30(runtime, record);
        } else {
            record->item = nullptr;                  // 008E6287
        }
    }
}

// 008E8A30 __thiscall(ECX = manager, binding), RET 4: the first item (slots 0..7
// in order) granted through the binding leaves its slot.
void remove_items_bound_to_008e8a30(cp::PowerupRuntime& runtime, std::size_t building) {
    for (std::int32_t slot = 0; slot < static_cast<std::int32_t>(cp::kPowerupPlayerSlots);
         ++slot) {
        std::list<cp::PowerupItem>& inventory =
            runtime.inventories[static_cast<std::size_t>(slot)];
        for (auto it = inventory.begin(); it != inventory.end(); ++it) {
            if (it->binding != building) continue;   // 008E8A6D, item+8
            release_item_modifiers_008e6230(runtime, *it);  // 008E8AB1
            if constexpr (kPowerupSessionBroadcastCc13Bound) {
                // 008E8AC0..008E8B2F: 008E6E00(slot, item, 2, [binding+4], 0) to the
                // transport peers.
                if (runtime.host->session_word_1fe4() == 1) {
                    runtime.host->send_powerup_message67(slot, *it, 2);
                    ++runtime.counts.session_messages;
                }
            } else {
                require_unhosted_session(runtime, "008E8A30");  // 008E8AC0, message 67
            }
            // 008E8B37..008E8B88: an active item's records anywhere drop it.
            if (it->cls != nullptr && it->cls->group_08 == 2) {
                for (std::list<PowerupModifier>& list : runtime.category_lists) {
                    for (PowerupModifier& record : list) {
                        if (record.item == &*it) record.item = nullptr;
                    }
                }
            }
            inventory.erase(it);                     // 008E8B8E..008E8BF0
            refresh_notify_gate(runtime, slot);
            refresh_hud_if_shown_008e4660(runtime);  // 008E8BF4..008E8C09
            return;
        }
    }
}

// LABELLED stand-in for 00BD2FC0 with ECX = 0, BSP_RandomThreads stream 0, the
// only stream 008EC650 draws on. Stream 0; its seed and MT sequence are not
// modelled, so the values differ from the image's. Stream 1, the gameplay
// stream (GameGunneryHost::ship_ai_draw), is unaffected: nothing here draws on
// it. Private to this file; the gunnery and HUD stream-0 consumers keep their
// own draws (a shared stream-0 generator is the follow-up, see the header).
// The generator lives in the runtime with the default seed and is reset with
// the manager (destroy_powerup_manager_004d2d84), so every manager lifetime,
// and so every scene of a multi-mission run, starts the same sequence; the
// image's stream 0 runs on across scenes instead.
std::uint32_t stream0_stand_in_00bd2fc0(cp::PowerupRuntime& runtime) {
    ++runtime.counts.stream0_draws;
    return static_cast<std::uint32_t>(runtime.stream0_stand_in());
}

// 00585EB0 then 00584110: strip the set's characters from the front, then the back.
void trim_set_both(std::string& text, std::string_view set) {
    const std::size_t first = text.find_first_not_of(set);
    if (first == std::string::npos) {
        text.clear();
        return;
    }
    text.erase(0, first);
    text.erase(text.find_last_not_of(set) + 1);
}

// 008E65E0 on manager+1B8h (008E5BB0 lower bound, then the _stricmp equality
// test): the random_names row for a token, or null.
const std::vector<NativeString>* random_names_row(cp::PowerupRuntime& runtime,
    const std::string& token) {
    if (!runtime.manager->random_names_1b8) return nullptr;
    PowerupNameVectorMap& rows = *runtime.manager->random_names_1b8;
    NativeStringStorage& strings = crt_string_storage();
    NativeString key;
    key.assign_0041e870(strings, token.c_str());
    const auto found = lower_bound_powerup_names_008e5bb0(rows, key);
    const bool hit = found != rows.end()
        && !native_string_less_case_insensitive_00443d00(key, found->first);
    key.release_to(strings);
    return hit ? &found->second : nullptr;
}

// 008EC650 __thiscall(ECX = manager, NativeString* out, NativeString chunk),
// RET 0Ch: the name one "]" chunk of a bracketed PowerupClassID picks.
std::string pick_from_bracket_list_008ec650(cp::PowerupRuntime& runtime,
    const std::string& chunk) {
    // 008EC6A9..008EC6BC: the first character is the mode (0 for an empty chunk).
    const char mode = chunk.empty() ? '\0' : chunk[0];
    std::string result;  // 008EC67E: stays empty unless a pick lands
    auto& cache = runtime.bracket_picks;  // manager+1ACh
    auto entry = cache.find(chunk);       // 008EC83F 008E65E0, keyed by the chunk
    if (entry == cache.end()) {
        // 008EC6DF..008EC7DB: the chunk less its first character, "[]" trimmed
        // from both ends, "," appended.
        std::string list = chunk.empty() ? std::string{} : chunk.substr(1);
        trim_set_both(list, "[]");
        list += ',';
        std::vector<std::string> names;
        std::size_t pos = 0;  // ebp
        while (pos < list.size()) {  // 008ECA86
            // 008EC870..008EC8AF: strcspn for ","; the appended "," means one
            // is always found, so the image's -1 arm (a restart at 0) is
            // unreachable; it throws here rather than loop.
            std::size_t end = list.find(',', pos);
            if (end == std::string::npos) {
                throw std::logic_error("008EC650 token without ','");
            }
            // 008EC8B3..008EC94A: a "*" token gets the mode letter inserted in
            // front of it, so "*" looks up "<mode>*".
            if (list[pos] == '*') {
                list.insert(pos, 1, mode);  // 00531030, 008E64A0
                end += 1;
            }
            if (pos <= end) {  // 008EC94E
                // 008EC961..008ECA5B: a random_names row expands to its names,
                // any other token is kept as it is.
                const std::string token = list.substr(pos, end - pos);
                if (const std::vector<NativeString>* row = random_names_row(runtime, token)) {
                    for (const NativeString& name : *row) {
                        names.emplace_back(name.data() != nullptr
                            ? std::string(name.data(), name.length()) : std::string{});
                    }
                } else {
                    names.push_back(token);
                }
            }
            pos = end + 1;  // 008ECA7F
        }
        if (mode == 'A') {
            // 008ECA97..008ECAEE: one draw now, and the cache keeps that name.
            const std::uint32_t draw = stream0_stand_in_00bd2fc0(runtime);
            if (names.empty()) {
                throw std::logic_error("008EC650 'A' chunk with no names (image DIV by zero)");
            }
            names = std::vector<std::string>{names[draw % names.size()]};
        }
        entry = cache.emplace(chunk, std::move(names)).first;  // 008EC460
    }
    // 008ECB18..008ECB9A: the cache row for the chunk.
    std::vector<std::string>& row = entry->second;
    if (mode == 'A') {
        // 008ECBB5..008ECC04, no draw. The key is case-insensitive, so an 'A'
        // chunk can meet a row an "a" chunk built; an emptied row is the
        // image's invalid-parameter report (008ECBCD).
        if (row.empty()) throw std::logic_error("008EC650 'A' pick from an empty row");
        result = row.front();
    } else if (!row.empty()) {
        // 008ECCAD..008ECD63: one draw per pick, uniform by modulo.
        const std::uint32_t draw = stream0_stand_in_00bd2fc0(runtime);
        const std::size_t index = draw % row.size();
        result = row[index];
        if (mode == 'T') row.erase(row.begin() + static_cast<std::ptrdiff_t>(index));  // 008ECD66..008ECDB8
    }
    return result;
}

}  // namespace

void command_building_powerup_init_008ed720(std::size_t building,
    std::string_view powerup_class_id) {
    if constexpr (kPowerUpSubsystemCc13Bound) {
        cp::PowerupRuntime& runtime = cp::powerup_runtime();
        // The host has no manager when the class registry did not load.
        if (runtime.manager != nullptr && runtime.host != nullptr) {
            // 008ED73E..008ED759: sessions 0 and 1 only; others clear binding+8.
            const std::int32_t session = runtime.host->session_word_1fe4();
            if (session == 0 || session == 1) {
                // 008ED762: CMP [record+4], 1 / JNZ 008ED9A5 skips unless the creation
                // record's kind is 1; the scene creation that calls this satisfies it.
                // 008ED7AB..008ED7C5: a "[" past the first character takes the
                // bracket arm, 008EC650's pick per "]" chunk.
                const std::size_t bracket = powerup_class_id.find('[');
                if (bracket != std::string_view::npos && bracket > 0) {
                    const std::string text(powerup_class_id);
                    const PowerupClassConfig* bound = nullptr;  // binding+8
                    bool observed = false;
                    std::size_t pos = 0;  // esi
                    while (pos < text.size()) {  // 008ED901
                        // 008ED7D5..008ED806: strcspn for "]". With none left the
                        // image's -1 sends esi back to 0 and it loops forever,
                        // regranting each chunk; that throws here instead.
                        const std::size_t close = text.find(']', pos);
                        if (close == std::string::npos) {
                            throw std::logic_error("008ED720 bracket tail without ']' (image loops forever)");
                        }
                        if (pos < close) {  // 008ED809
                            // 008ED81A..008ED87C: the chunk with " |" trimmed from
                            // both ends, through 008EC650.
                            std::string chunk = text.substr(pos, close - pos);
                            trim_set_both(chunk, " |");
                            const std::string pick = pick_from_bracket_list_008ec650(runtime, chunk);
                            // 008ED8A5..008ED8D5: each chunk's class into
                            // binding+8 (null on a miss); a hit grants and
                            // registers the observer (00694A60, refcounted).
                            bound = cp::find_powerup_class_008e6cb0(runtime, pick);
                            if (bound != nullptr) {
                                grant_008ed4b0(runtime, building, bound);
                                observed = true;
                            }
                        }
                        pos = close + 1;  // 008ED8FE
                    }
                    // 008ED620 reads binding+8 when it fires, so the regrant
                    // takes the last chunk's class, null when that chunk missed.
                    if (observed) runtime.observed_bindings[building] = bound;
                } else {
                    // 008ED964..008ED99D: the plain name (008E6CB0) into binding+8;
                    // on a hit, the grant and the observer registration (00694A60).
                    const PowerupClassConfig* cls =
                        cp::find_powerup_class_008e6cb0(runtime, powerup_class_id);
                    if (cls != nullptr) {
                        grant_008ed4b0(runtime, building, cls);
                        runtime.observed_bindings[building] = cls;
                    }
                }
            }
        }
    } else {
        (void)building;
        (void)powerup_class_id;
    }
}

void powerup_manager_observed_event_008ed620(std::size_t unit, int code) {
    if constexpr (kPowerUpSubsystemCc13Bound) {
        cp::PowerupRuntime& runtime = cp::powerup_runtime();
        if (code == 5 && runtime.manager != nullptr && runtime.host != nullptr) {
            // Only bindings 008ED720 registered observe their building.
            const auto found = runtime.observed_bindings.find(unit);
            if (found != runtime.observed_bindings.end()) {
                // 008ED62A..008ED64C: the observed unit's vtable[4]()'s
                // vtable[64h](), its binding, then 008E8A30 and 008ED4B0.
                remove_items_bound_to_008e8a30(runtime, unit);
                grant_008ed4b0(runtime, unit, found->second);
            }
        }
    } else {
        (void)unit;
        (void)code;
    }
}

void expire_due_modifiers_008eb110() {
    if constexpr (kPowerUpSubsystemCc13Bound) {
        cp::PowerupRuntime& runtime = cp::powerup_runtime();
        if (runtime.manager != nullptr && runtime.host != nullptr) {
            // 008EB139..008EB181: FLD record+18h, FLD clock, FCOMI, JBE: a
            // record expires once the clock is past its end. The next node is
            // read before the call.
            const float now = runtime.host->clock_00f876a4();
            for (std::list<PowerupModifier>& list : runtime.category_lists) {
                for (auto it = list.begin(); it != list.end();) {
                    PowerupModifier* const record = &*it;
                    ++it;
                    if (now > record->end_time) expire_modifier_008e8c30(runtime, record);
                }
            }
        }
    }
}

}  // namespace bsp::game_powerups
