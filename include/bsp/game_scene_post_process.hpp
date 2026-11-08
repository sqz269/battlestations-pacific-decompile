#pragma once
#include <array>
#include <memory>
#include <string>
#include <utility>

namespace bsp { struct ScenePropertyBlock; }
namespace bsp::game {
class GameNativeRendererApplication;

// Owning output of 004E8510 for one parsed scene. Keep the property-model
// types out of the mission UI's separate ScenePropertyValue declaration.
class GameScenePostProcessRecord {
public:
    std::array<std::string, 4> remap_names() const;
    void apply(GameNativeRendererApplication&) const;
private:
    struct Impl;
    explicit GameScenePostProcessRecord(std::shared_ptr<const Impl> values)
        : values_(std::move(values)) {}
    std::shared_ptr<const Impl> values_;
    friend GameScenePostProcessRecord read_scene_post_process_record(
        const ScenePropertyBlock& defaults, const ScenePropertyBlock& authored);
};

GameScenePostProcessRecord read_scene_post_process_record(
    const ScenePropertyBlock& defaults, const ScenePropertyBlock& authored);
} // namespace bsp::game
