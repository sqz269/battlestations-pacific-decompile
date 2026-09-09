#include "bsp/vfs_search_defaults.hpp"

namespace bsp {
VfsCandidateRegistrations make_asset_search_registrations_00738360_fragment() {
    VfsCandidateRegistrations result;
    // Full startup body00738360..0073bad4, SHA-256:
    // 69fee27dbfafde84f1de4c8debe61839d9b25b82d6cc3b6771828d039174e0da.
    // 00be2310 prepends new groups, so later shaderfx precedes textures.
    result.groups.push_back({{"shfx"}, {
        "shaderfx/", "shaderfx/plane/", "shaderfx/ship/", "shaderfx/common/",
        "shaderfx/particles/", "shaderfx/terrain/", "shaderfx/ocean/",
        "shaderfx/lights/", "shaderfx/postprocess/", "shaderfx/gui/"}});
    // 00738401 registers dds; 007384d2 registers tga. Prefix values below are
    // the lowercase/slash-normalized results of00be2600, in native call order.
    result.groups.push_back({{"dds", "tga"}, {
        "textures/", "models/textures/", "models/textures/noseart/",
        "models/gui/map/units/", "models/gui/map/icons/",
        "interface/textures/mainmenu/", "interface/textures/",
        "particles/textures/", "particles/textures/anims/fragments/",
        "effects/flares/textures/", "fonts/", "weather/", "terrain/",
        "effects/", "effects/coast/", "effects/watertracer/", "effects/foam/",
        "effects/caustics/", "effects/lightning/", "effects/traceline/",
        "effects/foliage/", "effects/postprocess/", "interface/textures/terkep/"}});
    return result;
}
}
