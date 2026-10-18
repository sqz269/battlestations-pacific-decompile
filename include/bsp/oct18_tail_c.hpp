#pragma once
#include <cstdint>

// Exact-address lane tC leaves. Names describe hypotheses, not recovered symbols.
// Raw getters borrow the original 32-bit storage. Callback setters receive the
// actual image cell explicitly; they do not create a replacement singleton.
// Evidence and original ABIs: docs/OCT18_TAIL_C.md. No application wiring.
namespace bsp::oct18_tail_c {
const void* __fastcall accepted_invite_info_00a3e460(const void* owner);
void __fastcall enumerate_input_noop_00a90ed0(void* owner);
std::uint8_t __fastcall gui_layer_visible_00aa38e0(const void* layer);
void set_pointer_activation_reader_00aa6be0(std::uint32_t& image_f8bc84,
                                          std::uint32_t reader);
void set_selection_callback_00aab5e0(std::uint32_t& image_f8bcfc,
                                    std::uint32_t callback);
std::uint32_t __fastcall terrain_field3c_00ad56d0(const void* owner);
float __fastcall foliage_scalar0c_00ad5720(const void* owner);
std::uint32_t __fastcall foliage_bush_texture_00ad57d0(const void* owner);
std::uint32_t __fastcall foliage_bush_normal_texture_00ad57e0(const void* owner);
float __fastcall atmosphere_time_of_day_00bbdd30(const void* owner);
float __fastcall atmosphere_sky_blend_00bbdd40(const void* owner);
const void* __fastcall atmosphere_wind_direction_00bbdd60(const void* owner);
float __fastcall atmosphere_wind_speed_00bbdd70(const void* owner);
std::uint32_t __fastcall atmosphere_skybox_00bbddb0(const void* owner);
} // namespace bsp::oct18_tail_c
