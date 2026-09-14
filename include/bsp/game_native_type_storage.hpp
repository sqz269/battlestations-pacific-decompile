#pragma once

#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_stream_type_ids.hpp"

#include <cstdint>

namespace bsp::game {

// One application's process-owned type cells. The host must retain this object
// through the shared singleton drain and any later native type consumers.
// The counter, its publication, and the common bootstrap are borrowed only by
// the initializer calls; this owner cannot keep a short-lived service alive.
class GameNativeTypeStorage final {
public:
    GameNativeTypeStorage() noexcept;
    GameNativeTypeStorage(const GameNativeTypeStorage&) = delete;
    GameNativeTypeStorage& operator=(const GameNativeTypeStorage&) = delete;
    GameNativeTypeStorage(GameNativeTypeStorage&&) = delete;
    GameNativeTypeStorage& operator=(GameNativeTypeStorage&&) = delete;

    // Stable aggregate views: their references always target this same set of
    // guards and descriptors. GameNativeVfsRuntimeInputs can borrow stream_types().
    LightTypeBootstrapStorage& light_types() noexcept { return light_types_; }
    NativeStreamTypeIdStorage& stream_types() noexcept { return stream_types_; }

    // Preserve the caller's CRT order: memory type, separate physical pool
    // initializer, then physical type. Neither entry initializes that pool.
    void initialize_memory_00cd8fc0(TypeIdCounterLifetime& existing_counter,
        LightTypeBootstrap& common_root_bootstrap);
    void initialize_physical_00cd9030(TypeIdCounterLifetime& existing_counter,
        LightTypeBootstrap& common_root_bootstrap);

private:
    void require_common_bootstrap(const LightTypeBootstrap&) const;

    volatile std::uint8_t root_guard_0109db80_{};
    volatile RootTypeDescriptor root_0109db84_{};
    volatile std::uint8_t node_guard_0108ff54_{};
    volatile NodeTypeDescriptor node_0108ff90_{};
    volatile std::uint8_t light_guard_0109010d_{};
    volatile LightTypeDescriptor light_0109018c_{};
    volatile std::uint8_t directional_guard_0109010e_{};
    volatile DirectionalLightTypeDescriptor directional_0109019c_{};
    volatile std::uint8_t file_guard_0109db54_{};
    volatile NativeFileTypeDescriptor file_0109db58_{};
    volatile std::uint8_t memory_guard_0109db94_{};
    volatile NativeDerivedFileTypeDescriptor memory_0109dba0_{};
    volatile std::uint8_t physical_guard_0109dc2c_{};
    volatile NativeDerivedFileTypeDescriptor physical_0109dc30_{};
    LightTypeBootstrapStorage light_types_;
    NativeStreamTypeIdStorage stream_types_;
};

} // namespace bsp::game
