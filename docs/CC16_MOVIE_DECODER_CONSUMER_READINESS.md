# Movie decoder consumer readiness

Frozen source: `38e16c60111b0f0914f0a2f529e3324cd8845a1b`, including the
default-OFF movie-factory root packet. This is a read-only producer/lifetime
review, not admission of that root packet or a decoder consumer. A coherent
production consumer is **not ready**. The first bounded prerequisite is genuine
raw Movie default construction; the proposed scope is below. No source,
CMake, ledger, Ghidra, installation or frozen-peer file was changed.

## Actual caller and current boundary

`GameMissionFrameHost::Impl` creates `GameMissionResultHost(log, control, vfs,
locale)` and attaches it to the mission Lua host. `kLuaPlayBinkMovieBound` is
currently true; older OFF comments in these files are historical. The genuine
Lua dispatch row at `0089A480` reaches `lua_play_bink_movie_0089a480`, then
`play_named_movie_0089a390`, which prefixes `movies/` and records the
`004CD390` end-movie request. That is a caller of the mission-result protocol,
not evidence of Bink opening or displaying a clip.

Its `MovieBridge` receives only the retained log, VFS and current LocaleTables.
`ensure_movie_resources` only logs. `load_movie_page` and child lookup return
null; `movie_widget_state` returns one fallback semantic member. Decoder open
returns false, completion returns true, handle presence returns false, and the
other decoder/GUI operations log missing implementations. The missing-file
dialog returns zero: wiring open without the genuine Retry/Cancel boundary
would leave the existing retry loop retrying indefinitely (only result 2 exits).

`run_movie_completion_004f89d0` explicitly invokes the completion because no
movie player runs. Future frame completion must retire this substitution and
preserve the actual callback/mission-drain ordering; request/completion counters
alone cannot witness playback. Mission-frame destruction detaches the Lua
borrower, but supplies no decoder/page/texture retirement today.

## Native receiver and creation epochs

The checked native loaded hook is `00AAE770..00AAEB53` (996 bytes, 328 listing
rows), ECX widget, plain RET. Its beginning is:

| Native site | Actual operation |
| --- | --- |
| AAE78F | AA7170 deactivation/bounds callback on the same raw widget |
| AAE794 | Read CURRENT F8AEFC publication |
| AAE79A | Read CURRENT widget byte F4, after AA7170 |
| AAE7A1/A3 | Read current factory prefix, then its slot at +4 |
| AAE7A6/A7 | Push the full zero-extended byte and call that slot |
| AAE7A9/AD | Read current widget byte 74, then store returned shared pointer at F0 |
| AAE7BD/C0 | If byte74 is zero, call the returned decoder's current slot44 |
| AAEB2B..3F | After geometry/provider calls, read CURRENT FC length; call AAE320 with loop1 if nonzero |

There is no invented null-decoder guard in that sequence. Preserve these reads,
captures and stores if a raw entry is later admitted. The existing
`gui_movie_loaded_hook_00aae770` takes `flag_74`, `source_size_fc` and `index_f4`
**by value**. Supplying snapshots to that semantic helper cannot preserve the
native epochs when callbacks alter raw fields. It needs a genuine raw-storage
composition, not a cast or a wrapper around those snapshots.

Original CFEA78 has deleting entry 737950 and the +4 word 737730. A Source
creation bridge must validate the genuine retained Ready factory allocation
and current CFEA78/profile identity before composing the real
`create_movie_decoder_00737730`. Neither a numeric PE code token nor an arbitrary
subclass vcall is callable host code. CFEA60 is proved here only for its existing
base deleting route; adjacent bytes do not prove a base creation interface.
There is no current root-to-MovieBridge creation/borrow API. The owner already
exposes its same borrowed `decoder_imports()`, but the root services are private.

## Raw widget producer is still missing

AA6560's Movie branch tail-jumps at AA65B4 to AA1770. That factory requests
130h, obtains an actual Movie pool slot through AAD990/AAD6D0, then calls
AAFE20 for null source or AAFFF0 for copy source. `GuiMoviePoolTraits` has
134h slots: 130h object plus the trailing slab-index DWORD. That allocator
exists and does not construct the payload. Generic `GuiWidgetConstructionHost`
still requires the concrete constructors; no production implementation supplies
the raw Movie pair.

`GuiMovieFields` contains C++ string/vector members, and `MovieWidgetState` is
explicitly a projection whose offsets do not match the raw widget. Neither is
raw 130h backing. Their value initialization cannot establish native untouched
bytes or allocator words. In particular, the complete 142-byte/32-op native
AAFE20 calls real AA9390(type10), stores D5C270 and zeros only:

- DWORDs EC, F0, F8, FC, 100, 108, 110, 114, 118, 124, 128, 12C;
- bytes 104 and 105.

It **does not initialize F4**. It also leaves F5..F7, 106..107, 10C, 11C and
120 untouched, alongside other base/padding bytes not written by AA9390.
The historical `GuiMovieFields{}` helper clears F4 and constructs semantic
containers; its comment is not raw-byte evidence. The complete raw AA9390
implementation and its actual D7A24C input already exist.

AAD560 initializes F4 only after AAA710 and the property-reader callback. It
compares current size20 and size24 to the actual D7A24C word using
UCOMISS/LAHF/TEST/JP, admitting ordered equality only. The MovieName header is
the actual FC/100 pair. AAFFF0 also needs its actual AA9520/model-clone context
and native string pool; its native copy does not write F4 or 105. The existing
semantic copy resets them. These are raw-source/custody gaps, not new missing
native-body credit.

Normal AAFEF0 first stamps D5C270, invokes actual F0 slot8(flags1), and clears
F0 only after return. It then destroys/frees native 24h extratitle and 14h
subtitle records, releases the MovieName buffer through its sized pool, and
destroys the real base. The semantic `clear/shrink_to_fit` helper is not that
raw lifetime. A raw consumer needs the genuine vector/string/child deletion
providers and interrupted-operation custody before its slot can be reused.

Current `GameNativeGuiPageApplication` is childless-only and default OFF.
Its actual key visitor rejects every nonzero mapped widget type at AAAD89
before AA6560. Thus an existing genuine raw Page does not yet supply a Movie.
The full `NativeGuiWidgetPropertyChildrenContext` does require real factory,
read18/created74/loaded78 and matching Model/list/parenting providers; no
default-success implementation closes them.

## Existing decoder bodies and required providers

| Route | Existing Source and binding obligation |
| --- | --- |
| 737730 | Allocates38h in the actual singleton malloc/new-handler domain, calls A4C660(flag1), returns shared base A+10h; ignored stacked DWORD, native RET4. Keep captured A separately from S. |
| A4C660/A4CEE0 | Complete flag1 constructor and shared28h state exist; native table words/vtordisp describe data, not host callable tables. No arbitrary further-derived object claim. |
| A4CA40/A4CFB0 | Concrete open is ECX=A+8 via A4C800, RET0C. Reset time/completed, close an old handle, retain original filename, copy/BDD600-normalize path, select track0, BinkOpen(4000h), retain actual handle/null result. No invented running/loop/volume/duration reset. |
| A4D140/A4D1B0 | Release/create real texture (width,height,1,16h,1), DoFrame, acquire/lock/upload/finish/release actual surface, then NextFrame only when current is not last. No dummy/null-success texture or surface. |
| A4CD70/A4CC10 | Actual handle supplies Wait/GetRealtime(mode1); cached duration, sentinel and float-store/loop/completion ordering remain. Subtitle clock instead requires GetRealtime(actual handle,out38h,0). |
| A4CD30/A4CF30 | Log, mark completed, close nonnull actual handle, then clear handle; full state destruction also releases filename without clearing the native header. Close alone retains texture and filename. |
| A4C860 | A-based complete destruction calls A4C770 then tails A4CF30; no allocation free. |
| A4C820/A4C880 | Shared-state scalar route adjusts S to captured A=S-10h, destroys bases/state, conditionally BF65AC-frees A for flags&1, returns A, RET4. Use the matching actual malloc/free domain, never free S or a Bink handle with CRT. |

No production `MovieDecoderHost`, `MovieBaseHost` or
`GuiMovieLoadedHookHost` supplies this whole graph. The following are distinct
remaining producers/whole compositions, not functions to replace with stubs:

- **Root creation and module borrow:** a reviewed coupled root consumer API,
  real current factory/profile bridge, same thirteen-import owner and captured
  decoder allocation custody. The root guard is still false.
- **VFS/string:** exact `GameVfsHost::probe_and_replace_file_name` now exists
  over the same native BDD600 context and pool. It copies normalized output on
  both normal Boolean exits; decoder ignores that Boolean. A future host must
  retain that actual runtime/pool and propagate failure, not call BDD6E0 or
  fabricate a NativeString header over C++ string bytes.
- **Renderer/geometry:** ready-only `render_resource_providers()` exposes the
  genuine runtime-texture/surface/getter/holder/direct-terminal domains.
  Borrowing does not install direct terminals or admit their users. Movie
  creation/upload/release must compose them with actual material/node/quad
  buffers, lock outputs and matching reference-counted destruction. The loaded
  hook still requires its real mesh, vertex/index streams, draw section,
  material, widget50/58 and format providers. AAC820 must reread actual node
  material and decoder texture for each of slots0/1/2.
- **Settings/locale:** the same retained current LocaleTables is genuinely
  available. Raw settings+09 is loaded, but it is not MediaManager+14.
  The 18h native media producer/getter A4C5A0/4C1710 exists; A4C5A0 does not
  initialize +14. The actual quick-settings callback still logs missing
  `ui_set_subtitles_enabled`; MovieBridge's false member is not this producer.
- **Lua/Text/completion:** full raw AB0520/AB00C0/AADBA0 composition needs the
  original Lua ownership/library/override/error domain, native14h/24h records,
  actual text child/localisation, extratitle cloning/deletion and page lifetime.
  Current schedule parsing only projects subtitle rows in a separate Lua state;
  it is not that whole operation. Genuine widget completion may retire the
  owner: native AADF40 closes first, reloads F8 callback, invokes it, then makes
  no further widget access. Detach mission Lua/result borrowers before drain.
- **Arithmetic/error custody:** actual mutable CRT `feature_word()` exists at
  0109EEA4. No decoder host borrows it. The current decoder volume helper has
  finite/value-model and floating-state limits; a Boolean mode callback plus
  host double multiplication is not the exact ambient ST0/FTOL epoch proof.
  `kPc24MovieCc13Bound=true` is an existing qualified PC24 model, not permission
  to force FP controls or infer ambient precision. Keep these limitations until
  a concrete caller domain or exact ambient correction is independently proved.

## Module and dependent-owner ordering

The frozen root process keeper retains the Bink module/import tables, factory
owner, heap and deletion context even after normal flags1 factory retirement.
That prevents unqualified FreeLibrary; it does **not** retain future borrowed
renderer, VFS, Lua, page, texture or mission contexts automatically. Before any
decoder escapes, its owner must retain all of those providers and terminal
metadata. Close/destroy actual decoder resources and retire completion/page
borrowers before their providers and the factory/root shutdown. An interrupted
open/frame/close/delete must keep the entire reached graph and stop ordinary
drain before dangling receiver/profile reads. Do not retry, repair publications
or unload merely because a prefix or setup callback returned normally.

The primary's later installed-file sound audit is reported context, not an
independent/runtime finding here. It says SetSoundSystem may return existing
saved-callback status when a new callback returns null. Setup status and
`sound_returned` cannot witness a fresh open, callback installation/use, audio,
video frames or quiescence. Callback machine shape, semantic SDK contract,
original ABI/FH3/SEH/fault transport and runtime playback remain separate gates.

## Smallest bounded next packet

Propose **only** AAFE20 raw default construction, with new
`include/bsp/native_gui_movie_default_storage.hpp`,
`src/native_gui_movie_default_storage.cpp`,
`docs/CC16_NATIVE_GUI_MOVIE_DEFAULT_STORAGE.md` and
`reports/cc16_native_gui_movie_default_storage.json`, after fresh leases.
Suggested explicit API: actual fresh aligned 130h storage plus required
`const volatile uint32_t& actual_one_00d7a24c`, return the same storage. Reuse
`construct_native_gui_widget_base_00aa9390(raw,10,actual_one)` and preserve the
exact listed derived stores/untouched bytes. No fake raw class, value-initialized
payload, implicit F4 default, callback or table of PE functions is appropriate.
The existing pool is the real eventual AA1770 producer, not a new heap.

This is a prerequisite source repair, not an unused decoder wrapper advertised
as activation. Keep semantic APIs/consumers and all guards unchanged. Stop at
the raw constructor if its base/actual-cell custody cannot be satisfied; do not
claim raw AA1770/copy/destructor, Page child admission, loaded geometry or a
playback consumer from it. The following coherent consumer must jointly close
those named raw/provider/epoch/lifetime gates; it is not ready for a source
lease today. No new/duplicate/vendor-body, original-ABI or activation credit.

## Evidence and future runtime boundary

Target-verified wrappers used the configured `bsp` project
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86/image400000;
live/snapshot count was77631. Native constructor/copy/factory/property assembly,
loaded-hook creation/tail epochs and decoder deletion routes were checked.
Existing full decoder source/contracts were read; this is not a fresh full
instruction audit of every decoder or the 380-line subtitle operation.
The report pins the unchanged Source inputs, installed artifacts and native
listings. No build, test, probe, game, module load or imported API call ran.

Read-only inventory found136 installed BIK paths. Small candidates and the
actual us_outro file are hash-pinned in the report; existence/bytes do not prove
decoding. Future admitted gates need actual nonnull open, sound/callback use
under the accepted contract, real frame/decode/upload/presentation progression,
completion/skip and close/destruction before provider retirement, plus failure
custody. Static widths, setup status, synthetic clocks and request counters do
not replace those receipts. Keep the root and subtitle guards OFF.
