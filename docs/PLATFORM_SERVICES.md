# Platform and XLive service composition

Addresses: 00becb20, 00bece70, 00beccd0, 00bec1a0, 00beca40, 00a409f0, 00a40110, 00a3f3e0, 00a3f440, 00a3e600.

This batch reconstructs 29 normal game-function bodies across cursor policy,
input focus, notification dispatch, XLive pumping and cached sign-in handling.
It supplies concrete composition classes for the existing frame/load message
loops. The SDK is called through the original imports, not reconstructed.

`PlatformServices` implements both PlatformLoopCallbacks and ResourceLoadEventHost.
Pretranslation calls XLive ordinal5030. Its frame runs 00BECE70, which calls the
bound application frame and then cursor policy with loading=false. The existing
load-message pump drains messages then calls that same policy with loading=true.
ShowCursor and mouse cooperative-level operations call actual Windows/DirectInput
APIs. The policy preserves native display-count loops and reload points.

Lookup, reset and backend polling all read the same published input pointer.
The separate InputFocusResetHost supplies only its genuine lazy action-manager
getter, so mismatched backend pointers cannot enter through that interface.
The backend owns its canonical fixed slots and active vectors; no shadow tables
or fabricated device registrations are created. ApplicationFrameService binds
an actual owner identity to its existing frame fields and host.

`XLiveManagerRuntime` composes the notification drain, cached-user/debounce
helpers, online pump, real clock sampling and XLiveSdkAdapter. It binds +120 and
+3BC directly to pump storage. The profile poll and asynchronous UI call share
one durable seven-word output block. The debounce timestamp's frequency low
word remains the existing OnlineSystemState.field_10. Indexed cached-state
loads after +8C use the same username bytes, including their defined-byte mask;
they are not a separate user-state array. The library must outlive all adapters
and pending operations. Supplied state requires established constructor values.

The SDK adapter forwards 15 operations; the notification/pretranslation adapter
adds five original imports. Every ordinal was checked against the original game
IAT and both installed DLL export tables. Calls keep their Win32 stdcall stack
shape, full XOVERLAPPED storage and required buffer lifetimes. Descriptive game
names remain hypotheses, and these C++ object layouts are not the original ABI.

Validation includes the combined Win32 Release build, both existing CTests,
eight native seed matches, and a link probe proving the composed host classes
are concrete and their referenced methods link. Focused ignored fixtures cover
cursor transitions, reentrant mouse reset, notification ordering, asynchronous
pump transitions and sign-in timing/overlap. The notification implementation
also received an independent assembly review. These fixtures use explicit
recording hosts and do not establish hardware, overlay or gameplay behavior.

Ghidra now has four formerly missing input entries. Five false-no-return call
continuations were repaired without changing any callee's no-return flag.
All 29 reviewed functions were annotated and saved, and their exports refreshed.
The incorrect standalone-STL classification of 004BA6D0 was removed, preserving
its old tag in reports/orch3_input_classification.json. See the per-packet docs
and reports for original ABIs, branch evidence and uncertainty.

Actual XLive loader probes remain unsuccessful. Microsoft's installed DLL fails
with error182 and has an unsatisfied msidcrl40 ordinal43 import. The installed
Team Wolfpack replacement, loaded from a hash-identical isolated copy, exits
before LoadLibrary returns. Its failure cause remains unresolved; neither DLL
was patched or replaced. No account or network initialization was attempted.

The executable host is owned by another orchestrator and was not edited in this
batch. It does not yet invoke this new service composition. Remaining work is
to establish a working, appropriate XLive library environment; recover the
manager constructor and asynchronous cancellation/lifetime; bind actual device
enumeration and lazy input ownership; and supply real game localization,
callbacks and update-path helpers. Application-frame dependencies and its
previously documented unwind boundary also remain. No runnable-game or gameplay
completion is claimed.
