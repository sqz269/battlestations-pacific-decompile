# Application streamed-dialog composition

`bsp::game::GameSoundDialogRuntime` connects the existing sound runtime to the
recovered actual 234h alternate owner, 5Ch logical channels, 54h streams and 20h
tables. It owns the alternate allocation and supplies concrete services for its
update and destruction. This application class has no original image address,
native object ABI, vtable or new reconstruction claim.

## Construction and borrowed services

Construct the facade before `GameSoundRuntime`. Supply it as the core's
`alternate_shutdown`, bind `update_alternate` to `dialog.update(pointer, dt)`,
and supply the same external alternate publication word to both. After core
construction, call `dialog.attach(core, core.fmod())`; start the core, then call
`dialog.startup()`. Attachment and startup are each one-shot. Core shutdown or
the shared singleton drain can delete the dialog owner through the facade.
The facade's later destructor does not repeat native member destruction.

The facade borrows the core's VFS/sample scanner, strings, current sound-owner
word, installed FMOD adapter and `SoundLifetimeAccess`. It creates no separate
lifetime manager. The application supplies references to the actual format
counts at E12EF0, one bits at D7A24C and fade-rate double at CE3DC8, plus the
fallback C string at 1090AB4. Live analysis and installed PE bytes establish
counts `{0,1,2,6}`, one bits `3F800000` and the double represented by
`00 00 00 40 33 33 D3 3F`. The initial 1090AB4 byte lies in the PE loader's
zero-filled data region. These are required borrowed inputs, with no invented
immutable replacement inside the facade. The F8BBEE fallback aliases byte 2
of the existing core's live F8BBEC storage.

The table parser's temporary first-channel word is unobservable scratch: after
copying it to an appended row, the recovered parser overwrites that row word
before any further external call. The facade supplies scratch zero; this is
not a substitute for a global or a claim about unwritten native stack bytes.

Callback230 is allowed to remain null in the actual owner. Publishing a callback
requires `GameSoundDialogCallbackHost`, which receives the current callback
token and actual ECX string temporary. A reached callback without its mission
translation throws. Mission request producers and the callback implementation
remain outside this packet.

## Concrete routes

| Facade service | Existing implementation | Native boundary |
|---|---|---|
| Alternate startup / deleting call | A79230 / A790D0 | ECX actual234h; deleting flags byte, RET4 |
| Alternate update | A789C0 | ECX actual234h, float dt, RET4 |
| Logical update / start | A78820 / A783F0 | ECX actual5Ch; dt and gain, RET8 / no stack arguments |
| Stream construction | A877D0 | ECX actual54h; owned by-value8h name then retained table pointer, RET0C |
| Start / row gain / gain / tick | A867B0 / A86670 / A864F0 / A874D0 | Existing actual-storage stream interfaces |
| Stream stop / scalar delete | A86BF0 / A87B30 | ECX actual54h; RET / flags byte, RET4 |
| Table load / scalar delete | A87060 / A796F0 | ECX actual20h, name pointer / flags byte, RET4 |
| Stream filename resolution | Existing BDF4C0 VFS fragment | Mutates the actual8h header; constructor ignores AL |

The consuming stream constructor receives exactly the name and table owners
prepared by A783F0. The facade adds no copy, retain or argument destructor.
Current native profiles select final-reference handling: D5B360 invokes stream
scalar deletion with bit 0, D58F80 invokes table scalar deletion, and D5B074
uses the existing sample runtime. Unknown current profiles throw. The logical
configuration's sample-reference interpretation follows its existing producer
chain, including 004ED2D0, A83FD0 and 004E7BB0; it is not inferred from field
shape alone.

The concrete BDF4C0 implementation uses the existing normalization and ordered
VFS candidates, preserving mutation even on a false result. It is explicitly
a fragment: diagnostic logging and full native ABI equivalence remain outside
its coverage. No fallback filename or successful resolution is synthesized.

The source routes reuse [logical](SOUND_DIALOG_LOGICAL.md),
[owner](SOUND_STREAM_OWNER.md), [runtime](SOUND_STREAM_RUNTIME.md),
[table](SOUND_DIALOG_TABLE.md), [alternate](SOUND_ALTERNATE_OWNER.md) and
[core](GAME_SOUND_RUNTIME.md) contracts. The primary integration owns their
native annotations and CALL evidence. This packet makes no Ghidra mutations.

## Ownership and failure boundary

The facade tracks the allocation independently of the mutable publication.
Explicit shutdown unregisters that allocation before A790D0, matching the
manager's A882C0 consumer order. Scalar flags with bit 0 clear release the native
members but retain the allocation; the facade later frees it without invoking
the destructor again. Reentered teardown and unknown owner profiles are errors.
A failed native teardown poisons the facade; destruction terminates instead of
silently reusing partially destroyed storage.

Failed startup does not invoke an incomplete derived destructor. After the
native constructor's own unwind, application cleanup removes the captured
allocation from the shared registration, clears publication only if it still
names that allocation, and frees its 234h storage. A79230's native late raw
allocation boundary remains: failure after constructing the physical pair or
first logical channel can leave those earlier allocations alive. The constructor
does not initialize all tail pointer fields before that sequence, so generic
facade cleanup cannot safely inspect them. This packet does not claim balanced
allocation ownership for every constructor failure.

## Initial verification and EOF investigation

Release Win32 builds use `/W4 /WX`; both existing CTests pass, including 456
native math comparisons. The standard eight seed spans match the installed
executable. No permanent tests were added.

The ignored fixture uses the installed FMOD DLLs, existing physical VFS/Lua,
real XLive load-event service, QPC clock and shared raw singleton manager.
Constants are read from the installed PE with explicit loader zero-fill for
the fallback byte. It stages actual logical configuration fields because the
mission request API is not part of this packet. Core startup selects the native
no-sound output. Ticks enter A87BF0 and its actual alternate-update service.

Installed `sound/music/Chance.fsb` and its real `.def` produce a stereo row,
163272 ms duration, state2, and nonnull sound/channel handles. Real speaker-level
calls succeed. Native stop clears both handles, drops the physical reference,
and the existing stream restarts through the logical branch. Both explicit core
shutdown and raw singleton drain balance two opens/two closes, all tracked
strings and all core sample/resource/class ownership.

The initial EOF refinement exposed a core/library ownership
boundary. With looping disabled, seeking the real stream near EOF reaches
native state3 while retaining sound and channel words. One fixture reference
allows observing those words after the manager releases its logical and
physical references; transferring that reference back to the logical slot lets
normal native dialog teardown release the object. The captured FMOD sound
remains live afterward. Leaving it to core EventSystem teardown, rather than
releasing it in the fixture, currently leaves two file opens and one close.
`Sound_GetLength` still succeeds after EventSystem release; tracked strings,
resources and classes are empty and FMOD reports no errors. The post-release
query is diagnostic only: a stale allocation can still return cached metadata,
so it does not prove that the sound allocation remains live after EventSystem
release. The callback-count imbalance also needs independent close tracking
before attributing it to an unclosed file. The failed check is
preserved in the report and ignored log. No production workaround hides this
source state3 retention edge. Full EOF cleanup and gameplay validation are
not claimed by that historical run. The integrated correction below supersedes
its unresolved host-adapter ownership assessment; the earlier observations
and failed log remain evidence of the installed SDK's behavior.

## Integrated host cleanup correction

The primary integration now preserves the native state3, Sound and Channel
retention while completing ownership of its own VFS adapters. The installed
EventSystem release returns success but emits only one Close callback for two
Open callbacks in this EOF case. `GameSoundRuntime` tracks only the adapters
created by its successful host opens, forwards the real EventSystem release,
and captures its result. After successful release has returned, it invokes
the existing A7B750 close helper on any remaining tracked adapters. These are
host VFS handles, not FMOD Sound or Channel pointers. It never releases a dead
Sound or changes the recovered stream stop/destruction sequence. A failed
release cannot authorize adapter reclamation or callback/library disposal.

`file_closes` continues to count actual SDK Close callbacks. The new
`file_reclaims` counts additional host adapter cleanup, and
`file_handles_pending` reports still-owned adapters. The ownership check is
`file_opens == file_closes + file_reclaims`, with pending equal to zero;
reclamation is not reported as an SDK callback.

The integrated fixture in the primary checkout, recorded in
`local/sound_dialog_facade_integrated_z.log`, now passes all three paths:

| Installed fixture path | Opens | SDK closes | Host reclaims | Pending | Tracked strings | FMOD errors |
|---|---:|---:|---:|---:|---:|---:|
| Stop/restart, explicit core shutdown | 2 | 2 | 0 | 0 | 0 | 0 |
| Stop/restart, raw singleton drain | 2 | 2 | 0 | 0 | 0 | 0 |
| EOF with native retained handles | 2 | 1 | 1 | 0 | 0 | 0 |

The EOF fixture proves handle retention before SDK teardown, then lets the
core perform real EventSystem release and host adapter recovery. It neither
queries nor releases the stale Sound afterward. The earlier post-release
GetLength result remains a diagnostic observation, not proof of a live SDK
allocation. All three runs end with empty sample, resource and class ownership.

This is installed-library fixture proof for the concrete core/facade composition
and its host cleanup policy. Application startup/render-loop wiring, mission
request producers, a nonnull callback230 translation, and gameplay validation
remain outside this packet. The native late-allocation constructor boundary
and BDF4C0 resolver fragment limitations are unchanged. This documentation
correction changes no source, native annotation or ABI claim.
