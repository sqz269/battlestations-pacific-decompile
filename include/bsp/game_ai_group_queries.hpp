#pragma once

#include "bsp/ai_tuning_globals.hpp"
#include "bsp/oct18_tail_c.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace bsp::game {

// Read-only query views, not replacement native objects. The host owns the
// membership/order, handles, refreshed positions and class weights. Only the
// slots read by 00A2C720 / 00A10C60 / 00A10D50 are projected here; none of these
// queries writes back. Unused native storage has no invented field values.
class AiGroupListQuery {
    struct Node {
        Node* next;
        Node* previous;
        void* entity;
    };
    static_assert(sizeof(Node) == 12);

public:
    template<class Handle>
    AiGroupListQuery(const std::vector<std::size_t>& members, Handle handle)
        : nodes_(members.size()) {
        // bsp.py show 00a2c720: owner token +563Ch, sentinel +5640h,
        // count +5644h; node next +0h and entity +8h. Keep vector storage
        // fixed until the query returns so the sentinel and links stay live.
        sentinel_.next = nodes_.empty() ? &sentinel_ : &nodes_.front();
        sentinel_.previous = nodes_.empty() ? &sentinel_ : &nodes_.back();
        sentinel_.entity = nullptr; // end node has no member
        for (std::size_t i = 0; i < nodes_.size(); ++i) {
            nodes_[i].next = i + 1 == nodes_.size() ? &sentinel_ : &nodes_[i + 1];
            nodes_[i].previous = i == 0 ? &sentinel_ : &nodes_[i - 1];
            nodes_[i].entity = handle(members[i]);
        }
        void* owner = this;
        Node* head = &sentinel_;
        const auto count = static_cast<std::uint32_t>(members.size());
        std::memcpy(storage_.data() + 0x563c, &owner, sizeof(owner));
        std::memcpy(storage_.data() + 0x5640, &head, sizeof(head));
        std::memcpy(storage_.data() + 0x5644, &count, sizeof(count));
    }

    AiGroupListQuery(const AiGroupListQuery&) = delete;
    AiGroupListQuery& operator=(const AiGroupListQuery&) = delete;
    void* data() noexcept { return storage_.data(); }

    void bind_first_entity(void* entity) noexcept {
        if (nodes_.empty()) std::abort();
        nodes_.front().entity = entity;
    }

private:
    alignas(4) std::array<std::byte, 0x5648> storage_;
    Node sentinel_;
    std::vector<Node> nodes_;
};

struct AiGroupLeaderQuery {
    std::array<std::byte, 0xc8> unused_prefix;
    std::uint8_t world_pose_current;
    std::array<std::byte, 0x33> unused_gap;
    float position[3];
    double class_weight;

    AiGroupLeaderQuery(const float* refreshed_position, double weight) noexcept
        : world_pose_current(1), class_weight(weight) {
        // 00A10C90/00A10CCD read C8h, then FC/104h. The host has just read
        // the leader's current world position through unit_position_00fc.
        // Publishing cache-valid here describes that completed read, not a
        // fabricated pose. Class weight is 009FFD70 on this same first member.
        if (refreshed_position == nullptr) std::abort();
        std::memcpy(position, refreshed_position, sizeof(position));
    }
};
static_assert(offsetof(AiGroupLeaderQuery, world_pose_current) == 0xc8);
static_assert(offsetof(AiGroupLeaderQuery, position) == 0xfc);

struct AiCommandGroupQuery {
    std::uint32_t unused_vtable;
    void* owner_group;
};
static_assert(offsetof(AiCommandGroupQuery, owner_group) == 4);

inline void __cdecl ai_group_query_invalid_iterator() { std::abort(); }

// AiBindings has a context-free tuning callback. Keep the actual host block
// borrowed for exactly the synchronous query, restoring an enclosing binding.
inline thread_local const AiTuningBlock* g_ai_group_query_tuning = nullptr;

class AiGroupMergeQueryBinding {
public:
    explicit AiGroupMergeQueryBinding(const AiTuningBlock& tuning) noexcept
        : previous_(g_ai_group_query_tuning) {
        g_ai_group_query_tuning = &tuning;
        bindings_.checked_iterator_bf6713 = ai_group_query_invalid_iterator;
        bindings_.refresh_pose_00414db0 = reinterpret_cast<oct18_tail_c::RawThis>(unexpected_pose);
        bindings_.zero_vector_f87574 = image_zero_vector_;
        bindings_.active_tuning_00a371a0 = active_tuning;
        bindings_.class_weight_009ffd70 =
            reinterpret_cast<double (__thiscall*)(void*)>(class_weight);
        // Other callbacks belong to different bodies and are never called by
        // these two queries. No stand-in callees are supplied for them.
    }

    ~AiGroupMergeQueryBinding() { g_ai_group_query_tuning = previous_; }
    AiGroupMergeQueryBinding(const AiGroupMergeQueryBinding&) = delete;
    AiGroupMergeQueryBinding& operator=(const AiGroupMergeQueryBinding&) = delete;
    const oct18_tail_c::AiBindings& bindings() const noexcept { return bindings_; }

private:
    static void* __cdecl active_tuning() {
        if (g_ai_group_query_tuning == nullptr) std::abort();
        return const_cast<float*>(g_ai_group_query_tuning->field);
    }
    // On Win32 these no-stack-argument fastcall bridges consume ECX and leave
    // EDX unused, exactly the __thiscall callback ABI. A double returns in ST0;
    // 00A10D50 itself retains the original own-weight FSTP-qword and comparison.
    static double __fastcall class_weight(void* entity, void*) {
        return static_cast<const AiGroupLeaderQuery*>(entity)->class_weight;
    }
    static void __fastcall unexpected_pose(void*, void*) {
        // Every admitted view already contains a refreshed position. A stale
        // cache flag is a broken binding invariant, never a no-op refresh.
        std::abort();
    }
    // The empty-group fallback is the binary's zero vector at 00F87574.
    inline static constexpr float image_zero_vector_[3] = {0.0f, 0.0f, 0.0f};
    const AiTuningBlock* previous_;
    oct18_tail_c::AiBindings bindings_{};
};

} // namespace bsp::game
