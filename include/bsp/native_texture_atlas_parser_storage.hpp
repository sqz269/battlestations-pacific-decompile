#pragma once
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_string_copy_concat_exact.hpp"
#include "bsp/native_texture_atlas_item_storage.hpp"
#include "bsp/native_texture_atlas_item_parameters_storage.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include <cstdint>
#include <exception>
#include <list>

namespace bsp {
namespace game { class GameNativeReadOnlyData; class GameNativeRendererApplication; }
struct NativePathCanonicalizerRawContext;
struct NativeTextureAtlasParserFrame;

// Cold borrowed composition. No atlas, buffer, renderer, scratch, publication,
// pool or feature cell is created. All arguments name the SAME genuine process,
// canonical pool/feature/RO, shared F8C2C8 and Source-created renderer graph.
// Keep these exact objects through partial-operation disposition and full drain.
class NativeTextureAtlasParserContext final {
public:
    NativeTextureAtlasParserContext(const game::GameNativeReadOnlyData&,
        NativeStringRawPoolContext&, const volatile std::uint32_t& actual_feature_0109eea4,
        char* actual_shared_line_scratch_00f8c2c8, NativePathCanonicalizerRawContext&,
        game::GameNativeRendererApplication&);
private:
    friend std::uint8_t parse_native_texture_atlas_storage_00aeeaf0(void*,
        volatile std::uint32_t&, NativeTextureAtlasParserFrame&,
        NativeTextureAtlasParserContext&);
    const game::GameNativeReadOnlyData& data_;
    NativeStringRawPoolContext& strings_;
    const volatile std::uint32_t& feature_;
    char* const scratch_;
    NativePathCanonicalizerRawContext& canonical_;
    game::GameNativeRendererApplication& renderer_;
    NativeTextureCacheContext& cache_;
    NativeTextureAtlasItemStorageContext items_;
    NativeTextureAtlasItemParameterContext parameters_;
    const char* const literals_[6]; // TextureAtlas, '/', '{', '}', empty, TextureItem.
};

enum class NativeTextureAtlasParserPhase { fresh, entered, complete, failed };
// Native S+10..S+50 homes, after the parser's extra PUSH EBX. No initializer
// seeds their preimages. Header-token0 and item-name-token reuse one4h HOME;
// canonical/slash raw8 is reused, then its first DWORD becomes allocation HOME.
struct NativeTextureAtlasParserHomes final {
    NativePooledTextStorage line;
    std::uint32_t flags;
    NativePooledTextStorage texture_token, keyword, header_or_item_name_token;
    std::uint32_t manager_home, saved_texture_home;
    std::uint32_t canonical_slash_allocation[2], directory[2], texture_name[2];
    std::uint32_t full_name[2], item_name[2];
};
// Source-adopted BEFORE this iteration's real BF681B(30h). Distinct nonmoving
// helper frames and outgoing argument HOMEs survive every later iteration.
struct NativeTextureAtlasParserItemCall final {
    NativeTextureAtlasParserItemCall() {}
    NativeTextureAtlasParserItemCall(const NativeTextureAtlasParserItemCall&)=delete;
    NativeTextureAtlasParserItemCall& operator=(const NativeTextureAtlasParserItemCall&)=delete;
    std::uint32_t constructor_homes[3], parameter_buffer_home;
    NativeTextureAtlasItemConstructFrame constructor;
    NativeTextureAtlasItemParameterFrame parameters;
    NativeTextureAtlasParserPhase phase{NativeTextureAtlasParserPhase::fresh};
    void* allocation{};
    void* item_return{};
    void* append_header{};
    std::uint32_t append_slot{}, active_site{};
    bool allocation_returned{}, constructor_entered{}, parameters_entered{};
    bool slot_written{}, count_incremented{};
    std::exception_ptr failure;
};
struct NativeTextureAtlasParserFrame final {
    NativeTextureAtlasParserFrame() {}
    NativeTextureAtlasParserFrame(const NativeTextureAtlasParserFrame&)=delete;
    NativeTextureAtlasParserFrame& operator=(const NativeTextureAtlasParserFrame&)=delete;
    NativeTextureAtlasParserFrame(NativeTextureAtlasParserFrame&&)=delete;
    NativeTextureAtlasParserFrame& operator=(NativeTextureAtlasParserFrame&&)=delete;
    NativeTextureAtlasParserHomes native;
    std::uint32_t preserve_home;
    NativeStringHeaderCopyExactFrame directory_copy;
    NativeStringHeaderConcatExactFrame full_name_concat;
    // The existing actual B319B0 acquisition, distinct for THIS reached call.
    // No second observation/creator/factory record or shared scratch substitute.
    NativeTextureCacheAcquired texture_acquired;
    std::list<NativeTextureAtlasParserItemCall> item_calls;
    NativeTextureAtlasParserPhase phase{NativeTextureAtlasParserPhase::fresh};
    void* actual_manager{};
    void* saved_buffer_receiver{};
    void* captured_renderer{};
    std::uint32_t captured_profile_address{}, captured_slot64{};
    void* texture_register{};
    void* texture_append_header{};
    std::uint32_t texture_append_slot{}, active_site{};
    std::int32_t native_state{-1};
    NativeTextureAtlasParserItemCall* active_item{};
    bool cache_entered{}, texture_slot_written{}, texture_count_incremented{};
    std::uint8_t normal_result{};
    std::exception_ptr failure;
};

// Whole478-row ordinary NORMAL AEEAF0, original ECX manager1Ch, one buffer
// DWORD HOME/RET4, AL Boolean. New four-argument Source API, not original ABI.
// The incoming HOME is real initialized ordinary DWORD backing, borrowed via
// volatile reference. Capture buffer BEFORE rewind. Directory canonicalization
// is a distinct throwaway raw8: it is never assigned back. Receiver F8D394 is
// captured BEFORE concat; its CURRENT profile-address/slot64 AFTER concat. The
// admitted SAME Source-created renderer invokes existing actual B319B0/cache.
// Actual owning texture pointer append precedes any item-body line. Real raw
//735FF0/735F90 and current table/count/capacity/write/increment order remain.
//
// Every matched item adopts metadata before BF681B; then token1, exact name8,
// genuine AEE4D0 with CURRENT incoming buffer HOME+C. Cleanup bits/order remain.
// After cleanup copy CURRENT incoming HOME bits into a DISTINCT child AEE620
// HOME. Its later F32/CW reuse NEVER overwrites the still-live parser input.
// Parameters execute even for original allocation-null; no default/recovery is
// added. Restore saved texture after item append; footer returns directory,
// TextureName, line and returns1 even on EOF. No default UV/fake graph exists.
//
// Caller retains actual1Ch manager, actual1Ch initialized buffer, all reached
// raw headers/payload/arrays/items/textures, acquisitions/frames/provider graph,
// genuine shared scratch, canonical pool/feature/RO/renderer and CPU/DF/lifetime
// domains. Serialize scratch; forbid writes into private Source controls/HOMEs
// except the declared argument/header aliases. Native private-stack aliases,
// FH3/SEH/hardware/null/unmasked-FP faults/Origin/ABI/runtime/game are qualified.
// Fresh retained Frame cannot replay. C++ escape records failure and barethrows
// WITHOUT outer cleanup/free/replay or graph erasure. Keep full partial debt;
// inherited actual cache failure-destructor policy still requires retention.
std::uint8_t parse_native_texture_atlas_storage_00aeeaf0(void* actual_manager1c,
    volatile std::uint32_t& actual_buffer_home, NativeTextureAtlasParserFrame&,
    NativeTextureAtlasParserContext&);
} // namespace bsp
