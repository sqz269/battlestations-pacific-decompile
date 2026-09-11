# Panel, observer and object Handle integration

This wave continues the isolated `agent/orch4-20260910` work after the voice
update batch. It closes the previously required whole-routine panel palette,
panel advancement and observer cleanup calls, and replaces the startup host's
local null Handle registry with persistent reconstructed callbacks.

## Shared state and direct calls

`VoicePanelState` now owns the same queue, queued count, current NativeString
and state used by `PanelSequenceView`. Its existing `field_24` and `field_34`
remain the readiness gates. `voice_panel_sequence` binds these fields by
reference; it does not maintain a second panel state.

The timed voice updater calls `advance_panel_sequence_004527f0` directly using
the current panel owner's context. Palette prefixes call the recovered
`panel_palette_value_0044ec00` on the current actual tree, and copy all16 value
bytes into the row without interpreting float bits. The palette uses actual
Win32 node links, signed keys and a value at node+10h. Panel command containers
remain explicitly documented standard-container projections.

Two native scratch values are not deterministically initialized. Missing
palette entries copy four residual stack words; selection at priority exactly
-1e10 before any winner can compare a residual float word. Both are explicit
required inputs at their native use sites. There is no invented default color,
selection seed, or new exception policy for these paths.

`VoiceAttachedEntry` now embeds `NativeObserverOwnerStorage` at native+18;
static assertions also establish the entity pointer at+2C. The destructor
writes that same owner's vtable, then directly unregisters its actual edge pair
and destroys the owner before slot teardown and allocation release. A pure
entity alias resolves the actual first endpoint. The recovered observer service
uses actual arrays/edges, the existing singleton domain and recursive lock,
current dispatch slots and required real edge virtual deletion. No whole
observer-cleanup callback remains in the voice manager.

`OBJECT_HANDLE_RESOLVERS.md` covers numeric table lookup, reverse object word,
live Lua `Ptr` lookup and callback registration. Startup retains the callable
registry for later reader consumers. Actual world table allocation/population
is still a dependency, not a synthetic object registry.

## Evidence and validation

Worker packets recovered16 panel routines,10 palette routines and10 observer
bodies plus the standard `std::remove` specialization. The primary recovered5
new Handle/Lua helpers and connected the existing startup installer. Independent
reviews checked complete object-callback listings, panel advancement and
observer unregister ordering before integration.

The combined MSVC Win32 build and both existing CTests passed. Six ignored
fixtures passed against the integrated library: actual palette tree insertion,
panel erase/reentry, observer lifetime, manager-to-observer cleanup, the
sequence-to-panel NaN terminal branch, and real-Lua/native-table Handle lookup.
The manager case preserves both canonical concrete and noncanonical virtual
line deletion. The sequence case reaches real panel erase and then observes
the panel pointer replaced by its log callback. No permanent tests were added.

These checks do not establish native ABI/SEH compatibility, floating-point
exception/status identity, table-owner construction, rendered colors or a
running game. The required native service boundaries and valid-storage domains
are retained in each packet's header/document.

## Ghidra updates and one retained limitation

Seven observer callsite continuations and two panel entry-destructor gaps were
repaired and saved through the write lock. The panel erase tail
0045129D..004512D2 was also decoded after clearing the false `_free` override.
Its complete tail, including RET0C at004512D0, is represented in C++ and the
worker's raw-byte evidence. Ghidra's stored00451020 function body remains short.

The configured bridge rejects `run_script_inline` because script execution is
disabled. The attempted metadata extension therefore made no change. It is
recorded in `reports/panel_sequence_body_extension.json`; the original function
was preserved. No full auto-analysis, deletion/recreation or configuration
change was substituted.

The supported prototype editor corrected006944C0 to an ECX/EDX fastcall over
two mutable three-word arrays. Readback preserves its earlier comment and now
shows the proven detach loop in00695530 pseudocode. The endpoint returned plain
text after applying the change; a follow-up readback confirmed and saved it
without reapplying. Original signature and before/after evidence are retained.

Three misleading inventory tags were removed with prior bookmark/ledger values:
0044D2C0 normally links/rebalances,00451020 erases, and00452740 advances panel
commands. Correct library/CG identities remain. Scoped names, comment readback,
exports and final integration revision are recorded in the integration report.

## Remaining reconstruction

The next direct panel dependencies are00451C90 row publication and00705E00
message-record resolution. Palette tree construction/loading, panel queue
construction/insertion, actual world handle-table owners and edge virtual
deleting destructors remain explicit boundaries. Existing real GUI, sound,
script and singleton services remain required. These are separate from the
stored-body metadata limitation and leave useful reconstruction work available.
