# Dialog, message and observer integration

This wave connects the panel sequence to recovered character lookup, row
publication and message-record resolution. `PanelSequenceHost` now supplies
current owner/context aliases; the sequence executes `00451C90` and `00705E00`
directly. `VoicePanelState` retains the character map, actual palette pointer,
default pause, queue, current name and scheduling gates together. The dialog
loader's view aliases those same fields. The canonical `VoiceClipRecord` now
also retains its auxiliary string and timed-key array, preserving the borrowed
record identity across message creation, scheduled admission and playback.

`005B6910` was incorrectly classified as STL. Its complete body validates a
row index, reloads the widget and calls virtual `+34(false)`. Its three other
arguments are ignored. The implementation preserves that behavior while
`00451C90` still performs the original character-map lookup and manager reloads.
The old name/tag/bookmark is retired with prior values recorded; confirmed
vector-copy and compiler deleting-destructor identities remain intact.

Actual observer edges now have concrete creation, repeated registration and
canonical deletion. The current `00CF7E64` vtable routes directly to the
recovered deleting wrapper from unregister and detach-all; a noncanonical
vtable still requires its actual virtual dispatch. Both endpoints retain the
same allocated edge and actual array storage.

The dialog loader opens a real temporary Lua 5.1.1 owner, runs the base script
and VFS overrides, then reads live character/pause/color data. The numeric
accessors preserve float32 rounding before integer conversion or normalization.
The mode selecting native SSE2 versus x87 integer conversion is an explicit
live input. An independent review corrected table replacement to release the
old destination before copying the new reference.

Source evidence, native ABIs, scope and validation are recorded in:

- `DIALOG_CONFIG.md` and `reports/dialog_config.json`;
- `PANEL_PUBLICATION.md` and `reports/panel_publication.json`;
- `MESSAGE_RECORD_RESOLVER.md` and `reports/message_record_resolver.json`;
- `OBSERVER_EDGES.md` and `reports/observer_edges.json`.

The combined build, fixture results, exact revisions and Ghidra readback are
recorded in `reports/dialog_voice_integration.json`. Existing tests and focused
fixtures establish the stated code paths; they do not establish a running game.

Remaining work includes the panel owner constructor `00452660`, actual palette
sentinel/lifetime helpers `0044AB50` and their cleanup, and the `004DC6A0`
global-subsystem caller that invokes construction and loading. The generic GUI
reader still needs the recovered float32/CRT numeric binding, although this
loader already uses it. Message resource loading, observer dispatch population,
world handle-table owners, startup completion and gameplay validation remain.

Ghidra's `00693CA0` listing is complete and its false return flow was repaired,
but its initial 28-byte body membership/10-instruction metric differs from the
31-byte/11-instruction listing. No stored-membership repair is claimed. The
previous `00451020` body-extension limitation is also unchanged.
