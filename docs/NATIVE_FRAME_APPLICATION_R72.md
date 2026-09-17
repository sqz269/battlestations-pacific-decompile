# Application native frame binding R72

Addresses: 00b2b200, 00b2d8e0, 00b2f4a0, 00b21430, 004c11f0, 00b1f280, 00b1f330, 00b1f6b0, 00bd0400

## Change

The executable's frontend draw is now surrounded by the existing complete source
BeginFrame B2B200, Clear B21430 and default EndFrame B2F4A0 -> B2D8E0 providers.
Previously GameDeviceHost issued its own Clear/BeginScene/EndScene/Present sequence.
This packet composes recovered bodies; it does not reconstruct the remaining
FrontEndFrame scene/resource/command producers or move them into original order.
The background color and frontend overlay remain executable-side choices.

The stable private FrameGraph borrows the R69 renderer/reset/binding/cache/physical
owner domains, real D3DX module, R70 entry-cache publication, canonical raw strings
and raw singleton manager. It owns the actual F8D440 queue publication. The complete
4C11F0 getter constructs/registers it, and the R71 complete scalar destructor is
bound into the raw manager before the first getter. Drain requires both queue and
entry-cache publications to become null and observes the native worker joined.

XLiveRender is the actual loaded xlive.dll ordinal5002, borrowed from SoundServices.
An isolated probe called that real import before online startup and observed S_OK;
the application also reaches it in every tested native end frame. This proves
neither online initialization nor a complete online lifecycle. The idle native
worker now has the same substantive begin/end contexts, but is not started by this
packet; its end context has no main-thread Present observer.

The F8D39C resource service remains null, taking BeginFrame's actual null branch.
The current frontend produces no native queue commands or debug records; their
full providers retain the existing empty-count branches. Nonempty paths still
require real execution/model/camera contexts and persistent child frames. No
success callback substitutes for these missing domains. Queue allocation preserves
the B1F280 preimage read at +20, including the possible stop/ack wait when it is 2;
no pre-zero or timeout changes that native behavior.

## Observation and host behavior

An optional source-only, non-aliasing Present observation records the HRESULT
returned by the one actual COM call. Native return/branch/global semantics remain
unchanged; this is not original ABI or native storage. The application counts
successful presents separately from native skips. A failed reached HRESULT counts
as neither. The --frames exit check accepts completed loop ticks in those two
categories, preserving its loop-count meaning without inventing a successful
Present for the native startup inhibit branch.

The source frame phase is retained during frontend work. A begin/end exception or
an interrupted frontend/capture prevents normal raw drain, using the existing
process-retention policy. No synthetic native rollback is introduced.

Live Ghidra/PE bytes verify E1306C starts at 1 and 108D4CC at loader-zero. Both
become stable FrameGraph cells; the original first-frame clear is preserved.
The existing diagnostic screenshot now runs after frontend drawing and before
native EndFrame, so later debug/XLive/clear-request operations can change the final
pixels. It is explicitly a frontend capture, not a final-display parity claim or
the unbound native asynchronous screenshot worker.

## Evidence

- Strict MSVC Win32 /MD /W4 /WX /fp:strict build and three existing CTests pass.
  No repository tests were added.
- Eight function spans plus profiles/scalars: 1,852 live bytes equal the original
  PE/loader bytes. B2D96A's six-byte LEA EBX,[EBX] alignment is jumped over at
  B2D968; no flow repair is needed. Direct call rows are mechanically checked.
- Real pre-online XLiveRender probe returns 00000000. Exact private SDK hashes,
  executable hashes and application logs are recorded in the report/local archive.
- Two-frame title run exits 0: inhibit 1 -> 0, one native skip, one real S_OK
  Present, same empty queue, zero entry usage, queue/cache publications cleared,
  worker joined, final retained device/API releases both return zero.
- Eight-frame press-start run exits 0: one skip, seven S_OK Presents; ScreenVisible
  state5; same clean raw drain. Frontend capture shows the menu frame/header with
  an empty interior. The title capture shows the map and sign-in prompt. Neither
  capture establishes complete interactive UI or gameplay.
- The first candidate also completed two frames and drained cleanly, but exited 1
  because its old launch gate required two Presents. Its logs/image/hash are
  preserved under local/application_r72/initial_present_accounting.
- Current session1 was active on the console, with three D3D9 adapters and successful
  HAL caps queries. The prior R70 disconnected-session failure is not current proof.

## Remaining boundaries / follow-up

Bind the substantive resource-service lifecycle and nonempty command/debug/model
domains before claiming the original frontend frame. Online startup, active worker
production/concurrency, runtime reset/device replacement, failed-Present behavior,
native FH3/SEH, async native capture and gameplay remain unproved. Current finite
source profile mappings and source contexts are not binary ABI replacements.

Report: reports/native_frame_application_r72.json. Artifacts are scoped local
provenance, not a redistributable original game or complete reproducible toolchain.
