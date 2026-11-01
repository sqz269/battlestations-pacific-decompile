#pragma once
#include "bsp/native_pooled_text.hpp"
#include <cstdint>
#include <exception>
#include <new>

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Cold, stable borrows of the genuine raw pool, shared F8C2C8 line scratch,
// installed PE literals and CE5F90 double. No atlas, texture, text buffer,
// pointer publication, default scalar, callback or Native owner is made here.
class NativeTextureAtlasItemParameterContext final {
public:
    NativeTextureAtlasItemParameterContext(const game::GameNativeReadOnlyData&,
        NativeStringRawPoolContext&, char* actual_shared_line_scratch_00f8c2c8);
private:
    friend std::uint8_t read_native_texture_atlas_item_parameters_00aee620(void*,
        volatile std::uint32_t&, struct NativeTextureAtlasItemParameterFrame&,
        NativeTextureAtlasItemParameterContext&);
    NativeStringRawPoolContext& strings_;
    char* const scratch_;
    const char* const literals_[8]; // {, }, empty, Param, U1, V1, U2, V2.
    const volatile double& packed_scale_00ce5f90_;
};

enum class NativeTextureAtlasItemParameterPhase { fresh, entered, complete, failed };
// Native S+20 is token2's U2 pointer then the conversion DWORD. Switching the
// active trivial Source member performs no write; the x87 block's reached
// stores supply the new representation. Historical token pointers are not read
// after that switch. This is an explicit operand alias, not a second HOME.
union NativeTextureAtlasItemParameterU2Home {
    NativePooledTextStorage token;
    std::uint32_t conversion;
    NativeTextureAtlasItemParameterU2Home() {}
    ~NativeTextureAtlasItemParameterU2Home() {}
};
struct NativeTextureAtlasItemParameterFrame final {
    NativeTextureAtlasItemParameterFrame() {
        ::new (static_cast<void*>(&value_u2.token)) NativePooledTextStorage;
    }
    NativeTextureAtlasItemParameterFrame(const NativeTextureAtlasItemParameterFrame&) = delete;
    NativeTextureAtlasItemParameterFrame& operator=(const NativeTextureAtlasItemParameterFrame&) = delete;
    NativeTextureAtlasItemParameterFrame(NativeTextureAtlasItemParameterFrame&&) = delete;
    NativeTextureAtlasItemParameterFrame& operator=(NativeTextureAtlasItemParameterFrame&&) = delete;
    // Consecutive ordinary subobjects corresponding to Native S+0C..S+24.
    // Only line is explicitly zeroed by this entry. Token constructors own
    // their first writes; no Frame-wide initialization seeds any native byte.
    NativePooledTextStorage line, parameter, first_token, value_u1, value_v1;
    NativeTextureAtlasItemParameterU2Home value_u2;
    NativePooledTextStorage value_v2;
    // Source history, not a Native stack/Origin/cleanup capability.
    NativeTextureAtlasItemParameterPhase phase{NativeTextureAtlasItemParameterPhase::fresh};
    void* actual_item{};
    std::uint32_t active_site{};
    std::int32_t native_state{-1};
    std::exception_ptr failure;
};

// Whole ordinary AEE620 normal payload/call order, including the final x87
// packing. Original ECX item30h, one buffer DWORD HOME/RET4, AL1 becomes this
// four-argument Source API. actual_buffer_home is initialized ordinary DWORD
// backing borrowed through a volatile view. Its CURRENT bits supply each
// reached buffer receiver. After the last line call the SAME HOME is reused
// for the F32 reload and saved x87 CW; it is not restored to a buffer pointer.
//
// First seek uses its current HOME at each reached iteration; the immediate
// second line call uses the last seek's captured receiver. Main-loop line
// calls reload the HOME. Param then U1/V1/U2/V2 comparisons and token/release
// order are literal. Missing fields retain their actual item preimages, EOF
// still packs them, no default UV/extent is supplied. Each atof ST0 is stored
// directly to its reached F32 field before any other C++/provider call.
// Final six FISTP stores use Native RC|0C00 and restore the reached saved CW
// after each store, retaining the original x87 stack/read/store schedule.
//
// Caller proves genuine item30/text-buffer/raw string/pool/RO/shared-scratch
// lifetime, readable initialized reached fields and operand/argument aliases.
// Serialize shared scratch and forbid callback writes to private Source
// controls. All Frame subobjects and incoming HOME are retained separately
// from item/buffer/provider-write extents except the declared U2 alias.
// Fresh per-call Frame; no replay. An escaping C++ exception records failure
// and barethrows WITHOUT new outer cleanup, token replay or record erasure.
// Keep every partial allocation/header/helper/provider debt through explicit
// external disposition. Inherited raw text/token cleanup policies remain.
// This is NORMAL payload, not Native FH3/SEH/Origin/fault/unmasked-FP or original
// stack/return-high-bytes/ABI parity. The current Source CRT supplies atof and
// full stricmp behavior; no Original/vendor reconstruction credit is claimed.
// This leaf creates no production atlas, Page/type6 binding or activation.
std::uint8_t read_native_texture_atlas_item_parameters_00aee620(void* actual_item30,
    volatile std::uint32_t& actual_buffer_home,
    NativeTextureAtlasItemParameterFrame&, NativeTextureAtlasItemParameterContext&);
} // namespace bsp
