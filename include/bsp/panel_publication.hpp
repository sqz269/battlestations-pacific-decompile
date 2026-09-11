#pragma once

#include "bsp/panel_publication_types.hpp"
#include "bsp/panel_sequence.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// 0044F220: ECX=map at panel+4, NativeString* stack, RET4, EAX=&mapped int.
// Existing values retain identity. A missing name inserts the actual ZERO
// default, then returns that writable value for the configuration loader.
// Keys own independent pooled-string copies; input names remain borrowed.
std::int32_t& lookup_panel_character_0044f220(PanelCharacterMap&,
    const NativeString& name, NativeStringStorage&);

// New host lifecycle helper, not an address reconstruction. NativeString has
// no implicit destructor; use this before discarding an owning projected map.
void clear_panel_character_map(PanelCharacterMap&, NativeStringStorage&);

struct PanelPublicationContext {
    // Alias the SAME panel owner as the supplied PanelSequenceContext.
    PanelCharacterMap& characters_04;
    const SingletonLifetimeCallbacks& validation;
};

// Game-specific routine; the old STL_inst classification was a bad heuristic.
// ECX=voice manager; index/enabled/text*/character stack;
// RET10. The last three arguments are UNUSED: full assembly always dispatches
// row.widget+0 vslot34(false). A returning00BF6713 may repair manager.rows_94;
// the selected row/widget is loaded after that callback, without revalidation.
void hide_published_panel_row_005b6910(VoicePlaybackManager&, std::uint32_t index,
    std::uint32_t enabled, const NativeString* text, std::int32_t character,
    VoiceSubtitleHost&, const SingletonLifetimeCallbacks&);

// 00451C90: ECX=panel owner, RET. Captures its current queued entry once, then
// reloads the global voice manager at each count check AND each row dispatch.
// Character-name lookup/insertion still occurs for inactive slots. The slot
// pointer survives lookup callbacks and must remain valid through dispatch.
void publish_panel_rows_00451c90(PanelSequenceContext&, PanelPublicationContext&);

} // namespace bsp
