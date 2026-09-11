# Voice owner and timed-sequence updates

This batch continues `VOICE_SERVICES_INTEGRATION.md` from the isolated
`agent/orch4-20260910` integration worktree. The existing BSP project/program
is the only Ghidra analysis source. Worker scope, address ownership and
validation records remain distinct from the primary integration.

## Manager state corrections

Assembly and the record producer establish that manager `+74` is a borrowed
pending record pointer and `+70` is a signed slot index. The readiness test's
former `disabled_74` name is superseded by `pending_record_74`. It still blocks
readiness when nonnull. Native manager `+64/+68/+6C` is an allocator/sentinel/count
list for attached voice entries; the earlier `blocked_6c` field is now
`attached_count_6c`. The original readiness predicate still requires zero.

The canonical manager owns one `VoiceScheduledRows` projection at `+94`.
Scheduled bindings alias its actual fields, rather than mirror pending/selected
state. Timed keys are reached through the same borrowed `VoiceClipRecord`
identity at native `+30` (begin `+34`, end `+38`); no copy or text-ID lookup is
substituted. `+84` is the selected row, `+88/+8C` are the two tail timers,
`+44..+50` is the base Float4, and `+28` is the enclosing panel widget.
C++ default member values do not establish the native manager constructor.

## Complete sequence update: 005BBF10

`update_voice_sequence_005bbf10` reconstructs the complete 520-instruction body,
native ECX=manager, float delta at stack+4, RET4 and active result in AL.
The inclusive final instruction is `005BC636 RET4` (three bytes, end `005BC638`).
The verified listing has no flow gaps. Its descriptive name is a hypothesis.

1. Action `E6` clears both tail timers. A captured nonnegative slot index is
   polled; if active, the same slot is stopped through recovered `007026F0`.
2. Walk the current row vector. Nonselected visible rows subtract delta from
   their fade countdown and clamp ordered values to `[0, manager+78]`.
   Positive values publish alpha as countdown divided by `+78`; zero, negative
   or unordered values run the empty-panel text sequence. Selection is reread
   after GUI calls, allowing that same row to become current.
3. Current rows with no pending record and active timed keys capture the record's
   key vector identity and one elapsed float (`mission clock - row start`).
   A nonzero key end time advances only when elapsed is strictly greater.
   End time zero instead polls the row slot; only a nonzero poll followed by
   a second, successful sound-completion virtual call advances the key.
4. Advancement clears panel text, rereads the current key's callback string,
   copies it and invokes the existing thread-safe mission Lua dispatcher.
   String cleanup finishes before incrementing the current row index. At most
   one key is advanced on this update; there is no invented catch-up loop.
5. The current key is displayed only when the captured elapsed time exceeds
   `float(start + double(0.1f))`. The binary64 constant at `00D7A3A0` is
   `3FB99999A0000000`; the zero sentinel at `00D7A218` is `00000000`.
   Equality and unordered comparisons do not display it.
6. Copy key text. Native CRT `strcspn` finds the first caret or terminator;
   a positive result different from the stored length selects the prefix path.
   Parse a substring with native-width `atol`, destroy it, resolve the current
   panel owner's signed-integer color map, copy Float4 into the row, then replace
   text with the suffix. A nonzero parsed key dispatches decoration virtual
   `+88(key,0,1.0f)`. The original key pointer supplies its current force byte
   after these callbacks. Embedded NUL follows CRT terminator behavior too.
7. Decrement `+8C` and invoke recovered `005B9420`. Once idle and `+8C` is not
   ordered-positive, clear selection to `FFFFFFFF`, decrement `+88`, and advance
   the separate panel state machine once `+88` is also not ordered-positive.
   Its result controls the enclosing panel's visibility and this function's AL
   return. The native diagnostic `vanmeg` is emitted only for a true result.

Row/key addresses captured by native code are retained across the same calls;
their storage must remain valid until its next use. Vector counts, key indices,
selected rows and manager widget pointers are reread at their native boundaries.
The implementation does not snapshot an entire frame or collapse callback effects.

## Panel text helper: 005B6710

`set_voice_panel_text_005b6710` reconstructs all 112 instructions, native
ECX=manager, NativeString* and byte force stack arguments, RET8. Its final
instruction is `005B6879 RET8` (three bytes, end `005B687B`), with no flow gaps.
Show when nonempty text and subtitle preference permit it, or when force is
nonzero. Show/hide group, background and decoration in order. The hidden branch
submits empty literal text twice. The visible branch submits localized text
twice, measures the current first text widget, resizes the background and
repositions the decoration using the recovered binary64 constants and float
spill sites. GUI fields are reread after callbacks; the manager identity stays
the incoming ECX owner throughout.

## Connected worker routines and unresolved services

`VOICE_LINE_LIFETIME.md` records line destruction, scalar deletion and serialized
construction. `SCHEDULED_VOICE.md` records admission, pending polling and record
start. `VOICE_MANAGER_UPDATE.md` records normal-line updates, dirty relayout,
attached-entry updates/removal, slot stop and volume helpers. These routines
reuse canonical voice/GUI/string/sound storage and concrete preceding bodies.

The manager's canonical `00CF0ED4` line vtable now calls the reconstructed
scalar deleting destructor directly. Other vtables retain the required actual
virtual dispatch. Attached queue bindings alias the canonical sentinel/count.
A focused integrated heap-node fixture verifies both destructor routes,
clip-vector release, queue counts and callback-sensitive cleanup order.

Remaining services include actual GUI virtual dispatch and widget-tree identity,
native allocation/observer storage and real audio/alternate channels. The
sequence caller still requires current panel-state advancement `004527F0` and
map lookup-or-insert `0044EC00`. The latter is **not** an ARGB converter. Missing
palette entries must obey the actual map insertion/default behavior; this batch
does not invent fallback colors. The GUI reader follow-up forwards the actual
looked-up Lua table token through a required resolver service and aligns the
evaluated String path with the native NUL scan. Its existing live String path
already matches that scan. The actual game handle resolver remains required;
forwarding the correct identity does not implement it.

The code targets MSVC Win32. Float arithmetic in the sequence/text helper uses
the native x87 spill boundaries; signaling-NaN payload/quieting, unmasked traps,
floating-point status, native binary layouts/SEH and corrupt-container failure
equivalence are excluded. No full game, audio output or rendered UI claim follows
from compilation or service fixtures. Final validation and annotation records
are in `reports/voice_update_integration.json`.

The combined Win32 build and both existing CTests passed at `6f8c726`.
Five ignored fixtures passed against that library: scheduled admission,
manager cleanup with both destructor routes, line lifetime, the quiet-NaN
sequence terminal branch, and real-Lua reader identity/String conversion.
The integration helper then merged current main and passed build plus both
CTests again at `4ad5319`. Main advanced during that build and was merged to
`d219a42` afterward; the later merge is not a claim of another completed build.
Fifteen scoped names/evidence comments were saved and read back, preserving
prior comments, and affected exports were refreshed. Five false `_free`
callsite continuations were repaired. Two disproven `stl_probable` bookmarks
were removed with their original ledger/bookmark records retained in the report.
