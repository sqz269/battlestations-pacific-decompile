# Live locale binding for the dormant subtitle bridge

Packet codex-movie-live-locale binds the schedule's language input to the
mission's same retained LocaleTables object. Base source is
5c2547156c0d8d83d79f7b2600c6f6ea99cd975a. The copied language argument used elsewhere
remains unchanged. kMovieSubtitlesBound stays OFF.

## Provider and source contract

The existing startup/menu chain passes GameLocaleHost::tables() to
GameMissionHost, whose Impl already retains LocaleTables&. Its sole frame
construction now forwards that same reference through the frame and result
constructors into MovieBridge. Only MovieBridge stores an additional
const LocaleTables&; there is no new locale owner or copied table.

Each schedule selection reads the current table's language() and compares its
nonnull c_str() to english with actual CRT _stricmp. The former unavailable
language_is_english_ Boolean field is removed. No initial summary string or
constructor-time value is cached. LocaleTables::set_language_00aa09d0 updates
the same language_ member, so a later genuine setter change is visible to the
borrower on its next selection.

The independent frame language argument still follows its existing copy/move
path for unrelated consumers. No GameStartupHost, GUI/Text/raw Model owner,
CMake or ledger changes are required.

The native contract is narrower than a new native body:

- 00AADBA0 receives its movie in ECX and ends with RET at 00AADE44.
- 00AADCC4 pushes english; 00AADCC9 selects current 00F8BC4C+4018;
  00AADCCF calls 00425850.
- 00425850 receives an actual NativeString header in ECX, a stacked C-string,
  returns the Boolean in AL and consumes 4 bytes. Its nonnull pair delegates
  to actual CRT _stricmp at 00BF7FBF. Existing src/native_string_compare.cpp
  preserves that comparison.
- This binding compares two genuine C-strings directly. It does not fabricate
  a NativeString header or claim the new C++ interfaces are native ABI entries.

Project C:/Users/sqz269/bsp.gpr and program /battlestationspacific.exe were
verified through the wrappers; live/snapshot function count was 77631. Bounded
show queries checked original Source and the register-input assembly. The
saved project and ledgers were not mutated. Names remain descriptive hypotheses.

## Lifetime and failure custody

The existing application owns the locale table longer than menu/mission
consumers. GameMenuHost forwards the same table into GameMissionHost. On scene
reload the mission resets the previous frame before replacing its Lua owner.
The frame owns the result/movie bridge; its Impl detaches the mission result
from Lua before member destruction. The application deletes menu/mission
before deleting the locale owner.

The new constructors only forward/store the reference; they perform no locale
lookup, logging, allocation of a table or ownership transfer. Destruction does
not clear, release or drain the locale provider. The VFS borrow and its source/
stream failure policy are unchanged. Reference lifetime is an explicit
constructor contract.

## Validation and retained limits

The standard scripts/build.ps1 MSVC Win32 Release four-node build passed.
Both existing CTest entries passed: reconstructed_math and tool_tests
(2/2, 11.97 seconds). No C++ source was edited after the build began;
the five recorded source hashes were rechecked afterward. The build log,
CTest log and generated artifacts are hashed in
reports/cc16_movie_subtitle_locale_binding.json. No new tests or framework,
game run or runtime probe was added.

This is build-checked source plumbing for one genuine provider. The default
OFF branch continues its existing callbacks/logging. The language-read branch
has not been consumed by an active movie, and no genuine language-change/
subtitle-playback runtime proof is claimed. The previous 13-pair/two-VFS receipt
remains exact 409 historical evidence; it is not validation of this new packet.

The other inputs remain unresolved: actual decoder/Bink clock, MediaManager
subtitle option+14 application/publication, real page/child callback reachability,
native Text/localisation/widget/extratitles ownership and delivery, and original
Lua library/override/error/owner semantics. No settings+09 alias, language
default, clock, decoder or widget stand-in is introduced. The guard remains OFF
until those genuine dependencies and root-controlled activation gates close.

No new native-function body, ABI, callback-reachability or playback credit is
added. Original install/save/profile files are unchanged.
