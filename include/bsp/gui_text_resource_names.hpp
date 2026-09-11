#pragma once
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/native_font_resources.hpp"

namespace bsp {
struct GuiTextResourceNameServices {
    GuiWidgetOwnerRuntime& widgets;
    NativeRenderActualOwners& actual_owners;
};
struct GuiTextFontNameServices {
    GuiTextResourceNameServices& names;
    FontRegistryStartup*& published_registry_00f8bf44;
    SingletonLifetimeManager& singleton_lifetime;
    NativeFontResourceOwners& fonts;
    const volatile float& one_00d7a24c;
};

//00AB8E70..8ED6, ECX Text, name-header pointer stack, RET4 at8ED4/length3.
// Canonical typed-string domain: assign unless source aliases SAME member,
// then always capture actual shader+1EC. If nonnull, release its actual+04
// reference/current terminal callback and clear the LIVE slot AFTER callback,
// even when callback rebinds it. Equal spelling does not skip invalidation.
// Updates the existing has_cached_shader projection too; no new shader state.
void set_gui_text_shader_name_00ab8e70(GuiTextLifetime&, const std::string&,
    GuiTextResourceNameServices&);

//00AB8C30..8CD4, ECX Text, name-header pointer stack, RET4 at8CD2/length3.
// Native length/current CRT equality early-out; changed path stores font_name,
// invokes the EXISTING live singleton getter and stable owned-descriptor lookup,
// validates the SAME actual font-resource association, publishes borrowed font,
// then stores current descriptor alpha_texture_scale(+1C), or LIVE D7A24C for
// a genuine lookup miss. Only then captures/releases/clears actual shader.
// Does not reload geometry or copy scale_ratio(+18). Returned bool is a C++
// convenience (changed name), not a recovered native return-value contract.
bool set_gui_text_font_name_00ab8c30(GuiTextLifetime&, const std::string&,
    GuiTextFontNameServices&);

// Both use the SAME GuiTextWidget std::string members already owned by the
// canonical lifetime. No new native-header cache or second name/font table.
// Stable null-free DWORD-length names, successful typed-string allocation,
// current host CRT comparison, live owners and registered actual shader profiles
// are required. Native pooled-string layout/allocation callbacks and SEH are
// not reproduced. Registry/descriptors/actual fonts must outlive every Text
// borrowing them; FontRegistryStartup's compatibility vector/resources are
// never substituted for its stable descriptor or the actual FontData/images.
} // namespace bsp
