# Follow parameter-copy adoption — CC11

Packet `cc11_land_parameter_copy_adoption` connects the primary-proved complete
`009BE150` kernel to the existing ordinary Follow entry through an opt-in
abstract constructor-entry adapter. `NativeLandTaskParameterCopyConstructorCalls`
inherits the SAME borrowed task view and executable-profile requirements from
`NativeLandTaskStateEntryConstructorCalls`. Its final copy method directly calls
`copy_native_land_follow_parameters_009be150(destination, nullptr, source)`.
There is no guard, allocation, pointer replacement or self-copy shortcut.
All other constructor, singleton, field-map, reindex and observer services
remain required. This does not bind the task arena or existing game hosts.

Changed source: `include/bsp/native_land_state_entries.hpp` and
`src/native_land_state_entries.cpp`. The primary owns
`native_land_follow_parameters.hpp/.cpp`, kernel recovery and full integration
build. Worker Ghidra/metadata/CMake writes are absent.

## Fixed primary kernel and caller

Primary revision `2cd789583` supplies the public MSVC Win32 bridge and its
proof in `docs/PILOT_BOT_LAND_PARAMETER_KERNEL_CC11.md` and
`reports/pilot_bot_land_parameter_kernel_cc11.json`. The complete 87-instruction
body is `009BE150..009BE28B` exclusive, 315 bytes, SHA256
`48dfafc30042b9594b4ee2dbcee1ab2252b8039676f27a6134b1fabd9601cf1a`.
Native ECX holds destination, source is on stack, EAX returns destination,
RET4 removes source. The source fastcall bridge reserves unused EDX. The
primary independently compared the original installed-PE body with the actual
production TU: 144 cases, zero failures, all instruction bytes equal. Its
mixed IEEE, x87 control/status/stack, disjoint/self-alias and padding proof
is routine-level evidence, not game or constructor validation.

The existing caller `009BED80..009BEE25` calls the actual singleton at `BED84`,
then loads existing state+6Ch at `BED89` and calls this operation at `BED92`.
After the copy it reloads +6Ch and captures state+4 before its stores. Those
loads, the fixed amplitude FLD/FSTP kernel, fresh cached squadron after reindex,
captured leader/observer order and final +85h clear are unchanged. The new
adapter ignores the returned destination, as this caller does.

The generated C++ method pushes its received source at +00, loads received
destination into ECX at +04, zeros unused EDX at +08, calls the primary kernel
at +0A and returns from the C++ two-argument method with RET8 at +0F. That method
and the inherited constructor-entry class have new SOURCE ABIs; they are not
the original task profile or a drop-in task constructor.

## Actual parameter storage and alias domain

Storage audit of `009C2980..009C2A55` observes singleton call `009C29DB`,
`ADD EAX,380h` at `29F5`, then publication to existing state+6Ch at `29FC`.
This constructor allocates no parameter block. It installs actual Follow
profile `00D20AB8`, embedded observer profile `00CF89B4`, initializes observer
array fields and watched +2Ch, and has a private EH/random-x87 tail. The
complete constructor and its other fields/services remain unbound; the worker
does not implement or recover a new numeric/register-ABI contract for it.

With unchanged singleton publication, entry destination and source are the
EXACT SAME block. The copy must execute: signaling NaNs can quiet and ambient
x87 status can change. If publication changes, the old captured destination
and fresh source must both remain live; this adapter cannot retain an old
singleton or repair a dangling pointer.

Each passed block exposes A4h valid bytes; destination is writable and source
is readable. Exact self alias and disjoint spans are admitted. Partial overlap
is outside the primary-validated domain. Forty binary32 FLD/FSTP lanes execute
at offsets `00h..64h` and `6Ch..A0h`, with raw byte copies at `68h/69h` between
them. Holes `6Ah/6Bh` are neither read nor written by the kernel. No Boolean
normalization, raw struct-copy replacement, padding clear or tuning default is
supplied. Existing `GameTuningBlock` describes singleton+380h through +423h;
its +3EAh/+3EBh padding corresponds to these untouched holes. That layout does
not supply actual singleton publication, construction or lifetime binding.

The primary numerical domain requires masked x87 exceptions, no pending
unmasked exception and one available stack slot. Ambient control and existing
stack values are preserved; status follows the actual operations. Invalid
spans, partial overlap, unmasked traps, asynchronous faults/private unwind,
structural reentry, concurrency and death/observer lifetime assumptions remain
outside the caller's ordinary contract.

The outer task's separate plane+3FCh and ONE canonical squadron+404h cells,
state identities and executable profiles remain the same borrowed storage.
The adapter creates no translated cache. It does not implement lower
`009F9CE0`/`009AFE70`, complete composite `009B2E50`, base `0099C6F0`, queue
producer `006C0B50`, actual Follow constructor `009C2980`, controller, arena,
scalar destruction, deferred owner scheduling or routing.

## Focused connected verification

Fresh actual kernel, entry and outer-constructor TUs were compiled using MSVC
Win32 `19.51.36244.0`, `/O2 /MD /std:c++17 /EHsc /DNDEBUG`; the primary kernel
also uses `/Gy /W4 /WX /fp:strict`. The ignored manifested source fixture links
those fresh objects with pinned existing support libraries using
`/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF`. The new copy method is final: the
fixture removes its former partial 12-byte copy override and uses the real
production adapter and kernel.

The one existing scenario now uses caller-owned A4h fixture blocks with exact
offset/size assertions. Its outer-to-Follow path models a constructor-cached
old block and changed singleton publication: destination/source are disjoint.
It checks all 164 bytes against expected forty lanes, copied raw bytes68/69
and destination holes preserved against different source padding; source
bytes remain unchanged. The initial-null-squadron branch then models normal
unchanged publication and exact self alias, including a signaling payload
`7F800001` in a lane outside amplitude+8. The fixed primary operation quiets
it to `7FC00001`, so a self-copy shortcut fails. All lanes/bytes/holes are
checked again. Existing fresh state+4/+Ch, observer replacement/null/unchanged
branches, Park/MoveTo/profile guard, aliases-after-entry, final +85h and x87
control/TOP checks remain passing. The older hook/cruise/validity/canonical/
outer fixture also still passes.

Constructor/singleton/reindex effects, source executable tables and controlled
state-field replacements are SOURCE fixture instrumentation, not native
constructor implementations or observed runtime reentry. Existing actual
observer-prefix/register/unregister source executes on explicit fixture
storage with an already-published lock. Its real world profiles, context and
lifetime binding remain required. This connected fixture is not a native
whole-caller differential test.

The fresh kernel COFF symbol's 315 bytes independently match the primary's
original-PE extraction and the SHA256 above. Support libraries are pinned
under `local/cc11_land_copy_bsp_{core,lua511,zlib121}.lib`; exact hashes are
in the report. No full worker CMake build or new tracked tests were added.
Focused live verification checks five Follow call rows plus the two constructor
dependency call sites, with the outer profile+4 row explicitly indirect.
JSON parsing and staged diff checks pass. Original full task ABI, arena,
singleton/private constructor, queue, observer/death lifetimes and gameplay
remain unbound despite the complete adopted copy kernel.
