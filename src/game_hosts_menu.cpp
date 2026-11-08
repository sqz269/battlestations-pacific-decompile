// bsp_game.exe milestone 2c: the frame's game state and profiler, the title
// pump and the main menu. See include/bsp/game_hosts_menu.hpp for the address
// list and the evidence.
#include "bsp/game_hosts_menu.hpp"
#include "bsp/frontend_seg16_24_28_cc13.hpp"
#include "bsp/game_hosts_scene_contents.hpp"

#include "bsp/frontend_entry.hpp"
#include "bsp/loading_screen_elements.hpp"
#include "bsp/locale_text_lookup.hpp"
#include "bsp/native_frame_clock_lifetime.hpp"
#include "bsp/native_frame_clock_publication.hpp"
#include "bsp/engine_core_cc13.hpp"
#include "bsp/engine_services_cc13.hpp"
#include "bsp/sound_frame_service.hpp"
#include "bsp/game_native_input_configuration_application.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_input_runtime.hpp"
#include "bsp/ingame_gui_cc13.hpp"
#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/oct18_tail_a.hpp"                // 0054ED00
#include "bsp/frontend_managers.hpp"
#include "bsp/frontend_screen_sets.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/frontend_states.hpp"
#include "bsp/frontend_seg12_17_cc13.hpp"     // 00563370, 00555770
#include "bsp/frontend_seg15_22_cc13.hpp"
#include "bsp/frontend_seg19_21_cc13.hpp"
#include "bsp/frontend_screens_cc13.hpp"
#include "bsp/game_native_settings_process.hpp"
#include <cstdlib>
#include "bsp/frontend_screen_animation.hpp"  // 00683610
#include "bsp/options_menu_screens.hpp"
#include "bsp/segs_13_15_22_44_cc13.hpp"      // 0051E4D0
#include "bsp/fmod_configuration_library.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/game_hosts_hud.hpp"
#include "bsp/game_hosts_hud_world.hpp"
#include "bsp/game_hosts_mission.hpp"
#include "bsp/game_hint_passes.hpp"
#include "bsp/game_profile_load.hpp"
#include "bsp/game_profile_owner.hpp"
#include "bsp/game_sound_runtime.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/main_menu_screens.hpp"
#include "bsp/main_menu_command_owner.hpp"
#include "bsp/native_render_queue_destruction.hpp"
#include "bsp/press_start_screen.hpp"
#include "bsp/render_queue_seg83_cc13.hpp"
#include "bsp/title_init.hpp"
#include "bsp/award_grant.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_title_sound.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_sample_runtime.hpp"
#include "bsp/sound_stream_owner.hpp"
#include "bsp/sound_stream_runtime.hpp"
#include "bsp/engine_services_cc13.hpp"
#include "bsp/game_native_vfs_application.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_fileblock_identifier.hpp"
#include "bsp/native_fileblock_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_fileblock_scope.hpp"
#include "bsp/native_vfs_open_logging.hpp"

#include <shlobj.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <cwctype>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace bsp::game {
namespace {

// The helm `input` line's requests (inject_input_action_hold), taken into the
// menu host's records at the next frame's input prologue: action -> frames.
std::map<int, unsigned>& pending_input_holds() {
    static std::map<int, unsigned> holds;
    return holds;
}

// Packet cc9_menu_pump_once: during a mission frame the front-end pump 004f8830
// runs only from the mission frame's own 004c40f0 passes (GameMenuHost::frame).
constexpr bool kMenuPumpYieldsToMissionFrameBound = true;  // ON: 17 rows identical (GUNNERY_OPEN_ITEMS 76.4)

// Packet cc15-fe1: the front-end frame's two GUI-manager calls, 00aa0e50 at
// 004e4c38..004e4c52 and 00aa0e00 after it, run blocking_screen.cpp's bodies on
// the host manager's +50h pointer slot instead of only being recorded.
constexpr bool kFrontEndPointerCc13Bound = true;

// Packet cc15-ls1: the front-end shell's begin_loading (0057cb60) and end_loading
// (0057c250) run frontend_entry.cpp's begin_loading_screen / end_loading_screen
// over MenuLoadingScreenHost and the Impl's LoadingScreen, instead of being only
// recorded. The host state they reach is the frame-layout selector 00518250
// (mode 0 selects set 2 with commit 1) and the GUI enable 00aa0e00 (under
// kFrontEndPointerCc13Bound); every other call is a record row.
// ON moves the front end, and the image does the same: mode 0 at 0057cd14 pushes
// EBX (1 since 0057cc22, callee-saved through the calls between) and 2, then
// 004c1ac0 (no arguments) and 00518250(set 2, commit 1) at 0057cd1e. Committing
// set 2 loads that set's own layout rows from 00E08500 / 00E08604 / 00E0862C,
// among them its FE_frame / FE_frame_title. So pages 63 -> 67, widgets 776 -> 792 and the
// two page scripts' Lua getters (128 -> 130) on USN01 (cc15-ls1 pair) are
// the image's behaviour, not a host artefact.
// cc15-ls2: the same switch runs 0057BEC0 (report_loading_progress) at both
// report sites over that screen. A report with no screen is the image's no-op.
constexpr bool kLoadingScreenCc13Bound = true;

// 004f8710 publishes screen ids; the level-4 set the main-menu manager raises
// has exactly one element (docs/MAIN_MENU_PATH.md).
constexpr int kNoScreenSlot = -1;

std::string path_step_text(MainMenuPathStep step) {
    const std::string_view name = main_menu_path_step_name(step);
    return std::string(name.data(), name.size());
}

// kWireGoal2Chain54Bound: the EngineServiceCalls 00BE0AE0 reaches. Its body
// (engine_services_cc13.cpp) calls only 00BDF950 and 00BE0980; both route to
// the existing native FileBlock bodies over the VFS host's live context, the
// same pair 00BE0A30 (byte-identical code, other EH handler) runs for the title
// block. Every other service is outside this body's contract and throws.
namespace es = bsp::engine_services_cc13;
using es::NativeArrayHeader;
using es::NativeContainerHeader;
using es::NativeIterator;

class MenuFileBlockScopeCalls final : public es::EngineServiceCalls {
public:
    MenuFileBlockScopeCalls(NativeStringStorage& strings, NativeVfsFileBlockScopeContext& scopes,
        NativeVfsFileBlockScopeAcquired& entry)
        : strings_(strings), scopes_(scopes), entry_(entry) {}

    // 00BE0B5E: ECX = the scope.
    void prepare_fileblock_identifier_00bdf950(void* scope) override {
        prepare_native_fileblock_identifier_00bdf950(scope, strings_);
    }
    // 00BE0B6F: ECX = [0109CEEC] reloaded after 00BDF950, stack (&scope+14h,
    // gate); 00BE0980 reads the gate's low byte.
    void enter_fileblock_00be0980(void* vfs, const NativeString* name, std::uint32_t arg) override {
        enter_native_vfs_fileblock_00be0980(vfs, name, static_cast<std::uint8_t>(arg), scopes_, entry_);
    }
    void invalid_parameter_00bf6713() override { unused("invalid_parameter_00bf6713"); }
    void advance_vfs_mount_iterator_00bd97e0(NativeIterator&) override { unused("advance_vfs_mount_iterator_00bd97e0"); }
    void advance_physical_index_iterator_00bd9860(NativeIterator&) override { unused("advance_physical_index_iterator_00bd9860"); }
    void retreat_vfs_mount_iterator_00bd99b0(NativeIterator&) override { unused("retreat_vfs_mount_iterator_00bd99b0"); }
    void retreat_physical_index_iterator_00bd9a40(NativeIterator&) override { unused("retreat_physical_index_iterator_00bd9a40"); }
    void advance_filestore_resident_iterator_00be4c30(NativeIterator&) override { unused("advance_filestore_resident_iterator_00be4c30"); }
    void retreat_filestore_resident_iterator_00be4d20(NativeIterator&) override { unused("retreat_filestore_resident_iterator_00be4d20"); }
    void advance_filestore_pending_iterator_00be4e40(NativeIterator&) override { unused("advance_filestore_pending_iterator_00be4e40"); }
    void retreat_filestore_pending_iterator_00be4db0(NativeIterator&) override { unused("retreat_filestore_pending_iterator_00be4db0"); }
    void* allocate_list_head_0c_00bda960() override { unused("allocate_list_head_0c_00bda960"); }
    void* allocate_list_head_28_00bda980() override { unused("allocate_list_head_28_00bda980"); }
    void* allocate_tree_head_20_00bdaba0() override { unused("allocate_tree_head_20_00bdaba0"); }
    void* allocate_tree_head_24_00bdabf0() override { unused("allocate_tree_head_24_00bdabf0"); }
    void* allocate_filestore_resident_head_00be55e0() override { unused("allocate_filestore_resident_head_00be55e0"); }
    void* allocate_filestore_pending_head_00be5690() override { unused("allocate_filestore_pending_head_00be5690"); }
    void* allocate_pooled_string_list_head_004c3020() override { unused("allocate_pooled_string_list_head_004c3020"); }
    void destroy_physical_index_subtree_00bdf7a0(void*) override { unused("destroy_physical_index_subtree_00bdf7a0"); }
    void destroy_vfs_mount_subtree_00bdf7e0(void*) override { unused("destroy_vfs_mount_subtree_00bdf7e0"); }
    void erase_filestore_pending_subtree_00be66c0(void*) override { unused("erase_filestore_pending_subtree_00be66c0"); }
    void erase_filestore_resident_subtree_00be6720(void*) override { unused("erase_filestore_resident_subtree_00be6720"); }
    void erase_physical_index_range_00be0c30(NativeIterator&, NativeContainerHeader&, NativeIterator, NativeIterator) override { unused("erase_physical_index_range_00be0c30"); }
    void erase_vfs_mount_range_00be0d00(NativeIterator&, NativeContainerHeader&, NativeIterator, NativeIterator) override { unused("erase_vfs_mount_range_00be0d00"); }
    void erase_vfs_mount_iterator_00be0080(NativeIterator&, NativeContainerHeader&, NativeIterator) override { unused("erase_vfs_mount_iterator_00be0080"); }
    void destroy_vfs_request_payload_00be1220(void*) override { unused("destroy_vfs_request_payload_00be1220"); }
    void clear_vfs_request_list_00be19e0(NativeContainerHeader&) override { unused("clear_vfs_request_list_00be19e0"); }
    void destroy_allocation_stats_00be27f0(void*) override { unused("destroy_allocation_stats_00be27f0"); }
    void construct_allocation_stats_base_00be2750(void*) override { unused("construct_allocation_stats_base_00be2750"); }
    void destroy_window_platform_base_00be2b10(void*) override { unused("destroy_window_platform_base_00be2b10"); }
    void destroy_filestore_payload_00be5d70(void*) override { unused("destroy_filestore_payload_00be5d70"); }
    void destroy_frame_clock_base_00bedea0(void*) override { unused("destroy_frame_clock_base_00bedea0"); }
    void construct_frame_clock_base_00bede00(void*) override { unused("construct_frame_clock_base_00bede00"); }
    void destroy_physical_read_request_names_00bf3880(void*) override { unused("destroy_physical_read_request_names_00bf3880"); }
    void destroy_lua_state_owner_00bd4270(void*) override { unused("destroy_lua_state_owner_00bd4270"); }
    void destroy_text_queue_00bec730(void*) override { unused("destroy_text_queue_00bec730"); }
    void* create_text_queue_sentinel_00bec710() override { unused("create_text_queue_sentinel_00bec710"); }
    void destroy_ref_counted_base_00bd30f0(void*) override { unused("destroy_ref_counted_base_00bd30f0"); }
    void copy_pooled_string_list_004d48a0(void*, const void*) override { unused("copy_pooled_string_list_004d48a0"); }
    void reserve_vfs_pair_vector_00bdcd10(NativeArrayHeader&, std::int32_t) override { unused("reserve_vfs_pair_vector_00bdcd10"); }
    void copy_vfs_string_pair_00bdcc60(void*, const void*) override { unused("copy_vfs_string_pair_00bdcc60"); }
    void destroy_vfs_string_pair_00bdb850(void*) override { unused("destroy_vfs_string_pair_00bdb850"); }
    void resize_vfs_pair_vector_00bdec70(NativeArrayHeader&, std::int32_t) override { unused("resize_vfs_pair_vector_00bdec70"); }
    bool resolve_existing_vfs_name_00bdf4c0(void*, const NativeString&) override { unused("resolve_existing_vfs_name_00bdf4c0"); }
    void visit_matching_mounts_00bdd0a0(void*, const NativeString&, void*) override { unused("visit_matching_mounts_00bdd0a0"); }
    void* select_search_group_value_list_00bdb2e0(const NativeIterator&, std::int32_t) override { unused("select_search_group_value_list_00bdb2e0"); }
    void erase_render_alias_list_node_004d0990(NativeIterator&, void*, void*) override { unused("erase_render_alias_list_node_004d0990"); }
    void list_size_check_00bded80(NativeContainerHeader&, std::uint32_t) override { unused("list_size_check_00bded80"); }
    void* create_tracked_section_00bd1860() override { unused("create_tracked_section_00bd1860"); }
    void destroy_tracked_section_0041cc80(void*) override { unused("destroy_tracked_section_0041cc80"); }
    void set_debug_feature_enabled_00be8680(void*, const NativeString&, bool) override { unused("set_debug_feature_enabled_00be8680"); }
    void reserve_debug_feature_records_00be8df0(NativeArrayHeader&, std::int32_t) override { unused("reserve_debug_feature_records_00be8df0"); }
    void reserve_stream_writer_chunks_00bf05d0(NativeArrayHeader&, std::int32_t) override { unused("reserve_stream_writer_chunks_00bf05d0"); }
    void reserve_string_table_rows_00b80c70(NativeArrayHeader&, std::int32_t) override { unused("reserve_string_table_rows_00b80c70"); }
    void reserve_physical_stream_slots_00bf30c0(NativeArrayHeader&, std::int32_t) override { unused("reserve_physical_stream_slots_00bf30c0"); }
    void physical_stream_open_00bf52a0(void*, std::uint32_t, std::uint32_t) override { unused("physical_stream_open_00bf52a0"); }
    bool pc_storage_file_exists_00beb040(void*, std::uint32_t, const char*) override { unused("pc_storage_file_exists_00beb040"); }
    void compose_error_message_00bf1b50(void*, NativeString&, const NativeString&) override { unused("compose_error_message_00bf1b50"); }
    void* read_structured_string_00bea010(void*, NativeString&) override { unused("read_structured_string_00bea010"); }
    std::uint32_t decode_tga_00befdc0(void*) override { unused("decode_tga_00befdc0"); }
    std::uint32_t read_dword_slot34_and_debit_00bf0280(void*, std::uint32_t*) override { unused("read_dword_slot34_and_debit_00bf0280"); }
    std::uint32_t read_dword_and_debit_00bf02a0(void*, std::uint32_t*) override { unused("read_dword_and_debit_00bf02a0"); }
    float read_float_and_account_00bf02c0(void*, std::uint32_t*) override { unused("read_float_and_account_00bf02c0"); }
    void stream_reader_construct_00bf09a0(void*) override { unused("stream_reader_construct_00bf09a0"); }
    void stream_reader_assign_stream_00bf0430(void*, void*) override { unused("stream_reader_assign_stream_00bf0430"); }
    void profiler_clear_history_00be3380(void*) override { unused("profiler_clear_history_00be3380"); }
    char* text_scanner_peek_token_00bee8e0(void*) override { unused("text_scanner_peek_token_00bee8e0"); }
    std::uint32_t text_scanner_skip_whitespace_00beedb0(void*) override { unused("text_scanner_skip_whitespace_00beedb0"); }
    bool text_scanner_peek_bool_00beec40(void*, char*) override { unused("text_scanner_peek_bool_00beec40"); }
    void append_checked_string_vector_00450540(void*, const NativeString&) override { unused("append_checked_string_vector_00450540"); }
    void* lower_bound_filestore_pending_00be5530(NativeContainerHeader&, const void*) override { unused("lower_bound_filestore_pending_00be5530"); }
    void* upper_bound_filestore_pending_00be5630(NativeContainerHeader&, const void*) override { unused("upper_bound_filestore_pending_00be5630"); }
    void* upper_bound_extension_prefix_00bda2c0(NativeContainerHeader&, const NativeString&) override { unused("upper_bound_extension_prefix_00bda2c0"); }
    void* lower_bound_indexed_name_00bda260(NativeContainerHeader&, const NativeString&) override { unused("lower_bound_indexed_name_00bda260"); }
    void canonicalize_path_00bee390(NativeString&, const NativeString&) override { unused("canonicalize_path_00bee390"); }

private:
    [[noreturn]] static void unused(const char* service) {
        throw std::logic_error(std::string("00BE0AE0 does not reach EngineServiceCalls::") + service);
    }
    NativeStringStorage& strings_;
    NativeVfsFileBlockScopeContext& scopes_;
    NativeVfsFileBlockScopeAcquired& entry_;
};

// kWireGoal2Chain54Bound: one 00686380 / 006898C0 load-block frame, immovable
// from 00426500 through 00BDCB30. The image keeps both as stack locals of
// 00686380 / 006898C0: the 18h log builder (00426500 at 006863A6 / 006898E0)
// and the 1Ch FileBlockScope (00BE0AE0 at 006863C4 / 00689900). The VFS
// publication, pooled string storage, raw pool and FileBlock scope context are
// the VFS host's own (GameVfsHost borrow_raw_services / raw_strings /
// native_application().runtime().borrow_fileblock_scopes).
struct MenuLoadBlockFrame {
    alignas(4) std::uint32_t builder[6]{}; // 00426500: three actual 8h headers
    alignas(4) std::uint32_t scope[7]{};   // 00BE0AE0: actual 1Ch FileBlockScope
    NativeFileBlockOwnerContext context;    // 00BDCB30's VFS/string/scope domain
    NativeVfsFileBlockScopeAcquired entry;  // the 00BE0980 call 00BE0AE0 makes
    NativeFileBlockOwnerAcquired exit;      // the 00BDCB30 call
    MenuFileBlockScopeCalls calls;
    bool opened{};
    bool closed{};

    MenuLoadBlockFrame(GameNativeVfsRawServices services, NativeStringRawPoolContext& raw,
        NativeVfsFileBlockScopeContext& scopes)
        : context{services.actual_vfs_publication_0109ceec, services.strings, raw, scopes},
          calls(services.strings, scopes, entry) {}
    MenuLoadBlockFrame(const MenuLoadBlockFrame&) = delete;
    MenuLoadBlockFrame& operator=(const MenuLoadBlockFrame&) = delete;
};

// The open frames, innermost last (the image's frames are stack locals, so they
// nest). A frame a native call escaped from is retained: never replayed and
// never destroyed, as the VFS host retains its title frame.
struct MenuLoadBlockFrames {
    std::vector<std::unique_ptr<MenuLoadBlockFrame>> open;
    MenuLoadBlockFrames() = default;
    MenuLoadBlockFrames(const MenuLoadBlockFrames&) = delete;
    MenuLoadBlockFrames& operator=(const MenuLoadBlockFrames&) = delete;
    ~MenuLoadBlockFrames() {
        for (auto& frame : open)
            if (frame && !frame->closed) static_cast<void>(frame.release());
    }
};

// kWireGoal2Chain57Bound: the main-menu manager's title stream, [00E198AC]+50h,
// over the live sound core GameStartupHost::SoundServices publishes through
// GameSingletonHost::bind_sound_runtime. The contexts are the ones
// GameTitleSound binds on that core (game_title_sound.cpp). Built by 005884A0:
// operator new(54h) at 005884D5, 00A877D0 at 00588528, 00A867B0 at 0058855C.
// Released by 00686C90 at 00686EDE..00686EF0 (InterlockedDecrement of +4h, then
// at zero vtable D5B360 +0h, 00BD30E0, whose +4h(1) is 00A87B30) when the mission load destroys
// the manager; teardown reaches the same 00686C90 through 004DA650, so the
// destructor releases a stream that load never released. The menu host is
// deleted before the sound core (game_hosts.cpp), as 004DA650 runs before the
// core's shutdown.
class MenuTitleMusicStream final : public SoundStreamOwnerHost, public NativeRefCountedDeleteCalls {
public:
    MenuTitleMusicStream(GameSoundRuntime& core, void*& slot)
        : core_(core), slot_(slot),
          table_{core.samples().sample_context().strings, core.samples(), format_counts_00e12ef0_, 0},
          runtime_{core.lifetime_bindings().global_00f8bbd8, core.fmod(), table_.strings,
              one_00d7a24c_, fade_rate_00ce3dc8_, core.words().null_data_00f8bbec.data() + 2,
              &null_integer_format_01090ab4_},
          owner_{table_, core.samples(), *this} {}
    MenuTitleMusicStream(const MenuTitleMusicStream&) = delete;
    MenuTitleMusicStream& operator=(const MenuTitleMusicStream&) = delete;
    ~MenuTitleMusicStream() override { release(); }

    // 005884D5..00588537: null when the 54h allocation fails (0058852F).
    void* construct(std::string_view path) {
        void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x54, 0x54});
        if (allocation == nullptr) return nullptr;
        const std::string text(path);
        NativeString owned_name;
        try {
            // 00A877D0 consumes the name on every exit; no caller-side destruction.
            construct_native_string_cstring_0041e870(&owned_name, text.c_str(), table_.strings);
            return construct_sound_stream_00a877d0(allocation, owned_name, nullptr, owner_);
        } catch (...) {
            singleton_lifetime_free(allocation);
            throw;
        }
    }
    // 0058853D..00588587: a temporary native copy of the track, then 00A867B0.
    void start(void* stream, std::string_view track) {
        const std::string text(track);
        NativeString selected;
        construct_native_string_cstring_0041e870(&selected, text.c_str(), table_.strings);
        struct Cleanup {
            NativeString& value;
            NativeStringStorage& strings;
            ~Cleanup() { destroy_native_string_header_0041dd20(&value, strings); }
        } cleanup{selected, table_.strings};
        start_sound_stream_00a867b0(stream, selected, runtime_);
    }
    // 00686ED1..00686EF0 on the +50h slot.
    void release() {
        void* const stream = slot_;
        if (stream == nullptr) return;
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
                static_cast<unsigned char*>(stream) + 4)) == 0) {
            invoke_native_ref_counted_delete_00bd30e0(stream, *this);
        }
        slot_ = nullptr;
    }

    void resolve_stream_name_00bdf4c0(void* name) override {
        static_cast<void>(core_.samples().resolve_name_00bdf4c0(*static_cast<NativeString*>(name)));
    }
    void stop_stream_00a86bf0(void* stream) override { stop_sound_stream_00a86bf0(stream, runtime_); }
    void delete_vslot04(void* stream, std::uint32_t profile, std::uint32_t flags) override {
        if (profile != 0x00D5B360u)
            throw std::logic_error("title stream release requires its D5B360 stream profile");
        scalar_delete_sound_stream_00a87b30(stream, static_cast<std::uint8_t>(flags), owner_);
    }

private:
    GameSoundRuntime& core_;
    void*& slot_;
    // The image's constants (PE: E12EF0 {0,1,2,6}, D7A24C 3F800000h,
    // CE3DC8 0.3f widened, 1090AB4 an empty C string), as SoundServices binds them.
    const std::array<std::uint32_t, 4> format_counts_00e12ef0_{0, 1, 2, 6};
    const volatile std::uint32_t one_00d7a24c_{0x3f800000};
    const volatile double fade_rate_00ce3dc8_{0.30000001192092896};
    char null_integer_format_01090ab4_{};
    SoundDialogTableContext table_;
    SoundStreamRuntimeContext runtime_;
    SoundStreamOwnerContext owner_;
};

// Goal 4 chain 312 (kWireGoal4Chain312Bound): 004CA440's front-end branch
// reaches 004CA5D4, 00735B50 BSP_SoundFrame_ServiceInline, directly:
// 004CA4E7..004CA529 take the in-game view arms only when [00E198C4] is
// non-null, and that word is null in the front-end states (see
// bsp::NullInterfaceSoundFrame). 00735B50 then skips 0068A670 for the null
// interface and dispatches [00F8BBD8]->vtable[4] (00735BE7..00735C1B), the
// per-frame update 00A87BF0 / 00A7E630 (bsp.py disasm-raw 00735b50), over the
// live core GameStartupHost::SoundServices publishes.

}  // namespace

// ---------------------------------------------------------------------------
// GameFrameProfiler
// ---------------------------------------------------------------------------

GameFrameProfiler::GameFrameProfiler(GameHostLog& log, int slot_count)
    : log_(log), slot_count_(slot_count < 2 ? 2 : slot_count) {
    // 00be3820 makes six allocations from one capacity and fills the colour
    // array with FFFFFF0Fh. The capacity is 00e15118 + 32h; 00e15118 is the
    // static counter count, which the image does not carry, so the executable
    // chooses one and records the substitution.
    const std::size_t slots = static_cast<std::size_t>(slot_count_);
    records_.resize(slots);
    history_.resize(slots * static_cast<std::size_t>(kProfilerHistoryFrames));
    start_offsets_.resize(slots * static_cast<std::size_t>(kProfilerHistoryFrames));
    current_.resize(slots);
    colors_.assign(slots, kProfilerDefaultSlotColor);
    display_.resize(slots);
    counters_.records = records_.data();
    counters_.history = history_.data();
    counters_.start_offsets = start_offsets_.data();
    counters_.current = current_.data();
    counters_.colors = colors_.data();
    counters_.display = display_.data();
    counters_.slot_count = slot_count_;

    class Clock final : public cc13_engine_core::PerfClockHost {
    public:
        explicit Clock(GameFrameProfiler& profiler) : profiler_(profiler) {}
        std::int64_t query_performance_counter() override {
            return profiler_.query_performance_counter();
        }
        void sleep_ms(std::uint32_t milliseconds) override { Sleep(milliseconds); }
    private:
        GameFrameProfiler& profiler_;
    } clock(*this);
    // bsp.py show 00BE3310 --asm: QPC, Sleep(100), QPC, delta * 10.
    // 00CD8F30 stores the result in 0109DB48; 00BE3360 / 00BE34D0
    // divide by it. Keep the actual measured ticks/second, not its inverse.
    tick_scale_ = cc13_engine_core::calibrate_ticks_per_second_00be3310(clock);
    log_.notef("profiler slots=%d history=%d spare=%d ticks_per_second=%.9f "
        "(0109db48 calibrated by 00be3310)",
        slot_count_, kProfilerHistoryFrames, kProfilerSpareSlots, tick_scale_);
}

std::int64_t GameFrameProfiler::query_performance_counter() {
    ++samples_;
    // Chain 851: the running profiler consumes the actual Win32 counter through
    // the one reconstructed query body. bsp.py show 00BE3240: QPC into a local,
    // return its full EDX:EAX value. This replaces the host's direct API query;
    // calibration and frame slots still take exactly one sample per request.
    return engine_services_cc13::query_performance_counter_00be3240();
}

const ProfilerCounterRecord& GameFrameProfiler::frame_record() const noexcept {
    return records_[static_cast<std::size_t>(app_update_slot())];
}

// ---------------------------------------------------------------------------
// GameMenuHost::Impl
// ---------------------------------------------------------------------------

namespace {

// Image-shaped storage for the two constructor-only leaves. Unknown bytes are
// left untouched, as in the original allocation. The actual registered flags
// are the +4h/+5h subobject, rather than a copied/default second pair.
struct MenuMissionTreeImage {
    std::uint32_t primary_vtable;
    FrontEndScreen flags;
    std::uint8_t unknown_06[2];
    std::uint32_t listener_vtable;
    std::uint8_t unknown_0c[0x0c];
    std::array<std::uint32_t, 3> vector_18;
    std::uint32_t unknown_24;
    std::array<std::uint32_t, 3> vector_28;
};
static_assert(sizeof(FrontEndScreen) == 2);
static_assert(offsetof(MenuMissionTreeImage, flags) == 4);
static_assert(offsetof(MenuMissionTreeImage, listener_vtable) == 8);
static_assert(offsetof(MenuMissionTreeImage, vector_18) == 0x18);
static_assert(offsetof(MenuMissionTreeImage, vector_28) == 0x28);
static_assert(sizeof(MenuMissionTreeImage) == 0x34);

struct MenuCreditsImage {
    std::uint32_t primary_vtable;
    FrontEndScreen flags;
    std::uint8_t unknown_06[2];
    std::uint32_t listener_vtable;
    std::uint8_t unknown_0c[0x1c];
    std::array<std::uint32_t, 3> vector_28;
    std::uint8_t unknown_34[8];
    float field_3c;
    float field_40;
    float field_44;
    std::uint8_t unknown_48[8];
    std::array<std::uint32_t, 3> vector_50;
};
static_assert(offsetof(MenuCreditsImage, flags) == 4);
static_assert(offsetof(MenuCreditsImage, listener_vtable) == 8);
static_assert(offsetof(MenuCreditsImage, vector_28) == 0x28);
static_assert(offsetof(MenuCreditsImage, field_3c) == 0x3c);
static_assert(offsetof(MenuCreditsImage, field_40) == 0x40);
static_assert(offsetof(MenuCreditsImage, field_44) == 0x44);
static_assert(offsetof(MenuCreditsImage, vector_50) == 0x50);
static_assert(sizeof(MenuCreditsImage) == 0x5c);

// Adapts the existing front-end base constructor to a leaf's registered flags.
class MenuLeafBaseConstructor final : public FrontendScreenConstructorHost {
public:
    explicit MenuLeafBaseConstructor(FrontEndScreen& flags) : flags_(flags) {}
    void construct_frontend_base() override { construct_front_end_screen_004f7180(flags_); }
private:
    FrontEndScreen& flags_;
};

// operator new(size) at an Init call site (00686380 / 006898C0) for a leaf
// whose constructor stores more than the two image-shaped records above
// carry: the block is this owner's for the screen record's lifetime, as the
// HUD's leaf blocks are its owner's (game_hosts_hud.cpp HudLeafScreenStorage).
// The front-end base's two flag bytes at +4h/+5h are an actual FrontEndScreen,
// so the registry borrows the very base the leaf constructed instead of a
// second copy. The block is zeroed; the native allocation is not, and nothing
// reads a byte the leaf did not store before the screen's own register runs.
class MenuLeafBlock {
public:
    explicit MenuLeafBlock(std::size_t size_bytes)
        : bytes_(std::make_unique<std::uint8_t[]>(size_bytes)), size_(size_bytes) {
        if (size_bytes < 6) throw std::logic_error("front-end leaf screen smaller than its base");
        flags_ = ::new (static_cast<void*>(bytes_.get() + 4)) FrontEndScreen{};
    }
    std::uint8_t* data() const noexcept { return bytes_.get(); }
    FrontEndScreen& flags() const noexcept { return *flags_; }
    bool contains(const void* p, std::size_t bytes) const noexcept {
        const auto* b = static_cast<const std::uint8_t*>(p);
        return b >= data() && bytes <= size_ && b <= data() + (size_ - bytes);
    }
    // 00BF681B blocks the leaf constructor links into its own fields (a tree
    // head node); the native destructor frees them.
    void* allocate_owned(std::size_t bytes) {
        owned_.push_back(std::make_unique<std::uint8_t[]>(bytes));
        return owned_.back().get();
    }

private:
    std::unique_ptr<std::uint8_t[]> bytes_;
    std::size_t size_;
    FrontEndScreen* flags_{nullptr};
    std::vector<std::unique_ptr<std::uint8_t[]>> owned_;
};

// 004F7180 on a leaf's own block (bsp.py show 004f7180): the base vtable
// 00CEAE54, then both flag bytes cleared. The leaf replaces the vtable.
void construct_menu_leaf_base(MenuLeafBlock& block, const void* p) {
    if (p != block.data())
        throw std::logic_error("004F7180 base belongs to a different front-end screen");
    construct_front_end_screen_004f7180(block.flags());
    std::memcpy(block.data(), &kFrontEndScreenBaseVtable, sizeof(std::uint32_t));
}

// 00683610 writes the scroller in place at the screen's own offset, so the
// typed scroller must have the native 5Ch layout (bsp.py show 00683610 --asm).
static_assert(sizeof(FrontEndScreenScroller) == 0x5C);
void construct_menu_leaf_scroller(MenuLeafBlock& block, void* scroller) {
    if (!block.contains(scroller, sizeof(FrontEndScreenScroller)))
        throw std::logic_error("00683610 scroller is not a member of this front-end screen");
    auto* const typed = ::new (scroller) FrontEndScreenScroller;
    construct_front_end_screen_scroller_00683610(*typed);
}

// 00626630, the rewards (scoring) screen: the base and its three scrollers.
class MenuLeafRewardsBinding final : public ingame_gui_cc13::RewardsConstructHost {
public:
    explicit MenuLeafRewardsBinding(MenuLeafBlock& block) : block_(block) {}
    void front_end_screen_construct_004f7180(ingame_gui_cc13::Raw screen) override {
        construct_menu_leaf_base(block_, screen.p);
    }
    void scroller_construct_00683610(ingame_gui_cc13::Raw scroller) override {
        construct_menu_leaf_scroller(block_, scroller.p);
    }
private:
    MenuLeafBlock& block_;
};

// 0051E4D0, the briefing screen: the base and the head node of its +12Ch tree.
class MenuLeafBriefingBinding final : public segs_13_15_22_44_cc13::BriefingConstructHost {
public:
    explicit MenuLeafBriefingBinding(MenuLeafBlock& block) : block_(block) {}
    void front_end_screen_construct_004f7180(ingame_gui_cc13::Raw screen) override {
        construct_menu_leaf_base(block_, screen.p);
    }
    void* tree_buy_head_node_0051acb0(ingame_gui_cc13::Raw tree) override {
        if (!block_.contains(tree.p, 0x0C))
            throw std::logic_error("0051ACB0 tree is not a member of this front-end screen");
        // bsp.py show 0051acb0 --asm: operator new(1Ch), +0h/+4h/+8h = 0,
        // colour +18h = 1, isnil +19h = 0. The constructor then sets isnil
        // and links the head to itself.
        auto* const node = static_cast<std::uint8_t*>(block_.allocate_owned(0x1C));
        node[0x18] = 1;
        node[0x19] = 0;
        return node;
    }
private:
    MenuLeafBlock& block_;
};

// 00563370 (leaderboards) and 00555770 (control binding): the base and
// 00683610 on their embedded scroller blocks.
class MenuLeafScrollerScreenBinding final : public cc13_fe3::ScrollerScreenConstructHost {
public:
    explicit MenuLeafScrollerScreenBinding(MenuLeafBlock& block) : block_(block) {}
    void front_end_screen_construct_004f7180(ingame_gui_cc13::Raw screen) override {
        construct_menu_leaf_base(block_, screen.p);
    }
    void gui_scroller_construct_00683610(void* scroller) override {
        construct_menu_leaf_scroller(block_, scroller);
    }
private:
    MenuLeafBlock& block_;
};

// 00527D00, the control layout screen: the 004F7180 base only (bsp.py show
// 00527d00 --asm), so the other Seg15LeafConstructHost services stay a loud
// failure.
class MenuLeafControlLayoutBinding final : public frontend_seg15_22_cc13::Seg15LeafConstructHost {
public:
    explicit MenuLeafControlLayoutBinding(MenuLeafBlock& block) : block_(block) {}
    void front_end_screen_construct_004f7180(ingame_gui_cc13::Raw screen) override {
        construct_menu_leaf_base(block_, screen.p);
    }
    void* operator_new_00bf681b(std::size_t) override { return unreached("00BF681B"); }
    void simple_3d_scene_construct_0093cb10(ingame_gui_cc13::Raw) override { unreached("0093CB10"); }
    void* tree_buy_head_node_005359a0() override { return unreached("005359A0"); }
    void* list_buy_head_node(std::uint32_t) override { return unreached("list head node"); }
    void* list_buy_head_node_0053da70() override { return unreached("0053DA70"); }
    void* tree_buy_head_node(std::uint32_t) override { return unreached("tree head node"); }
private:
    [[noreturn]] static void* unreached(const char* service) {
        throw std::logic_error(std::string("00527D00 does not call ") + service);
    }
    MenuLeafBlock& block_;
};

// Goal 5 chain 813. 006898C0 constructs the actual 270h options-screen
// receiver; 005F6030 constructs BCh settings at +D8h/+194h and mallocs ten
// language-name bytes at +26Ch (bsp.py show 005f6030). Borrow the already
// initialized process context: these are genuine settings owners, including
// their pooled clan strings and the shared input/profile publications.
// The menu still retains its GUI pages separately, so this owner releases
// only the members it constructed, in native teardown order. The complete
// page-removing destructor and native virtual ABI remain unbound.
class MenuLeafOptionsBinding final : public cc13_fe1::OptionsLifetimeHost {
public:
    explicit MenuLeafOptionsBinding(MenuLeafBlock& block)
        : block_(block), settings_(game_native_settings_process().embedded_settings_context()) {}
    ~MenuLeafOptionsBinding() override {
        std::free(language_buffer_);
        for (std::size_t i = constructed_.size(); i != 0; --i) {
            if (constructed_[i - 1]) game_settings_destroy_008d78d0(member(i - 1));
        }
    }
    void frontend_construct_004f7180(void* screen) override {
        construct_menu_leaf_base(block_, screen);
    }
    void game_settings_construct_008d7710(void* settings) override {
        const std::size_t index = member_index(settings);
        if (constructed_[index]) throw std::logic_error("options settings already constructed");
        ::new (settings) NativeGameSettingsStorage;
        NativeGameSettingsOperation operation;
        construct_native_game_settings_008d7710(settings, settings_, operation);
        constructed_[index] = true;
    }
    void game_settings_destroy_008d78d0(void* settings) override {
        const std::size_t index = member_index(settings);
        if (!constructed_[index]) throw std::logic_error("options settings not constructed");
        NativeGameSettingsOperation operation;
        destroy_native_game_settings_008d78d0(settings, settings_, operation);
        constructed_[index] = false;
    }
    void* malloc_00bf9f1a(std::size_t size) override {
        if (size != 10 || buffer_requested_)
            throw std::logic_error("unexpected options language-buffer allocation");
        buffer_requested_ = true;
        language_buffer_ = std::malloc(size);
        return language_buffer_;
    }
    void free_00bf9dc8(void* buffer) override {
        if (buffer != language_buffer_)
            throw std::logic_error("options language buffer belongs to another owner");
        std::free(language_buffer_);
        language_buffer_ = nullptr;
    }
    // Registration/page/destruction services are outside this constructor.
    // Its three callees are all bound above; reject use at another entry.
    void frontend_register_004f71d0(void*) override { unavailable("004F71D0"); }
    void frontend_destruct(void*) override { unavailable("004F71A0"); }
    std::uint32_t load_page_00aa5840(std::string_view) override { unavailable("00AA5840"); }
    void remove_and_destroy_page(std::uint32_t) override { unavailable("00AA31F0"); }
    void screen_bind_layout(void*) override { unavailable("layout virtual"); }
    void scroller_construct_00683610(void*) override { unavailable("00683610"); }
    std::array<float, 3> widget_resolved_position_00aa6750(std::uint32_t) override {
        unavailable("00AA6750");
    }
    void multi_main_layout_achievement_locks_005ea030(void*) override { unavailable("005EA030"); }
    void free_block(void*) override { unavailable("vector free"); }
private:
    void* member(std::size_t index) const noexcept {
        return block_.data() + (index == 0 ? 0xD8 : 0x194);
    }
    std::size_t member_index(void* settings) const {
        if (settings == member(0)) return 0;
        if (settings == member(1)) return 1;
        throw std::logic_error("settings are not a member of this options screen");
    }
    [[noreturn]] static void unavailable(const char* service) {
        throw std::logic_error(std::string("005F6030 does not call ") + service);
    }
    MenuLeafBlock& block_;
    NativeGameSettingsContext& settings_;
    std::array<bool, 2> constructed_{};
    void* language_buffer_{};
    bool buffer_requested_{};
};

struct MenuLeafBacking {
    std::unique_ptr<MenuMissionTreeImage> mission_tree;
    std::unique_ptr<MenuCreditsImage> credits;
    std::unique_ptr<MenuLeafBlock> block;
    // Release the settings and malloc buffer before their receiver block.
    std::unique_ptr<MenuLeafOptionsBinding> options;
    FrontEndScreen* flags{};
    std::uint32_t* primary_vtable{};
};

// 00686732 / 006867FE construct during main-menu Init; 00686E14 deletes
// after unregister. OFF inherits an empty base and constructs no holder.
struct MenuLeafOwner {
    std::unique_ptr<MenuLeafBacking> native_leaf;
};
struct NoMenuLeafOwner {};
using MenuLeafOwnership = MenuLeafOwner;

template <class Screen>
void retain_menu_leaf(Screen& screen, std::unique_ptr<MenuLeafBacking> native) {
    screen.native_leaf = std::move(native);
}

template <class Screen>
void release_menu_leaf(Screen& screen) {
    if (screen.native_leaf) {
        // Constructor-only backing owns no raw page/vector elements. The
        // existing menu and mission hosts retain those separate owners.
        screen.owned_flags = *screen.flags;
        screen.flags = &screen.owned_flags;
        screen.native_leaf.reset();
    }
}

// The size each Init call site passes to operator new for a constructor that
// runs on a MenuLeafBlock: the recovered class tables' object_size.
std::size_t menu_leaf_block_size(std::uint32_t constructor) noexcept {
    for (const MainMenuScreenClass& entry : kMainMenuScreens)
        if (entry.constructor == constructor) return entry.object_size;
    for (const OptionsScreenClass& entry : kOptionsScreens)
        if (entry.constructor == constructor) return entry.object_size;
    return 0;
}

// 00686380's 00626630, 0051E4D0 and 00563370, and 006898C0's 00527D00 and
// 00555770, each on a zeroed block of its call site's operator new size.
std::unique_ptr<MenuLeafBacking> construct_menu_leaf_block(
    std::size_t size_bytes, std::uint32_t constructor, GameHostLog& log) {
    switch (constructor) {
    case 0x00626630u: case 0x0051E4D0u: case 0x00563370u:
    case 0x00527D00u: case 0x00555770u: case 0x005F6030u: break;
    default: return nullptr;
    }
    if (size_bytes != menu_leaf_block_size(constructor)) return nullptr;
    auto backing = std::make_unique<MenuLeafBacking>();
    backing->block = std::make_unique<MenuLeafBlock>(size_bytes);
    MenuLeafBlock& block = *backing->block;
    const ingame_gui_cc13::Raw raw{block.data()};
    switch (constructor) {
    case 0x005F6030u: {
        auto binding = std::make_unique<MenuLeafOptionsBinding>(block);
        cc13_fe1::options_screen_construct_005f6030(block.data(), *binding);
        backing->options = std::move(binding);
        log.implemented("FrontEndManager::options_constructor", "005f6030");
        break;
    }
    case 0x00626630u: {
        MenuLeafRewardsBinding b(block);
        ingame_gui_cc13::rewards_construct_00626630(raw, b);
        log.implemented("FrontEndManager::rewards_constructor", "00626630");
        break;
    }
    case 0x0051E4D0u: {
        MenuLeafBriefingBinding b(block);
        segs_13_15_22_44_cc13::briefing_construct_0051e4d0(raw, b);
        log.implemented("FrontEndManager::briefing_constructor", "0051e4d0");
        break;
    }
    case 0x00563370u: {
        MenuLeafScrollerScreenBinding b(block);
        cc13_fe3::leaderboards_construct_00563370(raw, b);
        log.implemented("FrontEndManager::leaderboards_constructor", "00563370");
        break;
    }
    case 0x00527D00u: {
        MenuLeafControlLayoutBinding b(block);
        frontend_seg15_22_cc13::control_layout_construct_00527d00(raw, b);
        log.implemented("FrontEndManager::control_layout_constructor", "00527d00");
        break;
    }
    case 0x00555770u: {
        MenuLeafScrollerScreenBinding b(block);
        cc13_fe3::control_binding_construct_00555770(raw, b);
        log.implemented("FrontEndManager::control_binding_constructor", "00555770");
        break;
    }
    default: break;
    }
    backing->flags = &block.flags();
    backing->primary_vtable = reinterpret_cast<std::uint32_t*>(block.data());
    return backing;
}

std::unique_ptr<MenuLeafBacking> construct_menu_leaf(
    std::size_t size_bytes, std::uint32_t constructor, GameHostLog& log) {
    if (auto block = construct_menu_leaf_block(size_bytes, constructor, log)) return block;
    // Adapts the native constructor field references to the menu's owned backing.
    if (!((constructor == 0x005CA880u && size_bytes == sizeof(MenuMissionTreeImage)) ||
          (constructor == 0x0052FCE0u && size_bytes == sizeof(MenuCreditsImage)))) return nullptr;
    auto backing = std::make_unique<MenuLeafBacking>();
    if (constructor == 0x005CA880u && size_bytes == sizeof(MenuMissionTreeImage)) {
        backing->mission_tree.reset(new MenuMissionTreeImage);
        auto& image = *backing->mission_tree;
        MenuLeafBaseConstructor base(image.flags);
        // 005CA880: the existing constructor supplies the vtable and fields;
        // OFF keeps create_screen's constructor-address registry record.
        frontend_seg19_21_cc13::mission_tree_screen_construct_005ca880(
            frontend_seg19_21_cc13::MissionTreeScreenConstructionView{
                image.primary_vtable, image.listener_vtable,
                {&image.vector_18[0], &image.vector_18[1], &image.vector_18[2],
                 &image.vector_28[0], &image.vector_28[1], &image.vector_28[2]}}, base);
        backing->flags = &image.flags;
        backing->primary_vtable = &image.primary_vtable;
        log.implemented("FrontEndManager::mission_tree_constructor", "005ca880");
    } else if (constructor == 0x0052FCE0u && size_bytes == sizeof(MenuCreditsImage)) {
        backing->credits.reset(new MenuCreditsImage);
        auto& image = *backing->credits;
        MenuLeafBaseConstructor base(image.flags);
        // 0052FCE0: the existing constructor supplies vtables, empty vectors
        // and rdata seeds; OFF keeps create_screen's registry record.
        frontend_seg15_22_cc13::credits_construct_0052fce0(
            frontend_seg15_22_cc13::CreditsScreenConstructionView{
                image.primary_vtable, image.listener_vtable,
                {&image.vector_28[0], &image.vector_28[1], &image.vector_28[2],
                 &image.vector_50[0], &image.vector_50[1], &image.vector_50[2]},
                image.field_3c, image.field_40, image.field_44}, base);
        backing->flags = &image.flags;
        backing->primary_vtable = &image.primary_vtable;
        log.implemented("FrontEndManager::credits_constructor", "0052fce0");
    } else {
        return nullptr;
    }
    return backing;
}

} // namespace

// OFF has no borrow storage or initialization. ON borrows the already-created
// phase-6 LocaleTables for the menu Init registration sites; it owns no registry.
template<bool Bound>
struct MenuLocaleTableBorrow {
    void bind_menu_locale_tables(LocaleTables&) noexcept {}
    LocaleTables* borrowed_menu_locale_tables() const noexcept { return nullptr; }
};
template<>
struct MenuLocaleTableBorrow<true> {
    LocaleTables* tables{};
    void bind_menu_locale_tables(LocaleTables& value) noexcept { tables = &value; }
    LocaleTables* borrowed_menu_locale_tables() const noexcept { return tables; }
};

struct GameMenuHost::Impl : MenuLocaleTableBorrow<true> {
    // One registry slot. The flag bytes live in whichever object owns them
    // (004f7180 clears +4h and +5h in the base constructor), so this record
    // points at them rather than holding a second copy.
    struct MenuScreen : MenuLeafOwnership {
        int slot{kNoScreenSlot};
        std::string name;
        // Owned when the leaf type has no reconstruction of its own; otherwise
        // `flags` points at the object that carries the two bytes.
        FrontEndScreen owned_flags{};
        FrontEndScreen* flags{nullptr};
        // The layouts the screen's virtual +24h reports to 004f83b0.
        std::vector<GuiLayoutPage*> pages;
        std::uint32_t enter_virtual{0};
        std::uint32_t exit_virtual{0};
        std::uint32_t update_virtual{0};
        bool press_start{false};
    };

    GameHostLog& log;
    GameFrontendHost& frontend;
    GameNativeRendererApplication* native_renderer{};
    GameStateSlot& state;
    long press_start_frame{-1};
    GameMenuSummary summary;

    // --- the registry, 00e18b60 --------------------------------------------
    FrontEndWorld world{};
    GameFrontEndFrameState frame_state{};
    std::map<int, MenuScreen> screens;
    // 00E1930C's command widgets. The page registry owns their layouts; this
    // owner borrows the same widgets that the frontend renders.
    std::unique_ptr<MainMenuCommandOwner> command_widgets;
    // Layout handles the press-start host hands back. Index 0 is reserved
    // because the native treats a null layout handle as absent.
    std::vector<GuiLayoutPage*> layout_handles{nullptr};
    std::vector<GuiLayoutWidget*> element_handles{nullptr};

    // --- the title object, 00e198c8 ----------------------------------------
    TitleInitState title_init{};
    TitleHandoverState handover{};
    TitleScreen title_object{};
    AttractScreen attract_object{};
    AttractScreen* attract_singleton{}; // 00E198BC, published by 00689D90
    FrontEndFrameLayouts frame_layouts{};
    // Milestone 2i: the layouts 00518250 acquires for the mission's own set 3
    // are published hidden; see MenuFrameLayoutHost::gui_layout_acquire.
    bool acquire_hidden{false};

    // --- the press-start screen, registry slot 5Ch --------------------------
    PressStartScreen press_start{};
    PressStartGlobals press_globals{};
    PromptPulse prompt_pulse{};

    // --- the injected input edge, 004c43c0 ----------------------------------
    InputActionRecord press_action{};
    bool press_start_handoff_requested{false};
    // The helm `input` line's held actions (inject_input_action_hold): one record
    // per action, and the frames of hold left (0: released, or let go).
    std::map<int, InputActionRecord> injected_actions;
    std::map<int, unsigned> injected_frames_left;
    std::map<int, unsigned> injected_frames_without_2eh;   // this hold's frames 005484F0 skipped
    std::map<int, unsigned long long> injected_edge_frame;  // the frame of the last rising edge
    std::map<int, bool> injected_held_last_frame;
    // Set by the HUD when screen 2Eh's 005484F0 runs (note_screen_2eh_ran), and
    // cleared at the next frame's input prologue.
    bool screen_2eh_ran{false};

    // --- the state-request ring, game+5D8h ---------------------------------
    GameStateRequestQueue requests{};

    // --- the screen sets and the input contexts ----------------------------
    FrontEndScreenSetStack screen_sets{};
    GameInputContextSetStack input_contexts{};
    // Goal 4 chain 302: game+3Ch, borrowed from the application (null when unbound).
    GameNativeInputConfigurationApplication* input_configuration{};
    void publish_input_device_flags_sink();

    // --- the front-end managers --------------------------------------------
    FrontEndManagerRegistry manager_registry{};
    FrontEndInterfaceLock interface_lock{};
    FrontEndManagerSet managers{};
    MainMenuManager main_menu{};
    OptionsMenuManager options_menu{}; // chain 561: owner published as 00E198B8 by Init
    MainMenuMusicState title_music{};
    FrontEndShellState shell_state{};
    LoadingScreenConfig loading_globals{};
    LoadingScreen loading_screen{};  // the 00E194B4 singleton's recovered fields
    LoadingScreen* loading_screen_singleton{}; // actual source E194B4 publication
    frontend_seg19_21_cc13::LoadingScreenDeletionBinding loading_screen_deletion{};
    // C251: value projections of that SAME screen's +0C..+50 fields. Reset
    // publishes its +2C/+30 writes back to loading_screen, the progress owner.
    LoadingScreenElementState loading_elements;
    LoadingHintList loading_hints;
    LocaleTables* loading_locale{}; // borrows the already-loaded phase-6 tables
    // 0057BEC0's bindings: 00CEF258 is the double 128.0 (4060000000000000h)
    // and 0109EEA4 the CRT's SSE2 mode word, bound as GameStartupHost binds it.
    const volatile double loading_progress_scale_00cef258{128.0};
    const volatile std::uint32_t sse2_0109eea4{
        IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE ? 1u : 0u};
    // 0057BEC0 at every report: null unless 0057CB60 built the screen.
    void report_loading_progress_0057bec0(float progress) {
        const LoadingProgressCrtAccess crt{&loading_progress_scale_00cef258, &sse2_0109eea4};
        // 0057BFF0 publishes before virtual Init returns. Progress raised
        // inside Init must already see that SAME owner, even though the
        // legacy created flag is only set by begin_loading_screen later.
        report_loading_progress(loading_screen_singleton, progress, crt);
        if (loading_screen_singleton != nullptr) {
            loading_elements.rotation.progress = loading_screen_singleton->progress;
            loading_elements.rotation.progress_units = loading_screen_singleton->progress_units;
        }
    }
    GameSingletonHost& singletons;

    // --- the main-menu path -------------------------------------------------
    MainMenuPathState path{};
    MainMenuPathStep path_step{MainMenuPathStep::PressStartPoll};
    bool pumped_this_frame{false};

    // --- cc15 lane L2: the profile block game+650h and the hint tracker
    // [00E198D0]. Declared before the HUD and mission hosts that borrow them.
    GameProfileOwner profile{log};
    GameHintTrackerSlot hint_tracker{log, profile};

    // --- milestone 2h, the in-mission HUD screens ---------------------------
    std::unique_ptr<GameHudHost> hud;

    // --- milestone 2e, the scripted mission selection -----------------------
    std::unique_ptr<GameMissionHost> mission;
    bool mission_running{false};
    GameVfsHost& vfs; // Append the existing borrow without moving older fields.
    // cc15-prof: the storage Lua for the read-only profile load (00B6A020).
    GameScriptHost* profile_scripts{nullptr};
    // cc15-prof2: bound by the startup host; empty unless kQuickSettingsApplyCc15Bound.
    GameProfileSettingsSinks profile_settings_sinks{};
    void load_signed_in_profile_0067cc60();
    // cc15-hint2 (kHintTriggerPassesCc15Bound): the registry, input and locale
    // reads the hint tracker and its screen 34h make; bound at game_on_init.
    std::unique_ptr<HintScreenRegistry> hint_registry;
    // kWireGoal2Chain54Bound: the front-end managers' open load blocks, built by
    // FrontEndManager::open_load_block (00686380 at 006863C4, 006898C0 at
    // 00689900) and left by close_load_block (00686380 at 006868E4). Stays empty
    // with the switch OFF.
    MenuLoadBlockFrames load_blocks;
    // 00686380 / 006898C0: 00426500 builder, 00BD1A60 append of the label,
    // 00BE0AE0(scope, builder's first header, gate 1), 00425F80. False for a
    // label with no 00BE0AE0 site (GVMultiMenu::Init is a log label).
    bool open_load_block_00be0ae0(std::string_view name) {
        if (name != kMainMenuLoadBlock && name != kOptionsMenuLoadBlock) return false;
        // The same gated borrows GameVfsHost::open_title_fileblock takes.
        const GameNativeVfsRawServices services = vfs.borrow_raw_services();
        NativeVfsFileBlockScopeContext& scopes =
            vfs.native_application().runtime().borrow_fileblock_scopes();
        load_blocks.open.push_back(
            std::make_unique<MenuLoadBlockFrame>(services, vfs.raw_strings(), scopes));
        MenuLoadBlockFrame& frame = *load_blocks.open.back();
        const std::string label(name);
        // 006863A6 / 006898E0.
        construct_native_log_builder_00426500(frame.builder, frame.context.strings);
        // 006863BA / 006898F6: EAX is the builder, whose +0 header holds the label.
        void* const header =
            append_native_log_cstring_00bd1a60(frame.builder, label.c_str(), frame.context.strings);
        // 006863C4 / 00689900: PUSH 1 (006863AB / 006898E5), PUSH EAX, ECX = scope.
        // The body takes [0109CEEC] as a value; the image reloads it at 00BE0B6F,
        // and its only writer is 00BDA7F9 (VFS manager destruction).
        es::fileblock_scope_construct_00be0ae0(frame.scope, *static_cast<const NativeString*>(header), 1u,
            frame.context.actual_vfs_publication_0109ceec, frame.context.strings, frame.calls);
        // 006863D2 / 0068990F.
        destroy_native_log_builder_00425f80(frame.builder, frame.context.strings);
        frame.opened = true;
        return true;
    }
    // 006868E4: 00BDCB30(ECX = the scope) leaves the block and returns the
    // scope's name. False when no 00BE0AE0 frame is open.
    bool close_load_block_00bdcb30() {
        if (load_blocks.open.empty()) return false;
        MenuLoadBlockFrame& frame = *load_blocks.open.back();
        if (!frame.opened)
            throw std::logic_error("front-end load block requires a completed 00BE0AE0 construction");
        destroy_native_fileblock_00bdcb30(frame.scope, frame.context, frame.exit);
        frame.closed = true;
        load_blocks.open.pop_back();
        return true;
    }
    // kWireGoal2Chain57Bound: the title stream's services over the live sound
    // core, bound at the first 005884A0 that finds a started core. Declared after
    // title_music so it releases that slot first. Stays empty with the switch OFF.
    std::unique_ptr<MenuTitleMusicStream> title_stream;
    MenuTitleMusicStream* bind_title_stream() {
        if (title_stream) return title_stream.get();
        GameSoundRuntime* const core = singletons.native_deletion_bindings().sound_runtime;
        if (core == nullptr || !core->started()) return nullptr;
        title_stream = std::make_unique<MenuTitleMusicStream>(*core, title_music.title_stream);
        return title_stream.get();
    }
    // kWireGoal4Chain312Bound: 004CA5D4's 00735B50 over the same started core,
    // bound at the first front-end render that finds it. Stays empty OFF.
    std::unique_ptr<NullInterfaceSoundFrame> sound_frame;
    NullInterfaceSoundFrame* bind_sound_frame() {
        if (sound_frame) return sound_frame.get();
        GameSoundRuntime* const core = singletons.native_deletion_bindings().sound_runtime;
        if (core == nullptr || !core->started()) return nullptr;
        sound_frame = std::make_unique<NullInterfaceSoundFrame>(*core, application_camera_axes_crt());
        return sound_frame.get();
    }

    Impl(GameHostLog& log_in, GameFrontendHost& frontend_in, GameStateSlot& state_in,
        GameSingletonHost& singletons, long press_start_frame_in, GameVfsHost& vfs_in);

    MenuScreen* screen_at(int slot);
    // The executable's record of one registry slot. `external` is the object
    // that owns the two flag bytes, or null to let the record own them.
    MenuScreen& add_screen(int slot, std::string name, FrontEndScreen* external,
        std::uint32_t enter_virtual, std::uint32_t exit_virtual,
        std::uint32_t update_virtual, bool is_press_start);
    // 00aa5840 for a screen's own layout, with the byte 004f83b0 would publish.
    void attach_page(MenuScreen& screen, const std::string& layout_name);
    void initialize_command_widgets();
    void set_command_help_variant(std::uint8_t variant);
    void pump(float raw_delta);
    void advance_path(float raw_delta);
};

namespace {

// ---------------------------------------------------------------------------
// 004f83b0, the per-screen visibility commit
// ---------------------------------------------------------------------------

class MenuCommitHost final : public FrontEndScreenCommitHost {
public:
    explicit MenuCommitHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void collect_screen_children(int slot, std::vector<void*>& children) override {
        // Screen vtable +24h. The base 004f75d0 is a bare RET 4, so a screen
        // with no layouts leaves the list alone; the press-start override
        // 0067d860 appends its one layout handle through 004d6790.
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        if (screen == nullptr) return;
        for (GuiLayoutPage* page : screen->pages) {
            if (page != nullptr) children.push_back(page);
        }
        owner_.log.implemented("FrontEndScreen::collect_children", "004f75d0");
    }

    void set_child_visible(void* child, bool visible) override {
        // The child's vtable +34h, the GUI element visibility setter. 004F8434
        // reads the screen's applied byte +5h and forwards it here.
        auto* page = static_cast<GuiLayoutPage*>(child);
        if (page == nullptr) return;
        owner_.frontend.commit_page_visibility(*page, visible);
        ++owner_.summary.visibility_commits;
        owner_.log.implemented("FrontEndScreen::set_child_visible", "004f83b0+34h");
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 004f7620's two per-screen predicates
// ---------------------------------------------------------------------------

class MenuSetBindings final : public FrontEndScreenSetHostBindings {
public:
    explicit MenuSetBindings(GameMenuHost::Impl& owner) : owner_(owner) {}

    bool screen_manages_own_visibility(int slot) override {
        // Vtable +4h. The base 004f7570 is `xor al,al; ret`, so the screen sets
        // own every request. cc15-hint2: the hints screen 34h overrides it with
        // 0054D670, which returns true (vtable 00CEE290 +4h).
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (slot == 0x34) return true;
        }
        // Goal 5 chain 646: a leaf's own +4h on its block (43h: 00643DD0, true).
        bool self_managed = false;
        if (owner_.hud != nullptr && owner_.hud->leaf_screen_self_managed(slot, self_managed))
            return self_managed;
        static_cast<void>(slot);
        return front_end_screen_self_managed_004f7570();
    }
    bool screen_occludes_lower_levels(int slot) override {
        // Vtable +8h, base 004f7580, also false.
        // Packet cc13-releasegun: the map screen 4Ch overrides it with 005A8EF0,
        // `MOV AL,1 ; RET`. Only an armed plan publishes 4Ch.
        if (kHarnessReleaseGunMapBound && slot == 0x4c && owner_.hud != nullptr
            && owner_.hud->release_gun_armed()) {
            return true;
        }
        return front_end_screen_predicate_004f7580();
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 004f8710 and 004d8c00, the level-4 setters every manager calls
// ---------------------------------------------------------------------------

class MenuScreenSetHost final : public FrontEndScreenSetHost {
public:
    MenuScreenSetHost(GameMenuHost::Impl& owner, MenuSetBindings& bindings)
        : owner_(owner), bindings_(bindings) {}

    void set_gui_interface_set(const int* ids, std::size_t count) override {
        set_front_end_screen_set_004f8710(owner_.screen_sets, owner_.world.screens,
            bindings_, ids, count);
        owner_.log.implemented("FrontEndManager::set_gui_interface_set", "004f8710");
        owner_.summary.published_screen_id = count != 0 && ids != nullptr ? ids[0] : 0;
    }
    void set_game_interface_set(const int* ids, std::size_t count) override {
        set_game_input_context_set_004d8c00(owner_.input_contexts, ids, count);
        owner_.log.implemented("FrontEndManager::set_game_interface_set", "004d8c00");
    }

private:
    GameMenuHost::Impl& owner_;
    MenuSetBindings& bindings_;
};

// ---------------------------------------------------------------------------
// The two refcount helpers the interface records use
// ---------------------------------------------------------------------------

class MenuPayloadHost final : public FrontEndPayloadHost {
public:
    explicit MenuPayloadHost(GameMenuHost::Impl& owner) : owner_(owner) {}
    void retain_payload(void* payload) override {
        if (payload == nullptr) return;
        owner_.log.unimplemented("FrontEndManager::retain_payload", "00694a60");
    }
    void release_payload(void* payload) override {
        if (payload == nullptr) return;
        owner_.log.unimplemented("FrontEndManager::release_payload", "006952a0");
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// The press-start screen's host, 0067cfb0 and its neighbours
// ---------------------------------------------------------------------------

// 0067C880's virtual +20h queries stay on the captured native device. The
// application's finite dispatcher reads that device's current profile/state;
// it does not poll hardware or advance action history a second time.
class PressStartDeviceButtons final : public ingame_gui_cc13::DeviceButtonHost {
public:
    explicit PressStartDeviceButtons(NativeInputDeviceRuntime& devices) : devices_(devices) {}
    bool button_held_20(void* device, int button) override {
        return devices_.query_20(device, static_cast<std::uint32_t>(button)) != 0;
    }
private:
    NativeInputDeviceRuntime& devices_;
};

class MenuPressStartHost final : public PressStartScreenHost {
public:
    explicit MenuPressStartHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    // --- GUI layout and element --------------------------------------------
    std::uint32_t load_layout(const char* name) override {
        // 0067ca80: 004c12b0 then 00aa5840(name, 1, 0).
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(kPressStartScreenId);
        GuiLayoutPage* page = owner_.frontend.load_screen_page(name);
        owner_.log.implemented("PressStartScreen::load_layout", "00aa5840");
        if (page == nullptr) return 0;
        owner_.layout_handles.push_back(page);
        if (screen != nullptr) {
            screen->pages.push_back(page);
            // A screen is born neither wanted nor active, so the byte 004f83b0
            // would publish to a freshly bound layout is false.
            owner_.frontend.commit_page_visibility(*page, screen->flags != nullptr
                && screen->flags->active);
        }
        return static_cast<std::uint32_t>(owner_.layout_handles.size() - 1);
    }
    std::uint32_t find_element(std::uint32_t layout, const char* name) override {
        GuiLayoutPage* page = layout < owner_.layout_handles.size()
            ? owner_.layout_handles[layout] : nullptr;
        if (page == nullptr) return 0;
        GuiLayoutWidget* widget = owner_.frontend.find_page_child(*page, name);
        if (widget == nullptr) return 0;
        owner_.element_handles.push_back(widget);
        return static_cast<std::uint32_t>(owner_.element_handles.size() - 1);
    }
    void release_layout(std::uint32_t layout) override {
        GuiLayoutPage* page = layout < owner_.layout_handles.size()
            ? owner_.layout_handles[layout] : nullptr;
        owner_.frontend.release_screen_page(page);
    }
    void register_screen(int screen_id) override {
        register_front_end_screen_004f71d0(owner_.world.screens, owner_.press_start.base,
            screen_id);
        owner_.log.implemented("FrontEndScreen::register", "004f71d0");
    }
    void clear_page_context() override {
        owner_.log.unimplemented("PressStartScreen::clear_page_context", "00518d60");
    }
    void publish_layout(void* sink, std::uint32_t layout) override {
        // 004d6790(sink, screen+8h): the layout joins the list 004f83b0 walks.
        auto* children = static_cast<std::vector<void*>*>(sink);
        GuiLayoutPage* page = layout < owner_.layout_handles.size()
            ? owner_.layout_handles[layout] : nullptr;
        if (children != nullptr && page != nullptr) children->push_back(page);
        owner_.log.implemented("PressStartScreen::publish_layout", "004d6790");
    }

    // --- the pulsing prompt element ----------------------------------------
    void set_element_color(std::uint32_t element, float r, float g, float b,
        float a) override {
        GuiLayoutWidget* widget = element < owner_.element_handles.size()
            ? owner_.element_handles[element] : nullptr;
        if (widget != nullptr) owner_.frontend.set_widget_color(*widget, r, g, b, a);
        owner_.log.implemented("PressStartScreen::set_element_color", "0067cfb0+vtable50");
    }
    void set_element_flag(std::uint32_t element, bool value) override {
        GuiLayoutWidget* widget = element < owner_.element_handles.size()
            ? owner_.element_handles[element] : nullptr;
        if (widget != nullptr) owner_.frontend.set_widget_visible(*widget, value);
        owner_.log.implemented("PressStartScreen::set_element_flag", "0067d081");
    }

    // --- XenonSystemManager 00F8ABE8, which the process does not build ------
    SignInPhase sign_in_phase() override {
        owner_.log.unimplemented("XenonSystemManager::sign_in_phase", "00a3e500");
        return SignInPhase::Idle;
    }
    bool profile_change_pending() override {
        owner_.log.unimplemented("XenonSystemManager::profile_change_pending", "00a3e3b0");
        return false;
    }
    bool has_selected_user() override {
        owner_.log.unimplemented("XenonSystemManager::has_selected_user", "00a3e510");
        return false;
    }
    bool user_operation_busy() override {
        owner_.log.unimplemented("XenonSystemManager::user_operation_busy", "00a3e540");
        return false;
    }
    bool sign_in_blocked() override {
        owner_.log.unimplemented("XenonSystemManager::sign_in_blocked", "00f8abe8+3e8");
        return false;
    }
    int invitee_slot() override {
        owner_.log.unimplemented("XenonSystemManager::invitee_slot", "00a3e470");
        return 0;
    }
    void request_sign_in(int pad_index) override {
        static_cast<void>(pad_index);
        owner_.log.unimplemented("XenonSystemManager::request_sign_in", "00a3f3d0");
    }
    void reset_sign_in_state() override {
        owner_.log.unimplemented("XenonSystemManager::reset_sign_in_state", "00a40020");
    }

    // --- input --------------------------------------------------------------
    bool action_pressed_this_frame(int action) override {
        return owner_.press_start_frame >= 0
            && action == kPressStartInputAction
            && action_pressed_this_frame_004c43c0(owner_.press_action);
    }
    void* primary_device() override {
        // bsp.py show 0067CFB0: 0067D19A..1AE captures the existing
        // F8BBF4 backend's class-2/index-0 device (004BA6D0), including its
        // real null result when that vector is empty.
        void* const device = input_runtime().input_device(2, 0);
        owner_.log.implemented("PressStartScreen::primary_device", "004ba6d0");
        return device;
    }
    bool device_button_held(void* device) override {
        // bsp.py show 0067C880: +20h(0) || +20h(0Eh), with the second
        // query skipped when the first succeeds. Reuse the existing body
        // on the same device the live press-start update just captured.
        PressStartDeviceButtons buttons(input_runtime().devices());
        const bool held = ingame_gui_cc13::press_start_device_button_held_0067c880(device, buttons);
        owner_.log.implemented("PressStartScreen::device_button_held", "0067c880");
        return held;
    }
    int device_pad_index(void* device) override {
        // 0067D29F..2AE invokes +34h on this same captured device.
        const int index = input_runtime().devices().identifier_vslot34(device);
        owner_.log.implemented("PressStartScreen::device_pad_index", "0067c880+34h");
        return index;
    }
    void rebind_primary_input() override {
        owner_.log.unimplemented("PressStartScreen::rebind_primary_input", "0067c970");
    }
    bool input_edge_latch() override {
        owner_.log.unimplemented("PressStartScreen::input_edge_latch", "004b43b0");
        return false;
    }

    // --- the shared menu command screen 00425d10 ---------------------------
    bool menu_screen_active() override {
        // The 0x290 singleton is not built here, so it is never up.
        return owner_.world.menu.base.active;
    }
    bool menu_command_pending() override {
        return owner_.world.menu.pending_command_a != 0
            || owner_.world.menu.pending_command_b != 0;
    }
    void clear_prompt_slot(int slot) override {
        static_cast<void>(slot);
        owner_.log.unimplemented("MenuCommandScreen::clear_prompt_slot", "00532a20");
    }
    void clear_all_prompt_slots() override {
        owner_.log.unimplemented("MenuCommandScreen::clear_all_prompt_slots", "00530650");
    }
    void close_menu_screen() override {
        owner_.log.unimplemented("MenuCommandScreen::close", "004b6e50");
    }
    void raise_prompt(int slot, const char* message_key, int kind) override {
        static_cast<void>(slot);
        static_cast<void>(kind);
        owner_.log.unimplemented("MenuCommandScreen::raise_prompt", "00531b00");
        owner_.log.notef("press-start prompt %s requested", message_key);
    }

    // --- the profile block at game+650h -------------------------------------
    void reset_profile_007fdb20() override {
        if constexpr (kProfileOwnerCc15Bound) {
            owner_.profile.reset_to_defaults_007fdb20();
            owner_.log.implemented("PressStartScreen::reset_profile", "007fdb20");
        } else {
            owner_.log.unimplemented("PressStartScreen::reset_profile", "007fdb20");
        }
    }
    void reset_award_tracker_004374f0() override {
        owner_.log.unimplemented("PressStartScreen::reset_award_tracker", "004374f0");
    }
    const char* signed_in_gamertag() override {
        owner_.log.unimplemented("PressStartScreen::signed_in_gamertag", "00a3eae0");
        return "";
    }
    void set_profile_name(const char* name) override {
        static_cast<void>(name);
        owner_.log.unimplemented("PressStartScreen::set_profile_name", "007f9290");
    }
    void set_profile_display_name(const char* name) override {
        static_cast<void>(name);
        owner_.log.unimplemented("PressStartScreen::set_profile_display_name", "007f9340");
    }
    std::uint64_t signed_in_xuid() override {
        owner_.log.unimplemented("PressStartScreen::signed_in_xuid", "00a3eb00");
        return 0;
    }
    void set_profile_xuid(std::uint64_t xuid) override {
        static_cast<void>(xuid);
        owner_.log.unimplemented("PressStartScreen::set_profile_xuid", "007fdb20+48h");
    }
    std::uint64_t save_id() override {
        owner_.log.unimplemented("PressStartScreen::save_id", "00a3e5d0");
        return 0;
    }
    void reset_save_manager_00bd3450() override {
        owner_.log.unimplemented("PressStartScreen::reset_save_manager", "00bd3450");
    }
    void commit_profile_007fae70() override {
        owner_.log.unimplemented("PressStartScreen::commit_profile", "007fae70");
    }

    // --- the save/storage manager at 0109CECC -------------------------------
    bool storage_device_required() override {
        owner_.log.unimplemented("StorageManager::device_required", "0109cecc+21h");
        return false;
    }
    bool storage_busy() override {
        owner_.log.unimplemented("StorageManager::busy", "0109cecc+8h");
        return false;
    }
    bool storage_query_1c(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::query_1c", "0109cecc+vtable1c");
        return false;
    }
    bool storage_query_14(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::query_14", "0109cecc+vtable14");
        return false;
    }
    void request_read_007ff100(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::request_read", "007ff100");
    }
    void request_write_007fa710(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::request_write", "007fa710");
    }

    // --- the offline bring-up in the phase-2 arm ---------------------------
    void reset_offline_profile() override {
        owner_.log.unimplemented("PressStartScreen::reset_offline_profile", "008d5b50");
    }
    void apply_input_settings() override {
        owner_.log.unimplemented("PressStartScreen::apply_input_settings", "006ac030");
    }
    void refresh_input_bindings() override {
        owner_.log.unimplemented("PressStartScreen::refresh_input_bindings", "008d44c0");
    }
    void commit_input_manager() override {
        // 0067CEB9 (apply_sign_in) and 0067D50F (the no-user arm): ECX =
        // [00E188A8]+3Ch, CALL 00698A10, unconditionally (no +55Ch gate,
        // unlike OnInitOnce 004DD739): Inputs.lua is re-read into the action
        // owner, rebound, ticked once at zero seconds, and the keyboard
        // defaults preserved when [00F88A30] is clear.
        if (owner_.input_configuration == nullptr) {
            owner_.log.unimplemented("PressStartScreen::commit_input_manager", "00698a10");
            return;
        }
        owner_.input_configuration->load();
        owner_.log.implemented("PressStartScreen::commit_input_manager", "00698a10");
    }

    // --- frame tail ---------------------------------------------------------
    void frontend_tail_event(int code) override {
        static_cast<void>(code);
        owner_.log.unimplemented("PressStartScreen::frontend_tail_event", "00427190");
    }
    void request_main_menu_state() override {
        // 0068d8a0 BSP_TitleScreen_Skip. The main-menu path owns that step, so
        // the request is handed to it rather than enqueued twice; see
        // docs/GAME_EXECUTABLE.md, milestone 2c.
        owner_.log.implemented("PressStartScreen::request_main_menu_state", "0068d8a0");
        owner_.press_start_handoff_requested = true;
    }

private:
    GameInputRuntime& input_runtime() const {
        if (!owner_.input_configuration)
            throw std::logic_error("press-start input requires the application's input runtime");
        return owner_.input_configuration->input_runtime();
    }
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 00518250, the front-end frame layout selector
// ---------------------------------------------------------------------------

class MenuFrameLayoutHost final : public FrontEndFrameLayoutHost {
public:
    explicit MenuFrameLayoutHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void* gui_layout_acquire(const char* name) override {
        // 004c12b0 then 00aa5840(&name, 1, 1). These two layouts belong to the
        // 0x164 singleton at 00e18d80, not to a registry screen, so no 004f83b0
        // ever reaches them and the sprite bridge's substitute rule still
        // decides whether they are drawn.
        owner_.log.implemented("FrontEndFrameLayouts::acquire", "00518380");
        GuiLayoutPage* page = owner_.frontend.load_screen_page(name);
        if (page != nullptr) {
            // Not screen owned in the native sense; publish the same "shown"
            // byte the title state leaves them at so the rule is explicit.
            // Milestone 2i: set 3 is the pause pair, which the mission load
            // acquires while nothing is paused. No pause screen exists in this
            // process to publish a byte for them, so the executable publishes
            // them hidden and says so; that is milestone 2b's substitute rule
            // for a page no screen owns, with the executable choosing the value.
            owner_.frontend.commit_page_visibility(*page, !owner_.acquire_hidden);
        }
        return page;
    }
    std::int32_t gui_layout_refcount(void* layout) override {
        auto* page = static_cast<GuiLayoutPage*>(layout);
        return page != nullptr ? page->reference_count : 0;
    }
    void gui_manager_release_layout(void* layout) override {
        // 00518330: the last reference goes, and the GUI manager destroys the
        // layout, so its widget tree stops existing and nothing draws it.
        // GuiPageRegistry in this process owns every page for the whole run and
        // has no destroy path, so the substitute for that destruction is to push
        // the page hidden before releasing it. Milestone 2j: without this the
        // committing 00518250 at 004c9e06 released the handle and the sprite
        // bridge kept drawing FE_frame and FE_frame_title from widget records
        // that outlive the page.
        auto* page = static_cast<GuiLayoutPage*>(layout);
        if (page != nullptr && page->root) {
            owner_.frontend.commit_page_visibility(*page, false);
        }
        owner_.frontend.release_screen_page(page);
    }
    void gui_layout_release_ref(void* layout) override {
        auto* page = static_cast<GuiLayoutPage*>(layout);
        if (page != nullptr && page->reference_count > 0) --page->reference_count;
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 0068d8d0 and 0068d850, the title handover
// ---------------------------------------------------------------------------

// Borrow the screen installed by 0068D8D0/0067CA80 and its existing input
// provider for the duration of 0068D850. No second screen or owner is created.
class MenuPressStartUpdateAdapter final : public engine_seg11_35_66_cc13::FrontEndScreenObject {
public:
    MenuPressStartUpdateAdapter(GameMenuHost::Impl& owner, MenuPressStartHost& press)
        : owner_(owner), press_(press) {}

    void update_20(float seconds) override {
        const PressStartOutcome outcome = update_press_start_screen_0067cfb0(
            owner_.press_start, owner_.press_globals, owner_.prompt_pulse, seconds,
            press_);
        owner_.log.implemented("PressStartScreen::update", "0067cfb0");
        if (outcome != PressStartOutcome::Idle) {
            owner_.log.notef("press-start outcome %d", static_cast<int>(outcome));
        }
    }

private:
    GameMenuHost::Impl& owner_;
    MenuPressStartHost& press_;
};

class MenuTitleHandoverHost final : public TitleHandoverHost {
public:
    MenuTitleHandoverHost(GameMenuHost::Impl& owner, MenuPressStartHost& press,
        MenuCommitHost& commit) : owner_(owner), press_(press), commit_(commit) {}

    void reset_storage_availability() override {
        owner_.log.unimplemented("TitleScreen::reset_storage_availability", "00bd3450");
    }
    void request_game_state(int state) override {
        enqueue_state_request_004d3ed0(owner_.requests, static_cast<std::uint32_t>(state));
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        owner_.log.implemented("TitleScreen::request_game_state", "004d7920");
    }
    FrontEndScreen* create_press_start_screen() override {
        // operator new(18h) then 0067c840, which chains 004f7180.
        construct_press_start_screen_0067c840(owner_.press_start);
        owner_.add_screen(kPressStartScreenId, "press_start",
            &owner_.press_start.base, 0x0067cb40u, 0x0067c870u, 0x0067cfb0u, true);
        owner_.log.implemented("TitleScreen::create_press_start_screen", "0067c840");
        return &owner_.press_start.base;
    }
    void screen_register(FrontEndScreen& screen) override {
        static_cast<void>(screen);
        register_press_start_screen_0067ca80(owner_.press_start, press_);
        owner_.summary.press_start_registered =
            owner_.world.screens.slots[kPressStartScreenId] == &owner_.press_start.base;
        owner_.summary.press_start_slot = kPressStartScreenId;
    }
    void screen_commit(FrontEndScreen& screen) override {
        commit_front_end_screen_visibility_004f83b0(screen, kPressStartScreenId, commit_);
        owner_.log.implemented("FrontEndScreen::commit_visibility", "004f83b0");
    }
    void screen_enter(FrontEndScreen& screen) override {
        static_cast<void>(screen);
        enter_press_start_screen_0067cb40(owner_.press_start, press_);
        ++owner_.summary.screen_enters;
        owner_.log.implemented("PressStartScreen::enter", "0067cb40");
    }
    void update_title_owner(float raw_delta) override {
        static_cast<void>(raw_delta);
        owner_.log.unimplemented("TitleScreen::update_title_owner", "00f8bbf4+4h");
    }
    void update_press_start_screen(float raw_delta) override {
        // 004F71F0 dispatches the real screen's +20h; OFF keeps the direct press-start update.
        MenuPressStartUpdateAdapter screen(owner_, press_);
        engine_seg11_35_66_cc13::update_front_end_screen_004f71f0(screen, raw_delta);
        owner_.log.implemented("TitleScreen::update_press_start_screen", "004f71f0");
    }

private:
    GameMenuHost::Impl& owner_;
    MenuPressStartHost& press_;
    MenuCommitHost& commit_;
};

// ---------------------------------------------------------------------------
// 004c9a70, GGame::OnInitTitle
// ---------------------------------------------------------------------------

class MenuTitleInitHost final : public TitleInitHost,
    private cc13_fe5::AttractScreenConstructionHost {
public:
    MenuTitleInitHost(GameMenuHost::Impl& owner, MenuFrameLayoutHost& layouts,
        MenuTitleHandoverHost& handover, MenuCommitHost& commit)
        : owner_(owner), layouts_(layouts), handover_(handover), commit_(commit) {}

    void open_named_block(const char* label) override {
        owner_.vfs.open_title_fileblock(label);
    }
    void close_named_block() override {
        owner_.vfs.close_title_fileblock();
    }
    void reset_player_profile() override {
        if constexpr (kProfileOwnerCc15Bound) {
            // 004C9AFA: ECX = game+650h. Destroys and re-allocates +64h empty.
            owner_.profile.reset_to_defaults_007fdb20();
            owner_.log.implemented("TitleInit::reset_player_profile", "007fdb20");
        } else {
            owner_.log.unimplemented("TitleInit::reset_player_profile", "007fdb20");
        }
    }
    void load_texture_atlas(const char* path) override {
        // 00af0060 on the atlas manager 00f8c26c. The sprite bridge parses the
        // installed descriptors itself; the native manager is another owner's.
        owner_.log.unimplemented("TitleInit::load_texture_atlas", "00af0060");
        owner_.log.notef("title atlas %s requested", path);
    }
    void select_front_end_frame_set(FrontEndFrameSet set, bool commit) override {
        select_front_end_frame_set_impl(set, commit);
    }
    void* create_attract_screen() override {
        auto* screen = cc13_fe5::attract_screen_construct_00689d90(
            owner_.attract_object, owner_.manager_registry,
            owner_.attract_singleton, *this);
        owner_.log.implemented("TitleInit::create_attract_screen", "00689d90");
        return screen;
    }
    void attract_screen_init(void* screen) override {
        // 004C9BBD..004C9BC5 calls vtable +4h; 00689BD0 is a bare RET.
        cc13_fe5::attract_screen_virtual04no_op_00689bd0(
            {reinterpret_cast<std::uint8_t*>(screen)});
        owner_.log.implemented("TitleInit::attract_screen_init", "00689bd0");
    }
    void* create_title_screen() override {
        auto* title = cc13_fe5::title_screen_construct_0068d760(
            owner_.title_object, owner_.manager_registry, owner_.world.title);
        owner_.log.implemented("TitleInit::create_title_screen", "0068d760");
        return title;
    }
    void title_screen_activate(void* screen) override {
        auto& title = *static_cast<TitleScreen*>(screen);
        // 0068D8D0 receives the constructor's object. Its +40h is the
        // source and destination of the activation projection, including
        // the skip-title arm, which does not create a new screen.
        owner_.handover.press_start = title.attached_screen;
        activate_title_screen_0068d8d0(owner_.handover, owner_.world.screens, handover_);
        title.attached_screen = owner_.handover.press_start;
        owner_.log.implemented("TitleInit::title_screen_activate", "0068d8d0");
    }
    void title_screen_skip() override {
        owner_.log.unimplemented("TitleInit::title_screen_skip", "0068d8a0");
    }
    void reset_mission_player_records(void* records) override {
        static_cast<void>(records);
        owner_.log.unimplemented("TitleInit::reset_mission_player_records", "00916980");
    }
    bool sign_in_state_valid() override {
        owner_.log.unimplemented("TitleInit::sign_in_state_valid", "0067c8f0");
        return false;
    }
    void reset_sign_in_state() override {
        owner_.log.unimplemented("TitleInit::reset_sign_in_state", "00a40020");
    }
    void rebind_primary_input() override {
        owner_.log.unimplemented("TitleInit::rebind_primary_input", "0067c970");
    }
    void movie_screen_exit() override {
        owner_.log.unimplemented("TitleInit::movie_screen_exit", "00e18d48+vtable1c");
    }
    void movie_screen_commit() override {
        // 004f83b0 on 00e18d48. No movie screen is registered here, so the
        // cleared byte reaches no child.
        static_cast<void>(commit_);
        owner_.log.unimplemented("TitleInit::movie_screen_commit", "004f83b0");
    }

private:
    void* load_attract_page_00aa5840(std::string_view name) override {
        // 00689E15..00689E1F pushes (screen_flag=1, add_reference=0).
        // load_screen_page forwards screen_flag and keeps add_reference=false.
        return owner_.frontend.load_screen_page(std::string(name), 1);
    }
    void select_front_end_frame_set_impl(FrontEndFrameSet set, bool commit) {
        bsp::select_front_end_frame_set(owner_.frame_layouts, layouts_,
            static_cast<int>(set), commit);
        owner_.log.implemented("TitleInit::select_front_end_frame_set", "00518250");
    }

    GameMenuHost::Impl& owner_;
    MenuFrameLayoutHost& layouts_;
    MenuTitleHandoverHost& handover_;
    MenuCommitHost& commit_;
};

// ---------------------------------------------------------------------------
// 004f8830's per-screen virtuals
// ---------------------------------------------------------------------------

class MenuScreenHost final : public FrontEndScreenHost {
public:
    MenuScreenHost(GameMenuHost::Impl& owner, MenuPressStartHost& press,
        MenuCommitHost& commit) : owner_(owner), press_(press), commit_(commit) {}

    void screen_exit(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        ++owner_.summary.screen_exits;
        if (screen != nullptr && screen->press_start) {
            exit_press_start_screen_0067c870(owner_.press_start);
            owner_.log.implemented("PressStartScreen::exit", "0067c870");
            return;
        }
        // Packet cc9_role_retake_4bh: screen 2Eh's +1Ch, 005470A0.
        if (kHudWeaponGroupEnterExitBound && owner_.hud != nullptr && slot == 0x2e) {
            owner_.hud->exit_weapon_group_screen_005470a0();
            return;
        }
        // cc15-hint2: screen 34h's +1Ch, 0054E3D0.
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (slot == 0x34 && owner_.hint_tracker.built() != nullptr) {
                owner_.hint_tracker.built()->hints_screen_34().exit_0054e3d0();
                return;
            }
        }
        // Packet cc13-releasegun: screen 46h's +1Ch, 0064A490.
        if (kHarnessReleaseGunMapBound && owner_.hud != nullptr && slot == 0x46
            && owner_.hud->release_gun_armed()) {
            owner_.hud->exit_ship_view_screen_0064a490();
            return;
        }
        // Goal 5 chain 646: the leaf +1Ch virtuals the HUD runs on their blocks.
        if (owner_.hud != nullptr && owner_.hud->exit_leaf_screen(slot)) return;
        // Every other screen's +1Ch is its own routine, not run here.
        owner_.log.unimplemented("FrontEndScreen::exit", "004f75b0");
        owner_.log.notef("screen %d exit virtual %08lx", slot,
            screen != nullptr ? static_cast<unsigned long>(screen->exit_virtual) : 0ul);
    }

    void screen_enter(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        ++owner_.summary.screen_enters;
        if (screen != nullptr && screen->press_start) {
            enter_press_start_screen_0067cb40(owner_.press_start, press_);
            owner_.log.implemented("PressStartScreen::enter", "0067cb40");
            return;
        }
        // Packet cc9_force_select_unit: screen 44h's +18h, 006488D0 (vtable
        // 00CF5A9C +18h = 00CF5AB4), for its 00648290 list rebuild.
        if (kForceSelectUnitBound && owner_.hud != nullptr && slot == 0x44) {
            owner_.hud->enter_hud_root_screen_006488d0();
            return;
        }
        // Packet cc9_role_retake_4bh: screen 2Eh's +18h, 005494C0.
        if (kHudWeaponGroupEnterExitBound && owner_.hud != nullptr && slot == 0x2e) {
            owner_.hud->enter_weapon_group_screen_005494c0();
            return;
        }
        // cc15-hint2: screen 34h's +18h, 0054E380.
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (slot == 0x34 && owner_.hint_tracker.built() != nullptr) {
                owner_.hint_tracker.built()->hints_screen_34().enter_0054e380();
                return;
            }
        }
        // Goal 5 chain 646: the leaf +18h virtuals the HUD runs on their blocks.
        if (owner_.hud != nullptr && owner_.hud->enter_leaf_screen(slot)) return;
        if (screen != nullptr && screen->enter_virtual == 0x005987f0u) {
            // 00598B21..00598B29: main-menu entry passes zero to 00549620
            // on the retained 00E1930C command owner. The rest of 005987F0
            // remains unimplemented here; this is its help-widget tail.
            owner_.set_command_help_variant(0);
        }
        owner_.log.unimplemented("FrontEndScreen::enter", "004f75a0");
        if (screen == nullptr) return;
        owner_.log.notef("screen %d (%s) enter virtual %08lx", slot, screen->name.c_str(),
            static_cast<unsigned long>(screen->enter_virtual));
    }

    void screen_update(int slot, float seconds) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        ++owner_.summary.screen_updates;
        if (screen != nullptr && screen->press_start) {
            const PressStartOutcome outcome = update_press_start_screen_0067cfb0(
                owner_.press_start, owner_.press_globals, owner_.prompt_pulse, seconds,
                press_);
            owner_.log.implemented("PressStartScreen::update", "0067cfb0");
            if (outcome != PressStartOutcome::Idle) {
                owner_.log.notef("press-start outcome %d", static_cast<int>(outcome));
            }
            return;
        }
        // Milestone 2k: two of the 42 in-mission screens have their update
        // virtual reconstructed, and the pump is where the native calls them.
        // 005c0f20 is slot 35h's and 006435d0 is slot 4Dh's; both are reached
        // only once the applied interface publishes their slot into level 1.
        if (owner_.hud != nullptr && slot == 0x35) {
            owner_.hud->update_minimap_screen_005c0f20(seconds);
            return;
        }
        // cc15-hint2: screen 34h's update, 0054E6B0.
        if constexpr (kHintTriggerPassesCc15Bound) {
            if (slot == 0x34 && owner_.hint_tracker.built() != nullptr) {
                owner_.hint_tracker.built()->hints_screen_34().update_0054e6b0(seconds);
                return;
            }
        }
        if (owner_.hud != nullptr && slot == 0x4d) {
            owner_.hud->update_markers_screen_006435d0(seconds);
            return;
        }
        // Packet cc9_ship_screen_update: screen 45h's 0064DD30.
        if (kHudShipScreenUpdateBound && owner_.hud != nullptr && slot == 0x45) {
            owner_.hud->update_ship_screen_0064dd30(seconds,
                screen != nullptr && screen->flags != nullptr && screen->flags->active);
            return;
        }
        // Packet cc9_screen_46h: screen 46h's 0064D610.
        if (kHudShipViewScreenBound && owner_.hud != nullptr && slot == 0x46) {
            owner_.hud->update_ship_view_screen_0064d610(seconds,
                screen != nullptr && screen->flags != nullptr && screen->flags->wanted);
            return;
        }
        // Packet cc9_screen_49h: screen 49h's 0067BF00.
        if (kHudFollowScreenBound && owner_.hud != nullptr && slot == 0x49) {
            owner_.hud->update_follow_screen_0067bf00();
            return;
        }
        // Packet cc9_screen_2eh_group: screen 27h's 0067BB50, the role take.
        if (kHudRoleScreenPumpBound && owner_.hud != nullptr && slot == 0x27) {
            owner_.hud->update_role_screen_0067bb50();
            return;
        }
        // Packet cc9_screen_29h: screen 29h's 00527260, the unit pick.
        if (kHudUnitPickScreenBound && owner_.hud != nullptr && slot == 0x29) {
            owner_.hud->update_unit_pick_screen_00527260(seconds);
            return;
        }
        // Packet cc9_screen_44h: screen 44h's 00649860, the HUD root.
        if (kHudRootScreenBound && owner_.hud != nullptr && slot == 0x44) {
            owner_.hud->update_hud_root_screen_00649860(seconds);
            return;
        }
        // Packet cc9_screen_50h: screen 50h's 00683020.
        if (kHudWarningScreenBound && owner_.hud != nullptr && slot == 0x50) {
            owner_.hud->update_warning_screen_00683020(seconds);
            return;
        }
        // Packet cc9_movie_camera_mover_bind: screen 37h's 005CBAF0, while 2Ch
        // is applied; the HUD steps the movie mover from it.
        if (kMovieMoverBound && owner_.hud != nullptr && slot == 0x37) {
            owner_.hud->update_movie_screen_005cbaf0(seconds);
            return;
        }
        // Packet cc13-relgaps gap 2: screen 4Ch's 005B45C0, while the map is up;
        // its helm release and persisted local-player map usage are hosted.
        if (kMapScreenHelmReleaseCc13Bound && owner_.hud != nullptr && slot == 0x4c) {
            owner_.hud->update_map_screen_005b45c0(seconds);
            return;
        }
        // Packet cc9_screen_26h_2eh: these three slots' update virtual is the
        // base 004F75C0 itself, `RET 4`, so the call does nothing. Goal 5
        // chain 636: the dispatch calls that base body.
        if (kHudBaseUpdateScreensBound && (slot == 0x26 || slot == 0x2e || slot == 0x3e)) {
            engine_seg11_35_66_cc13::base_screen_update_004f75c0(seconds);
            owner_.log.implemented("FrontEndScreen::base_update", "004f75c0");
            return;
        }
        // Goal 5 chain 646: the leaf +20h virtuals the HUD runs on their blocks.
        if (owner_.hud != nullptr && owner_.hud->update_leaf_screen(slot, seconds)) return;
        owner_.log.unimplemented("FrontEndScreen::update", "004f75c0");
    }

    void screen_commit(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        if (screen == nullptr || screen->flags == nullptr) return;
        commit_front_end_screen_visibility_004f83b0(*screen->flags, slot, commit_);
        owner_.log.implemented("FrontEndScreen::commit_visibility", "004f83b0");
        owner_.frontend.invalidate_bridge();
    }

    void menu_command_update(float seconds) override {
        static_cast<void>(seconds);
        owner_.log.unimplemented("MenuCommandScreen::update", "00425d10+vtable20");
    }
    void menu_command_exit() override {
        owner_.log.unimplemented("MenuCommandScreen::exit", "00425d10+vtable1c");
    }
    void menu_command_commit() override {
        owner_.log.unimplemented("MenuCommandScreen::commit", "004f83b0+menu");
    }

private:
    GameMenuHost::Impl& owner_;
    MenuPressStartHost& press_;
    MenuCommitHost& commit_;
};

// The main-menu destructor inlines the base close sequence at
// 00686DA0..00686DB9. Keep its leaf-exit and visibility bindings while the
// reconstructed close body owns the active test and both flag stores.
class MainMenuClosingScreenHost final : public FrontEndScreenHost {
public:
    MainMenuClosingScreenHost(GameMenuHost::Impl& owner, MenuCommitHost& commit,
        FrontEndScreenHost& screens, std::size_t& exited)
        : owner_(owner), commit_(commit), screens_(screens), exited_(exited) {}

    void screen_exit(int slot) override {
        static_cast<void>(slot);
        owner_.log.unimplemented("MainMenuScreen::exit", "00686dad");
        ++exited_;
    }
    void screen_commit(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        if (screen == nullptr || screen->flags == nullptr) return;
        commit_front_end_screen_visibility_004f83b0(*screen->flags, slot, commit_);
        owner_.log.implemented("MainMenuManager::commit_screen_visibility", "00686db9");
    }
    void screen_enter(int slot) override { screens_.screen_enter(slot); }
    void screen_update(int slot, float seconds) override { screens_.screen_update(slot, seconds); }
    void menu_command_update(float seconds) override { screens_.menu_command_update(seconds); }
    void menu_command_exit() override { screens_.menu_command_exit(); }
    void menu_command_commit() override { screens_.menu_command_commit(); }

private:
    GameMenuHost::Impl& owner_;
    MenuCommitHost& commit_;
    FrontEndScreenHost& screens_;
    std::size_t& exited_;
};

// ---------------------------------------------------------------------------
// 004e4b9d..004e4d2c, the front-end branch of GGame::OnMove
// ---------------------------------------------------------------------------

class MenuFrameHost final : public FrontEndFrameHost {
public:
    MenuFrameHost(GameMenuHost::Impl& owner, MenuTitleHandoverHost& handover)
        : owner_(owner), handover_(handover) {}

    int game_state() override { return owner_.state.value; }

    bool render_queue_retained() override {
        // 00B1BF90 on the renderer's actual 00F8D440 queue (004E4BBA CALL 004C11F0,
        // MOV ECX,EAX, 004E4BC1 CALL 00B1BF90, 004E4BC6 TEST EAX,EAX); OFF keeps false.
        // The queue is GameNativeRendererApplication's FrameGraph::queue, the
        // 00F8D440 cell its end frame fills through 004C11F0 (00B1F280 sets
        // +20h = 0), published by bind_platform_services as
        // native_deletion_bindings().render_queue and retired at renderer drain.
        // Before the first end frame the cell is empty; 004C11F0 would build
        // the queue with +20h = 0 there, so the frame opens as below.
        const NativeRenderQueueDestructionContext* const published =
            owner_.singletons.native_deletion_bindings().render_queue;
        const NativeRenderCommandQueueStorage* const queue =
            published != nullptr ? published->actual_queue_00f8d440 : nullptr;
        if (queue != nullptr) {
            // Own label: the call table keeps a row's first status, and the
            // first call (before frame 1's end) takes the unpublished arm.
            owner_.log.implemented("FrontEndFrame::render_queue_retention_control", "00b1bf90");
            return cc13_seg83::render_queue_mode_00b1bf90(queue) != 0;
        }
        owner_.log.unimplemented("FrontEndFrame::render_queue_retained", "00b1bf90");
        return false;
    }
    void renderer_begin_frame() override {
        // 00f8d394 vtable +0Ch. The milestone's own Clear/BeginScene stands in.
        owner_.log.unimplemented("FrontEndFrame::renderer_begin_frame", "00f8d394+0ch");
    }
    void update_logo_sequence() override {
        owner_.log.unimplemented("FrontEndFrame::update_logo_sequence", "00685170");
    }
    void reinitialize_title_screen() override {
        owner_.log.unimplemented("FrontEndFrame::reinitialize_title_screen", "004db220");
    }
    void update_title_screen(float raw_delta) override {
        {
            // The state-2 driver tested world.title before this callback.
            // 0068D850 reads that very object's +40h; retain the handover's
            // other real state while taking the attachment from its owner.
            auto live_title = owner_.handover;
            live_title.press_start = owner_.world.title->attached_screen;
            update_title_screen_0068d850(live_title, handover_, raw_delta);
        }
        owner_.log.implemented("FrontEndFrame::update_title_screen", "0068d850");
    }
    // 00aa0e50 is MOV ECX,[ECX+50h]; JMP [vtable+34h]: the pointer widget's
    // visibility setter, not a screen clear (this row was gui_clear_screens).
    void gui_set_pointer_visible(bool visible) override {
        if (kFrontEndPointerCc13Bound) {
            owner_.frontend.gui_manager_set_pointer_visible_00aa0e50(visible);
            owner_.log.implemented("FrontEndFrame::gui_set_pointer_visible", "00aa0e50");
            return;
        }
        owner_.log.unimplemented("FrontEndFrame::gui_set_pointer_visible", "00aa0e50");
    }
    void gui_set_enabled(bool enabled) override {
        if (kFrontEndPointerCc13Bound) {
            owner_.frontend.gui_manager_set_enabled_00aa0e00(enabled);
            owner_.log.implemented("FrontEndFrame::gui_set_enabled", "00aa0e00");
            return;
        }
        owner_.log.unimplemented("FrontEndFrame::gui_set_enabled", "00aa0e00");
    }
    void gui_update(float raw_delta, int mode) override {
        static_cast<void>(raw_delta);
        static_cast<void>(mode);
        owner_.log.unimplemented("FrontEndFrame::gui_update", "00aa4f80");
    }
    void game_render() override {
        // 004CA44E: nothing runs unless game+34h is 1 (the frame begun by
        // 004E4BB5..004E4BDB, frame_state.render_queue_open here). In the
        // front-end states [00E198C4] is null, so 004CA4E7 falls through to
        // 004CA5D4's 00735B50. The rest of 004CA440 keeps its record.
        if (owner_.frame_state.render_queue_open) {
            if (NullInterfaceSoundFrame* const sound = owner_.bind_sound_frame()) {
                service_sound_frame_00735b50(sound->context());
                owner_.log.implemented("FrontEndFrame::sound_frame_service", "00735b50");
            } else {
                owner_.log.unimplemented("FrontEndFrame::sound_frame_service", "00735b50");
            }
        }
        owner_.log.unimplemented("FrontEndFrame::game_render", "004ca440");
    }
    void game_finish_render_frame() override {
        // 004ca1f0 clears game+34h. The milestone's own Present is the frame
        // boundary here, so the projected byte is cleared with it.
        owner_.frame_state.render_queue_open = false;
        owner_.log.unimplemented("FrontEndFrame::game_finish_render_frame", "004ca1f0");
    }
    bool state_requests_pending() override { return owner_.requests.count != 0; }
    void drain_state_requests() override {
        if (owner_.requests.count == 0) return;
        const std::uint32_t request = front_state_request(owner_.requests);
        pop_front_state_request(owner_.requests);
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        apply_drained_game_state(owner_.state, static_cast<int>(request));
        owner_.log.implemented("FrontEndFrame::drain_state_requests", "004e4430");
        owner_.log.notef("state request %lu drained by the front-end branch",
            static_cast<unsigned long>(request));
    }

private:
    GameMenuHost::Impl& owner_;
    MenuTitleHandoverHost& handover_;
};

// ---------------------------------------------------------------------------
// 005884a0, the title music the shell starts
// ---------------------------------------------------------------------------

class MenuTitleMusicHost final : public TitleMusicHost {
public:
    explicit MenuTitleMusicHost(GameMenuHost::Impl& owner) : owner_(owner) {}
    bool movie_clip_active(std::string_view clip) override {
        static_cast<void>(clip);
        owner_.log.unimplemented("TitleMusic::movie_clip_active", "004f8ba0");
        return false;
    }
    void* create_music_stream(std::string_view path) override {
        // 00588528: 00A877D0 builds the 54h stream on the live sound core;
        // OFF keeps the null stand-in. No started core: the OFF record.
        MenuTitleMusicStream* const title = owner_.bind_title_stream();
        void* const stream = title != nullptr ? title->construct(path) : nullptr;
        if (title != nullptr) {
            owner_.log.implemented("TitleMusic::create_music_stream", "00a877d0");
        } else {
            owner_.log.unimplemented("TitleMusic::create_music_stream", "00a877d0");
        }
        owner_.log.notef("title music path %.*s", static_cast<int>(path.size()), path.data());
        return stream;
    }
    std::string_view selected_track_name() override {
        owner_.log.unimplemented("TitleMusic::selected_track_name", "00e19504");
        return {};
    }
    void open_stream(void* stream, std::string_view track) override {
        // 0058855C: 00A867B0 starts the stream. The track is the host's empty
        // one, which is 00E19504's: that .data string is zero in the PE and
        // its other xrefs (0058BE63, 0059A16B) are c_str() fallbacks, not
        // writers. OFF keeps the record.
        if (stream != nullptr && owner_.title_stream != nullptr) {
            owner_.title_stream->start(stream, track);
            owner_.log.implemented("TitleMusic::open_stream", "00a867b0");
        } else {
            owner_.log.unimplemented("TitleMusic::open_stream", "00a867b0");
        }
    }
    float music_volume() override {
        owner_.log.unimplemented("TitleMusic::music_volume", "00f889a8");
        return 0.0f;
    }
    void set_stream_volume(void* stream, float volume) override {
        static_cast<void>(stream);
        static_cast<void>(volume);
        owner_.log.unimplemented("TitleMusic::set_stream_volume", "00a864f0");
    }
    void play_stream(void* stream) override {
        // 005885AD: 00A85C20 requests the fade-in on the stream 00A867B0
        // started (state +20h, fade-out +Ah; sets +Bh and +Ch). OFF keeps the record.
        if (stream != nullptr) {
            request_sound_stream_fade_in_00a85c20(stream);
            owner_.log.implemented("TitleMusic::play_stream", "00a85c20");
        } else {
            owner_.log.unimplemented("TitleMusic::play_stream", "00a85c20");
        }
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 00686380 and its siblings, the three front-end managers
// ---------------------------------------------------------------------------

class MenuManagerHost final : public FrontEndManagerHost {
public:
    explicit MenuManagerHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void open_load_block(std::string_view name) override {
        // 00be0ae0: FileBlockScope_Construct enters the label's block on the live
        // VFS; OFF keeps the record row and the note.
        if (owner_.open_load_block_00be0ae0(name)) {
            owner_.log.implemented("FrontEndManager::open_load_block", "00be0ae0");
        } else {
            owner_.log.unimplemented("FrontEndManager::open_load_block", "00be0ae0");
        }
        owner_.log.notef("front-end load block %.*s", static_cast<int>(name.size()),
            name.data());
    }
    void close_load_block() override {
        // 00bdcb30: FileBlock_Destroy leaves the block 00be0ae0 entered; OFF keeps
        // the record row.
        if (owner_.close_load_block_00bdcb30()) {
            owner_.log.implemented("FrontEndManager::close_load_block", "00bdcb30");
        } else {
            owner_.log.unimplemented("FrontEndManager::close_load_block", "00bdcb30");
        }
    }
    void report_loading_progress(float progress) override {
        if constexpr (kLoadingScreenCc13Bound) {
            owner_.report_loading_progress_0057bec0(progress);
            owner_.log.implemented("FrontEndManager::report_loading_progress", "0057bec0");
        } else {
            static_cast<void>(progress);
            owner_.log.unimplemented("FrontEndManager::report_loading_progress", "0057bec0");
        }
    }
    void* create_screen(std::size_t size_bytes, std::uint32_t constructor_address) override {
        // Construct the leaves named by 00686380's and 006898C0's tables whose
        // constructors are bound (construct_menu_leaf); the other main-menu
        // classes keep the constructor-address registry record.
        owner_.log.unimplemented("FrontEndManager::create_screen", "00bf681b");
        auto native = construct_menu_leaf(size_bytes, constructor_address, owner_.log);
        const MainMenuScreenClass* recovered = nullptr;
        for (const MainMenuScreenClass& entry : kMainMenuScreens) {
            // A constructed leaf's own primary vtable selects its class.
            // That body-produced value is what 004F71D0 dispatches through.
            const bool matches = native
                ? entry.primary_vtable == *native->primary_vtable
                : entry.constructor == constructor_address;
            if (matches) {
                recovered = &entry;
                break;
            }
        }
        if (recovered == nullptr && native) {
            // 006898C0's leaves (options_menu_screens.hpp), selected the same
            // way by the primary vtable their constructor stored.
            for (const OptionsScreenClass& entry : kOptionsScreens) {
                if (entry.primary_vtable != *native->primary_vtable) continue;
                GameMenuHost::Impl::MenuScreen& screen = owner_.add_screen(entry.screen_id,
                    std::string(entry.layout_page), native->flags, entry.enter_virtual,
                    entry.exit_virtual, entry.update_virtual, false);
                retain_menu_leaf(screen, std::move(native));
                owner_.log.notef("front-end screen id=%d layout=%.*s constructor=%08lx size=%lu",
                    entry.screen_id, static_cast<int>(entry.layout_page.size()),
                    entry.layout_page.data(), static_cast<unsigned long>(constructor_address),
                    static_cast<unsigned long>(entry.object_size));
                return &screen;
            }
        }
        if (recovered == nullptr) {
            owner_.log.notef("front-end screen %08lx (%zu bytes) has no recovered class",
                static_cast<unsigned long>(constructor_address), size_bytes);
            return nullptr;
        }
        GameMenuHost::Impl::MenuScreen& screen = owner_.add_screen(recovered->screen_id,
            std::string(recovered->primary_layout), native ? native->flags : nullptr,
            recovered->enter_virtual, recovered->exit_virtual, recovered->update_virtual, false);
        retain_menu_leaf(screen, std::move(native));
        owner_.log.notef("front-end screen id=%d layout=%.*s constructor=%08lx size=%lu",
            recovered->screen_id, static_cast<int>(recovered->primary_layout.size()),
            recovered->primary_layout.data(),
            static_cast<unsigned long>(constructor_address),
            static_cast<unsigned long>(recovered->object_size));
        return &screen;
    }
    void register_screen(void* screen) override {
        // The leaf's virtual +10h, which chains 004f71d0 and, in five of the
        // seven, binds the GUI layout. Only the main menu's layout is loaded
        // here; the other six bind data tables or pages no screen set raises.
        auto* record = static_cast<GameMenuHost::Impl::MenuScreen*>(screen);
        if (record == nullptr) return;
        register_front_end_screen_004f71d0(owner_.world.screens, *record->flags,
            record->slot);
        ++owner_.summary.screens_registered;
        owner_.log.implemented("FrontEndScreen::register", "004f71d0");
        if (record->slot == kMainMenuPathScreenId) {
            // 00582F30 chains 004F71D0 and then calls its own slot +14h, which
            // is 005861B0: FE_main plus the three page roots the mission-detail
            // page drives and the widget handles it keeps.
            owner_.attach_page(*record, "FE_main");
            if (owner_.mission != nullptr) {
                GuiLayoutPage* main_page =
                    record->pages.empty() ? nullptr : record->pages.back();
                owner_.mission->bind_main_menu_layout_005861b0(main_page);
                owner_.mission->build_top_level_page_00584ae0();
                owner_.log.implemented("MainMenuScreen::bind_layout", "005861b0");
            }
            owner_.summary.main_menu_pages_loaded = record->pages.size();
        } else if (record->slot == kInterfaceMissionTree && owner_.mission != nullptr) {
            // 005CAAF0, the mission-tree screen's own slot +10h override. It
            // reads Scripts/datatables/MissionTree.lua rather than binding a
            // GUI layout, which is why the recovered class table names a data
            // table where the other six name a page.
            owner_.mission->load_mission_tree_005caaf0();
        } else {
            owner_.log.unimplemented("FrontEndScreen::bind_layout", "004f7590");
        }
    }
    void hide_screen(void* screen) override {
        static_cast<void>(screen);
        owner_.log.unimplemented("FrontEndManager::hide_screen", "004f75b0");
    }
    void destroy_screen(void* screen) override {
        static_cast<void>(screen);
        owner_.log.unimplemented("FrontEndManager::destroy_screen", "004f75e0");
    }
    void cache_music(std::string_view path) override {
        // The FileStore cache and remove pair GVMainMenu uses for its two .fsb
        // paths and their .def siblings (docs/FRONTEND_MANAGERS.md).
        owner_.log.unimplemented("FrontEndManager::cache_music", "004fc150");
        owner_.log.notef("front-end music cached %.*s", static_cast<int>(path.size()),
            path.data());
    }
    void remove_music(std::string_view path) override {
        static_cast<void>(path);
        owner_.log.unimplemented("FrontEndManager::remove_music", "004fc150+remove");
    }
    void register_locale_table(std::string_view name) override {
        // 00686240's registrations use the application's existing locale owner; OFF keeps the log stand-in.
        auto* const tables = owner_.borrowed_menu_locale_tables();
        if (tables == nullptr)
            throw std::logic_error("Main-menu locale registration requires the phase-6 tables");
        tables->register_table_00aa0d30(std::string(name));
    }
    void reload_locale_tables() override {
        owner_.log.unimplemented("FrontEndManager::reload_locale_tables", "00aa06d0");
    }
    void set_gui_layer_enabled(int layer, bool enabled) override {
        static_cast<void>(layer);
        static_cast<void>(enabled);
        owner_.log.unimplemented("FrontEndManager::set_gui_layer_enabled", "00aa0e00");
    }
    bool returning_from_mission() override { return false; }   // 00e198b0
    bool mission_result_pending() override { return false; }   // game+1ee1h
    void clear_returning_from_mission() override {
        owner_.log.unimplemented("FrontEndManager::clear_returning_from_mission", "00e198b0");
    }
    bool script_gc_enabled() override { return false; }
    void run_script_gc() override {
        owner_.log.unimplemented("FrontEndManager::run_script_gc", "006b8ad0");
    }
    void set_game_state(int state) override { owner_.state.value = state; }
    OptionsMenuManager* create_options_manager() override {
        auto* manager = cc13_fe5::options_menu_construct_00689800(
            owner_.options_menu, owner_.manager_registry);
        owner_.log.implemented("FrontEndManager::create_options_manager", "00689800");
        return manager;
    }
    MainMenuManager* create_main_menu_manager() override {
        // 004E41B0 constructs before Init. Run the recovered body over the
        // actual owner used below and publish its base in the real registry.
        auto* manager = cc13_fe5::main_menu_construct_00686170(
            owner_.main_menu, owner_.manager_registry);
        owner_.log.implemented("FrontEndManager::create_main_menu_manager", "00686170");
        return manager;
    }
    MultiMenuManager* create_multi_menu_manager() override {
        owner_.log.unimplemented("FrontEndManager::create_multi_menu_manager", "006887e0");
        return nullptr;
    }
    void refresh_multiplayer_state() override {
        owner_.log.unimplemented("FrontEndManager::refresh_multiplayer_state", "00576b10");
    }
    // Label check (cc15-fe1): 006895CE tests the byte 00F88950 and 006895D7
    // calls 00687C00, which under the singleton-lifetime lock unregisters and
    // deletes the object in 00F8894C (BSP_Singleton_Destroy_00f8894c,
    // frontend_seg16_24_28_cc13.cpp). Nothing in it refreshes content; the
    // downloadable-content names are unsupported. The process never builds
    // that singleton, so the byte stays false here.
    bool downloadable_content_present() override { return false; }
    void refresh_downloadable_content() override {
        owner_.log.unimplemented("FrontEndManager::refresh_downloadable_content", "00687c00");
    }
    void refresh_online_services() override {
        owner_.log.unimplemented("FrontEndManager::refresh_online_services", "008d2f50");
    }
    bool profile_name_empty() override { return false; }
    void create_default_profile() override {
        owner_.log.unimplemented("FrontEndManager::create_default_profile", "0076f0e0");
    }
    bool online_signed_in() override { return false; }
    void clear_multiplayer_entry_reason() override {
        owner_.log.unimplemented("FrontEndManager::clear_multiplayer_entry_reason", "00e19610");
    }
    void reset_interface_stack() override {
        owner_.log.unimplemented("FrontEndManager::reset_interface_stack", "004d95f0");
    }
    // Label check (cc15-fe1): 004DB1A6..004DB1AF is 004D7970(0) then 004DA780
    // with ECX = the game, and 004DA780 is the whole session teardown
    // (BSP_Game_TeardownSessionState, 55 callees). clear_all_screen_set_levels_004da780
    // in main_menu_path.cpp covers only its 004DA99D..004DA9CF slice, so it is
    // not wired here as the whole call.
    void lower_in_mission_interface() override {
        owner_.log.unimplemented("FrontEndManager::lower_in_mission_interface", "004da780");
    }
    void raise_front_end_dirty_flag() override {
        owner_.log.unimplemented("FrontEndManager::raise_front_end_dirty_flag", "00e08874");
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 0057cb60 / 0057c250, the loading screen (packet cc15-ls1)
// ---------------------------------------------------------------------------

// The loading screen's native calls. The renderer, the screen object, its four
// OFF retains the element/worker records. The game state stands for 00E188A8
// throughout the shell; the render worker remains a separate integration.
// C251 binds only 0057C560 and 0057C990's synchronous init/reset domain.
// The renderer's separate 25-Hz callback is not installed by this adapter.
class MenuLoadingElementHost final : public LoadingScreenElementHost,
    private LocaleTextRuntimeHost {
public:
    explicit MenuLoadingElementHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void register_front_end_screen(int screen_id) override {
        auto& record = owner_.screens[screen_id];
        record.slot = screen_id;
        record.name = "loading";
        record.flags = &owner_.loading_screen.base;
        record.pages.clear();
        register_front_end_screen_004f71d0(owner_.world.screens,
            owner_.loading_screen.base, screen_id);
        owner_.log.implemented("LoadingScreen::register_screen", "004f71d0");
    }
    void* create_gui_page(std::string_view name) override {
        // bsp.py show 0057c560: 0057C5C2 pushes 0,0 before 00AA5840.
        // Borrow the same page registry/text bridge as every other menu page.
        GuiLayoutPage* page = owner_.frontend.load_screen_page(std::string(name), 0);
        if (page == nullptr || !page->root)
            throw std::runtime_error("LoadingScreen::Init could not load FE_loading");
        owner_.screens.at(kLoadingScreenId).pages.push_back(page);
        owner_.frontend.commit_page_visibility(*page, owner_.loading_screen.base.active);
        return page;
    }
    void* find_element(void* parent, std::string_view name) override {
        // This adapter's only caller is 0057C560: parent is its actual page.
        // Use the current 00AA7E00 direct-child body (its second argument is
        // unused), rather than the older loading packet's recursion hypothesis.
        auto* page = static_cast<GuiLayoutPage*>(parent);
        return owner_.frontend.find_page_child(*page, std::string(name));
    }
    ClockTimestamp sample_clock() override {
        // 0057C393..3A9 samples CURRENT AB0 slot20, including fixed-clock mode.
        // Reuse the startup clock's publication/methods; never advance another.
        const auto* clock = owner_.singletons.native_deletion_bindings().frame_clock;
        if (clock == nullptr)
            throw std::logic_error("Loading hint requires the live application clock");
        const NativeFrameClockPublicationContext publication{
            clock->actual_clock_publication_01090ab0, clock->methods};
        ClockTimestamp result;
        sample_published_native_frame_clock(publication, &result);
        return result;
    }
    std::int32_t resolve_localised_length(std::string_view key) override {
        if (owner_.loading_locale == nullptr)
            throw std::logic_error("Loading hint requires the phase-6 locale tables");
        LocaleTextResolver resolver(*owner_.loading_locale, *this);
        const auto text = resolver.resolve(std::string(key));
        return static_cast<std::int32_t>(text.size());
    }
    void set_element_text(void* element, std::string_view key, bool localise) override {
        if (!localise || element == nullptr)
            throw std::logic_error("Loading hint requires the bound localized Text widget");
        owner_.frontend.set_widget_text_source(*static_cast<GuiLayoutWidget*>(element),
            std::string(key));
        owner_.log.implemented("LoadingScreen::set_hint_source", "00abaed0");
    }

    // None of these operations occurs in BindElements/ResetHints. Refuse a
    // future worker/configuration call instead of supplying invented results.
    void update_element(void*, float) override { outside_init_reset(); }
    void set_element_image(void*, int, int, float) override { outside_init_reset(); }
    void set_element_visible(void*, bool) override { outside_init_reset(); }
    void set_element_size(void*, LoadingPlateSize) override { outside_init_reset(); }
    LoadingPlateSize element_size(void*) override { outside_init_reset(); }
    float element_caption_width(void*) override { outside_init_reset(); }
    void update_online_stats() override { outside_init_reset(); }

private:
    std::string context_string_00b692c0(const std::string&) override {
        // The current application Lua owner is not exposed to this host.
        // Plain installed loading keys need no context; do not guess for mods.
        throw std::logic_error("Contextual loading hint requires the current application Lua owner");
    }
    char16_t crt_uppercase_00c0391c(char16_t value) override {
        return static_cast<char16_t>(std::towupper(static_cast<std::wint_t>(value)));
    }
    [[noreturn]] static void outside_init_reset() {
        throw std::logic_error("Loading element adapter is limited to init/reset");
    }
    GameMenuHost::Impl& owner_;
};

class MenuLoadingScreenHost final : public LoadingScreenHost,
    public frontend_seg19_21_cc13::LoadingScreenDestructionHost {
public:
    explicit MenuLoadingScreenHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void renderer_begin_frame() override { rec("LoadingScreen::renderer_begin_frame", "00f8d394+0Ch"); }
    void renderer_end_frame() override { rec("LoadingScreen::renderer_end_frame", "00f8d394+14h"); }
    void renderer_begin_worker_mode() override { rec("LoadingScreen::renderer_begin_worker_mode", "00f8d394+1Ch"); }
    bool game_object_present() override { return true; }
    void session_suspend() override { rec("LoadingScreen::session_suspend", "0076c190"); }
    void session_resume() override { rec("LoadingScreen::session_resume", "0076c230"); }
    void gui_set_enabled(bool enabled) override {
        if constexpr (kFrontEndPointerCc13Bound) {
            owner_.frontend.gui_manager_set_enabled_00aa0e00(enabled);
            owner_.log.implemented("LoadingScreen::gui_set_enabled", "00aa0e00");
        } else {
            static_cast<void>(enabled);
            rec("LoadingScreen::gui_set_enabled", "00aa0e00");
        }
    }
    void gui_update(float seconds, int mode) override {
        static_cast<void>(seconds);
        static_cast<void>(mode);
        rec("LoadingScreen::gui_update", "00aa4f80");
    }
    void gui_refresh() override {
        if (owner_.native_renderer == nullptr)
            throw std::logic_error("LoadingScreen::End requires the startup renderer");
        if (owner_.native_renderer->reset_gui_render_queue())
            owner_.log.implemented("LoadingScreen::gui_refresh", "00aa2b80");
        else
            owner_.log.unimplemented("LoadingScreen::gui_refresh_no_device", "00aa2b80");
    }
    // C251 publishes the loading screen's own page through the real registry.
    void screen_registry_commit() override {
        MenuCommitHost commit(owner_);
        commit_front_end_screen_visibility_004f83b0(owner_.loading_screen.base,
            kLoadingScreenId, commit);
        owner_.log.implemented("LoadingScreen::screen_registry_commit", "004f83b0");
    }
    void screen_pump_reset() override { rec("LoadingScreen::screen_pump_reset", "004f8ac0"); }
    void close_previous_screen() override { rec("LoadingScreen::close_previous_screen", "00e19698"); }
    void load_texture_atlas(const char* path) override {
        owner_.log.notef("loading screen atlas %s", path);
        rec("LoadingScreen::load_texture_atlas", "00af0060");
    }
    void create_screen() override {
        auto& binding = owner_.loading_screen_deletion;
        binding = {&owner_, &owner_.loading_screen.lifetime,
            [](void* context, void* hook, std::uint32_t flags) {
                auto& owner = *static_cast<GameMenuHost::Impl*>(context);
                if (hook != &owner.loading_screen.lifetime || flags != 1u)
                    throw std::logic_error("loading screen drain requires its registered member and deleting flag 1");
                MenuLoadingScreenHost host(owner);
                host.screen_destroy();
                // Host storage remains allocated with Impl; retire the SAME
                // owner instead of freeing an interior semantic projection.
                owner.loading_screen.created = false;
            }};
        auto& admitted = owner_.singletons.native_deletion_bindings().loading_screen;
        if (admitted != nullptr && admitted != &binding)
            throw std::logic_error("loading screen lifetime already has another owner");
        admitted = &binding; // before the recovered constructor registers
        auto context = lifetime_context();
        frontend_seg19_21_cc13::loading_screen_construct_0057c1c0(
            owner_.loading_screen, context);
        owner_.log.implemented("LoadingScreen::create_screen", "0057c1c0");
    }
    void screen_init() override {
        MenuLoadingElementHost elements(owner_);
        bind_loading_screen_elements_0057c560(owner_.loading_elements.elements, elements);
        owner_.log.implemented("LoadingScreen::bind_elements", "0057c560");
    }
    void screen_show() override { rec("LoadingScreen::screen_show", "00cef264+18h"); }
    void screen_stop_worker() override { rec("LoadingScreen::screen_stop_worker", "00cef264+1Ch"); }
    void screen_destroy() override {
        auto context = lifetime_context();
        frontend_seg19_21_cc13::loading_screen_destruct_0057c2e0(
            owner_.loading_screen, context, *this);
        owner_.log.implemented("LoadingScreen::screen_destroy", "0057c2e0");
    }
    void lifetime_unregister() override {
        // 0057C250 first unregisters directly; 0057C090 subsequently repeats
        // the search under the captured manager lock and clears E194B4.
        auto lifetime = owner_.singletons.sound_lifetime();
        // 0057C29C..B6 captures/pushes the member before 00415350.
        auto* const registered = owner_.loading_screen_singleton
            ? &owner_.loading_screen_singleton->lifetime : nullptr;
        auto manager = lifetime.get_manager_00415350();
        manager.unregister_object(registered);
        owner_.log.implemented("LoadingScreen::lifetime_unregister", "00bcfca0");
    }
    void remove_loading_page() override {
        auto* page = static_cast<GuiLayoutPage*>(owner_.loading_elements.elements.page);
        owner_.frontend.release_screen_page(page);
        owner_.loading_elements = {};
        owner_.loading_hints = {};
    }
    void mark_loading_screen_destroyed() override {
        owner_.shell_state.loading_screen_destroyed = true;
    }
    void destroy_loading_frontend_base() override {
        unregister_front_end_screen_004f71a0(owner_.world.screens, owner_.loading_screen.base);
        owner_.screens.erase(frontend_seg19_21_cc13::loading_screen_get_screen_id_0057c230());
    }
    void select_menu_layout(int layout, int flag) override {
        // 004c1ac0 returns the frame singleton the host's frame_layouts stand
        // for; 00518250 is the same selector the title bring-up runs.
        const bool commit = flag != 0;
        owner_.acquire_hidden = !commit;
        MenuFrameLayoutHost layouts(owner_);
        bsp::select_front_end_frame_set(owner_.frame_layouts, layouts, layout, commit);
        owner_.acquire_hidden = false;
        owner_.frontend.invalidate_bridge();
        owner_.log.implemented("LoadingScreen::select_menu_layout", "00518250");
    }
    void element_set_image(int element, int index, float alpha) override {
        static_cast<void>(element);
        static_cast<void>(index);
        static_cast<void>(alpha);
        rec("LoadingScreen::element_set_image", "element+88h");
    }
    void element_set_visible(int element, bool visible) override {
        static_cast<void>(element);
        static_cast<void>(visible);
        rec("LoadingScreen::element_set_visible", "element+34h");
    }
    void element_set_rect(int element) override {
        static_cast<void>(element);
        rec("LoadingScreen::element_set_rect", "element+58h");
    }
    void bind_loading_image(const std::string& name) override {
        static_cast<void>(name);
        rec("LoadingScreen::bind_loading_image", "00abaed0");
    }
    float element_aspect_source() override {
        rec("LoadingScreen::element_aspect_source", "element+114h");
        return 0.0f;
    }
    void prepare_extra_elements() override {
        // 0057CE83 -> 0057C990 -> 0057C360, then tail-jump 0057C4C0.
        // Capture the keys from the actual 0057CFF0 config publication.
        owner_.loading_hints.keys = owner_.loading_globals.images;
        MenuLoadingElementHost elements(owner_);
        reset_loading_hints_0057c990(owner_.loading_elements, owner_.loading_hints, elements);
        // Both projections name the singleton's same native +2C/+30 pair.
        owner_.loading_screen.progress_units = owner_.loading_elements.rotation.progress_units;
        owner_.loading_screen.progress = owner_.loading_elements.rotation.progress;
        owner_.log.implemented("LoadingScreen::reset_hints", "0057c990");
    }
    void start_render_worker(int rate) override {
        static_cast<void>(rate);
        rec("LoadingScreen::start_render_worker", "00aa4040");
    }

private:
    frontend_seg19_21_cc13::LoadingScreenLifetimeContext lifetime_context() {
        return {owner_.singletons.sound_lifetime(), owner_.loading_screen_singleton};
    }
    void rec(const char* method, const char* address) {
        owner_.log.unimplemented(method, address);  // literals: the log indexes by pointer
    }

    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 004e4000, BSP_Game_EnterFrontEndShell
// ---------------------------------------------------------------------------

// 0090C5D0's host calls (award_grant.hpp), one per native call in body order,
// over this process's actual string pool: 0041E870 and 004261A0 are the raw-
// header bodies of native_physical_file_date.cpp, 00419CC0 and 00BD1510 the
// pool getter and return on the same 01090AA8/01090AA0/01090AA4 cells that
// game_native_string_process() publishes. kWireGoal2Chain111Bound only.
static_assert(sizeof(MidwayNativeString) == 8 && offsetof(MidwayNativeString, length) == 0
        && offsetof(MidwayNativeString, data) == 4,
    "0090C5D0's stack strings are the native eight-byte header (+0 length, +4 data)");
class MidwaySaveFolderProbe final : public MidwaySaveFolderHost {
public:
    explicit MidwaySaveFolderProbe(NativeStringRawPoolContext& strings) noexcept
        : strings_(strings) {}

    int sh_get_special_folder_path_a(char (&buffer)[0x104], int csidl, int create) override {
        return SHGetSpecialFolderPathA(nullptr, buffer, csidl, create); // [00CE22F0], 0090C5F6
    }
    void native_string_from_cstring_0041e870(MidwayNativeString& s, const char* text) override {
        construct_native_string_cstring_0041e870(&s, text, strings_); // 0090C60D, 0090C626
    }
    void native_string_concat_004261a0(const MidwayNativeString& left, MidwayNativeString& out,
        const MidwayNativeString& right) override {
        concatenate_native_string_headers_004261a0(&left, &out, &right, strings_); // 0090C63F
    }
    void* pool_singleton_00419cc0() override {
        return native_string_pool_get_or_create_00419cc0(strings_.actual_published_01090aa8,
            strings_.actual_manager_publication_01090aa0);
    }
    void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size, int) override {
        // 00BD1510's third stack argument (1 at all three sites) is unused.
        return_native_string_pool_00bd1510(static_cast<NativeStringPoolStorage*>(pool), block,
            size, strings_.actual_small_returns_disabled_01090aa4);
    }
    const char* empty_text_00f899a0() override {
        // 00F899A0 is a zero BSS byte; read only for a null concatenation buffer,
        // which the nonempty suffix rules out.
        static const char empty = '\0';
        return &empty;
    }
    int create_directory_a(const char* path, void* security) override {
        return CreateDirectoryA(path, static_cast<LPSECURITY_ATTRIBUTES>(security)); // [00CE227C]
    }
    std::uint32_t get_last_error() override { return GetLastError(); } // [00CE2258]

private:
    NativeStringRawPoolContext& strings_;
};

class MenuShellHost final : public FrontEndShellServices {
public:
    MenuShellHost(GameMenuHost::Impl& owner, MenuManagerHost& managers,
        MenuPayloadHost& payloads, MenuScreenSetHost& screens)
        : owner_(owner), managers_(managers), payloads_(payloads), screens_(screens) {}

    void renderer_set_budget(std::uint32_t value) override {
        static_cast<void>(value);
        owner_.log.unimplemented("FrontEndShell::renderer_set_budget", "00f8d394+18h");
    }
    void probe_texture_memory(const char* label) override {
        static_cast<void>(label);
        owner_.log.unimplemented("FrontEndShell::probe_texture_memory", "00f8d394+70h");
    }
    void probe_sound_memory(const char* label) override {
        // Native 00a7a460 calls on_out_of_sound_memory on the live FMOD library; OFF keeps the probe stub.
        GameSoundRuntime* sound = owner_.singletons.native_deletion_bindings().sound_runtime;
        if (sound != nullptr) {
            sound->fmod().on_out_of_sound_memory(label);
            owner_.log.implemented("FrontEndShell::probe_sound_memory", "00a7a460");
        } else {
            owner_.log.unimplemented("FrontEndShell::probe_sound_memory", "00a7a460");
        }
    }
    void probe_effect_memory(const char* label) override {
        owner_.log.implemented("FrontEndShell::probe_effect_memory", "004c1650");
        owner_.singletons.probe_gameplay_effect_memory(label);
    }
    bool title_screen_present() override { return owner_.world.title != nullptr; }
    void destroy_title_screen() override {
        // 0068D7E0 -> 00684FA0 erases this manager from 00E19898. Remove
        // the newly owned base here too, before the main menu activates;
        // otherwise 00684700 would visit a destroyed title manager and
        // clear screen sets. The press-start lifetime remains with the
        // existing screen owner below; this is not whole-dtor wiring.
        auto& managers = owner_.manager_registry.managers;
        managers.erase(std::remove(managers.begin(), managers.end(),
            &owner_.title_object.base), managers.end());
        owner_.world.title = nullptr;
        owner_.title_init.title_screen = nullptr;
        owner_.log.implemented("FrontEndShell::destroy_title_screen", "004e4071");
    }
    bool front_end_manager_b8_present() override { return owner_.managers.options != nullptr; }
    bool front_end_manager_ac_present() override { return owner_.managers.main_menu != nullptr; }
    bool front_end_manager_b4_present() override { return owner_.managers.multi_menu != nullptr; }
    void open_load_block(const char* label) override {
        owner_.log.unimplemented("FrontEndShell::open_load_block", "00be0a30");
        owner_.log.notef("front-end shell load block %s", label);
    }
    void close_load_block() override {
        owner_.log.unimplemented("FrontEndShell::close_load_block", "00bdcb30");
    }
    LoadingScreenConfig& loading_globals() override { return owner_.loading_globals; }
    void begin_loading(LoadingScreenMode mode) override {
        if constexpr (kLoadingScreenCc13Bound) {
            MenuLoadingScreenHost host(owner_);
            begin_loading_screen(owner_.loading_screen, mode, owner_.loading_globals, host);
            owner_.log.implemented("FrontEndShell::begin_loading", "0057cb60");
        } else {
            static_cast<void>(mode);
            // 0057cb60 builds the 58h loading screen and starts its render worker.
            // That screen is another packet; the shell loads synchronously here.
            owner_.log.unimplemented("FrontEndShell::begin_loading", "0057cb60");
        }
    }
    void report_progress(float progress) override {
        if constexpr (kLoadingScreenCc13Bound) {
            owner_.report_loading_progress_0057bec0(progress);
            owner_.log.implemented("FrontEndShell::report_progress", "0057bec0");
        } else {
            static_cast<void>(progress);
            owner_.log.unimplemented("FrontEndShell::report_progress", "0057bec0");
        }
    }
    void end_loading() override {
        if constexpr (kLoadingScreenCc13Bound) {
            // 00E194B4 is null unless 0057cb60 built the screen.
            if (owner_.loading_screen.created) {
                owner_.log.notef("loading screen progress at end: units=%d progress=%.4f (0057bec0)",
                    static_cast<int>(owner_.loading_screen.progress_units),
                    static_cast<double>(owner_.loading_screen.progress));
            }
            MenuLoadingScreenHost host(owner_);
            end_loading_screen(owner_.loading_screen_singleton, host);
            owner_.log.implemented("FrontEndShell::end_loading", "0057c250");
        } else {
            owner_.log.unimplemented("FrontEndShell::end_loading", "0057c250");
        }
    }
    void game_on_init() override {
        // 004e3aa0's first store is game+5D4h = 3.
        owner_.state.value = kGameStateFrontEndInit;
        owner_.log.implemented("FrontEndShell::game_on_init", "004e3ac2");
        // cc15-ob4 (kSceneDatabaseCc15Bound): 0046F350 at 004E3E60, the scene
        // database at [00E18680] (docs/CC15_OBJECT_SCENE_DATABASE.md).
        // Image order: 004E3E60 runs before the tracker's 004E3EFB.
        if constexpr (kSceneDatabaseCc15Bound) scene_database_create_0046f350(owner_.log);
        if constexpr (kHintTrackerCc15Bound) {
            // 004E3EFB: the first 004E1CA0 builds the tracker, then 006921B0
            // reloads the hint tables on it (HintSystemMessages.lua into the
            // definition map +30h).
            owner_.hint_tracker.get_004e1ca0().reload_hint_tables_006921b0(owner_.vfs);
            if constexpr (kHintTriggerPassesCc15Bound) {
                owner_.hint_tracker.get_004e1ca0().bind_registry(owner_.hint_registry.get());
            }
        }
    }
    void poll_platform_session_events() override {
        owner_.log.unimplemented("FrontEndShell::poll_platform_session_events", "004db290");
    }
    std::int32_t game_state() override { return owner_.state.value; }
    void create_manager_b8() override {
        // 004E4171 constructs, then calls virtual +4h (006898C0). Its
        // own publication is what the frame/request/teardown paths read.
        auto* manager = managers_.create_options_manager();
        if (manager == nullptr) return;
        init_options_menu_006898c0(*manager, owner_.managers,
            owner_.interface_lock, managers_, payloads_);
        owner_.log.implemented("FrontEndShell::create_manager_b8", "006898c0");
    }
    void create_manager_ac() override {
        MainMenuManager* manager = managers_.create_main_menu_manager();
        if (manager == nullptr) return;
        init_main_menu_00686380(*manager, owner_.managers, owner_.interface_lock, managers_,
            payloads_, screens_, owner_.manager_registry);
        owner_.managers.main_menu = manager;
        // 00684E10 already inserted this base before 00686380 ran.
        owner_.log.implemented("FrontEndShell::create_manager_ac", "00686380");
    }
    void create_manager_b4() override {
        owner_.log.unimplemented("FrontEndShell::create_manager_b4", "006887e0");
    }
    void lua_collect_garbage() override {
        owner_.log.unimplemented("FrontEndShell::lua_collect_garbage", "006b8ad0");
    }
    int manager_ac_mode() override {
        return owner_.managers.main_menu != nullptr
            ? owner_.managers.main_menu->base.applied.interface_id : 0;
    }
    void reset_manager_ac_mode() override {
        // 004e4259. Step 9 of the main-menu path owns this site, so the shell
        // records it and leaves the push to the path; see docs/GAME_EXECUTABLE.md.
        owner_.log.implemented("FrontEndShell::reset_manager_ac_mode", "004e4259");
    }
    void manager_ac_enter() override {
        owner_.log.implemented("FrontEndShell::manager_ac_enter", "004e4269");
    }
    void manager_ac_start_sub() override {
        owner_.log.implemented("FrontEndShell::manager_ac_start_sub", "004e4274");
    }
    bool post_state_hook_wanted() override { return false; }
    void post_state_hook() override {
        owner_.log.unimplemented("FrontEndShell::post_state_hook", "004bfc70");
    }
    bool award_gate_open() override {
        // 0090C5D0 at 004E42F2: midway_save_folder_preexisted_0090c5d0 over the
        // actual string pool and the Win32 shell/file calls; OFF keeps false.
        MidwaySaveFolderProbe probe(game_native_string_process().raw_context());
        const bool preexisted = midway_save_folder_preexisted_0090c5d0(probe);
        owner_.log.implemented("FrontEndShell::award_gate_open", "0090c5d0");
        owner_.log.notef("front-end shell: 0090C5D0 Midway save folder preexisted=%d",
            preexisted ? 1 : 0);
        return preexisted;
    }
    int award_id(const char* key) override {
        static_cast<void>(key);
        owner_.log.unimplemented("FrontEndShell::award_id", "006b8da0");
        return 0;
    }
    bool award_system_ready() override { return false; }
    bool award_session_ready() override { return false; }
    void grant_award(int id) override {
        static_cast<void>(id);
        owner_.log.unimplemented("FrontEndShell::grant_award", "00a410a0");
    }
    void record_award(const char* key, int value) override {
        static_cast<void>(key);
        static_cast<void>(value);
        owner_.log.unimplemented("FrontEndShell::record_award", "007fbe20");
    }
    void send_network_quit() override {
        owner_.log.unimplemented("FrontEndShell::send_network_quit", "0076fad0");
    }

private:
    GameMenuHost::Impl& owner_;
    MenuManagerHost& managers_;
    MenuPayloadHost& payloads_;
    MenuScreenSetHost& screens_;
};

// ---------------------------------------------------------------------------
// The main-menu path itself
// ---------------------------------------------------------------------------

class MenuPathHost final : public MainMenuPathHost {
public:
    MenuPathHost(GameMenuHost::Impl& owner, MenuShellHost& shell, MenuPayloadHost& payloads,
        MenuScreenSetHost& screen_sets, MenuSetBindings& bindings, MenuScreenHost& screens,
        MenuCommitHost& commit, MenuTitleMusicHost& music)
        : owner_(owner), shell_(shell), payloads_(payloads), screen_sets_(screen_sets),
          bindings_(bindings), screens_(screens), commit_(commit), music_(music) {}

    bool input_action_pressed(int action) override {
        if (owner_.press_start_handoff_requested) return true;
        return action == kPressStartInputAction
            && action_pressed_this_frame_004c43c0(owner_.press_action);
    }

    void reset_storage_availability() override {
        owner_.log.unimplemented("TitleSkip::reset_storage_availability", "00bd3450");
    }
    void request_game_state(std::int32_t state) override {
        enqueue_state_request_004d3ed0(owner_.requests, static_cast<std::uint32_t>(state));
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        owner_.log.implemented("TitleSkip::request_game_state", "004d7920");
        owner_.log.notef("state request %d enqueued by the press-start skip", state);
    }
    void release_state_request_hold() override {
        owner_.state.requests_held = false;
        owner_.log.implemented("TitleSkip::release_state_request_hold", "0068d8c6");
    }

    std::int32_t drain_state_request() override {
        if (owner_.requests.count == 0) return 0;
        const std::uint32_t request = front_state_request(owner_.requests);
        pop_front_state_request(owner_.requests);
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        apply_drained_game_state(owner_.state, static_cast<int>(request));
        owner_.log.implemented("MainMenuPath::drain_state_request", "004e4430");
        return static_cast<std::int32_t>(request);
    }
    void enter_front_end_shell() override {
        const FrontEndShellOutcome outcome = bsp::enter_front_end_shell(owner_.shell_state,
            shell_);
        owner_.summary.shell_entered = outcome == FrontEndShellOutcome::ShellReady;
        owner_.log.implemented("MainMenuPath::enter_front_end_shell", "004e4000");
        owner_.log.notef("front-end shell entry outcome=%s",
            outcome == FrontEndShellOutcome::ShellReady ? "ShellReady"
                                                        : "AbortedByPlatformEvent");
    }

    int main_menu_manager_mode() override {
        return owner_.managers.main_menu != nullptr
            ? owner_.managers.main_menu->base.applied.interface_id : 0;
    }
    void push_interface_request(int interface_id, void* payload) override {
        if (owner_.managers.main_menu == nullptr) return;
        push_interface_request_004cc460(owner_.managers.main_menu->base,
            owner_.interface_lock, interface_id, payload, payloads_);
        owner_.log.implemented("MainMenuPath::push_interface_request", "004cc460");
    }
    void activate_main_menu_manager() override {
        if (owner_.managers.main_menu == nullptr) return;
        activate_front_end_manager_00684700(owner_.manager_registry,
            owner_.managers.main_menu->base, owner_.managers.in_mission,
            owner_.interface_lock, payloads_, screen_sets_);
        owner_.summary.main_menu_manager_active = owner_.managers.main_menu->base.active;
        owner_.log.implemented("MainMenuPath::activate_main_menu_manager", "00684700");
    }
    void start_main_menu_screen_object() override {
        start_title_music_005884a0(owner_.title_music, kMainMenuTitleMusic, music_);
        owner_.log.implemented("MainMenuPath::start_main_menu_screen_object", "005884a0");
    }
    void set_game_state(std::int32_t state) override {
        owner_.state.value = state;
        owner_.log.implemented("MainMenuPath::set_game_state", "004e4279");
        owner_.log.notef("game state %d (front-end shell ready)", state);
    }

    bool service_pending_interface_requests() override {
        const bool serviced = service_pending_interface_requests_006840f0(owner_.managers,
            owner_.interface_lock, payloads_, screen_sets_);
        owner_.log.implemented("MainMenuPath::service_pending_interface_requests", "006840f0");
        if (serviced) return true;
        // Run-time correction. On a cold boot 00686380 has already run 00684600
        // with INTF_MAINMENU at 006868AD, so manager+4h equals manager+20h
        // before the shell's push at 004E4259 and 006840F0 has nothing to
        // service. docs/MAIN_MENU_PATH.md's step 13 assumes a mismatch that this
        // path does not produce, so the channel is reported as already applied
        // and the path publishes the screen set itself.
        MainMenuManager* manager = owner_.managers.main_menu;
        return manager != nullptr && manager->base.active
            && manager->base.applied.interface_id == manager->base.pending.interface_id
            && manager->base.applied.interface_id == kMainMenuPathInterfaceId;
    }
    bool apply_interface_request(int interface_id, void* payload) override {
        if (owner_.managers.main_menu == nullptr) return false;
        const bool applied = apply_interface_request_00684600(owner_.managers.main_menu->base,
            owner_.interface_lock, interface_id, payload, payloads_);
        owner_.log.implemented("MainMenuPath::apply_interface_request", "00684600");
        return applied;
    }
    int map_interface_to_screen(int interface_id) override {
        // 00685848, the identity arm of the main menu's own +10h override.
        const int screen_id = main_menu_screen_id_00685820(interface_id);
        owner_.log.implemented("MainMenuPath::map_interface_to_screen", "00685820");
        return screen_id;
    }

    void publish_screen_set_level4(const int* ids, std::size_t count) override {
        set_front_end_screen_set_004f8710(owner_.screen_sets, owner_.world.screens,
            bindings_, ids, count);
        owner_.log.implemented("MainMenuPath::publish_screen_set_level4", "004f8710");
        owner_.summary.published_screen_id = count != 0 && ids != nullptr ? ids[0] : 0;
        owner_.log.notef("level-4 screen set published: %zu id(s), first=%d", count,
            owner_.summary.published_screen_id);
    }
    void publish_input_context_set_level4(const int* ids, std::size_t count) override {
        set_game_input_context_set_004d8c00(owner_.input_contexts, ids, count);
        owner_.log.implemented("MainMenuPath::publish_input_context_set_level4", "004d8c00");
    }

    FrontEndScreenPhase pump_front_end_screens(float raw_delta) override {
        // 004c4165 inside BSP_Game_UpdateInterfaceOnly 004c40f0, which OnMove
        // calls unconditionally at 004e53b6 once the state leaves {1, 2, 4}.
        const FrontEndScreen* screen = owner_.world.screens.slots[
            owner_.path.published_screen_id >= 0
                && owner_.path.published_screen_id < kFrontEndScreenSlotCount
                    ? owner_.path.published_screen_id : 0];
        const FrontEndScreenPhase phase = screen != nullptr
            ? front_end_screen_phase(*screen) : FrontEndScreenPhase::Hidden;
        owner_.pump(raw_delta);
        return phase;
    }
    void commit_screen_visibility(int screen_id, bool visible) override {
        // 004f88e8. The pump above already ran 004f83b0 for this screen in its
        // enter pass; re-applying the same byte is idempotent and keeps the
        // native call site on the record.
        static_cast<void>(visible);
        screens_.screen_commit(screen_id);
        static_cast<void>(commit_);
    }
    void enter_screen(int screen_id) override {
        // 004f88f4. The pump's enter pass already called the same virtual.
        static_cast<void>(screen_id);
        owner_.log.notef("main-menu screen %d entered by the pump at 004f88f4", screen_id);
    }
    void update_screen(int screen_id, float raw_delta) override {
        static_cast<void>(screen_id);
        static_cast<void>(raw_delta);
    }

private:
    GameMenuHost::Impl& owner_;
    MenuShellHost& shell_;
    MenuPayloadHost& payloads_;
    MenuScreenSetHost& screen_sets_;
    MenuSetBindings& bindings_;
    MenuScreenHost& screens_;
    MenuCommitHost& commit_;
    MenuTitleMusicHost& music_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameMenuHost::Impl
// ---------------------------------------------------------------------------

GameMenuHost::Impl::Impl(GameHostLog& log_in, GameFrontendHost& frontend_in,
    GameStateSlot& state_in, GameSingletonHost& singletons_in, long press_start_frame_in,
    GameVfsHost& vfs_in)
    : log(log_in), frontend(frontend_in), state(state_in),
      press_start_frame(press_start_frame_in),
      singletons(singletons_in), vfs(vfs_in) {
    summary.press_start_frame = press_start_frame_in;
    world.title = nullptr;
    // The press-start action record is enabled so 004c43c0's own gate at
    // 00a92c88 does not skip it; every other action has no record at all.
    press_action.enabled = true;
    press_action.registered = true;
}

GameMenuHost::Impl::MenuScreen* GameMenuHost::Impl::screen_at(int slot) {
    const auto found = screens.find(slot);
    return found != screens.end() ? &found->second : nullptr;
}

GameMenuHost::Impl::MenuScreen& GameMenuHost::Impl::add_screen(int slot, std::string name,
    FrontEndScreen* external, std::uint32_t enter_virtual, std::uint32_t exit_virtual,
    std::uint32_t update_virtual, bool is_press_start) {
    MenuScreen& screen = screens[slot];
    screen.slot = slot;
    screen.name = std::move(name);
    // 004f7180 calls construct_front_end_screen_004f7180; OFF keeps default-owned flags.
    // External leaf objects already run their constructor (0067c840 for
    // press-start); only construct the registry-owned screen base here.
    screen.flags = external != nullptr ? external
        : &construct_front_end_screen_004f7180(screen.owned_flags);
    if (external == nullptr) {
        log.implemented("FrontEndScreen::construct", "004f7180");
    }
    screen.enter_virtual = enter_virtual;
    screen.exit_virtual = exit_virtual;
    screen.update_virtual = update_virtual;
    screen.press_start = is_press_start;
    return screen;
}

void GameMenuHost::Impl::attach_page(MenuScreen& screen, const std::string& layout_name) {
    GuiLayoutPage* page = frontend.load_screen_page(layout_name);
    log.implemented("FrontEndScreen::load_layout", "00aa5840");
    if (page == nullptr) {
        log.notef("screen %d layout %s did not load", screen.slot, layout_name.c_str());
        return;
    }
    screen.pages.push_back(page);
    // 004f7180 leaves a screen neither wanted nor active, so the byte 004f83b0
    // would publish for a freshly bound layout is that screen's current +5h.
    frontend.commit_page_visibility(*page, screen.flags != nullptr && screen.flags->active);
}

namespace {
class MenuCommandWidgetReset final : public MainMenuCommandResetHost {
public:
    MenuCommandWidgetReset(MainMenuCommandOwner& owner, GameFrontendHost& frontend)
        : owner_(owner), frontend_(frontend) {}
    void set_widget_visible(std::uint16_t offset, bool visible) override {
        auto* const widget = owner_.widget_at(offset);
        if (widget == nullptr)
            throw std::logic_error("command reset requires its loaded page widget");
        frontend_.set_widget_visible(*widget, visible);
    }
private:
    MainMenuCommandOwner& owner_;
    GameFrontendHost& frontend_;
};
}

void GameMenuHost::Impl::set_command_help_variant(std::uint8_t variant) {
    if (!command_widgets)
        throw std::logic_error("main-menu help variant requires its command widget owner");
    MenuCommandWidgetReset widgets(*command_widgets, frontend);
    set_main_menu_help_variant_00549620(command_widgets->bar, variant, widgets);
    log.implemented("MainMenuCommandBar::set_help_variant", "00549620");
}

void GameMenuHost::Impl::initialize_command_widgets() {
    if (command_widgets) return;
    // 004E3E27 constructs 00E1930C; OnInit publishes it and invokes its +10h
    // before title entry. Admit its page/widget ownership here, before this
    // host's first title entry. This is the page/reset subset of 0054C050,
    // not a claim that its text-code/listener setup is implemented.
    constexpr int slot = 0x58; // 0054A960, the command owner's registry ID
    if (screen_at(slot) != nullptr)
        throw std::logic_error("command widgets require their own registry slot");
    auto owner = std::make_unique<MainMenuCommandOwner>();
    auto& screen = add_screen(slot, "main_menu_commands", nullptr,
        0x00549fc0u, 0x00549570u, 0x00549580u, false);
    register_front_end_screen_004f71d0(world.screens, *screen.flags, slot);
    ++summary.screens_registered;
    attach_page(screen, "FE_helpline");
    if (screen.pages.size() != 1)
        throw std::logic_error("command widgets require FE_helpline");
    owner->help_page_24 = screen.pages.back();
    attach_page(screen, "_Buttonhelp");
    if (screen.pages.size() != 2)
        throw std::logic_error("command widgets require _Buttonhelp");
    owner->button_page_28 = screen.pages.back();
    auto* const frame = frontend.find_page_child(*owner->button_page_28, "button_FrameBox");
    if (frame == nullptr)
        throw std::logic_error("command widgets require button_FrameBox");
    frontend.set_widget_visible(*frame, false); // 0054C050, before widget binding
    for (std::size_t i = 0; i != main_menu_command_widget_bindings().size(); ++i) {
        const auto& binding = main_menu_command_widget_bindings()[i];
        auto* const page = binding.page_offset == 0x24
            ? owner->help_page_24 : owner->button_page_28;
        owner->widgets[i] = frontend.find_page_child(*page, std::string(binding.name));
        if (owner->widgets[i] == nullptr)
            throw std::logic_error("command widgets require their authored binding");
    }
    // 0054C050's final empty 0054B530 call starts with these two resets.
    // Supply the actual retained widgets, not temporary visibility facts.
    MenuCommandWidgetReset reset(*owner, frontend);
    hide_main_menu_alternate_widgets_005495e0(reset);
    hide_main_menu_glyph_widgets_005495a0(reset);
    command_widgets = std::move(owner);
    log.implemented("MainMenuCommandBar::hide_alternate_widgets", "005495e0");
    log.implemented("MainMenuCommandBar::hide_glyph_widgets", "005495a0");
}

void GameMenuHost::Impl::pump(float raw_delta) {
    MenuCommitHost commit(*this);
    MenuPressStartHost press(*this);
    MenuScreenHost screens_host(*this, press, commit);
    run_front_end_screen_pump(world, screens_host, raw_delta);
    pumped_this_frame = true;
    ++summary.pump_frames;
    log.implemented("FrontEndScreens::pump", "004f8830");
}

void GameMenuHost::Impl::advance_path(float raw_delta) {
    MenuCommitHost commit(*this);
    MenuPressStartHost press(*this);
    MenuScreenHost screens_host(*this, press, commit);
    MenuSetBindings bindings(*this);
    MenuScreenSetHost screen_set_host(*this, bindings);
    MenuPayloadHost payloads(*this);
    MenuManagerHost managers_host(*this);
    MenuShellHost shell(*this, managers_host, payloads, screen_set_host);
    MenuTitleMusicHost music(*this);
    MenuPathHost path_host(*this, shell, payloads, screen_set_host, bindings, screens_host,
        commit, music);

    // The native takes the whole run from the press to state 5 inside one
    // OnMove call and only reaches the pump on the following frame, so the loop
    // stops as soon as it needs a pump this frame has already spent.
    for (int guard = 0; guard < 24; ++guard) {
        const MainMenuPathStep next = advance_main_menu_path(path_step, path, path_host,
            raw_delta);
        if (next == path_step) break;
        if constexpr (kProfileOwnerCc15Bound && kProfileLoadCc15Bound) {
            if (path_step == MainMenuPathStep::PressStartPoll
                && next == MainMenuPathStep::TitleSkip) {
                load_signed_in_profile_0067cc60();
            }
        }
        log.notef("main-menu path %s -> %s", path_step_text(path_step).c_str(),
            path_step_text(next).c_str());
        path_step = next;
        summary.path_step = path_step_text(path_step);
        if (path_step == MainMenuPathStep::ScreenVisible) break;
        if (path_step == MainMenuPathStep::EnterScreen && pumped_this_frame) break;
    }
    press_start_handoff_requested = false;
}

// The press the host takes to the title skip stands in for the completed
// sign-in, so the profile half of 0067CC60 runs here, ahead of the skip, as
// natively the read's continuation 0067CA40 precedes it. The save id is the
// "%llx" of 00A3E5D0, the online XUID, which is 0 for a local profile: "0".
// Read-only throughout; see docs/CC15_PROFILE_LOAD.md.
void GameMenuHost::Impl::load_signed_in_profile_0067cc60() {
    if (profile_scripts == nullptr) return;
    try {
        static_cast<void>(load_signed_in_profile_read_only(profile, log, read_only_save_root(), "0",
            profile_scripts->files().owner_environment(profile_scripts->runtime(),
                profile_scripts->globals()), &profile_settings_sinks));
        log.implemented("PressStartScreen::apply_sign_in profile load", "0067cc60");
    } catch (const std::exception& error) {
        log.notef("profile load failed: %s", error.what());
    }
}

// ---------------------------------------------------------------------------
// GameMenuHost
// ---------------------------------------------------------------------------

namespace {

// cc15-hint2: the registry flag bytes (+4h wanted, +5h applied), the input
// action query 00535EE0 and the locale table 00A9EC70 the hint tracker reads.
class MenuHintRegistry final : public HintScreenRegistry {
public:
    MenuHintRegistry(GameMenuHost::Impl& owner, GameMenuHost& menu, LocaleTables& locale)
        : owner_(owner), menu_(menu), locale_(locale) {}

    bool screen_applied_05(int slot) override { return menu_.in_game_screen_applied(slot); }
    bool screen_wanted_04(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        return screen != nullptr && screen->flags != nullptr && screen->flags->wanted;
    }
    bool input_action_pressed_00535ee0(int action) override {
        return menu_.input_action_pressed(action);
    }
    const std::u16string* locale_text(const std::string& key) override {
        return locale_.find_00a9ec70(key);
    }

private:
    GameMenuHost::Impl& owner_;
    GameMenuHost& menu_;
    LocaleTables& locale_;
};

} // namespace

GameMenuHost::GameMenuHost(GameHostLog& log, GameFrontendHost& frontend, GameStateSlot& state,
    GameSingletonHost& singletons, long press_start_frame, GameVfsHost& vfs, GameScriptHost& scripts, LocaleTables& locale,
    std::string menu_select, long mission_frames, GameFrameProfiler* profiler,
    std::string language, long mission_complete_frame, long order_frame,
    float order_throttle, float order_rudder, float mission_frame_seconds,
    std::string trajectory_csv, std::string order_command, std::string order_command_target,
    float order_speed, bool order_speed_set)
    : impl_(std::make_unique<Impl>(log, frontend, state, singletons, press_start_frame, vfs)) {
    impl_->loading_locale = &locale;
    // Milestone 2h: the in-mission HUD registers into the same registry this
    // object owns, so the HUD host is built here and handed to the mission.
    impl_->hud = std::make_unique<GameHudHost>(log, *this);
    impl_->profile_scripts = &scripts;
    impl_->bind_menu_locale_tables(locale);
    if constexpr (kHintTriggerPassesCc15Bound) {
        impl_->hint_registry = std::make_unique<MenuHintRegistry>(*impl_, *this, locale);
    }
    if constexpr (kProfileOwnerCc15Bound) {
        // The profile constructor 007FEE20 runs 007FDB20 at 007FEF9F while
        // 004DDB90 builds the game, so +64h exists before any title reset.
        impl_->profile.reset_to_defaults_007fdb20();
        log.implemented("Game::construct_profile_reset", "007fef9f");
    }
    if (!menu_select.empty()) {
        impl_->mission = std::make_unique<GameMissionHost>(log, vfs, scripts, frontend,
            locale, std::move(menu_select), mission_frames, profiler, std::move(language),
            mission_complete_frame, impl_->hud.get(), order_frame, order_throttle,
            order_rudder, mission_frame_seconds, std::move(trajectory_csv),
            std::move(order_command), std::move(order_command_target), order_speed,
            order_speed_set);
        impl_->mission->bind_observer_runtime(singletons.observers());
        impl_->mission->bind_profile(impl_->profile, impl_->hint_tracker);
    }
}

GameMenuHost::~GameMenuHost() {
    // Retire a screen left by an interrupted load before its borrowed
    // binding and GUI owner disappear. A completed canonical drain already
    // cleared this publication, so it never recreates the manager here.
    if (impl_->loading_screen_singleton != nullptr) {
        MenuLoadingScreenHost loading(*impl_);
        loading.screen_destroy();
    }
    auto& binding = impl_->singletons.native_deletion_bindings().loading_screen;
    if (binding == &impl_->loading_screen_deletion) binding = nullptr;
}

void GameMenuHost::bind_input_context_manager(GameInputContextManagerBinding& binding) {
    binding.prepare();
    impl_->input_contexts.manager = &binding;
}

GameInputContextManagerBinding& GameMenuHost::input_context_manager() {
    if (impl_->input_contexts.manager == nullptr)
        throw std::logic_error("menu input context manager is not bound");
    return *impl_->input_contexts.manager;
}

void GameMenuHost::bind_native_renderer(GameNativeRendererApplication& renderer) {
    impl_->native_renderer = &renderer;
}

void GameMenuHost::bind_input_configuration(GameNativeInputConfigurationApplication* configuration) {
    impl_->input_configuration = configuration;
    impl_->publish_input_device_flags_sink();
}

// Goal 4 chain 302: the profile's settings apply (008D5B50) reaches 00699B80 on
// the same game+3Ch at 008D5BB6. Published into the sinks whichever binder runs
// last, so the application's later bind_profile_settings_sinks keeps it.
void GameMenuHost::Impl::publish_input_device_flags_sink() {
    if (input_configuration == nullptr) {
        profile_settings_sinks.set_input_device_flags_00699b80 = nullptr;
        return;
    }
    GameNativeInputConfigurationApplication* const configuration = input_configuration;
    profile_settings_sinks.set_input_device_flags_00699b80 =
        [configuration](bool swap_sticks, bool invert_camera_y, bool invert_plane_y,
            bool swap_map_sticks) {
            configuration->set_device_flags_004b46e0(swap_sticks, invert_camera_y, invert_plane_y,
                swap_map_sticks);
            return true;
        };
}

GameMissionHost* GameMenuHost::mission() const noexcept { return impl_->mission.get(); }
GameScriptHost& GameMenuHost::script_services() const {
    if (impl_->profile_scripts == nullptr)
        throw std::logic_error("HUD table registration has no application script services");
    return *impl_->profile_scripts;
}
void GameMenuHost::bind_profile_settings_sinks(const GameProfileSettingsSinks& sinks) {
    impl_->profile_settings_sinks = sinks;
    impl_->publish_input_device_flags_sink();
}

GameHudHost* GameMenuHost::hud() const noexcept { return impl_->hud.get(); }

void GameMenuHost::select_front_end_frame_set_00518250(int set, bool commit) {
    Impl& host = *impl_;
    const int previous = host.frame_layouts.active_set;
    // A set the mission load asks for is not a front-end page: nothing in this
    // process is a pause screen, so its layouts are published hidden.
    host.acquire_hidden = !commit;
    MenuFrameLayoutHost layouts(host);
    bsp::select_front_end_frame_set(host.frame_layouts, layouts, set, commit);
    host.acquire_hidden = false;
    host.log.implemented("MissionLoad::select_front_end_frame_set", "00518250");
    if (commit) {
        host.log.notef("front-end frame set %d selected with commit=1: 00518250 released "
            "every other set's backdrop, panel and title layouts and made %d the active set "
            "(it was %d), so the layouts the title bring-up acquired are gone", set,
            host.frame_layouts.active_set, previous);
    } else {
        host.log.notef("front-end frame set %d requested with commit=0: the active set stays "
            "%d and nothing is released, because 00518250 releases the other sets and records "
            "the new one only on a committing call (00518272, 0051864a)", set,
            host.frame_layouts.active_set);
    }
    host.frontend.invalidate_bridge();
}

// ---------------------------------------------------------------------------
// Milestone 2h: the four registry services the in-mission HUD needs
// ---------------------------------------------------------------------------

bool GameMenuHost::register_in_game_screen(int slot, const std::string& name,
    std::uint32_t register_virtual, std::uint32_t layout_virtual,
    FrontEndScreen* constructed_flags) {
    Impl& host = *impl_;
    if (slot < 0 || slot >= kFrontEndScreenSlotCount) return false;
    if (host.screen_at(slot) != nullptr) return false;
    // 004f7180 leaves both flag bytes clear, so the screen is registered
    // neither wanted nor active and only the level-set recompute can ask for it.
    Impl::MenuScreen& screen = [&]() -> Impl::MenuScreen& {
        // A reconstructed leaf already constructed this base. Keep the
        // registry on that same owner's flags; add_screen does not run
        // 004F7180 again for an external base.
        return host.add_screen(slot, name, constructed_flags, 0, 0, layout_virtual,
            false);
    }();
    screen.enter_virtual = register_virtual;
    if (register_virtual == 0x0054ED00u) {
        // Goal 5 chain 646: slot 2Ch's register virtual (kHudScreenLayouts) is
        // 0054ED00, `JMP 004F71D0` (bsp.py ghidra disasm 0054ed00): the base
        // store is the whole body.
        bsp::oct18_tail_a::frontend_register_thunk_0054ed00(host.world.screens, *screen.flags,
            slot);
        host.log.implemented("FrontEndScreen::register_thunk", "0054ed00");
    } else {
        register_front_end_screen_004f71d0(host.world.screens, *screen.flags, slot);
    }
    // cc15-hint2: 34h's register 0054D860 runs the base, builds its widgets,
    // and stores +4h = 1 last (0054D862), so the hints screen is wanted from
    // registration on and pumped every frame.
    if constexpr (kHintTriggerPassesCc15Bound) {
        if (slot == 0x34) {
            host.hint_tracker.get_004e1ca0().hints_screen_34().register_0054d860();
            screen.flags->wanted = true;
        }
    }
    ++host.summary.screens_registered;
    return true;
}

bool GameMenuHost::attach_in_game_page(int slot, const std::string& page_name) {
    Impl& host = *impl_;
    Impl::MenuScreen* screen = host.screen_at(slot);
    if (screen == nullptr) return false;
    const std::size_t before = screen->pages.size();
    host.attach_page(*screen, page_name);
    return screen->pages.size() != before;
}

bool GameMenuHost::in_game_page_has_child(int slot, const std::string& widget_name) {
    Impl& host = *impl_;
    Impl::MenuScreen* screen = host.screen_at(slot);
    if (screen == nullptr) return false;
    for (GuiLayoutPage* page : screen->pages) {
        if (page == nullptr) continue;
        if (host.frontend.find_page_child(*page, widget_name) != nullptr) {
            host.log.implemented("InGameInterface::screen_find_child", "00aa7e00");
            return true;
        }
    }
    return false;
}

bool GameMenuHost::in_game_screen_applied(int slot) {
    Impl::MenuScreen* screen = impl_->screen_at(slot);
    return screen != nullptr && screen->flags != nullptr && screen->flags->active;
}

bool GameMenuHost::in_game_screen_registered(int slot) {
    return impl_->screen_at(slot) != nullptr;
}

bool GameMenuHost::in_game_screen_wanted(int slot) {
    Impl::MenuScreen* screen = impl_->screen_at(slot);
    return screen != nullptr && screen->flags != nullptr && screen->flags->wanted;
}

std::size_t GameMenuHost::screen_set_level_size(int level) const {
    if (level < kFrontEndScreenSetLowestLevel || level > kFrontEndScreenSetHighestLevel) return 0;
    return impl_->screen_sets.levels[level - 1].size();
}

void GameMenuHost::clear_level3_screen_and_context_sets() {
    // 004F8670 is 004f8530's shape on level 3 (vector 00E18D18); 004D8B70's
    // tail is `PUSH 3 ; CALL 004d6410`, 004d8a50's setter on level 3. Both
    // with the empty list 0068AB80 passes.
    Impl& host = *impl_;
    MenuSetBindings bindings(host);
    static const int kNone[] = {0};
    set_front_end_screen_set_level(host.screen_sets, host.world.screens, bindings, 3, kNone, 1);
    publish_level3_input_contexts(kNone, 1);
}

void GameMenuHost::note_screen_2eh_ran() { impl_->screen_2eh_ran = true; }

GuiLayoutPage* GameMenuHost::in_game_page(int slot, const std::string& page_name) {
    Impl& host = *impl_;
    Impl::MenuScreen* screen = host.screen_at(slot);
    if (screen == nullptr) return nullptr;
    for (GuiLayoutPage* page : screen->pages) {
        if (page != nullptr && page->name == page_name) return page;
    }
    return nullptr;
}

GameFrontendHost& GameMenuHost::frontend() const noexcept { return impl_->frontend; }

void GameMenuHost::publish_level1_screen_set_004f8530(const int* ids, std::size_t count) {
    Impl& host = *impl_;
    MenuSetBindings bindings(host);
    set_front_end_screen_set_level(host.screen_sets, host.world.screens, bindings,
        kFrontEndScreenSetLowestLevel, ids, count);
}

void GameMenuHost::publish_level1_input_contexts_004d8a50(const int* ids,
    std::size_t count) {
    Impl& host = *impl_;
    // 004d8a50's tail is `PUSH 1 ; CALL 004d6410`, the same level setter
    // 004d8c00 reaches with 4: the list is stored at its level and 004c4300 is
    // run for that level.
    std::vector<int> pending;
    for (std::size_t i = 0; i < count && ids != nullptr; ++i) {
        if (ids[i] == 0) break;
        pending.push_back(ids[i]);
    }
    host.input_contexts.levels[kFrontEndScreenSetLowestLevel - 1] = pending;
    apply_input_context_levels_004c4300(host.input_contexts,
        kFrontEndScreenSetLowestLevel);
}

void GameMenuHost::publish_level2_screen_set_004f85d0(const int* ids, std::size_t count) {
    // Packet cc13-releasegun: 004F85D0 is 004f8530's shape on level 2 (its
    // global, then 004F7620 at 004F8643).
    Impl& host = *impl_;
    MenuSetBindings bindings(host);
    set_front_end_screen_set_level(host.screen_sets, host.world.screens, bindings, 2, ids,
        count);
}

void GameMenuHost::publish_level2_input_contexts_004d8ae0(const int* ids,
    std::size_t count) {
    publish_native_input_context_set(impl_->input_contexts, 2, ids, count);
}

void GameMenuHost::publish_level3_input_contexts(const int* ids, std::size_t count) {
    publish_native_input_context_set(impl_->input_contexts, 3, ids, count);
}

void GameMenuHost::pump_interface_only_004c40f0(float raw_delta) {
    impl_->pump(raw_delta);
}

void GameMenuHost::destroy_main_menu_manager_00686c90() {
    Impl& host = *impl_;
    host.log.unimplemented("MainMenuManager::deleting_destructor", "00687300");
    host.log.implemented("MainMenuManager::destroy", "00686c90");
    // 00686cce..00686d64 collects the seven screens at +58h and +5Ch..+70h, then
    // 00686d80..00686dd2 runs, for each one that is active, its exit virtual
    // +1Ch, clears +4h and +5h, and commits through 004f83b0 at 00686db9. That
    // commit is what publishes false to every layout the screen holds.
    MenuCommitHost commit(host);
    std::size_t exited = 0;
    for (const MainMenuScreenClass& entry : kMainMenuScreens) {
        Impl::MenuScreen* screen = host.screen_at(entry.screen_id);
        if (screen == nullptr || screen->flags == nullptr) continue;
        // 004b6e50 calls close_front_end_screen; OFF keeps the inlined 00686DA0..00686DB9 close.
        {
            MenuPressStartHost press(host);
            MenuScreenHost screens(host, press, commit);
            MainMenuClosingScreenHost closing(host, commit, screens, exited);
            close_front_end_screen(*screen->flags, entry.screen_id, closing);
            host.log.implemented("MainMenuManager::close_screen", "004b6e50");
        }
        // 00686e14, the screen's own vtable +0Ch with the deleting flag. The
        // seven classes are not reconstructed; unregistering the executable's
        // record is what a destroyed screen leaves behind in the registry.
        host.log.unimplemented("MainMenuScreen::deleting_destructor", "00686e14");
        unregister_front_end_screen_004f71a0(host.world.screens, *screen->flags);
        release_menu_leaf(*screen);
    }
    host.log.notef("main-menu manager destroyed: %zu of %zu screens exited and committed, "
        "all seven unregistered", exited, kMainMenuScreens.size());
    MenuSetBindings bindings(host);
    clear_front_end_manager_screen_set(host.screen_sets, host.world.screens, bindings,
        host.input_contexts);
    host.log.implemented("FrontEndManager::deactivate", "00683aa0");
    host.main_menu.base.active = false;  // +3Ch, cleared by 00683aa0
    host.summary.main_menu_manager_active = false;
    // 00686ED1..00686EF0, after 00683AA0 and the 00E198AC clear: release
    // the +50h title stream's reference (00A87B30 at zero). OFF built none.
    if (host.title_stream != nullptr && host.title_music.title_stream != nullptr) {
        host.title_stream->release();
        host.log.implemented("MainMenuManager::release_title_stream", "00a87b30");
    }
}

bool GameMenuHost::mission_exit_finished() const noexcept {
    // Milestone 2g: the mission left game state 0Dh through 004d7970 and the
    // drain reached request 04h, the front-end shell the debrief runs in.
    return impl_->mission != nullptr
        && impl_->mission->summary().mission_exit_completed;
}

void GameMenuHost::run_title_init_004c9a70() {
    Impl& host = *impl_;
    host.initialize_command_widgets();
    MenuCommitHost commit(host);
    MenuPressStartHost press(host);
    MenuFrameLayoutHost layouts(host);
    MenuTitleHandoverHost handover(host, press, commit);
    MenuTitleInitHost title(host, layouts, handover, commit);

    host.title_init.skip_title = false;
    host.handover.skip_title = false;
    host.handover.requests_held = host.state.requests_held;
    run_title_init(host.title_init, title);
    // 004c9b0d writes game+5D4h = 2 through the same field the frame reads.
    host.state.value = static_cast<int>(host.title_init.game_state);
    host.summary.title_init_ran = true;
    host.summary.game_state = host.state.value;
    host.path.game_state = host.state.value;
    host.path.state_requests_held = host.state.requests_held;
    host.log.implemented("Title bring-up GGame::OnInitTitle", "004c9a70");
    host.log.notef("title init: game state %d, press-start slot %d, registry screens %zu",
        host.state.value, host.summary.press_start_slot, host.screens.size());
    host.frontend.invalidate_bridge();
}

bool GameMenuHost::input_action_pressed(int action) {
    Impl& host = *impl_;
    if (action != kPressStartInputAction) {
        const auto it = host.injected_actions.find(action);   // the helm `input` line
        return it != host.injected_actions.end() && action_pressed_this_frame_004c43c0(it->second);
    }
    return action_pressed_this_frame_004c43c0(host.press_action);
}

bool inject_input_action_hold(int action, unsigned frames) {
    if (action == kPressStartInputAction || action < 0 || frames == 0) return false;
    pending_input_holds()[action] = frames;
    return true;
}

namespace {
struct InjectedViewAim {
    bool active{false};
    float yaw{0.0f};
    float pitch{0.0f};
    // `aim unit`: the tracked row and its optional fixed pitch.
    bool tracking{false};
    std::size_t track_unit{0};
    bool track_has_pitch{false};
    float track_pitch{0.0f};
    unsigned track_serial{0};   // bumped by every `aim unit` line
};
InjectedViewAim& injected_view_aim_state() {
    static InjectedViewAim aim;
    return aim;
}
}  // namespace

void inject_view_aim(float yaw, float pitch) {
    InjectedViewAim& aim = injected_view_aim_state();
    aim.active = true;
    aim.yaw = yaw;
    aim.pitch = pitch;
    aim.tracking = false;   // the next `aim` line ends an `aim unit` track
}

void inject_view_aim_track(std::size_t unit, bool has_pitch, float pitch) {
    InjectedViewAim& aim = injected_view_aim_state();
    aim.active = false;
    aim.tracking = true;
    aim.track_unit = unit;
    aim.track_has_pitch = has_pitch;
    aim.track_pitch = pitch;
    ++aim.track_serial;
}

unsigned injected_view_aim_track_serial() { return injected_view_aim_state().track_serial; }

bool injected_view_aim_track(std::size_t& unit, bool& has_pitch, float& pitch) {
    const InjectedViewAim& aim = injected_view_aim_state();
    if (!aim.tracking) return false;
    unit = aim.track_unit;
    has_pitch = aim.track_has_pitch;
    pitch = aim.track_pitch;
    return true;
}

void clear_injected_view_aim_track() { injected_view_aim_state().tracking = false; }

bool injected_view_aim(float& yaw, float& pitch) {
    const InjectedViewAim& aim = injected_view_aim_state();
    if (!aim.active) return false;
    yaw = aim.yaw;
    pitch = aim.pitch;
    return true;
}

void clear_injected_view_aim() { injected_view_aim_state().active = false; }

// The executable's action table holds one record, press-start 4Eh. Every other
// index is a record nothing starts, so both rules answer false for it, unless a
// helm `input` line holds it (inject_input_action_hold).
bool GameMenuHost::input_action_held(int action) {
    Impl& host = *impl_;
    if (action != kPressStartInputAction) {
        const auto it = host.injected_actions.find(action);   // the helm `input` line
        return it != host.injected_actions.end() && action_down_current(it->second);
    }
    return host.press_action.current_down && host.press_action.current_hold > 0.0f;
}

bool GameMenuHost::input_action_released(int action) {
    Impl& host = *impl_;
    if (action != kPressStartInputAction) {
        const auto it = host.injected_actions.find(action);   // the helm `input` line
        return it != host.injected_actions.end() && !action_down_current(it->second)
            && action_down_previous(it->second);
    }
    const bool held_now = host.press_action.current_down && host.press_action.current_hold > 0.0f;
    return !held_now && host.press_action.previous_down && host.press_action.previous_hold > 0.0f;
}

void GameMenuHost::frame(float raw_delta, unsigned long long frame_index) {
    Impl& host = *impl_;
    host.pumped_this_frame = false;

    // 00a92370's prologue: the current pair becomes the previous pair and the
    // current pair is zeroed, which is what makes the injected press one frame
    // long. 00a92aa0 then produces exactly the rising edge 004c43c0 reports.
    if (host.press_start_frame >= 0
        && frame_index == static_cast<unsigned long long>(host.press_start_frame)) {
        const InputEffectParam no_param{};
        start_action_00a92aa0(host.press_action, 1.0f, no_param, nullptr);
        host.summary.press_start_injected = true;
        host.log.notef("press-start input action %d injected on frame %llu",
            kPressStartInputAction, frame_index);
    } else {
        begin_action_frame_00a92370(host.press_action);
    }
    // The helm `input` line: the same three rules drive each held action, so the
    // first frame is the rising edge, the rest continue it, and the frame after
    // the last is the release.
    for (const auto& request : pending_input_holds()) {
        host.injected_actions[request.first];
        host.injected_frames_left[request.first] = request.second;
        host.injected_frames_without_2eh[request.first] = 0;
        host.log.notef("helm input action 0x%02X held for %u frame(s) from frame %llu",
            static_cast<unsigned>(request.first), request.second, frame_index);
    }
    pending_input_holds().clear();
    // Screen 2Eh's 005484F0 (the HUD's weapon-group reader) runs only while screen
    // 46h's update passes its gate (0064D62B wanted_04 / has_unit_1c): on USN01,
    // frames ~410-1823, until the script's SetSelectedUnit. The HUD reports each
    // frame it ran (note_screen_2eh_ran), after this prologue, so the previous
    // frame's held actions are judged here, one frame late. An edge or held frame
    // that no 005484F0 read is logged instead of being dropped silently.
    const bool screen_2eh_ran_last_frame = host.screen_2eh_ran;
    host.screen_2eh_ran = false;
    for (auto& entry : host.injected_actions) {
        const int action = entry.first;
        if (host.injected_held_last_frame[action] && !screen_2eh_ran_last_frame) {
            ++host.injected_frames_without_2eh[action];
            if (host.injected_edge_frame[action] + 1 == frame_index) {
                host.log.notef("helm input action 0x%02X edge on frame %llu lost: "
                    "screen 2Eh's 005484F0 did not run", static_cast<unsigned>(action),
                    frame_index - 1);
            }
        }
        unsigned& left = host.injected_frames_left[action];
        if (left == 0 && host.injected_held_last_frame[action]
            && host.injected_frames_without_2eh[action] > 0) {
            host.log.notef("helm input action 0x%02X hold ended on frame %llu: %u held "
                "frame(s) without screen 2Eh's 005484F0", static_cast<unsigned>(action),
                frame_index - 1, host.injected_frames_without_2eh[action]);
        }
        host.injected_held_last_frame[action] = left > 0;
        if (left > 0) {
            if (action_down_current(entry.second)) {
                continue_action_00a919f0(entry.second, 1.0f);
            } else {
                const InputEffectParam no_param{};
                start_action_00a92aa0(entry.second, 1.0f, no_param, nullptr);
                host.injected_edge_frame[action] = frame_index;
                host.injected_frames_without_2eh[action] = 0;
            }
            --left;
        } else {
            begin_action_frame_00a92370(entry.second);
        }
    }

    if (is_front_end_game_state(host.state.value)) {
        // 004e4b9d. The branch ends by pumping the registry at 004e4ca3.
        MenuCommitHost commit(host);
        MenuPressStartHost press(host);
        MenuScreenHost screens_host(host, press, commit);
        MenuTitleHandoverHost handover(host, press, commit);
        MenuFrameHost frame_host(host, handover);
        run_front_end_state_frame(host.frame_state, host.world, frame_host, screens_host,
            raw_delta);
        // 004e4ca3, the pump the branch's shared tail runs.
        host.log.implemented("FrontEndScreens::pump", "004f8830");
        host.pumped_this_frame = true;
        ++host.summary.pump_frames;
    } else {
        // 004e53b6, BSP_Game_UpdateInterfaceOnly, which OnMove calls on every
        // frame that does not take the front-end branch.
        host.log.implemented("FrontEndScreens::update_interface_only", "004c40f0");
        // Packet cc9_menu_pump_once (docs/GUNNERY_OPEN_ITEMS.md section 76):
        // 004e53b6 is the else arm of the simulation gate (004e53b2 JMPs over
        // it), and a mission frame runs 004e4a40 whole through
        // run_mission_frame_004e4a40, 004e5259 / 004e53b6 / 004e5469 included.
        // Pumping here as well ran every level-1 screen twice per mission
        // frame. OFF: the second pass stays.
        const bool mission_frame_pumps = kMenuPumpYieldsToMissionFrameBound
            && host.mission != nullptr && host.mission->next_advance_runs_mission_frame();
        if (host.path_step == MainMenuPathStep::ScreenVisible && !mission_frame_pumps) {
            // Once the path has settled the pump at 004c4165 is the only
            // front-end work left in the frame; before that the path's own
            // EnterScreen step performs it.
            host.pump(raw_delta);
        }
    }

    host.advance_path(raw_delta);

    // Milestone 2e. The scripted selection runs once the shell has published the
    // main-menu screen, which is the point at which the native player would be
    // looking at it. One step per frame, so the run can be photographed at any
    // stage with --screenshot-frame.
    if (host.mission != nullptr && host.path_step == MainMenuPathStep::ScreenVisible) {
        if (host.mission->advance(raw_delta)) host.mission_running = true;
    }

    host.summary.game_state = host.state.value;
    host.frame_state.render_queue_open = false;
}

const GameMenuSummary& GameMenuHost::summary() const noexcept { return impl_->summary; }

}  // namespace bsp::game
