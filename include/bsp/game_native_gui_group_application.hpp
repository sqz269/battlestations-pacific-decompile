#pragma once
#include "bsp/native_group_owner.hpp"
#include "bsp/native_model_owner.hpp"
#include <exception>
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameNativeReadOnlyData;

// Standalone renderer-side cGroup prerequisite. Borrow the SAME node/string/
// type/pool/actual-owner domains and the Text application's existing parenting
// runtime. The child remains its sole NativeModelReference and widget owner.
// This class creates no page, cGuiLayer, Screen, GUI manager or second model.
// Native constructor/destructor bodies are reused through new C++ interfaces;
// private FH3/SEH, original factory stack and complete raw widget ABI are not
// established. Serialize all entries with native frames and retirement.
class GameNativeGuiGroupApplication final {
public:
    // Pure service composition; no native allocation or construction here.
    GameNativeGuiGroupApplication(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameNativeReadOnlyData&, NativeNodeParentingRuntime&);
    ~GameNativeGuiGroupApplication() noexcept;
    GameNativeGuiGroupApplication(const GameNativeGuiGroupApplication&) = delete;
    GameNativeGuiGroupApplication& operator=(const GameNativeGuiGroupApplication&) = delete;

    // Allocate ONE actual18Ch slot from010902F4, B8F5E0 with a raw VFS-domain
    // name, bind its canonical actual reference, then B6E680 the SAME live Text
    // Model below it. Child must be unattached; the caller supplies its lifetime.
    NativeGroupReference& create_root(const char* name, NativeModelReference& text_model);
    NativeGroupReference& root();
    NativeGroupEnvironment& groups() noexcept;

    // Retire every queued/scene root reference before these entries. Detach
    // through the SAME B6E680 runtime before Text AB8EE0/AB75A0 retirement.
    // retire_root requires that detachment and exactly its creator credit;
    // B8EEC0 -> B6F310 -> B8F8C0 flags1 performs native destruction/pool return.
    void detach_text();
    void retire_root();

    // An entered failure is sticky/nonreplayable. Preserve this application,
    // Text, renderer and all borrowed services and exit before shared drain.
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_entry() const noexcept;
    const std::exception_ptr& failure() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
