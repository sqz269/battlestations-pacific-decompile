// Movie decoder base classes, packet cc13-online2 (docs/CC13_ONLINE_2.md).
#include "bsp/movie_bases_cc13.hpp"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <intrin.h>
#include <limits>

namespace bsp::movie_bases_cc13 {
namespace {

std::byte* bytes(void* p) noexcept { return static_cast<std::byte*>(p); }

template <class T>
T read_field(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
void write_field(void* object, std::ptrdiff_t offset, T value) noexcept {
    std::memcpy(bytes(object) + offset, &value, sizeof(value));
}

// MSVC vbtable: dword [1] is the offset from the vbtable holder to the shared
// virtual base. The native code reads it through the vbtable word; the host
// keeps those words as identities, so the four vbtables' dword [1] values are
// taken from the image (00D24DE4 = 0Ch, 00D24DEC = 10h, 00D24E3C = 8,
// 00D24F24 = 0Ch).
std::int32_t vbase_offset(const void* holder) noexcept {
    switch (read_field<std::uint32_t>(holder, 0)) {
    case 0x00D24DE0u: return 0x0C; // decoder, texture subobject
    case 0x00D24DE8u: return 0x10; // decoder, file subobject
    case 0x00D24E38u: return 0x08; // stand-alone MovieFileBase
    case 0x00D24F20u: return 0x0C; // stand-alone MovieTextureBase
    default: return 0;             // not a vbtable these routines see
    }
}

// `sub ecx, [ecx-4]` (and the optional `sub ecx, 8`).
void* vtordisp_adjust(void* self, std::ptrdiff_t extra) noexcept {
    return bytes(self) - read_field<std::int32_t>(self, -4) - extra;
}

// Stores a vtable identity and its vtordisp the way every base (de)structor
// here does: shared vtable at holder+vb, vtordisp = vb - shared_offset at
// holder+vb-4.
void store_shared_vtable(void* holder, std::uint32_t vtable, std::int32_t shared_offset) noexcept {
    const std::int32_t vb = vbase_offset(holder);
    write_field<std::uint32_t>(holder, vb, vtable);
    write_field<std::int32_t>(holder, vb - 4, vb - shared_offset);
}

void release_ref_slot(void*& object, MovieBaseHost& host) noexcept {
    if (object == nullptr) return;
    auto* count = reinterpret_cast<volatile long*>(bytes(object) + 4);
    if (_InterlockedDecrement(count) == 0) host.destroy_ref_object(object);
    object = nullptr;
}

constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818u;
constexpr std::uint32_t kDecoderSharedVtable = 0x00D24D98u;
constexpr std::uint32_t kFileBaseVbtable = 0x00D24E38u;
constexpr std::uint32_t kFileBaseSharedVtable = 0x00D24DF0u;
constexpr std::uint32_t kTextureBaseVbtable = 0x00D24F20u;
constexpr std::uint32_t kTextureBaseSharedVtable = 0x00D24ED8u;

} // namespace

void* media_singleton_scalar_delete_00a4c470(void* self, std::uint32_t flags,
    void*& published_00f8aef8, MovieBaseHost& host) {
    published_00f8aef8 = nullptr;
    write_field<std::uint32_t>(self, 0, kSingletonBaseVtable);
    if ((flags & 1u) != 0) host.free_00bf65ac(self);
    return self;
}

void nop_ret4_00a4c730() noexcept {}
void nop_00a4c740() noexcept {}
bool return_false_00a4c760() noexcept { return false; }
void nop_ret4_00a4c8f0() noexcept {}
std::uint32_t return_zero_00a4c900() noexcept { return 0; }
std::uint32_t return_zero_ret4_00a4c910() noexcept { return 0; }

void* texture_of_shared_00a4c750(const void* shared) noexcept { return read_field<void*>(shared, -8); }
void set_texture_of_shared_00a4d090(void* shared, void* value) noexcept { write_field<void*>(shared, -8, value); }

void* vtordisp_this_00a4c7e0(void* self) noexcept { return vtordisp_adjust(self, 0); }
void* vtordisp_this_00a4c7f0(void* self) noexcept { return vtordisp_adjust(self, 0); }
void* vtordisp_this_00a4c800(void* self) noexcept { return vtordisp_adjust(self, 8); }
void* vtordisp_this_00a4c810(void* self) noexcept { return vtordisp_adjust(self, 0); }
void* vtordisp_this_00a4c820(void* self) noexcept { return vtordisp_adjust(self, 0); }
void* vtordisp_this_00a4c830(void* self) noexcept { return vtordisp_adjust(self, 0); }
void* vtordisp_this_00a4c840(void* self) noexcept { return vtordisp_adjust(self, 0); }
void* vtordisp_this_00a4c850(void* self) noexcept { return vtordisp_adjust(self, 8); }
void* vtordisp_this_00a4c940(void* self) noexcept { return vtordisp_adjust(self, 0); }
void* vtordisp_this_00a4c9f0(void* self) noexcept { return vtordisp_adjust(self, 8); }
void* vtordisp_this_00a4cb40(void* self) noexcept { return vtordisp_adjust(self, 8); }
void* vtordisp_this_00a4d0a0(void* self) noexcept { return vtordisp_adjust(self, 0); }

bool thunk_return_false_00a4c7e0(void* self) noexcept {
    (void)vtordisp_this_00a4c7e0(self);
    return return_false_00a4c760();
}
void* thunk_texture_00a4c810(void* self) noexcept {
    return texture_of_shared_00a4c750(vtordisp_this_00a4c810(self));
}
void thunk_set_texture_00a4c840(void* self, void* value) noexcept {
    set_texture_of_shared_00a4d090(vtordisp_this_00a4c840(self), value);
}

void movie_decoder_destruct_bases_00a4c770(void* shared, bsp::MovieDecoderHost& host) noexcept {
    void* allocation = bytes(shared) - 0x10;
    // 00A4C789..00A4C7A4: shared vtable 00D24D98, vtordisp vb-10h, through
    // the file vbtable at A+0.
    store_shared_vtable(allocation, kDecoderSharedVtable, 0x10);
    // 00A4C7A8..00A4C7B8: the texture subobject is A+4 (null-checked), so the
    // texture destructor gets (A+4)+0Ch = S.
    void* texture_sub = allocation != nullptr ? bytes(allocation) + 4 : nullptr;
    void* texture_ecx = bytes(texture_sub) + 0x0C;
    // 00A4D0F0's texture release goes through the decoder host's resource
    // release (InterlockedDecrement(+4), vtable +0 at zero).
    store_shared_vtable(bytes(texture_ecx) - 0x0C, kTextureBaseSharedVtable, 0x0C);
    if (void* texture = read_field<void*>(texture_ecx, -8)) {
        host.release_movie_resource(texture);
        write_field<void*>(texture_ecx, -8, nullptr);
    }
    movie_file_base_destruct_00a4c920(bytes(shared) - 8);
}

void* movie_decoder_scalar_delete_00a4c880(void* shared, std::uint32_t flags,
    bsp::MovieDecoderHost& host, MovieBaseHost& base_host) {
    void* allocation = bytes(shared) - 0x10;
    movie_decoder_destruct_bases_00a4c770(shared, host);
    bsp::destroy_movie_decoder_state_00a4cf30(*static_cast<bsp::MovieDecoderState*>(shared), host);
    if ((flags & 1u) != 0) base_host.free_00bf65ac(allocation);
    return allocation;
}

void* movie_file_base_construct_00a4c8b0(void* self, std::uint32_t construct_vbase) noexcept {
    if (construct_vbase != 0) {
        write_field<std::uint32_t>(self, 0, kFileBaseVbtable);
        bsp::construct_movie_decoder_state_00a4cee0(*reinterpret_cast<bsp::MovieDecoderState*>(bytes(self) + 8));
    }
    store_shared_vtable(self, kFileBaseSharedVtable, 8);
    return self;
}

void movie_file_base_destruct_00a4c920(void* self_plus_8) noexcept {
    store_shared_vtable(bytes(self_plus_8) - 8, kFileBaseSharedVtable, 8);
}

void* movie_file_base_scalar_delete_00a4ca00(void* self_plus_8, std::uint32_t flags,
    bsp::MovieDecoderHost& host, MovieBaseHost& base_host) {
    void* file = bytes(self_plus_8) - 8;
    movie_file_base_destruct_00a4c920(self_plus_8);
    // 00A4CA21 passes file+8, which is the shared base only for a complete
    // stand-alone MovieFileBase (vb = 8).
    bsp::destroy_movie_decoder_state_00a4cf30(*static_cast<bsp::MovieDecoderState*>(self_plus_8), host);
    if ((flags & 1u) != 0) base_host.free_00bf65ac(file);
    return file;
}

void movie_set_all_track_volumes_00a4ccc0(bsp::MovieDecoderState& state,
    bsp::MovieDecoderHost& host, float volume) {
    float clamped = volume;
    if (0.0f > volume) clamped = 0.0f;
    else if (volume > 1.0f) clamped = 1.0f;
    const double scaled = static_cast<double>(clamped) * 32768.0;
    const std::int32_t value = std::isnan(scaled)
        ? (host.sse2_truncation_enabled() ? (std::numeric_limits<std::int32_t>::min)() : 0)
        : static_cast<std::int32_t>(scaled);
    for (std::uint32_t track = 0; track < 4; ++track)
        host.bink().set_volume(state.handle, track, value);
}

void* movie_texture_base_construct_00a4d040(void* self, std::uint32_t construct_vbase) noexcept {
    if (construct_vbase != 0) {
        write_field<std::uint32_t>(self, 0, kTextureBaseVbtable);
        bsp::construct_movie_decoder_state_00a4cee0(*reinterpret_cast<bsp::MovieDecoderState*>(bytes(self) + 0x0C));
    }
    store_shared_vtable(self, kTextureBaseSharedVtable, 0x0C);
    write_field<void*>(self, 4, nullptr);
    return self;
}

void movie_texture_base_destruct_00a4d0f0(void* self_plus_c, MovieBaseHost& host) noexcept {
    store_shared_vtable(bytes(self_plus_c) - 0x0C, kTextureBaseSharedVtable, 0x0C);
    void* texture = read_field<void*>(self_plus_c, -8);
    release_ref_slot(texture, host);
    write_field<void*>(self_plus_c, -8, texture);
}

void* movie_texture_base_scalar_delete_00a4d2f0(void* self_plus_c, std::uint32_t flags,
    bsp::MovieDecoderHost& host, MovieBaseHost& base_host) {
    void* texture_sub = bytes(self_plus_c) - 0x0C;
    movie_texture_base_destruct_00a4d0f0(self_plus_c, base_host);
    bsp::destroy_movie_decoder_state_00a4cf30(*static_cast<bsp::MovieDecoderState*>(self_plus_c), host);
    if ((flags & 1u) != 0) base_host.free_00bf65ac(texture_sub);
    return texture_sub;
}

void ref_ptr_pair_clear_00a4d0b0(void* pair) noexcept {
    write_field<void*>(pair, 0, nullptr);
    write_field<void*>(pair, 4, nullptr);
}

void** ref_ptr_release_00a4d0c0(void** slot, MovieBaseHost& host) noexcept {
    release_ref_slot(*slot, host);
    return slot;
}

} // namespace bsp::movie_bases_cc13
