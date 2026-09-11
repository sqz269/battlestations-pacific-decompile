#include "bsp/frontend_entry.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native loading progress requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
static_assert(offsetof(LoadingProgressCrtAccess, sse2_conversion_0109eea4) == 4);
static_assert(offsetof(LoadingProgressCrtAccess, convert_st0_00bf7420) == 8);
namespace {
// ECX/+30 field and EDX/+2C field are semantic projection addresses; stack
// contains incoming and required CRT access. The floating schedule is native.
__declspec(naked) void __fastcall report_progress_kernel(
    float*, std::int32_t*, float, const LoadingProgressCrtAccess*) {
    __asm {
        push ebx
        push esi
        push edi
        mov esi, edx
        mov edi, ecx
        sub esp, 4
        mov ebx, dword ptr [esp + 24]
        fld dword ptr [edi]
        fstp dword ptr [esp]
        fld dword ptr [esp + 20]
        fld dword ptr [esp]
        fcomip st, st(1)
        jbe incoming_selected
        movss xmm0, dword ptr [esp]
        jmp multiply_original_incoming
    incoming_selected:
        movss xmm0, dword ptr [esp + 20]
    multiply_original_incoming:
        mov edx, dword ptr [ebx]
        fmul qword ptr [edx]
        movss dword ptr [edi], xmm0
        mov ecx, dword ptr [ebx + 4]
        call dword ptr [ebx + 8]
        mov ecx, dword ptr [esi]
        cmp ecx, eax
        jg finished
        jz finished
        mov dword ptr [esi], eax
    finished:
        add esp, 4
        pop edi
        pop esi
        pop ebx
        ret 8
    }
}
// 00CE8254 and 00CE8274. The only "SLM_" strings in the image.
constexpr const char* kLoadBlockCold = "GILoading::SLM_LOAD_FRONTEND";
constexpr const char* kLoadBlockReturn = "GILoading::SLM_LOAD_FRONTEND_RETURN";

// 00CE82C8, 00CE82B0 and 00CE8298, in the order 004e4022..004e4062 uses them.
constexpr const char* kProbeTextures = "Textures before mainmenu";
constexpr const char* kProbeSounds = "Sounds before mainmenu";
constexpr const char* kProbeEffects = "Effects before mainmenu";

// 00F8D394+18h at 004e4027.
constexpr std::uint32_t kRendererBudget = 0x20000000u;

// 0057cc42 and 0057cccc; the atlases 0057cb60 loads through 00af0060.
constexpr const char* kAtlasAllButInGame = "interface/textures/allbutingame.ats";
constexpr const char* kAtlasMenu = "interface/textures/menu.ats";

// The three names 0057d0d0 pushes, in body order (00CEF348, then the two that
// follow it). Each is built with an explicit length of 0Dh before the memcpy.
constexpr const char* kLoadingImages[] = {"mp.loading_24", "mp.loading_05", "mp.loading_12"};

// Element slots of the loading screen, screen+1Ch..+28h, addressed in 0057cb60
// as DAT_00E194B4[7]..[10].
constexpr int kElementImage = 0; // +1Ch
constexpr int kElementMenuPanel = 1; // +20h
constexpr int kElementAspectSource = 2; // +24h
constexpr int kElementFrame = 3; // +28h

// 0057cd2b and 0057cd7a: 004c1ac0 then 00518250 with the same pair.
constexpr int kMenuLayoutForMenuBackground = 2;
constexpr int kMenuLayoutForLoadingImage = 1;

// 0057cd35: the image index the mode 0 arm passes to vtable +88h. Mode 1 passes
// the published visibility byte in the same position instead.
constexpr int kMenuBackgroundImageIndex = 2;

// 0057cd41 and 0057cd8c: the third argument of vtable +88h, 3F800000h.
constexpr float kElementImageAlpha = 1.0f;

// 0057cb60 mode 0, 0057cccc..0057cd68.
void configure_menu_background(LoadingScreenHost& host) {
    host.load_texture_atlas(kAtlasMenu);
    host.select_menu_layout(kMenuLayoutForMenuBackground, 1);
    host.element_set_image(kElementImage, kMenuBackgroundImageIndex, kElementImageAlpha);
    host.element_set_visible(kElementMenuPanel, true);
    host.element_set_visible(kElementAspectSource, false);
    host.element_set_visible(kElementFrame, false);
}

// 0057cb60 mode 1, 0057cd6a..0057ce80. The rect the frame element receives comes
// from a comparison of *(screen+24h)+114h against the constants at 00CEC380 and
// 00CEC3E8; both arms call vtable +58h, so only the argument differs and that
// difference is not reconstructed here.
void configure_loading_image(const LoadingScreenConfig& globals, LoadingScreenHost& host) {
    host.select_menu_layout(kMenuLayoutForLoadingImage, 1);
    host.element_set_image(kElementImage, globals.image_visible ? 1 : 0, kElementImageAlpha);
    host.element_set_visible(kElementMenuPanel, false);
    host.bind_loading_image(globals.picked);
    static_cast<void>(host.element_aspect_source());
    host.element_set_rect(kElementFrame);
    host.element_set_visible(kElementAspectSource, true);
    host.element_set_visible(kElementFrame, true);
}
}

const char* front_end_load_block_label(FrontEndLoadBlock block) noexcept {
    return block == FrontEndLoadBlock::Return ? kLoadBlockReturn : kLoadBlockCold;
}

// 004e407f..004e40ab. The guard is tested twice in the original: once at 004e4085
// to take the cold path directly, once again at 004e40a2 after the manager test,
// where it can no longer be false. The second test is dead, so this collapses it.
FrontEndLoadDecision decide_front_end_load(
    bool mission_init_done, bool manager_b8_present, bool manager_ac_present) noexcept {
    if (!mission_init_done) {
        return FrontEndLoadDecision::LoadCold;
    }
    if (manager_b8_present && manager_ac_present) {
        return FrontEndLoadDecision::SkipAlreadyResident;
    }
    return FrontEndLoadDecision::LoadReturn;
}

LoadingScreenConfig default_front_end_loading_config(bool current_image_visible) {
    LoadingScreenConfig config{};
    for (const char* name : kLoadingImages) {
        config.images.emplace_back(name);
    }
    // 0057d25a. The byte that 0057d107 set to 1 is overwritten with the live
    // global just before the return, so the seeded 1 never reaches a caller.
    config.image_visible = current_image_visible;
    return config;
}

void publish_loading_screen_config(LoadingScreenConfig& globals, const LoadingScreenConfig& source) {
    if (&globals == &source) {
        // 0057d002 compares the string members and 00506c80 the vector owners;
        // both make a self-publish a no-op that still stores the byte.
        globals.image_visible = source.image_visible;
        return;
    }
    globals.images = source.images;
    globals.picked = source.picked;
    globals.image_visible = source.image_visible;
}

void report_loading_progress(LoadingScreen* screen, float progress,
    const LoadingProgressCrtAccess& access) {
    if (screen == nullptr) {
        return; // 0057bec9
    }
    if (!access.scale_00cef258 || !access.sse2_conversion_0109eea4 || !access.convert_st0_00bf7420)
        throw std::invalid_argument("loading progress requires actual scale/global and ST0 CRT conversion");
    report_progress_kernel(&screen->progress, &screen->progress_units, progress, &access);
}

void begin_loading_screen(LoadingScreen& screen, LoadingScreenMode mode,
    const LoadingScreenConfig& globals, LoadingScreenHost& host) {
    // 0057cb7c..0057cbb0: begin/end run twice, clearing both swap-chain buffers.
    host.renderer_begin_frame();
    host.renderer_end_frame();
    host.renderer_begin_frame();
    host.renderer_end_frame();
    if (host.game_object_present()) {
        host.session_suspend();
    }
    host.gui_set_enabled(false);
    host.gui_update(0.0f, 0); // 0057cbd5: FLDZ, so the delta argument is +0.0f
    host.screen_pump_reset();
    host.close_previous_screen();
    host.load_texture_atlas(kAtlasAllButInGame);
    // 0057cc8d. The global is assigned inside 0057bff0, the lifetime-hook base
    // constructor, so it is already live when vtable +10h runs.
    if (!screen.created) {
        host.create_screen();
        host.screen_init();
        screen.created = true;
    }
    screen.mode = mode;
    screen.configured = true;
    switch (mode) {
    case LoadingScreenMode::MenuBackground:
        configure_menu_background(host);
        break;
    case LoadingScreenMode::LoadingImage:
        configure_loading_image(globals, host);
        break;
    default:
        // 0057ccc6 falls through to 0057ce83 for every other value, leaving the
        // four elements exactly as vtable +10h left them.
        screen.configured = false;
        break;
    }
    host.prepare_extra_elements();
    screen.base.wanted = true; // 0057ce8e, byte +4h
    screen.base.active = true; // 0057ce92, byte +5h
    host.screen_registry_commit();
    host.screen_show();
    host.renderer_begin_worker_mode();
    host.start_render_worker(kLoadingScreenWorkerRate);
}

void end_loading_screen(LoadingScreen* screen, LoadingScreenHost& host) {
    host.gui_refresh(); // 0057c250..0057c25b
    if (host.game_object_present()) {
        host.session_resume();
    }
    if (screen != nullptr) {
        if (screen->base.active) {
            host.screen_stop_worker();
        }
        screen->base.wanted = false;
        screen->base.active = false;
        host.screen_registry_commit();
    }
    // 0057c2ae. The registration node is screen+8h, or null when the screen is.
    host.lifetime_unregister();
    if (screen != nullptr) {
        host.screen_destroy(); // 004e43e7 -> 0057c2c9, vtable +0Ch with 1
        screen->created = false;
    }
}

FrontEndShellOutcome enter_front_end_shell(FrontEndShellState& state, FrontEndShellHost& host) {
    host.renderer_set_budget(kRendererBudget);
    host.probe_texture_memory(kProbeTextures);
    host.probe_sound_memory(kProbeSounds);
    host.probe_effect_memory(kProbeEffects);

    if (host.title_screen_present()) {
        host.destroy_title_screen(); // 004e4071, vtable +0h with 1, then null
    }

    const FrontEndLoadDecision decision = decide_front_end_load(state.mission_init_done,
        host.front_end_manager_b8_present(), host.front_end_manager_ac_present());
    if (decision != FrontEndLoadDecision::SkipAlreadyResident) {
        const FrontEndLoadBlock block = decision == FrontEndLoadDecision::LoadReturn
            ? FrontEndLoadBlock::Return
            : FrontEndLoadBlock::Cold;
        host.open_load_block(front_end_load_block_label(block));
        LoadingScreenConfig& globals = host.loading_globals();
        const LoadingScreenConfig fresh = default_front_end_loading_config(globals.image_visible);
        publish_loading_screen_config(globals, fresh);
        // 004e4114 zeroes ECX, so the shell always raises mode 0 here even
        // though the images it just published are the mode 1 material.
        host.begin_loading(LoadingScreenMode::MenuBackground);
        host.close_load_block();
    }

    host.report_progress(kFrontEndInitialProgress); // 004e4130
    host.game_on_init();
    state.state = kGameStateFrontEndInit; // 004e3ac2, the first store OnInit makes
    host.poll_platform_session_events();
    state.state = host.game_state(); // 004e4151 re-reads the field
    if (state.state != kGameStateFrontEndInit) {
        host.end_loading();
        return FrontEndShellOutcome::AbortedByPlatformEvent;
    }

    if (!host.front_end_manager_b8_present()) {
        host.create_manager_b8();
    }
    if (!host.front_end_manager_ac_present()) {
        host.create_manager_ac();
    }
    if (!host.front_end_manager_b4_present()) {
        host.create_manager_b4();
    }

    if (state.lua_vm_ready) {
        host.lua_collect_garbage(); // 004e4245
    }
    if (host.manager_ac_mode() != 4) {
        host.reset_manager_ac_mode(); // 004e4259
    }
    host.manager_ac_enter();
    host.manager_ac_start_sub();

    state.state = kGameStateFrontEndShellReady; // 004e4279

    if (host.post_state_hook_wanted()) {
        host.post_state_hook(); // 004e428a
    }

    // 004e428f..004e43e2. The outer guard is a find in the map at game+6F0h, so
    // the block runs at most once per profile; record_award is itself a
    // monotonic max-insert, which would make a repeat harmless anyway.
    if (!state.award_ga_hm_recorded && host.award_gate_open()) {
        const int id = host.award_id(kFirstMainMenuAwardKey);
        if (id >= kAwardIdMin && id <= kAwardIdMax && host.award_system_ready()
            && host.award_session_ready()) {
            host.grant_award(id);
            host.record_award(kFirstMainMenuAwardKey, 1);
            state.award_ga_hm_recorded = true;
        }
    }

    host.end_loading(); // 004e43e7

    if (state.network_quit_pending) {
        host.send_network_quit(); // 004e4403
    }
    state.network_quit_pending = false; // 004e440d, cleared on both arms
    return FrontEndShellOutcome::ShellReady;
}
}
