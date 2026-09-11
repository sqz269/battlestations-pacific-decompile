# Profile and settings read integration

Addresses: 008d6dc0, 008d64a0, 007fefe0, 004425c0, 00441a20,
00bd3450, 00bd4380, 00b67980, 00b66de0.

`PcProfileIoHost` composes the PC storage backend, actual Lua archive readers,
mission-score ownership, settings and keyboard restoration, settings application,
profile commit and the existing synchronous/interactive continuation driver.
It implements both profile and settings reader hosts; its writer is the existing
`TextProfileWriteHost`. UI, platform, renderer and downloaded-content effects
remain required services. The host and its references must outlive callbacks.
These are new C++ interfaces, not native layouts or binary replacements.

## Reader and writer evidence

008d6dc0 is ECX=settings, one reader pointer, RET4 through008d76b6. It enters
Options, supplies archive defaults, publishes compatibility, enters keyboardSetup,
reads/applies existing input mappings, then reads the remaining options. Volume
defaults are1.0, swap-sticks defaults false, both invert defaults true, and safe
area defaults false; these differ from other reset paths. MotionBlur is read
twice. ClanText is presence-guarded; absent content preserves the old string.
DownloadedContent is cleared even when absent and reads consecutive indices
starting at0 until the first missing key. After leaving Options, renderer
virtual+104 result+28 below0x200 clears shadows+84/+85 and old-film+90. The
overlapping pseudocode temporary is resolved by008d734b and008d768a..008d769c.

Assembly008d6a20..008d6a56 proves ClanText has value tag0 and reads the pointer
at+B8, part of NativeString+B4. The earlier integer projection was wrong.
008d6a80..008d6ab8 writes DownloadedContent as index keys with string values;
the earlier implementation reversed them. Native keyboard output occurs before
HardwareReported; waterDrops, oldFilmEffect and MotionBlur have omission guards
for true,1,true. The writer and its required variant-key interface now preserve
these facts. Existing test-host adaptation adds no new permanent cases.

## Lua lifetime and nested callbacks

007fefe0 builds the globals reader, deserializes, frees the buffer, closes the
storage Lua, notifies the manager, restores settings, applies and commits, then
destroys the reader. The profile manager can also close the owner at the tail of
deserialization after the reader has left its child tables. 00b67980 marks globals
untracked;00b66de6 returns without touching Lua when object+10 is zero. This
explains the native order; there is no evidence for blanket reference invalidation.

`GuiLua51Host` supports a borrowed interpreter and represents global roots with
LUA_GLOBALSINDEX without a registry reference. Tracked children/cursors must be
released before close. A balanced reader can release only its global root after
close without accessing freed Lua memory. Borrowed hosts never close the state.
`PcProfileIoHost` keeps separate stacks of reader frames, allowing a callback to
open another reader without replacing an outer reader still awaiting destruction.

00bd3450, ECX=manager RET, samples PC virtual+8's literal true into available+21,
then assigns state+8 from existing error+20. It does not clear operation or prompt
fields. 00bd4380, ECX=manager with string pointer and DWORD kind, RET8, copies
name/kind, calls the recovered archive read, and mirrors error into state. Unlike
the update route it adds no marker write or prompt and leaves the operation code.

## Validation and follow-up

Combined validation is recorded in reports/profile_read_integration.json. Native
save files and installed scripts are read only; filesystem fixtures use isolated
local roots. Real archive/Lua checks do not establish engine rendering, native
object ABI, keyboard-device dispatch, or gameplay. Required follow-up contracts
include DoFile VFS/override dispatch, keyboard006aa090/00a93750, and game/platform/
renderer/content hosts used by the composed persistence path.

Validation at source34bd240: MSVC Win32 Release and both existing CTests passed.
The ignored combined fixture loaded copied native player/quick, exercised a
synthetic checkpoint through the real nested manager/storage path, wrote and read
both save files plus isolated options.txt, and checked keyboard dispatch, callback
order, string/index fields, sparse defaults and post-close global-root destruction.
All three worker fixtures also pass against the combined library. Twelve worker
artifacts are preserved under local/worker-validation/profile-read-20260910e.
Original native save hashes were rechecked unchanged. UI/renderer/device effects
are recorded fixture services, not gameplay or render proof. Initial CMake
registration was corrected to individual deferred entries; both new modules were
compiled and linked before these passing checks.

After merging the other orchestrators' input, sound, memory and texture work,
validation was repeated at8a5ad85: both CTests and all four focused fixtures pass.
Seventeen Ghidra names/comments are saved and verified by readback; all three
prior comment fields are preserved and all seventeen exports forcibly refreshed.
The exact logs and annotation change record remain in ignored local evidence.
