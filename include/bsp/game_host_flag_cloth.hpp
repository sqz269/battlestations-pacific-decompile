#pragma once

#include "bsp/engine_seg11_35_66_cc13.hpp"

namespace bsp {
struct NativeGameGridContext;
struct NativeGameGridStorage;
struct NativeParticleModelManagerStorage;
}

namespace bsp::game {
// Borrow the renderer's existing grid/stream domain and the actual F8C274
// publication. This service owns no grids, CPU arrays, streams, clock or RNG.
// The application game owner must still construct/publish its three 84h grids
// through 4DDB90 -> 70BD70 before the F88C20 frame hook may use this service.
class GameHostFlagClothServices final : private engine_seg11_35_66_cc13::FlagClothCalls {
public:
    GameHostFlagClothServices(NativeGameGridContext&,
        NativeParticleModelManagerStorage* volatile&) noexcept;

    // Existing 70AA70 and 70A710 bodies over the SAME 70BD70 allocation. The
    // caller supplies the integer frame word at game+648, E19B10's raw float
    // cell and its existing presentation RNG. 70AA70 copies that word's bits;
    // it does not convert the integer to a float or read the mission clock.
    void update_wave(NativeGameGridStorage&, float delta, std::uint32_t game_tick_648,
        float& published_e19b10, engine_seg11_35_66_cc13::PresentationRandom&);
    void upload_vertices(NativeGameGridStorage&);

private:
    void calculate_normals_00709cf0(engine_seg11_35_66_cc13::FlagClothLayout&) override;
    void normalize_0042b260(float*) override;

    NativeGameGridContext& grid_;
    NativeParticleModelManagerStorage* volatile& manager_;
};
} // namespace bsp::game
