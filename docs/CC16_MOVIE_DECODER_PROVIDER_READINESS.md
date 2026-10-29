# Production movie decoder provider readiness

Reviewed main: 5c2547156c0d8d83d79f7b2600c6f6ea99cd975a. Pending locale evidence:
c30dd1e77e2b0d48338401e3f01d13f343cef4bb; it is read-only input here, not integrated
by this review. A complete production decoder/clock packet is not ready.
The shipped API exists, but no production import owner, factory composition
or concrete MovieDecoderHost supplies a real handle to the movie callbacks.

## Installed API: available, inspected without loading

The configured installed binkw32.dll is 178440 bytes, PE32/x86 (machine 014Ch,
magic 010Bh), SHA-256:
7f22055e8f62e228eb90f09846f9b668e0a0b1d29057af6e3b7bff1e08c0d724.
MSVC dumpbin read headers/exports and the installed executable's imports.
All eleven existing BinkImports entries plus _BinkSetSoundSystem@8 and
_BinkOpenDirectSound@4 match, 13/13. The report records the thirteen required ordinals/RVAs from the full
72-name export table. These are metadata and argument-byte decorations, not
DLL execution, codec-output or new runtime ABI proof. No vendor body was inspected
or reconstructed. The installed us_outro.bik exists (138540804-byte metadata);
it was not decoded.

There is no current GameModule/Bink loader implementation in this tree.
FmodConfigurationLibrary and XLiveLibrary provide real module ownership
patterns; NativeWindowFocusContext merely requires a supplied BinkPause
pointer and is not a module producer. A future Bink owner must resolve the
complete table from the selected installation's absolute path before
publishing consumers, keep the owned HMODULE through all handles/callbacks and
the shared drain, and release only on its normal ownership path. Interrupted
native work retains the corresponding providers until process exit. No
missing-export/default-import fallback or premature FreeLibrary is valid.

## Real in-house producer and lifetime contracts

The standalone factory constructor 00737710 requires same-owner singleton
registration 007366B0, stamps CFEA78 and invokes
BinkSetSoundSystem(BinkOpenDirectSound,0). The actual application inlines this:
0073DF9B PUSH 4, 0073DF9D allocator, 0073DFBC registration,
0073DFC7 table store and 0073DFCD Bink call. This proves a four-byte allocation
for this concrete startup producer, not arbitrary subclasses. Existing
MovieFactoryPrefix alone was not size proof.

Registration must use the real retained 00F8AEFC publication and shared manager/
captured section, including the current-publication reread. Normal deleting
leaf 00737950 invokes 00736750, frees the original owner iff flags&1 and returns
that owner. This production publication/deletion composition is missing.
Original table/code numbers are evidence identities, not callable host pointers.

Decoder factory 00737730 already allocates 38h, constructs complete flag1 and
returns allocation+10h. Retain the complete allocation separately from shared
MovieDecoderState; widget+F0 stores the shared pointer and its+4 is the real
Bink handle. Bink allocates its full handle; never allocate sizeof(BinkHandle)
or free it through the CRT.

Open 00A4CA40 copies a path, invokes exact 00BDD600 in place (Boolean ignored),
selects sound track 0 and calls BinkOpen(path,4000h). Ordinary null is false;
provider errors must retain their error/failure custody. Base open resets
time/completed before closing an old handle; close sets completed. Omitted
running/loop/volume/cached-duration resets must not be invented. Close logs,
marks complete, closes a nonnull handle and clears it, retaining filename/
texture. Destruction releases texture, closes and releases filename before
the complete allocation is freed.

MovieBridge still returns false from decoder_open and 0 from the missing-movie
dialog. Native Retry/Cancel flags 15h require result 2 to cancel, every other
result retries. That placeholder becomes an unbounded retry path if opening
is wired before the genuine dialog contract. Do not activate it.

The subtitle clock must call BinkGetRealtime(actual handle,38h output,0).
Decoder update uses mode1, cached frame duration and its own sentinel rules.
Neither that cached state.time nor a mission/render frame count, host delta,
or invented handle is the subtitle realtime producer. Preserve the actual
decode/wait/loop/close order through the existing typed routines.

## Renderer: genuine providers exist, admission still required

GameNativeRendererApplication::render_resource_providers() is ready-only and
returns the same runtime texture, surface, level-getter, holder and direct
terminal contexts. Borrowing creates no resource. The direct_terminals field
explicitly does NOT install itself in render_resources_lifetime(); producer/
lifetime admission must precede movie holders escaping to material/GUI users.

A concrete MovieDecoderHost must supply all real create/acquire/lock/finish/
release operations, preserve acquired-frame/native-survivor retention, and
keep those renderer owners alive. No zero surface, dummy texture, skipped
upload or success fallback closes that contract. Existing providers should
be borrowed rather than duplicated.

## Smallest ready dependency: exact filename replacement

The public GameVfsHost::direct_resolve calls 00BDD6E0, a separate input/output
operation whose normal false leaves output unchanged. The decoder requires
00BDD600: normalize the caller before probing, replace on success, and leave
the normalized caller on ordinary false. They cannot be aliased.

The actual kernel replace_native_vfs_file_name_00bdd600 is already complete.
GameNativeVfsRuntime::Impl already retains its lookup context, real physical
string pool and D683E8 profile; existing exists() uses the actual PooledHeader
domain. A bounded next packet can expose that exact operation through:

- include/bsp/game_native_vfs_runtime.hpp and src/game_native_vfs_runtime.cpp;
- include/bsp/game_hosts_vfs.hpp and src/game_hosts_vfs.cpp;
- address 00BDD600 for provider wiring, with zero new native-body credit.

All four files and the address were unleased at inspection; obtain fresh
leases. Reuse the same actual manager/context/string pool and proper owned
header. Copy the normalized name back on both normal exits; preserve Boolean,
provider exceptions and the existing retention policy. No aliases,
working-directory resolution, fabricated header or empty-path fallback.
The future MovieDecoderHost must ignore the Boolean as native open does.

Root should build with existing checks and choose exact found/missing mutable
filename evidence after freezing that candidate. Existing two VFS CLI cases
used another resolution/read path; they are not BDD600-specific proof.
This is one real provider dependency, not a complete decoder/clock closure.

A following Bink module/factory owner can use new bink_runtime_library.hpp/.cpp
and game_movie_factory_owner.hpp/.cpp, with primary-owned GameStartupHost
composition. Its thirteen imports are statically available. Actual factory
publication/deletion, module retention/error policy and bootstrap typing must
be specified before calling it production-ready. An unused import table
alone does not establish a caller or clock.

## Remaining blockers and primary handoff

The current bridge loads no page or child, returns a fallback widget state,
opens no decoder, and reports no open handle. Text/localisation/extratitles,
Lua owner/library/override/error semantics and real callback reachability stay
unclosed. MediaManager+14 needs its real settings application/publication;
settings+09 is not an alias. Locale c30 supplies only its separate table
reference and has not been turned ON.

Do not change arithmetic/ABI to work around those producers. Existing
00A4CA40 has concrete ECX=allocation+8 through adjustor 00A4C800 and RET0Ch;
use its established typed wrapper. kPc24MovieCc13Bound is true with a
display-only comment, not new gameplay proof. The decoder's 0109EEA4 conversion
selector is not automatically the GUI helper's cached OS SSE2 flag.
Any new register/vtable adapter, x87/NaN/control-state recovery or listing repair
requires the primary's max-effort review; none was recovered here.

The saved bsp.gpr and /battlestationspacific.exe were verified (77631 live/
snapshot functions). Only bounded wrappers and static source/PE metadata were
read. Tracked output is limited to this document and report, with ignored
local index/PE metadata scratch. No production code, source, CMake/ledger,
Ghidra or original installation changes,
DLL loading/execution, games, runtime probes, builds or tests were performed.
kMovieSubtitlesBound remains OFF. No activation, native-body, ABI or playback
credit is added. See the JSON report for hashes, exact exports and owner/API
contracts.
