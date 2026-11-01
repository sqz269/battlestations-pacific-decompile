#pragma once
#include "bsp/native_language_catalog_storage.hpp"
#include "bsp/native_texture_atlas_buffer_fit.hpp"
#include "bsp/native_texture_atlas_loader_string_exact.hpp"
#include "bsp/native_texture_atlas_parser_storage.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include <cstdint>
#include <exception>
#include <list>

namespace bsp {
namespace game { class GameNativeReadOnlyData; struct GameNativeVfsRawServices; }
struct NativeStringRawPoolContext;
struct NativeTextureAtlasLoaderFrame;
struct NativeTextureAtlasDescriptorFrame;
struct NativeTextureAtlasSplitNameFrame;

// Cold borrowed composition, with no native birth, registry, publication,
// buffers or alternate VFS/pool. Keep these EXACT objects and their underlying
// genuine process graph through every partial call and the eventual drain.
// The parser must be composed from the same RO/raw pool/live feature and the
// same Source-created renderer/cache already using these VFS services. That
// genuine creator/custody relationship is a caller obligation, not a token
// minted by this view, an empty manager, a numeric profile or a name lookup.
class NativeTextureAtlasLoaderContext final {
public:
    NativeTextureAtlasLoaderContext(const game::GameNativeReadOnlyData&,
        NativeStringRawPoolContext&, const volatile std::uint32_t& actual_feature_0109eea4,
        game::GameNativeVfsRawServices&, void* volatile& actual_atlas_publication_00f8c26c,
        const char* actual_empty_name_00f8c270, NativeTextureAtlasParserContext&,
        std::size_t actual_process_shared_scratch_extent);
private:
    friend void load_native_texture_atlas_storage_00af0060(void*,
        volatile std::uint32_t&, NativeTextureAtlasLoaderFrame&,NativeTextureAtlasLoaderContext&);
    friend void register_native_texture_atlas_descriptor_00aef280(void*,
        volatile std::uint32_t&, NativeTextureAtlasDescriptorFrame&,NativeTextureAtlasLoaderContext&);
    friend std::uint8_t match_native_texture_atlas_split_name_00aef3c0(
        const void*,const void*,NativeTextureAtlasSplitNameFrame&,NativeTextureAtlasLoaderContext&);
    const game::GameNativeReadOnlyData& data_;
    NativeStringRawPoolContext& strings_;
    const volatile std::uint32_t& feature_;
    game::GameNativeVfsRawServices& vfs_;
    void* volatile& atlas_publication_;
    const char* const empty_name_;
    NativeTextureAtlasParserContext& parser_;
    const std::size_t scratch_extent_; // SAME process owner actual allocation, no default.
    // Exact concrete object, not a supplied callback subclass. Its established
    // full 886280 normal raw-list schedule uses the SAME real BDD990 route.
    NativeLanguageResourceEnumerationCalls enumeration_calls_;
    const void* const extension_ats_, *const directory_default_;
    const void* const slash_, *const backslash_, *const underscore_;
    const void* const extension_dot_ats_;
};
enum class NativeTextureAtlasLoaderPhase { fresh, entered, complete, failed };

// Original matcher native raw homes S+10..S+3F, after all four nonvolatile
// pushes: saved candidate HOME is later overwritten by auxiliary raw8. Slash
// construction initially uses candidate; it is subsequently overwritten by the
// actual candidate clone. No default initializer invents those preimages.
struct NativeTextureAtlasSplitNameHomes final {
    std::uint32_t auxiliary[2], candidate[2], requested[2], temporary[2];
    std::uint32_t joined[2], stem[2];
};
struct NativeTextureAtlasSplitNameFrame final {
    NativeTextureAtlasSplitNameFrame() {}
    NativeTextureAtlasSplitNameFrame(const NativeTextureAtlasSplitNameFrame&)=delete;
    NativeTextureAtlasSplitNameFrame& operator=(const NativeTextureAtlasSplitNameFrame&)=delete;
    NativeTextureAtlasSplitNameHomes native;
    std::uint32_t preserve_home, first_replace_homes[3], second_replace_homes[3];
    std::uint32_t candidate_directory_homes[3], requested_directory_homes[3];
    std::uint32_t requested_tail_homes[3], candidate_tail_homes[3];
    std::uint32_t requested_suffix_homes[3], candidate_suffix_homes[3];
    std::uint32_t stem_homes[3], candidate_prefix_homes[3];
    NativeAtlasStringReplaceFrame first_replace, second_replace;
    NativeAtlasStringSubstringFrame candidate_directory, requested_directory;
    NativeAtlasStringSubstringFrame requested_tail, candidate_tail;
    NativeAtlasStringAssignFrame requested_assignment, candidate_assignment;
    NativeAtlasStringSubstringFrame requested_suffix, candidate_suffix, stem, candidate_prefix;
    NativeStringHeaderConcatExactFrame underscore_concat;
    NativeTextureAtlasLoaderPhase phase{NativeTextureAtlasLoaderPhase::fresh};
    std::uint32_t active_site{}, captured_slash_data{}, captured_slash_length{};
    std::uint32_t captured_backslash_data{}, captured_backslash_length{};
    std::uint32_t requested_length_register{}, candidate_length_register{};
    std::int32_t requested_slash_index{}, candidate_slash_index{}, native_state{-1};
    std::uint8_t normal_result{};
    std::exception_ptr failure;
};

struct NativeTextureAtlasDescriptorFrame final {
    NativeTextureAtlasDescriptorFrame() {}
    NativeTextureAtlasDescriptorFrame(const NativeTextureAtlasDescriptorFrame&)=delete;
    NativeTextureAtlasDescriptorFrame& operator=(const NativeTextureAtlasDescriptorFrame&)=delete;
    std::uint32_t native_name[2], preserve_home, parser_buffer_home;
    // Two distinct existing acquisitions. The BDF4C0 admission result never
    // substitutes for the independent resolve INSIDE the actual AF5850 body.
    NativeVfsNameResolutionAcquired admission_resolution, file_resolution;
    NativeParticleTextBufferReadFrame file_read;
    NativeTextureAtlasBufferFitFrame buffer_fit;
    NativeTextureAtlasParserFrame parser;
    NativeTextureAtlasLoaderPhase phase{NativeTextureAtlasLoaderPhase::fresh};
    void* ignored_receiver{};
    void* allocation_return{};
    void* text_register{};
    void* captured_descriptor{};
    void* captured_parser_manager{};
    const char* diagnostic_name{};
    std::uint32_t active_site{};
    std::int32_t native_state{-1};
    bool allocation_entered{}, allocation_returned{}, constructor_entered{};
    bool admission_entered{}, admission_result{}, file_entered{}, fit_entered{}, parser_entered{};
    bool text_destroyed{}, text_freed{};
    std::exception_ptr failure;
};

// Source metadata is retained per popped candidate BEFORE the real pop, not
// an atlas/name-vector projection. Native list and native candidate header live
// only in the enclosing frame's actual raw homes, as in the original loop.
struct NativeTextureAtlasLoaderCandidate final {
    NativeTextureAtlasLoaderCandidate() {}
    NativeTextureAtlasLoaderCandidate(const NativeTextureAtlasLoaderCandidate&)=delete;
    NativeTextureAtlasLoaderCandidate& operator=(const NativeTextureAtlasLoaderCandidate&)=delete;
    NativeTextureAtlasSplitNameFrame matcher;
    NativeTextureAtlasDescriptorFrame descriptor;
    std::uint32_t descriptor_home;
    NativeTextureAtlasLoaderPhase phase{NativeTextureAtlasLoaderPhase::fresh};
    std::uint32_t active_site{}, texture_index{};
    void* current_texture_array{};
    void* current_texture{};
    void* current_texture_name{};
    bool matcher_result{}, requested_already_present{}, descriptor_entered{};
    std::exception_ptr failure;
};
struct NativeTextureAtlasLoaderHomes final {
    std::uint32_t lowercase_request[2], directory[2], candidate[2], temporary[2];
    std::uint32_t list[3];
};
struct NativeTextureAtlasLoaderFrame final {
    NativeTextureAtlasLoaderFrame() {}
    NativeTextureAtlasLoaderFrame(const NativeTextureAtlasLoaderFrame&)=delete;
    NativeTextureAtlasLoaderFrame& operator=(const NativeTextureAtlasLoaderFrame&)=delete;
    NativeTextureAtlasLoaderHomes native;
    std::uint32_t preserve_home, suffix_homes[3], directory_homes[3];
    NativeAtlasStringSubstringFrame suffix, directory_substring;
    NativeLanguageResourceEnumerationOperation enumeration;
    std::list<NativeTextureAtlasLoaderCandidate> candidates;
    NativeTextureAtlasLoaderPhase phase{NativeTextureAtlasLoaderPhase::fresh};
    void* captured_manager{};
    const void* captured_request{};
    const char* diagnostic_name{};
    std::uint32_t active_site{};
    std::int32_t native_state{-1}, final_separator_index{};
    bool has_ats_suffix{}, list_cleared{}, sentinel_freed{};
    std::exception_ptr failure;
};

// Whole normal516-row AEF3C0. Original ECX=requested raw8, EDX=candidate raw8,
// plain RET/AL. Clone/lower/replace BOTH sides with genuine raw storage. Exact
// initial equality, unsigned length gate, same signed rfind('/') index (the
// established helper does not test position0), directory comparison/strip,
// two suffix tests and request-span-driven '_' prefix remain in original order.
std::uint8_t match_native_texture_atlas_split_name_00aef3c0(const void* requested8,
    const void* candidate8,NativeTextureAtlasSplitNameFrame&,NativeTextureAtlasLoaderContext&);

// Whole normal92-row AEF280. Native receiver is UNUSED. Incoming descriptor
// HOME is read AFTER real BF681B(1C)/AF5600. Independent BDF4C0 admission then
// actual AF5850, whose result is ignored. RELOAD CURRENT F8C26C after AF5850,
// initialize a DISTINCT parser buffer HOME, and directly invoke whole AEEAF0.
// On every normal arm destroy/free the actual captured TextBuffer and return
// current copied-name storage. No file-not-found success or parser substitute.
void register_native_texture_atlas_descriptor_00aef280(void* ignored_receiver,
    volatile std::uint32_t& descriptor_header_home,NativeTextureAtlasDescriptorFrame&,
    NativeTextureAtlasLoaderContext&);

// Whole normal304-row AF0060. ECX actual1C manager, one requested-header HOME,
// RET4. Capture that HOME once before logging. Raw lowercase/suffix/directory;
// actual886280(flags0), real557A90 per candidate, whole raw matcher, CURRENT
// captured-manager +14/+10 texture-array scan against CURRENT ORIGINAL REQUEST,
// then real AEF280. Every matched candidate starts a fresh scan after previous
// callbacks, so publication/registry/name changes retain their native epochs.
void load_native_texture_atlas_storage_00af0060(void* actual_manager1c,
    volatile std::uint32_t& requested_header_home,NativeTextureAtlasLoaderFrame&,
    NativeTextureAtlasLoaderContext&);

// Caller retains all native raw bytes, acquired children, unchanged genuine
// VFS/renderer/pool/canonical/feature/RO graph and failed attempts. No automatic
// cleanup, retry, rollback or synthetic empty graph is supplied. Inherited VFS
// string-wrapper/CRT/source exception contracts remain explicitly qualified.
// Native argument/private-stack aliases, ABI/FH3/SEH/fault/NoThrow/Origin,
// selected outer unwind, actual common.ats startup and Page/game validation
// remain external. No CFEAE8/singleton registration or activation occurs here.
} // namespace bsp
