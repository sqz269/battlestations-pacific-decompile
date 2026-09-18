#pragma once
#include <cstdint>
namespace bsp {
struct NativeGameStorage;
struct NativeGameConstructionContext;
struct NativeGameConstructionOperation;
struct NativeGameLifetimeContext;
struct NativeGameLifetimeOperation;
}
namespace bsp::game {
class GameNativeDynProcess;
// Stable owner for the actual game's complete one-slot primary table. Borrows
// already composed construction/lifetime services and the canonical Dyn owner;
// owns persistent operation frames, not the externally supplied 71A0h storage.
// The source tables are installed at the original constructor/destructor stores.
// Explicit construct and scalar_delete are required; destruction cannot infer
// cleanup of a partially constructed/destroyed native graph.
class GameNativeGameRuntime final {
public:
    enum class Phase {fresh,constructing,live,destroying,destroyed,failed,diagnostic_retired};
    GameNativeGameRuntime(const NativeGameConstructionContext&,
        const NativeGameLifetimeContext&,GameNativeDynProcess&);
    ~GameNativeGameRuntime();
    GameNativeGameRuntime(const GameNativeGameRuntime&)=delete;
    GameNativeGameRuntime& operator=(const GameNativeGameRuntime&)=delete;
    NativeGameStorage* construct(NativeGameStorage&,const void* actual_name_header);
    NativeGameStorage* scalar_delete(std::uint32_t flags);
    // 875E0C reloads [actual game publication]+18h before C5C540. Requires
    // this completed, still-published owner; never creates a substitute world.
    void simulate_physics_00875e0c(float step);
    Phase phase() const noexcept;
    NativeGameStorage* storage() const noexcept;
    const void* table() const noexcept {return methods_;}
    NativeGameConstructionOperation& construction_operation() noexcept;
    NativeGameLifetimeOperation& lifetime_operation() noexcept;
    // Only after the caller resolves every retained graph/child operation.
    // Does not free storage, undo publications, roll back, or permit replay.
    void acknowledge_diagnostic_cleanup();
private:
    const std::uintptr_t methods_[1];
    struct Impl;
    Impl* const impl_;
    static void* __fastcall scalar(void*,void*,std::uint32_t);
};
} // namespace bsp::game
