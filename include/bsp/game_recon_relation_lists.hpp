#pragma once

#include "bsp/cc13_seg49.hpp"

#include <array>
#include <cstddef>
#include <map>

namespace bsp {

// The live scan buckets behind GameGunneryHost's indexed recon APIs. The list
// nodes use the reconstructed native operations; their payload borrows the
// host's real unit index and sensor-pass level, not a fabricated native unit or
// 1Ch ReconEntry. Group/detection-record publication remains a separate seam.
//
// Evidence: bsp.py show 008073c0, 008040e0, 00804150. At 00807430..00807455
// every class's own/enemy/neutral lists move into its carry-over list. The
// 008065B0 scan moves surviving nodes into the current relation in scan order.
class GameReconRelationLists final : private cc13_seg49::ReconListStorageHost {
public:
    static constexpr std::size_t kClassCount = 0x61;
    static constexpr std::size_t kRelationCount = 3; // own, enemy, neutral

    struct Entry {
        std::size_t unit{};
        int level{};
        cc13_seg49::ReconNode* node{};
        cc13_seg49::ReconList* owner{};
    };

    GameReconRelationLists() = default;
    GameReconRelationLists(const GameReconRelationLists&) = delete;
    GameReconRelationLists& operator=(const GameReconRelationLists&) = delete;
    ~GameReconRelationLists() override {
        // Nodes borrow map entries. Free every node before its payload dies.
        for (auto& bucket : classes_) {
            for (auto& list : bucket.relations)
                cc13_seg49::recon_list_clear_008042b0(*this, list);
            cc13_seg49::recon_list_clear_008042b0(*this, bucket.carried);
        }
    }

    void begin_pass() {
        for (auto& bucket : classes_) {
            for (auto& source : bucket.relations)
                cc13_seg49::recon_list_splice_all_00804150(bucket.carried, source);
            for (auto* node = bucket.carried.head; node != nullptr; node = node->next)
                static_cast<Entry*>(node->payload)->owner = &bucket.carried;
        }
    }

    void refresh(int class_id, std::size_t relation, std::size_t unit, int level) {
        auto& bucket = classes_.at(static_cast<std::size_t>(class_id));
        auto& destination = bucket.relations.at(relation);
        auto& entry = bucket.entries.try_emplace(unit).first->second;
        entry.unit = unit;
        entry.level = level;
        if (entry.node != nullptr) {
            cc13_seg49::recon_list_splice_node_008040e0(destination, *entry.owner, entry.node);
        } else {
            cc13_seg49::recon_list_push_back_00804c30(*this, destination, &entry);
            entry.node = destination.tail;
        }
        entry.owner = &destination;
    }

    void retire_unscanned(int class_id) {
        auto& bucket = classes_.at(static_cast<std::size_t>(class_id));
        // Only entries absent from this class's current live registry remain
        // carried. Clear does not inspect payloads, so erase their owners first.
        for (auto* node = bucket.carried.head; node != nullptr; node = node->next) {
            const std::size_t unit = static_cast<const Entry*>(node->payload)->unit;
            bucket.entries.erase(unit);
        }
        cc13_seg49::recon_list_clear_008042b0(*this, bucket.carried);
    }

    const cc13_seg49::ReconList& list(int class_id, std::size_t relation) const {
        return classes_.at(static_cast<std::size_t>(class_id)).relations.at(relation);
    }

    template<class Buckets>
    void project(std::size_t relation, Buckets& output) const {
        output.clear();
        for (std::size_t id = 0; id < classes_.size(); ++id) {
            const auto& source = classes_[id].relations.at(relation);
            if (source.count == 0) continue;
            auto& records = output[static_cast<int>(id)];
            records.reserve(static_cast<std::size_t>(source.count));
            for (auto* node = source.head; node != nullptr; node = node->next) {
                const auto& entry = *static_cast<const Entry*>(node->payload);
                records.push_back({entry.unit, entry.level});
            }
        }
    }

private:
    void* operator_new_00bf681b(std::size_t size) override { return ::operator new(size); }
    void free_00bf65ac(void* block) override { ::operator delete(block); }

    struct ClassBucket {
        std::array<cc13_seg49::ReconList, kRelationCount> relations{};
        cc13_seg49::ReconList carried{};
        std::map<std::size_t, Entry> entries; // insertion preserves payload addresses
    };
    std::array<ClassBucket, kClassCount> classes_{};
};

} // namespace bsp
