#pragma once
#include "bsp/native_instance_geometry.hpp"
#include <cstddef>
#include <memory>

namespace bsp {
struct NativeModelStorageContext;
}
namespace bsp::game {
class GameNativeRendererApplication;
class GameNativeReadOnlyData;
class GameVfsHost;
class GameNativeModelDomain;

// Nonowning notification after native destruction, before canonical unbinding.
// The client and capture must survive every Model acquired through that client.
struct GameNativeModelRetirementObserver {
    void* context;
    void (*retire)(void*, void* actual, NativeModelReference&, std::size_t drawable_index) noexcept;
};
// Stable caller bookkeeping in the SAME sole Model record. These fields neither
// own the native creator nor introduce another drawable-to-Model association.
struct GameNativeModelAcquisition {
    NativeString caller_name;
    bool caller_name_live{};
    std::size_t drawable_index{static_cast<std::size_t>(-1)};
};

// New Source interface, not Original ABI. Each address-stable client retains
// immutable views/callback context over the renderer's one shared Model domain.
class GameNativeModelClient final {
public:
    ~GameNativeModelClient() noexcept;
    GameNativeModelClient(const GameNativeModelClient&) = delete;
    GameNativeModelClient& operator=(const GameNativeModelClient&) = delete;
    NativeInstanceGeometryAccess& raw_access() noexcept;
    // Available only to the genuinely legacy Mission client. Page never
    // retargets the typed instance environment or its immutable raw lifetime.
    NativeInstanceGeometryAccess& instance_access();
    GameNativeModelAcquisition& model_acquisition(NativeModelOwner&);
    NativeModelReference& resolve_actual_model(void* actual);
    bool requires_process_retention() const noexcept;
    bool empty() const noexcept;
    void after_native_renderer_drain();
private:
    friend class GameNativeModelDomain;
    struct Impl;
    explicit GameNativeModelClient(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
};

// Retained by the renderer after all borrowed providers and before its clients.
// Composing this service/clients runs no Native acquisition/type initializer.
class GameNativeModelDomain final {
public:
    ~GameNativeModelDomain() noexcept;
    GameNativeModelDomain(const GameNativeModelDomain&) = delete;
    GameNativeModelDomain& operator=(const GameNativeModelDomain&) = delete;
    std::unique_ptr<GameNativeModelClient> make_mission_client(GameNativeModelRetirementObserver);
    // Required real Page raw node/scene/tree/string lifetime storage. It must
    // be complete and stable before providers publish/prepare their first slot.
    std::unique_ptr<GameNativeModelClient> make_page_client(NativeModelStorageContext&);
    bool requires_process_retention() const noexcept;
    bool empty() const noexcept;
    void after_native_renderer_drain();
private:
    friend class GameNativeRendererApplication;
    friend class GameNativeModelClient;
    struct Impl;
    GameNativeModelDomain(GameNativeRendererApplication&, GameVfsHost&, GameNativeReadOnlyData&);
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
