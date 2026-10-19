#pragma once
#include "bsp/native_procedural_factory_table_construction.hpp"
#include <cstdint>

namespace bsp::game {
// Source-owned typed scratch for the genuine four-key BBCB40/BBC900 sequence.
// Bind these views directly; NEVER pass them to a wrapper that eagerly copies
// input preimages. Payload members intentionally have no initializers. The
// native caller/helper stores establish every reached value before its read.
//
// Qualified domain: absent/empty factory registry at entry, exact increasing
// CausticsTextureSource/ShoreWaveTextureSource0/1/2 order, no concurrent map
// mutation, canonical allocators/validation and borrowed frames retained through
// failure. Hint insertion is direct throughout. B1AF90 fallback is NOT admitted:
// its full-DWORD direction push otherwise consumes unresolved upper3 preimages.
// No Original stack location, register preimage or FH3 identity is supplied.
class GameNativeMissionFactoryFrames final {
    using Word = std::uint32_t;
    struct Node {
        volatile Word incoming[5], constructor[5], allocation_cleanup, placement_cleanup;
        NativeResourceRegistryNodeAllocationFrame frame;
        Node() noexcept
            : frame{{incoming[0], incoming[1], incoming[2], incoming[3], incoming[4]},
                {constructor[0], constructor[1], constructor[2], constructor[3], constructor[4]},
                allocation_cleanup, placement_cleanup} {}
    };
    struct Link {
        volatile Word output, left, parent, pair;
        NativeLegacySboStringStorage length_message;
        Node allocation;
        NativeResourceRegistryLinkFrame frame;
        Link() noexcept : frame{output, left, parent, pair, length_message, allocation.frame} {}
    };
    struct Pair {
        volatile Word output, pair, direction;
        NativeResourceRegistryTreeIterator cursor;
        Link link;
        NativeResourceRegistryPairInsertFrame frame;
        Pair() noexcept : frame{output, pair, direction, cursor, link.frame} {}
    };
    struct Hint {
        volatile Word output, pair;
        NativeResourceRegistryTreeIterator incoming_hint;
        NativeResourceRegistryInsertResult local;
        Link direct;
        Pair fallback;
        NativeResourceRegistryHintInsertFrame frame;
        Hint() noexcept : frame{output, incoming_hint, pair, local, direct.frame, fallback.frame} {}
    };
    struct Value {
        volatile Word name, temporary_pair[3];
        NativeResourceRegistryTreeIterator result;
        Hint hint;
        NativeResourceRegistryValueFrame frame;
        Value() noexcept : frame{name, result, temporary_pair, hint.frame} {}
    };
    struct Registration {
        volatile Word name, factory;
        NativeResourceRegistryTreeIterator result;
        Value value;
        NativeResourceRegistryRegistrationFrame frame;
        Registration() noexcept : frame{name, factory, result, value.frame} {}
    };
    struct Factory {
        volatile Word name, cleanup_self;
        Registration registration;
        NativeProceduralFactoryStorageFrame frame;
        Factory() noexcept : frame{name, cleanup_self, registration.frame} {}
    };
    struct Construction {
        volatile Word locals[5], eh_state, flags[4];
        Factory factory;
        NativeProceduralFactorySlotFrame cleanup[4];
        NativeProceduralFactoryTableConstructionFrame frame;
        Construction() noexcept
            : cleanup{{flags[0]}, {flags[1]}, {flags[2]}, {flags[3]}},
              frame{locals, eh_state, factory.frame, cleanup} {}
    };
    struct Setup {
        const Word source_receiver;
        volatile Word cleanup_allocation, eh_state;
        Construction construction;
        NativeProceduralFactoryTablePublicationFrame frame;
        explicit Setup(Word receiver) noexcept : source_receiver(receiver),
            frame{source_receiver, cleanup_allocation, eh_state, construction.frame} {}
    };
    struct Retirement {
        volatile Word cleanup_self, normal_flags, cleanup_flags[3];
        NativeProceduralFactorySlotFrame normal, cleanup[3];
        NativeProceduralFactoryTableFrame frame;
        Retirement() noexcept : normal{normal_flags},
            cleanup{{cleanup_flags[0]}, {cleanup_flags[1]}, {cleanup_flags[2]}},
            frame{cleanup_self, normal, cleanup} {}
    };
    Setup setup_;
    Retirement retirement_;
public:
    // A known Source caller identity only. BBCB40 overwrites its saved word
    // after successful allocation, before arming any cleanup that reads it.
    explicit GameNativeMissionFactoryFrames(Word source_receiver) noexcept : setup_(source_receiver) {}
    GameNativeMissionFactoryFrames(const GameNativeMissionFactoryFrames&) = delete;
    GameNativeMissionFactoryFrames& operator=(const GameNativeMissionFactoryFrames&) = delete;
    const NativeProceduralFactoryTablePublicationFrame& setup() const noexcept { return setup_.frame; }
    const NativeProceduralFactoryTableFrame& retirement() const noexcept { return retirement_.frame; }
};
} // namespace bsp::game
