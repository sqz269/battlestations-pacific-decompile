#pragma once

#include "bsp/game_native_vfs_runtime.hpp"

#include <filesystem>
#include <memory>
#include <optional>

namespace bsp {
class NativeVfsOwnerServices;
struct NativeStringRawPoolContext;
}

namespace bsp::game {
class GameHostLog;
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameNativeCanonicalDataOwner;
class GameNativeTypeStorage;

// Retained source owner for one application's raw VFS graph. Borrow the same
// GameSingletonHost and mapped data throughout startup and the shared drain.
// The caller must invoke singleton_host.shutdown() before destroying this
// bundle, including when initialize_core() throws after a getter/register call.
class GameNativeVfsApplication final {
public:
    GameNativeVfsApplication(GameHostLog&, GameSingletonHost&,
        GameNativeReadOnlyData&, const std::filesystem::path& original_executable);
    ~GameNativeVfsApplication();
    GameNativeVfsApplication(const GameNativeVfsApplication&) = delete;
    GameNativeVfsApplication& operator=(const GameNativeVfsApplication&) = delete;
    GameNativeVfsApplication(GameNativeVfsApplication&&) = delete;
    GameNativeVfsApplication& operator=(GameNativeVfsApplication&&) = delete;

    // One attempt. Bind deletion before type getters; initialize recovered
    // resource selectors/mesh types in their relative CRT order, memory type
    // CD8FC0, process pool CD9010, physical type CD9030, then retain and bind
    // the VFS runtime before BEDA60/registration. A nonzero CRT registration
    // status does not mean that the native pool construction failed.
    void initialize_core();
    GameNativeVfsRuntime& runtime();
    // Explicit opt-in after successful initialize_core(): publish the qualified
    // raw C3 symbol only at the retained manager's +90, then borrow that owner.
    // Normal startup keeps its Original identities. The caller serializes this
    // operation and all uses against startup, publication/callback mutation and
    // shutdown; finish every use before singleton_host.shutdown() begins. Keep
    // this bundle, runtime, host, referenced data/services and code image alive
    // through uses and the shared drain. The result owns nothing; no notifier,
    // I/O, +18 store or fixed-address publication is performed by this method.
    void* publish_and_borrow_raw_failure_manager();
    // Stable views of this application's existing raw string and VFS graph.
    // They create no publication, manager, provider or callback family. Keep
    // this application and GameSingletonHost alive through every consumer and
    // the shared singleton drain.
    NativeStringRawPoolContext& raw_strings() noexcept;
    // Address-only loan of this SAME constants owner's verified E17BF0
    // initial-image NUL snapshot. No new copy or mutable Original VA alias.
    // Retain this application through every borrower and the shared drain.
    const char* borrow_null_pattern_00e17bf0() const noexcept;
    // Own/borrow ONE stable raw BEE390 context over this same raw pool, the
    // runtime's actual null role/current-locale binding and SAME canonical
    // owner's live feature reference. Cold C++ metadata allocation only.
    // Runtime admission reads CURRENT Native publication; no feature VALUE,
    // raw owner-prefix/getter/callback/write/I/O/cleanup is performed here.
    // Keep this application/runtime/canonical owner through all uses and the
    // shared drain; actual atlas/caller failure retention remains required.
    NativePathCanonicalizerRawContext& raw_canonicalizer_for_atlas(
        const GameNativeCanonicalDataOwner& actual_canonical_owner);
    // Pure identity view over existing retained references; no startup,
    // publication, native payload read or provider construction is performed.
    bool borrows_graph(const GameSingletonHost&,
        const GameNativeReadOnlyData&) const noexcept;
    GameNativeVfsRawServices borrow_raw_services();
    NativeVfsOwnerServices& owners() noexcept;
    GameNativeTypeStorage& types() noexcept;
    // Empty until CD9010 returns. Its status describes CRT registration only.
    std::optional<int> physical_pool_registration_status() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
