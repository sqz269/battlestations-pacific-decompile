# Concrete title music composition

`GameTitleSound` replaces the older `TitleMusicHost` sequence with an actual
stream composition over the existing core sound and dialog services. It borrows
the current menu's native8h path and owning stream slot, the current movie
player and its existing widget host, the live E19504 C-string input and the
volatile F889A8 music volume. It creates no independent stream owner, movie
state, format table or lifetime manager. The shared menu-host source is left
for the primary integrator to bind; this packet does not claim that the running
application has switched to this helper.

## Recovered functions and interfaces

| Address | Implementation | Original ABI | Coverage |
|---|---|---|---|
| 005884A0 | `GameTitleSound::start_005884a0` | ECX ignored, no stack arguments, RET; no result | Complete normal sequence and documented C++ argument/EH ownership domain |
| 00584A30 | `intro_movie_is_active_00584a30` | No native arguments, AL Boolean, RET | Complete name temporary and current-player predicate sequence |
| 004F8BA0 | `movie_named_clip_is_playing_004f8ba0` | ECX actual34h player, borrowed8h name on stack, AL Boolean, RET4 | Complete captured-widget/name/playback predicate through existing GUI projection |
| 00A85C20 | `request_sound_stream_fade_in_00a85c20` | ECX actual54h stream, RET; no result | Complete25-byte body |

The names are hypotheses, not recovered symbols. The existing names
`BSP_MainMenu_StartTitleMusic` and `BSP_FrontEnd_IsIntroMovieActive` are preserved.
No library functions are renamed. Source APIs add explicit services and are not
native ABI/SEH replacements.

005884A0 first calls 00584A30 even when a title stream already exists. The latter
resizes a native string to22 characters, copies `movies/midwaytheme.bik` with
its terminator, then reloads the current player at E18D48. 004F8BA0 captures
player+20 once, compares the requested header against that widget's filename
using the real length-first, case-insensitive 00435C40 helper, and calls the
existing AAC8E0 predicate only on equality. AAC8E0 requires a decoder and returns
the inverse of its current completed flag. The selected-widget producer is
004F8A20; its source filename comes from AAE320. Existing `MoviePlayer` and
`MovieWidgetState` are reused; no duplicate GUI layout is introduced.

If the movie gate is false and current menu+50 is null, the start body allocates
54h, then captures current menu+40 and copy-constructs the by-value name through
00426060. The table argument is an owned null reference. A877D0 consumes both
arguments on success or exception; the caller adds no extra name copy, table
retain or argument cleanup. The constructor loads the actual `.def` table via
the existing VFS resolver and scanner. Publication to current menu+50 occurs
only after construction returns.

The next temporary is constructed by A41E870 from the live C string at E19504.
After that allocation, the body reloads current menu+50 for A867B0. It destroys
the temporary before loading F889A8 through FLD/FSTP, reloads the current stream
for A864F0, then reloads again for A85C20. The menu provider is called six times
on the full path. Returning references from `GameTitleSoundMenuView` avoids a
copied canonical path or stream slot. The allocator is the existing BF681B
malloc/new-handler/throw service; its exhausted-allocation behavior does not
return null. The source retains the native null branch but does not invent a
successful null-stream policy.

A85C20 does not start playback. It tests unsigned state20 <=2 and fade-out byteA
equal to zero, then stores byteB=1 and floatC=positive zero. States3 and negative
bit patterns are unchanged. Its five callers use the title stream at menu+50,
except 0052EF75, which uses the credits stream at +54. All use only ECX and no
stack arguments. The four callers of 005884A0 likewise provide no stack
arguments; differing ECX values are irrelevant because that body never reads
the incoming register.

## Corrected evidence and ownership

The old host comment calling E19504 an8h native string is incorrect: 0058853D
pushes its address to A41E870, a C-string constructor. The cited alleged writers
0058BE63/0058BE6F and 0059A16B are fallback/address reads, not writers. The
inspected initial PE bytes are zero. The helper borrows this input without
claiming a recovered track-selection producer or substituting its own empty
buffer. Likewise, 0073DB76 reads F889A8 and writes alternate-owner+218; it does
not produce the music-volume global. 00685CA2/00685CC8 also read it. Its current
application publication remains required.

00686170 initializes the actual menu strings and null stream slots; 00686380
fills title path+40 with `sound/music/titlescreen.fsb`. This new helper's view
must bind those canonical fields. The old semantic `MainMenuManager` stores
string views and the old title wrapper captures its stream; those are not
sufficient to preserve all native temporary ownership and manager reloads.
Primary integration should call the full new entry instead of adapting only
the old null-returning methods.

The menu owns the constructed stream's initial reference. `update_stream` and
`stop_stream` forward the actual A874D0/A86BF0 bodies; final reference release
uses InterlockedDecrement at stream+4 and the actual D5B360/A87B30 deleting
route. The helper's destructor does not release a second hidden owner. Callers
release menu references before core shutdown. Stream tables use the same
strings, scanner and borrowed constants as the dialog facade, with the existing
unobservable first-channel scratch convention. BDF4C0 remains its documented
normalization/candidate fragment; no successful fallback is invented.

005884A0's D9B16C map arms allocation cleanup before path construction. State1
also owns the null table argument; it returns to state0 before A877D0 because
the callee consumes the arguments. C6FDD0 frees the allocation on failure;
C6FDDB clears the argument reference; state2/C6FDE3 destroys the selected-name
temporary. A failed initial string construction has no string cleanup state.
A start failure after stream publication preserves that owning slot, while
destroying the completed selected temporary. The 00584A30 map at D9ACBC arms
its string cleanup only after the literal copy. Existing throwing allocation
and noexcept string-release boundaries remain explicit; this does not emulate
Windows exception machinery.

## Verification and remaining bindings

All four owned native ranges have existing Ghidra functions; their final
instructions, inclusive ends and exclusive ends are recorded in the report.
CALL rows cover every direct call in the owned bodies, all callers of their
entries, and the relevant EH tails. Ghidra was read-only throughout. The source
uses existing actual string, table, stream and FMOD implementations.
Release Win32 `/W4 /WX` builds and both existing CTests pass after the standard
eight seed spans are verified. The29-row native CALL/tail audit has no failures.

One ignored manifested Win32 fixture combines an18-case original-byte A85C20
differential check with the installed `titlescreen.fsb` lifecycle. All54h bytes
match for states0/1/2/3/FFFFFFFF/80000000 and fade-out bytes0/1/FF. The installed
fixture reaches state2 with real sound/channel handles, a stereo looping table,
138032 ms duration and four successful speaker-level calls. Its explicit live
volume0.375 reaches the stream, and actual tick advances fade from0 to0.006
in the final run linked against this worktree's built core library.
Native stop and final reference release balance one open/one close with no
host reclaim, no pending adapters and no tracked strings. An injected by-value
path-copy failure preserves the null slot and string baseline. A second start
still allocates/releases the intro-name temporary before its existing-stream
early-out.

The fixture uses the real MoviePlayer constructor's null selected-widget state;
unreached codec/UI methods throw. It does not claim active Bink movie or GUI
playback validation. Menu path and stream fields are a documented fixture view,
not execution of the native menu constructor. The E19504 input uses verified PE
initial storage, and volume0.375 is an explicit fixture input, not a guessed
game default. Application current-menu/movie bindings, the current settings
volume and any track-selection producer must be supplied by integration.
Gameplay, original object ABI and SEH compatibility remain unverified.
