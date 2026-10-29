#pragma once
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameVfsHost;
class GameSingletonHost;
class GameNativeReadOnlyData;

// ON-only composition of the existing genuine BBCB40/BBC900 factory setup.
// Metadata construction performs no native setup. create() requires an absent
// or empty shared factory registry and sole serialized authority for its four
// increasing keys. Keep this graph and all borrowed owners through raw drain.
class GameNativeMissionFactories final {
public:
    GameNativeMissionFactories(GameHostLog&, GameVfsHost&, GameSingletonHost&,
        GameNativeReadOnlyData&);
    ~GameNativeMissionFactories() noexcept;
    GameNativeMissionFactories(const GameNativeMissionFactories&) = delete;
    GameNativeMissionFactories& operator=(const GameNativeMissionFactories&) = delete;

    void create();
    // Borrow admission only: recheck the genuine live table/parent/registry
    // identities without setup, registration or ownership transfer. A failed
    // live validation latches failure and requires whole-graph retention.
    void require_lookup_ready();
    bool requires_process_retention() const noexcept;
    // Caller guarantees no later factory lookup, setup or asset acquisition.
    // Run after mission consumers retire and before the raw singleton drain.
    void retire_before_native_drain();
    void after_native_drain();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
