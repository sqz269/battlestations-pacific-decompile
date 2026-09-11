// Fonts and GUI bring-up of BSP_Application_Initialize.
// Evidence: docs/APP_INIT_FONTS_GUI.md. See the header for the address list.
#include "bsp/gui_startup.hpp"
#include <stdexcept>

#include <cstring>

namespace bsp {

void construct_locale_table_manager_0073c960(LocaleTableManagerImage& image) noexcept
{
    // 0073c960 runs 00736540 first with the same ECX; that helper stores the
    // singleton base vtable and publishes the object. The derived vtable then
    // overwrites the slot at 0073c982.
    image.vtable = kLocaleManagerDerivedVtable;

    // 0073c988..0073c98e. The vector's leading word at +0x04 is not written.
    image.table_names.first = 0;
    image.table_names.last = 0;
    image.table_names.end = 0;

    // 0073c991..0073c99b, memset(this + 0x14, 0, 0x4000).
    std::memset(image.buckets, 0, kLocaleBucketBytes);

    // 0073c9a3, written as [EBX + 0x4000] with EBX = this + 0x14.
    image.entry_count = 0;

    // 0073c9a9 / 0073c9af.
    image.language_name.size = 0;
    image.language_name.data = 0;

    // 0073c9b5..0073c9c1 and 0073c9cb..0073c9d7. Both leading words untouched.
    image.lanx_a.first = 0;
    image.lanx_a.last = 0;
    image.lanx_a.end = 0;
    image.lanx_b.first = 0;
    image.lanx_b.last = 0;
    image.lanx_b.end = 0;
}

void* diagnostic_sink_get_or_create_004c14c0(DiagnosticSinkSingleton& state) noexcept
{
    if (!state.constructed) {
        // 00bf681b(4) then the single store of &PTR_LAB_00ce752c. A failed
        // allocation leaves the global null and the registration still runs.
        state.constructed = true;
        state.vtable = kDiagnosticSinkVtable;
        ++state.construction_count;
        state.registered_with_lifetime_manager = true;
    }
    return state.constructed ? &state : nullptr;
}

GuiStartupResult run_gui_startup(GuiStartupHost& host)
{
    GuiStartupResult result;

    // 0073bafe: 008d4890 with the settings object in ECX, copied into the frame
    // string at S+0x14 by 0041e870.
    result.language_font_path = host.language_font_path();

    // 0073bb7e..0073bb99. The three frame strings are pushed in the order
    // language path, descriptor, root; 007371d0 consumes none of them and
    // 00ac3910 takes all three with RET 0xc, so the callee sees them as
    // (root, descriptor, language path).
    FontRegistry& registry = host.font_registry();
    result.font_descriptors_loaded = host.load_font_descriptors(
        registry, kFontRootPrefix, kFontDescriptorPath, result.language_font_path);

    // 0073bc0d: 0053bc00 then 00be9620. The adjusted pointer is discarded.
    host.preload_fallback_glyph_table();

    // 0073bc19: 004c12b0 then 00aa5e20 with the manager in ECX.
    void* manager = host.gui_manager();
    result.gui_manager = manager;

    void* group = nullptr;
    void* deferred_visibility_target = nullptr;
    bool deferred_visibility = false;
    bool deferred_visibility_value = false;
    for (const GuiManagerResource& entry : kGuiManagerResources) {
        void* parent = entry.kind == GuiResourceKind::Child ? group : nullptr;
        void* created = host.create_gui_resource(manager, parent, entry);
        if (entry.kind == GuiResourceKind::Group) {
            // The group becomes the ECX of every Child that follows it, even
            // when it is not stored on the manager, as `_Highlight` is not.
            group = created;
        }
        if (created != nullptr) {
            ++result.resources_created;
        }
        // +2Ch is assigned even when the renderer returns null. The whiteGui
        // legacy null result represents VFS rejection; see GuiResourceOwner
        // for the separate resolve and load contracts.
        if (created != nullptr || entry.offset != 0x28) {
            if (entry.offset != kGuiResourceNotStored) {
                host.store_gui_resource(manager, entry.offset, created);
                ++result.stores;
            }
            if (entry.second_offset != kGuiResourceNotStored) {
                host.store_gui_resource(manager, entry.second_offset, created);
                ++result.stores;
            }
            if (entry.sets_visibility && !entry.visibility_deferred) {
                if (created == nullptr) {
                    throw std::runtime_error("00aa5e20 requires GUI resource: " +
                        std::string(entry.name));
                }
                host.set_gui_resource_visibility(created, entry.visibility);
            }
        }
        // 00aa60a3: MousePtrFE_Icon's flag call trails MousePtrGUI_Icon's.
        if (deferred_visibility) {
            if (deferred_visibility_target == nullptr) {
                throw std::runtime_error("00aa5e20 requires MousePtrFE_Icon");
            }
            host.set_gui_resource_visibility(deferred_visibility_target,
                deferred_visibility_value);
            deferred_visibility = false;
            deferred_visibility_target = nullptr;
        }
        if (entry.sets_visibility && entry.visibility_deferred) {
            deferred_visibility_target = created;
            deferred_visibility_value = entry.visibility;
            deferred_visibility = true;
        }
    }
    if (deferred_visibility) {
        host.set_gui_resource_visibility(deferred_visibility_target, deferred_visibility_value);
    }

    // 00aa6305, byte +0x84 = 0.
    host.clear_gui_manager_ready_flag(manager);
    return result;
}
}
