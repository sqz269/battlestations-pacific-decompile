#pragma once

#include "bsp/ai_target_weights.hpp"
#include "bsp/ai_tuning_globals.hpp"
#include "bsp/cc13_seg70.hpp"

#include <array>
#include <cstdlib>
#include <new>
#include <stdexcept>

namespace bsp::game::detail {

// F8AB08..F8AB67 are loader-zero process globals, not fields of the scene's
// 1A8h AI controller. 004DD766 first creates the HighLvlAIGlobals singleton
// (004C1C50 -> 00A37130); its 00A335D0 loader inserts each authored 20h rule
// at 00A36FD5. Preserve the eight native data/count/capacity publications.
class AiForcedRuleTables final {
    using Array = bsp::cc13_seg70::ArrayRaw;
    struct Publications {
        std::array<Array, bsp::kAiTuningModeCount> modes{};
        Array global{};
    } tables_;
    static_assert(sizeof(Array) == 0x0c);
    static_assert(offsetof(Array, data) == 0);
    static_assert(offsetof(Array, count) == 4);
    static_assert(offsetof(Array, capacity) == 8);
    static_assert(offsetof(Publications, global) == 0x54);
    static_assert(sizeof(Publications) == 0x60);
    bool loaded_{false};

    struct Loader final : bsp::cc13_seg70::ForcedRuleInsertHost {
        AiForcedRuleTables& owner;
        const int mode;
        Loader(AiForcedRuleTables& storage, int row_mode) : owner(storage), mode(row_mode) {}
        bsp::cc13_seg70::Raw global(std::uint32_t address) override {
            constexpr auto first = bsp::cc13_seg70::kModeForcedRules;
            if (address < first || address > bsp::cc13_seg70::kGlobalForcedRules
                || (address - first) % sizeof(Array) != 0) {
                throw std::out_of_range("forced-rule table publication");
            }
            return {reinterpret_cast<std::uint8_t*>(&owner.tables_) + (address - first)};
        }
        void* operator_new_00bf55be(std::size_t bytes) override {
            void* const result = std::malloc(bytes);
            if (result == nullptr) throw std::bad_alloc();
            return result;
        }
        void free_00bf6989(void* block) override { std::free(block); }
        // This private adapter is only used by the loader's explicit-mode
        // 00A36FD5 site. The selected table is the current authored mode row;
        // no runtime negative-mode insertion is exposed by this owner.
        int ai_effective_game_mode_index_009ffc80() override { return mode; }
        void forced_target_weight_clear_00a038c0() override {
            // AiWeightModelBinding::memo_lookup/store in game_hosts_ai.cpp
            // retain no memo.
            // There are no cached weights to invalidate after an insertion.
        }
    };

    static bsp::AiForcedRuleRange range(const Array& table) noexcept {
        return {static_cast<const bsp::AiForcedTargetWeightRule*>(table.data), table.count};
    }
public:
    AiForcedRuleTables() = default;
    AiForcedRuleTables(const AiForcedRuleTables&) = delete;
    AiForcedRuleTables& operator=(const AiForcedRuleTables&) = delete;
    ~AiForcedRuleTables() {
        // The singleton destructor 00A33020 clears all eight counts; backing
        // arrays are process globals. Release source allocations at process exit.
        tables_.global.count = 0;
        std::free(tables_.global.data);
        for (Array& table : tables_.modes) {
            table.count = 0;
            std::free(table.data);
        }
    }
    void load() {
        if (loaded_) return;
        for (int mode = 0; mode < bsp::kAiTuningModeCount; ++mode) {
            Loader loader(*this, mode);
            // Existing installed-script rows are the producer input, not the
            // live query table. Replace the old direct scan of that manifest.
            for (const auto& rule : bsp::ai_shipped_forced_rules(mode)) {
                bsp::cc13_seg70::forced_target_weight_insert_00a32500(loader,
                    reinterpret_cast<const std::uint8_t*>(&rule), false, mode);
            }
        }
        loaded_ = true;
    }
    bsp::AiForcedRuleRange global_rules() const noexcept { return range(tables_.global); }
    bsp::AiForcedRuleRange mode_rules(int mode) const {
        return range(tables_.modes.at(static_cast<std::size_t>(mode)));
    }
};

inline AiForcedRuleTables& ai_forced_rule_tables() {
    static AiForcedRuleTables tables;
    return tables;
}

} // namespace bsp::game::detail
