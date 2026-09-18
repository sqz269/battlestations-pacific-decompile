#pragma once
#include "bsp/native_game_dynamics.hpp"
#include "bsp/dyn_dispatch_initialization.hpp"
#include <memory>

namespace bsp::game {
// Canonical process ownership for Dyn publication cells, all eight actual
// dispatch objects and their callable source tables, SAP/intersection tasks,
// and both solver task tables. Engine/world construction remains at its native
// game-constructor calls. This owner does not create a replacement world.
class GameNativeDynProcess final {
public:
    GameNativeDynProcess(const GameNativeDynProcess&)=delete;
    GameNativeDynProcess& operator=(const GameNativeDynProcess&)=delete;
    // One attempt; retain native construction and atexit result on registration
    // failure. Native CD91D0 closes the general-convex CS after borrowing users.
    int initialize_once_00cc8950();
    const NativeGameDynamicsContext& dynamics();
    const DynDispatchVtables& dispatch_tables();
private:
    friend GameNativeDynProcess& game_native_dyn_process(const CameraAxesCrtAccess&,
        const AvoidZoneDynHullMemory&);
    GameNativeDynProcess(const CameraAxesCrtAccess&,const AvoidZoneDynHullMemory&);
    ~GameNativeDynProcess();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// First bind selects the actual process allocator and borrowed CRT pointees.
// Later access must use the same services; publication-cell values may change.
// Default uses the process C++ new/delete allocator, shared by every Dyn owner.
GameNativeDynProcess& game_native_dyn_process(const CameraAxesCrtAccess&,
    const AvoidZoneDynHullMemory&);
GameNativeDynProcess& game_native_dyn_process(const CameraAxesCrtAccess&);
} // namespace bsp::game
