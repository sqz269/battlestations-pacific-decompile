#pragma once
// Movie decoder base classes, segment 72 (00A4C470-00A4D353), packet
// cc13-online2 (docs/CC13_ONLINE_2.md). Complements include/bsp/movie_decoder.hpp,
// which already holds the decoder's open/close/frame/volume routines.
//
// Native layout (MSVC virtual inheritance, one shared virtual base):
//   decoder allocation A (38h): +00h file vbtable 00D24DE8, +04h texture
//   vbtable 00D24DE0, +08h texture (ref-counted resource), +0Ch vtordisp,
//   +10h shared base (MovieDecoderState, vtable 00D24D98 when complete).
//   Stand-alone MovieFileBase: +00h vbtable 00D24E38, +04h vtordisp, +08h
//   shared base. Stand-alone MovieTextureBase: +00h vbtable 00D24F20, +04h
//   texture, +08h vtordisp, +0Ch shared base.
// The vtordisp word always sits at shared-4, and the texture at shared-8 in
// both the decoder and MovieTextureBase. Vtable words are evidence
// identifiers, not callable pointers. Build-tested only: not ABI-compatible,
// not game-validated.
#include "bsp/movie_decoder.hpp"

#include <cstdint>

namespace bsp::movie_bases_cc13 {

struct MovieBaseHost {
    virtual ~MovieBaseHost() = default;
    virtual void free_00bf65ac(void* block) = 0; // _free
    // Native vtable +0 of a ref-counted object whose count reached zero,
    // __thiscall with no stack argument (00A4D0DE CALL EAX).
    virtual void destroy_ref_object(void* object) = 0;
};

// 00A4C470 BSP_MovieSingleton_ScalarDelete, __thiscall(this, flags), RET 4,
// EAX = this. The media-manager singleton base: publication [00F8AEF8] = 0,
// vtable = 00CE3818 (singleton base), _free(this) when bit 0 is set.
void* media_singleton_scalar_delete_00a4c470(void* self, std::uint32_t flags,
    void*& published_00f8aef8, MovieBaseHost& host);

// Trivial slots. 00A4C730 and 00A4C8F0: RET 4, no body. 00A4C740: RET.
// 00A4C760: AL = 0. 00A4C900: EAX = 0. 00A4C910: EAX = 0, RET 4.
void nop_ret4_00a4c730() noexcept;
void nop_00a4c740() noexcept;
bool return_false_00a4c760() noexcept;
void nop_ret4_00a4c8f0() noexcept;
std::uint32_t return_zero_00a4c900() noexcept;
std::uint32_t return_zero_ret4_00a4c910() noexcept;

// 00A4C750, __thiscall(ECX = shared base, one ignored stack argument), RET 4:
// EAX = [ECX-8], the texture word of the decoder / MovieTextureBase.
void* texture_of_shared_00a4c750(const void* shared) noexcept;
// 00A4D090, __thiscall(ECX = shared base, value), RET 4: [ECX-8] = value.
void set_texture_of_shared_00a4d090(void* shared, void* value) noexcept;

// Vtordisp thunks: ECX -= [ECX-4] (the vtordisp word), optionally ECX -= 8,
// then JMP target. Each returns the ECX its target receives.
//   00A4C7E0 -> 00A4C760, 00A4C7F0 -> 00A4D1B0 (decode frame),
//   00A4C800 (-8) -> 00A4CA40 (open), 00A4C810 -> 00A4C750,
//   00A4C820 -> 00A4C880, 00A4C830 -> 00A4D140 (prepare frame),
//   00A4C840 -> 00A4D090, 00A4C850 (-8) -> 00A4C980 (apply volume),
//   00A4C940 -> 00A4CA00, 00A4C9F0 (-8) -> 00A4C980, 00A4CB40 (-8) ->
//   00A4CA40, 00A4D0A0 -> 00A4D2F0.
void* vtordisp_this_00a4c7e0(void* self) noexcept;
void* vtordisp_this_00a4c7f0(void* self) noexcept;
void* vtordisp_this_00a4c800(void* self) noexcept;
void* vtordisp_this_00a4c810(void* self) noexcept;
void* vtordisp_this_00a4c820(void* self) noexcept;
void* vtordisp_this_00a4c830(void* self) noexcept;
void* vtordisp_this_00a4c840(void* self) noexcept;
void* vtordisp_this_00a4c850(void* self) noexcept;
void* vtordisp_this_00a4c940(void* self) noexcept;
void* vtordisp_this_00a4c9f0(void* self) noexcept;
void* vtordisp_this_00a4cb40(void* self) noexcept;
void* vtordisp_this_00a4d0a0(void* self) noexcept;
// Thunks whose targets are in this file, composed: 00A4C7E0 (AL = 0),
// 00A4C810 (texture), 00A4C840 (set texture).
bool thunk_return_false_00a4c7e0(void* self) noexcept;
void* thunk_texture_00a4c810(void* self) noexcept;
void thunk_set_texture_00a4c840(void* self, void* value) noexcept;

// 00A4C770 BSP_MovieDecoder_DestructBases, __thiscall(ECX = shared base S =
// A+10h), RET. Shared vtable = 00D24D98, vtordisp = vbase-10h (0); then
// 00A4D0F0(S) (texture base, through the A+4 subobject, null-checked) and
// 00A4C920(A+8) (file base). Does not destroy the shared base itself.
void movie_decoder_destruct_bases_00a4c770(void* shared, bsp::MovieDecoderHost& host) noexcept;
// 00A4C880 BSP_MovieDecoder_ScalarDelete, __thiscall(ECX = S, flags), RET 4,
// EAX = A: 00A4C770(S), 00A4CF30(S), _free(A) when bit 0 is set.
void* movie_decoder_scalar_delete_00a4c880(void* shared, std::uint32_t flags,
    bsp::MovieDecoderHost& host, MovieBaseHost& base_host);

// 00A4C8B0 BSP_MovieFileBase_Construct, __thiscall(this, construct_vbase), RET
// 4, EAX = this. When the flag is set: vbtable = 00D24E38 and 00A4CEE0 on the
// shared base at +8. Always: shared vtable = 00D24DF0, vtordisp = vbase-8.
void* movie_file_base_construct_00a4c8b0(void* self, std::uint32_t construct_vbase) noexcept;
// 00A4C920 BSP_MovieFileBase_Destruct, __thiscall(ECX = file subobject + 8),
// RET: shared vtable = 00D24DF0, vtordisp = vbase-8 (8 inside the decoder).
void movie_file_base_destruct_00a4c920(void* self_plus_8) noexcept;
// 00A4CA00 BSP_MovieFileBase_ScalarDelete, __thiscall(ECX = file + 8, flags),
// RET 4, EAX = file: 00A4C920 inlined, 00A4CF30 on file + 8, _free(file).
void* movie_file_base_scalar_delete_00a4ca00(void* self_plus_8, std::uint32_t flags,
    bsp::MovieDecoderHost& host, MovieBaseHost& base_host);

// 00A4CCC0 (shared vtable slot +4 in 00D24E78 and 00D24ED8), __thiscall(ECX =
// shared base, float volume), RET 4. Clamp to [0, 1.0 (00D7A24C)], times
// 32768.0 (double 00D24E40), _ftol2 (00BF7420), then BinkSetVolume(handle
// +4, track, v) for tracks 0, 1, 2, 3. Unlike 00A4C980 there is no sqrt(0.5)
// factor and +0Ch is not written.
void movie_set_all_track_volumes_00a4ccc0(bsp::MovieDecoderState& state,
    bsp::MovieDecoderHost& host, float volume);

// 00A4D040 BSP_MovieTextureBase_Construct, __thiscall(this, construct_vbase),
// RET 4, EAX = this. When the flag is set: vbtable = 00D24F20 and 00A4CEE0 on
// the shared base at +0Ch. Always: shared vtable = 00D24ED8, vtordisp =
// vbase-0Ch, texture +4 = 0.
void* movie_texture_base_construct_00a4d040(void* self, std::uint32_t construct_vbase) noexcept;
// 00A4D0F0 BSP_MovieTextureBase_Destruct, __thiscall(ECX = texture subobject
// + 0Ch), RET: shared vtable = 00D24ED8, vtordisp = vbase-0Ch, then the
// texture at [ECX-8] is released as 00A4D0C0 and cleared.
void movie_texture_base_destruct_00a4d0f0(void* self_plus_c, MovieBaseHost& host) noexcept;
// 00A4D2F0 BSP_MovieTextureBase_ScalarDelete, __thiscall(ECX = texture + 0Ch,
// flags), RET 4, EAX = texture subobject: 00A4D0F0 inlined, 00A4CF30, _free.
void* movie_texture_base_scalar_delete_00a4d2f0(void* self_plus_c, std::uint32_t flags,
    bsp::MovieDecoderHost& host, MovieBaseHost& base_host);

// 00A4D0B0 BSP_RefPtrPair_Clear, __fastcall: [ECX] = [ECX+4] = 0.
void ref_ptr_pair_clear_00a4d0b0(void* pair) noexcept;
// 00A4D0C0 BSP_RefPtr_Release, __fastcall(ECX = slot), EAX = slot. A non-null
// object: InterlockedDecrement(object+4) (IAT 00CE2220); zero calls object
// vtable +0; then the slot is cleared. A null slot is left alone.
void** ref_ptr_release_00a4d0c0(void** slot, MovieBaseHost& host) noexcept;

} // namespace bsp::movie_bases_cc13
