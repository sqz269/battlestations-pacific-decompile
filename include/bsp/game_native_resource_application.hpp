#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeResourceManagerContext;
struct NativeGameResourceParsersContext;
struct NativeStringRawPoolContext;
struct NativeResourceStreamReadContext;
struct NativeResourceHierarchyParserContext;
class NativeMaterialParameterPool;
class NativeVfsRuntimeBindings;
class NativeResourceExtraItemReaderCalls;
class NativeAdoptedSubstreamDispatch;
}

namespace bsp::game {
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameVfsHost;

// Stable non-owning views of one resource application's genuine hierarchy
// services. Keep the application, VFS, mapped data and canonical process pools
// alive through every consumer and the shared drain. Do not retarget the read
// context. This view creates no node, record, resource or parser invocation.
struct GameNativeHierarchyServices {
    NativeResourceStreamReadContext& reads;
    NativeResourceHierarchyParserContext& hierarchy;
    NativeMaterialParameterPool& hierarchy_pool_0109022c;
};

// Stable, host-owned metadata over the same hierarchy read/string/node domain.
// Binding invokes no native reader or deletion. Actual item/handle ownership,
// interruption retention and retirement before shared drain remain caller gates.
struct GameNativeAnimationExtraItemServices {
    NativeResourceExtraItemReaderCalls& readers;
    NativeAdoptedSubstreamDispatch& references;

    GameNativeAnimationExtraItemServices(NativeResourceExtraItemReaderCalls& actual_readers,
        NativeAdoptedSubstreamDispatch& actual_references) noexcept
        : readers(actual_readers), references(actual_references) {}
    GameNativeAnimationExtraItemServices(const GameNativeAnimationExtraItemServices&) = delete;
    GameNativeAnimationExtraItemServices& operator=(const GameNativeAnimationExtraItemServices&) = delete;
};

// Actual resource-manager and thirteen parser publication cells, using the
// application's existing raw string pool and singleton lifetime manager.
// Retain this owner, strings, mapped data and GameSingletonHost through the
// complete shared drain. Destruction does not independently drain or replay it.
class GameNativeResourceApplication final {
public:
    GameNativeResourceApplication(GameSingletonHost&, NativeStringRawPoolContext&,
        GameNativeReadOnlyData&);
    ~GameNativeResourceApplication();
    GameNativeResourceApplication(const GameNativeResourceApplication&) = delete;
    GameNativeResourceApplication& operator=(const GameNativeResourceApplication&) = delete;

    void* manager_004c1400();
    void* animation_channels_parser_00736dd0();
    void* bone_parser_00736ea0();
    bool register_parser_00b80a50(void* actual_manager, void* actual_parser);

    // Borrow the actual context for subsequent native resource loading. No
    // semantic registry, extra manager or private string storage is supplied.
    NativeResourceManagerContext& raw_manager_context();
    NativeGameResourceParsersContext& raw_game_parsers_context();
    void* published_manager() const noexcept;
    std::size_t registered_parsers() const noexcept;
    std::uint32_t failure_entry() const noexcept;

private:
    friend class GameVfsHost;
    // Only the owning host may bind its own ready VFS/string domain. Mapped
    // data comes from this application's original construction, never a caller.
    const GameNativeHierarchyServices& borrow_hierarchy_services(
        NativeVfsRuntimeBindings&, NativeStringRawPoolContext&);
    const GameNativeAnimationExtraItemServices& borrow_animation_extra_item_services(
        NativeVfsRuntimeBindings&, NativeStringRawPoolContext&);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
