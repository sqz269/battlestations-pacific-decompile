#pragma once
#include <cstdint>

namespace bsp::oct18_tail_b {
// Address-suffixed, source-level interfaces over genuine Win32 object storage.
// Original receivers/stack cleanup are recorded in docs/OCT18_TAIL_B.md. These
// interfaces are not binary replacements. Every dependency below is required;
// this module supplies no fallback host and performs no game-host wiring.
class Host {
public:
    virtual ~Host() = default;
    virtual void run_lua_006b8ad0(const char*, int, int, int) = 0;
    virtual bool is_kind_vslot_5c(void*, int) = 0;
    virtual void movie_reset_vslot_2c(void*) = 0;
    virtual void copy_controller_00720450(void* destination, void* source) = 0;
    virtual bool category_vslot_48(void*, int) = 0;
    virtual void* fire_target_vslot_2c(void*) = 0;
    virtual void store_fire_target_00836240(void*, void*, std::uint8_t) = 0;
    virtual void visit_enables_00836510(void*, void*) = 0;
    virtual void visit_reference_vslot_10(void* visitor, const char* name,
                                         int type, void* value) = 0;
    virtual bool filter_vslot_00(void* filter, std::uint32_t value) = 0;
    // The original uses the canonical CRT invalid-iterator/range handler.
    // Providers must not return from this call.
    virtual void invalid_iterator_00bf6713() = 0;
};

bool class60_is_kind_007810f0(const void*, int);
bool class54_is_kind_0079a380(const void*, int);
bool class55_is_kind_0079d840(const void*, int);
void plane_clear_turbo_007b8a90(void*);
bool director_allows_category_008364b0(int) noexcept;
bool ship_director_allows_category_00836790(int) noexcept;
float gun_class_value_for_target_00836eb0(const void*, void*, Host&);
void* ship_tuning_block_00837de0(void* settings, const void* game);
void reset_stationary_class_table_0084eb60(Host&);
void movie_camera_track_set_time_00797da0(void* track, float time, Host&);
void director_copy_state_00836680(void* destination, void* source, Host&);
void director_visit_state_008367f0(void* director, void* visitor, Host&);
void* stationary_part_scene_0084efe0(const void* stationary);
bool consume_single_peer_flag_00784cd0(void* transport, Host&);
bool recon_subscription_matches_00968470(const void* subscription,
                                         const void* parameters, Host&);
} // namespace bsp::oct18_tail_b
