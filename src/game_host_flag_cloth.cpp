#include "bsp/game_host_flag_cloth.hpp"
#include "bsp/camera_decomposition.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_particle_model_manager.hpp"
#include "bsp/random_threads.hpp"

#include <array>
#include <cstring>
#include <stdexcept>

namespace bsp::game {
using engine_seg11_35_66_cc13::FlagClothLayout;
using engine_seg11_35_66_cc13::PresentationRandom;
using engine_seg11_35_66_cc13::VertexStreamCalls;
using engine_seg11_35_66_cc13::VertexStreamView;
using engine_seg11_35_66_cc13::update_flag_wave_0070aa70;
using engine_seg11_35_66_cc13::write_flag_vertices_0070a710;
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(offsetof(FlagClothLayout, count_04) == 0x04);
static_assert(offsetof(FlagClothLayout, positions_08) == 0x08);
static_assert(offsetof(FlagClothLayout, normals_1c) == 0x1c);
static_assert(offsetof(FlagClothLayout, wind_20) == 0x20);
static_assert(offsetof(FlagClothLayout, angle_2c) == 0x2c);
static_assert(offsetof(FlagClothLayout, columns_3c) == 0x3c);
static_assert(offsetof(FlagClothLayout, time_74) == 0x74);
static_assert(offsetof(FlagClothLayout, vertex_stream_78) == 0x78);
static_assert(sizeof(FlagClothLayout) == 0x7c);

template<class T> T read(const void* owner, Word offset) noexcept {
    return *reinterpret_cast<const volatile T*>(
        static_cast<const std::byte*>(owner) + offset);
}

// One invocation owns its two independent guard scopes. 70A710 re-reads the
// current particle manager's +30h at both scopes, independently of the logical
// stream's own renderer guard. Never substitute the renderer+04h lock here.
class VertexCalls final : public VertexStreamCalls {
public:
    VertexCalls(NativeGameGridContext& grid,
        NativeParticleModelManagerStorage* volatile& manager) noexcept
        : grid_(grid), manager_(manager) {}
    ~VertexCalls() override { render_unlock(); }

    void render_lock() override {
        auto* const manager = manager_;
        if (!manager) throw std::logic_error("flag upload requires the published particle manager");
        captured_ = manager->section_30;
        if (captured_) {
            EnterCriticalSection(&captured_->native);
            depth() = depth() + 1u;
        }
    }
    void render_unlock() override {
        if (captured_) {
            depth() = depth() - 1u;
            LeaveCriticalSection(&captured_->native);
            captured_ = nullptr;
        }
    }
    VertexStreamView lock_vslot10(void* stream, std::int32_t count) override {
        require_slot(stream, 0x10, 0x00b49980);
        // 70A710 passes 2*count, 0, 0. Use the same actual logical factory
        // profile and mapping services that 70B330 used for this +78h stream.
        (void)lock_native_logical_vertex_stream_00b49980(
            stream, grid_.graphics.streams.mapping, static_cast<Word>(count), 0, 0);
        return {read<std::uint8_t*>(stream, 0x08), read<std::int32_t>(stream, 0x0c),
            read<std::int32_t>(stream, 0x10), read<std::int32_t>(stream, 0x1c),
            read<std::int32_t>(stream, 0x28), read<std::int32_t>(stream, 0x34),
            read<std::int32_t>(stream, 0x38), read<std::int32_t>(stream, 0x3c),
            read<std::int32_t>(stream, 0x40), read<std::int32_t>(stream, 0x44)};
    }
    void unlock_vslot14(void* stream) override {
        require_slot(stream, 0x14, 0x00b49a80);
        unlock_native_logical_vertex_stream_00b49a80(stream, grid_.graphics.streams.mapping);
    }

private:
    volatile Word& depth() noexcept {
        return *reinterpret_cast<volatile Word*>(&captured_->depth);
    }
    void require_slot(const void* stream, Word offset, Word expected) const {
        const auto* const profile = grid_.graphics.streams.vertices.actual_logical_profile_00d61d6c;
        if (!stream || read<Word>(stream, 0) != 0x00d61d6c || !profile || profile[offset / 4] != expected)
            throw std::logic_error("flag upload requires the existing native grid vertex stream profile");
    }
    NativeGameGridContext& grid_;
    NativeParticleModelManagerStorage* volatile& manager_;
    TrackedCriticalSection* captured_{};
};
} // namespace

GameHostFlagClothServices::GameHostFlagClothServices(NativeGameGridContext& grid,
    NativeParticleModelManagerStorage* volatile& manager) noexcept
    : grid_(grid), manager_(manager) {}

void GameHostFlagClothServices::calculate_normals_00709cf0(FlagClothLayout& cloth) {
    calculate_native_game_grid_normals_00709cf0(&cloth);
}
void GameHostFlagClothServices::normalize_0042b260(float* vector) {
    std::array<float, 3> words;
    std::memcpy(words.data(), vector, sizeof words);
    normalize_camera_basis_0042b260(words);
    std::memcpy(vector, words.data(), sizeof words);
}
void GameHostFlagClothServices::update_wave(NativeGameGridStorage& storage, float delta,
    std::uint32_t game_tick_648, float& published_e19b10, PresentationRandom& random) {
    // 008E256E/81/94 receive the E19B08/04/0C allocations, not the unrelated
    // 19Ch renderable FlagModel constructed by 70BFC0 for individual units.
    // 70AA97 is a DWORD copy from game+648, the counter incremented at
    // 004E4FF5. Preserve the representation expected by the existing float
    // projection's +74/E19B10 fields, rather than numerically converting it.
    float frame_word;
    std::memcpy(&frame_word, &game_tick_648, sizeof frame_word);
    update_flag_wave_0070aa70(*reinterpret_cast<FlagClothLayout*>(&storage), delta,
        frame_word, published_e19b10, random, *this);
}
void GameHostFlagClothServices::upload_vertices(NativeGameGridStorage& storage) {
    // 008E259F/AA/B5 use those same grids after all three wave updates.
    VertexCalls calls(grid_, manager_);
    write_flag_vertices_0070a710(*reinterpret_cast<FlagClothLayout*>(&storage), calls);
}
} // namespace bsp::game
