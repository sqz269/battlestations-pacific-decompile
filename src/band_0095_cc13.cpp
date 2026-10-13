// cc13-gp95 (docs/CC13_GP_0095_0097.md). Evidence in include/bsp/band_0095_cc13.hpp.

#include "bsp/band_0095_cc13.hpp"

namespace bsp::band_0095_cc13 {
namespace {

// CRT _stricmp under the "C" locale: ASCII case folding only.
bool equals_ascii_insensitive(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        unsigned char x = static_cast<unsigned char>(a[i]);
        unsigned char y = static_cast<unsigned char>(b[i]);
        if (x >= 'A' && x <= 'Z') x = static_cast<unsigned char>(x - 'A' + 'a');
        if (y >= 'A' && y <= 'Z') y = static_cast<unsigned char>(y - 'A' + 'a');
        if (x != y) return false;
    }
    return true;
}

}  // namespace

const EventSubscriptionType* find_event_subscription_type_0097e360(std::string_view event,
                                                                   bool has_data) noexcept {
    // 0097E380: a null data pointer skips the "recon" __stricmp; 00425850 then rejects every
    // remaining name, so the native call at the end goes through a null object.
    if (!has_data) return nullptr;
    for (const auto& type : kEventSubscriptionTypes) {
        if (equals_ascii_insensitive(event, type.event)) return &type;
    }
    return nullptr;  // 0097E8B9 with piVar3 == 0
}

}  // namespace bsp::band_0095_cc13
