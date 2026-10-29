# Subtitle source integration review, 2026-10-29

Approve the default-OFF source/evidence integration against main
ad524e7dc72e037a07f9fc62cc4e4c176e756736, subject to the integrator's normal build and existing checks after
pending Dyn integration. No source correction or textual conflict is required.
The completed gameplay receipts retain their original frozen 409 scope.

## Exact source and integration

Reviewed source commits are 609ef74158b525c1d860bd5351dbfd195ce2689e and
4561c11dc8f3971f8e70ac899942cbaf185eaea4. Candidate source is f49e064d5e393c5ad8d0dfdd54d686b5669b2585;
runtime metadata freeze is 063c4be212b2e475a706b65d2f4ca067fe2656f0; final receipt is 1e7078eafa460a5a3f00ff67703f8ab050b33895.
The worker/main merge base is 409689e5e813d4aaaeb656abeb827d06a5842016.

All five modified source files have identical baseline 409/current-main blobs.
The cumulative delta from 409 to the finalized worker branch contains exactly
eleven paths: those five source files and six source/VFS/OFF evidence files.
Both source-only and full cumulative patches passed read-only git apply --check
at checkout 5f686b664. Main then advanced only two Dyn modules and their CMake
registration; all eleven affected paths are identical at reviewed main ad524e7dc.
No patch application, index mutation or source edit was performed in this review.

Merge the finalized agent/codex-nongui-backlog branch, or import its exact
eleven-path cumulative delta. Cherry-picking 1e7078eaf alone is insufficient:
its OFF evidence files were introduced by e9df37911 and updated by 063c4be21.
Retain that history or import the complete final files. The f49 merge itself
need not be replayed onto main, which already descends from 409.

The switch remains constexpr false. The three OFF callbacks retain their
original logging; added constructors only retain the same VFS reference.
Actual exists/open calls remain in the disabled branch. The Lua helper adds no
global objects, static constructors or registration. CMake already compiles
movie_subtitles.cpp and links Lua; no CMake edit is needed.

The 609 document's working-directory probe and the two initial reports'
runtime_pending statuses are historical checkpoint statements. A useful plain
cross-link is: "This records 609. Its working-directory probe was replaced by 456;
see CODEX_MOVIE_SUBTITLE_VFS.md and CODEX_MOVIE_SUBTITLE_OFF_GATES.md."
Do not rewrite the initial receipts as if they had tested the later source.

## What the retained evidence establishes

The final receipt records 13/13 fresh exact pairs against baseline 409, candidate
f49 and runtime metadata 063, plus two separate VFS CLI checks. Twenty-eight
fresh runs used the same E41 common XLive backend, original plans/environment/
masks and Normal priority. Pair, ordered-event and teardown differences were
zero; start frame/RNG matched. Old and new subtitle callback markers were absent.
The rejected artifact-path attempt remains preserved without acceptance credit.

The VFS CLI opened installed us_outro.mws with a complete initialized 97-byte
extent and rejected the unique absent path with expected exit 3. The installed
file hash remained unchanged. That proves the tested provider CLI path, not
MovieBridge callbacks, a returned-buffer digest or subtitle playback.

Twenty actual provider/interface files are unchanged from 409, including the
VFS/open/read graph, memory stream, movie/decoder/factory code, locale/settings
and native MediaManager context. This is a narrow source justification for
retaining the historical evidence while landing the OFF packet.

Main nevertheless has 91 source/build-path deltas from 409, including GUI/font/
Text/raw Model/host, ship-AI/settings-tail and Dyn changes. Its binary is not the
tested 409 baseline. Neither the 13 pairs nor the two VFS checks may be relabelled
as current-main gameplay or VFS runtime proof. Current-main static review and
the integration build are separate evidence. Any later edit to the packet or
provider paths needs another bounded review; current-main runtime claims need
a separately frozen current-main run set.

No ON, full 00AB0520 body, new native-body, ABI or playback credit follows.

## Ranked next packet

1. **Live locale reference: a genuine small closure ready to implement.**
   After integrating the source, claim 00AADBA0 host wiring and these five files:
   include/bsp/game_hosts_mission_frame.hpp, src/game_hosts_mission_frame.cpp,
   include/bsp/game_hosts_mission_result.hpp, src/game_hosts_mission_result.cpp,
   src/game_hosts_mission.cpp. GameMissionHost::Impl already retains the real
   LocaleTables& at mission.cpp:501; its load path is the sole frame constructor
   caller at:2666. Forward that same reference through frame/result to
   MovieBridge, then read language() on each subtitle selection. Preserve the
   frame's existing copied language argument for its unrelated consumers; the
   movie path receives this additional shared reference without a cached value. The original
   English comparison 00425850 uses actual CRT _stricmp for nonnull strings.
   Reuse that semantic comparison on the genuine table string; do not fabricate
   an actual NativeString header, cache the initial summary language or supply
   a false default provider. The existing parent locale lifetime encloses the
   frame/result/movie borrowers. No GameStartupHost or active GUI/Text/raw Model
   owner-file edit is required. The current lease inventory has no overlap
   with these proposed files/addresses; obtain fresh leases before work.

   Keep OFF. Build normally with existing checks; root chooses any matched
   exact-source OFF gates. Actual language-change/English-hide playback proof
   belongs to later ON work after all genuine providers and callers close.
   This packet does not supply settings+14, decoder time, widgets or extra bodies.

2. **Actual MediaManager settings publication: GUI-owner handoff required.**
   The complete 004C1710 getter and 00A4C5A0 constructor exist, but the GUI manager
   application retains its separate 00F8AEF8 media cell null. The constructor
   does not initialize +14. The real settings apply caller 008D5B50 must deliver
   settings+09 through ui_set_subtitles_enabled to that same owner's+14 before
   it is read. The concrete profile settings override at
   src/game_profile_load.cpp:368 explicitly reports that provider as missing. Do not alias settings+09 to a different object, invent a global,
   or interpret uninitialized bytes as a recovered default. Coordinate the
   same-context publication and 00A4C3F0/shared-drain custody with the GUI owner.
   The null argument in make_text's bindings is a locale comparator, not a
   MediaManager pointer; it is not evidence for this gap.

3. **Factory/decoder custody before a clock-only packet.**
   No production BinkImports binding or concrete MovieDecoderHost exists.
   MovieBridge page/child lookup is null; decoder_open/is_open remain false.
   The native 00AAE770 loaded hook calls factory 00F8AEFC virtual+4 and stores
   the result at movie+F0 (assembly 00AAE794..00AAE7AD). 00AADBA0 loads that
   decoder's handle+4 and calls IAT 00CE23B8 with (handle,buffer,0), before the
   IMUL/FILD/unsigned-correction/FDIV sequence 00AADBCD..00AADC0A.
   Close factory 007366B0/00737710/00737730, real Bink module/import lifetime,
   decoder 00A4CA40/00A4CD70/00A4CD30 and actual renderer/widget resource custody
   before wiring time. 00737710 requires actual BinkSetSoundSystem at 00CE23D0
   with BinkOpenDirectSound at 00CE23D4 and zero. MovieFactoryPrefix does not prove
   the full factory allocation size. Future codec-owner files can be disjoint,
   but a fake handle, host frame clock or partial transport is not a closure.

4. **Native widget/extratitles/Lua and callback reachability remain required.**
   The GUI owner must supply real Text/localisation and cloned-widget lifetime,
   including extratitles virtual+54/+50 calls. The original Lua library,
   override/error/owner contract remains uncertain. Current compiled raw Font/
   Text work does not establish that movie callbacks or these ownership paths
   execute. Keep kMovieSubtitlesBound OFF until those dependencies and genuine
   activation gates close; add no native-body or ABI credit.

## Review provenance

The wrapper verified saved C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe; live/snapshot count was 77631. Bounded lookup/show/
callers queries and assembly were read without Ghidra mutation. Source/patch
hashes, the 20 unchanged-provider hashes, exact receipt scope and proposed
ownership/gates are in reports/cc16_movie_subtitle_integration_review.json.

This review ran no games, runtime probes, builds or tests. Tracked output is
limited to this document and report; ignored per-checkout index/patch-check
scratch was generated. Original installation, saves, profiles, ledgers,
CMake, shared source and the frozen 409 trees/artifacts were not changed.
