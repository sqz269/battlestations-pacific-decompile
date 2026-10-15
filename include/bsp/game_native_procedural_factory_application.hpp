#pragma once

#include "bsp/native_procedural_factory_table_construction.hpp"
#include <memory>

namespace bsp::game {
class GameVfsHost;
class GameSingletonHost;
class GameNativeReadOnlyData;

// Required ordinary-Source value inputs, not inferred Original preimages.
// Every referenced DWORD, iterator/result (including padding), and raw SBO
// byte must already be initialized and live. Construction copies these values
// into this operation's own disjoint typed backing; it does not borrow these
// input frames after return or reproduce their address/alias relationships.
// No default frame values or incoming ECX are supplied by this API.
struct GameNativeProceduralFactorySourceInputs {
    const NativeProceduralFactoryTablePublicationFrame& setup;
    const NativeProceduralFactoryTableFrame& retirement;
};

// Concrete BBCB40/BBC900 setup and BBC5D0 retirement over one designated,
// caller-owned Source publication cell. The cell and all VFS/singleton/data
// owners outlive this object, residual failures, and the complete parent drain.
// Caller is the sole setup authority for this cell and the four registry keys.
// No fixed-address mutable-global or Original caller ABI admission is implied.
class GameNativeProceduralFactoryApplication final {
public:
    enum class Phase { prepared, setting_up, live, retiring, retired, drained, failed };

    GameNativeProceduralFactoryApplication(GameVfsHost&, GameSingletonHost&,
        GameNativeReadOnlyData&, void* volatile& actual_publication_01090900,
        const GameNativeProceduralFactorySourceInputs&);
    ~GameNativeProceduralFactoryApplication() noexcept;
    GameNativeProceduralFactoryApplication(const GameNativeProceduralFactoryApplication&) = delete;
    GameNativeProceduralFactoryApplication& operator=(const GameNativeProceduralFactoryApplication&) = delete;

    // Once only. Own all frames before genuine allocation/registration. A
    // successful admission requires all four real registered factory identities.
    void setup_00bbcb40();
    // Admission check for an existing genuine sampler consumer. Supplies no
    // alternate registry/cache/provider; rejects retirement, failure and drift.
    void require_lookup_ready();
    // The required true argument is the caller's explicit quiescence assertion,
    // not a claim this wrapper can observe arbitrary external factory lookups.
    // Verify owned table identity; invoke genuine shutdown once; preserve the
    // native dangling publication. Any entered failure poisons/retains the graph.
    void retire_00bbc5d0(bool all_factory_lookups_quiescent);
    // Call only after later loader/shared-manager retirement while all borrowed
    // owners still exist. No VFS re-entry, native cleanup, or publication clear.
    void after_parent_drain();

    Phase phase() const noexcept;
    bool requires_process_retention() const noexcept;
    const NativeProceduralFactoryTablePublicationAcquired& setup_evidence() const noexcept;
    const NativeProceduralFactoryTableShutdownAcquired& retirement_evidence() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
