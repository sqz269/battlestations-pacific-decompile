#pragma once
#include "bsp/native_player_profile_owner.hpp"

namespace bsp::game {
struct GameNativeVfsRawServices;
class GameNativeSettingsProcess;
class GameNativeReadOnlyData;

// Retained Source provider for the game+650h profile constructor. Owns only
// its concrete constructor calls and context; borrows the canonical VFS string
// wrapper, initialized settings owner, live profile-process refs and RO image.
// Keep this provider AND all borrowed owners alive through the profile graph
// and any failed construction/reset operation that retains this context.
class GameNativePlayerProfileContext final {
public:
    GameNativePlayerProfileContext(const GameNativeVfsRawServices&,
        GameNativeSettingsProcess&,const GameNativeReadOnlyData&);
    GameNativePlayerProfileContext(const GameNativePlayerProfileContext&)=delete;
    GameNativePlayerProfileContext& operator=(const GameNativePlayerProfileContext&)=delete;
    GameNativePlayerProfileContext(GameNativePlayerProfileContext&&)=delete;
    GameNativePlayerProfileContext& operator=(GameNativePlayerProfileContext&&)=delete;

    // Always returns the same context. Construction/borrowing runs no profile
    // constructor/reset, allocation or SDK call and changes no publication.
    NativePlayerProfileContext& borrow_construction_context() noexcept;

    // Teardown remains separate: its NativeGameProfileLifetimeContext must
    // share the eventual game's NativeGameLifetimeCalls object. These
    // constructor calls do not supply or replace that lifetime call service.
private:
    NativePlayerProfileCalls calls_;
    NativePlayerProfileContext context_;
};
} // namespace bsp::game
