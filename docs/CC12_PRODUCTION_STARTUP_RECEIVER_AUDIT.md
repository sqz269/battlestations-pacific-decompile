# Current production startup and receiver audit

The current reconstructed Win32 executable exits **0** after three startup ticks,
with a real D3D9 device, **two successful Presents** and one skipped initial Present.
This is a bounded startup smoke result, not faithful startup or gameplay completion.
The run remains at the press-start page and executes no mission frames.

## Runtime cause and controlled progression

The executable is the unchanged Source121 artifact, SHA-256
47b7f1b50976229168c952497f760c5282c7b88d2bc966b51635f3baadf4ba39.
The launch uses the Original installation for input and checkout-local personal
settings. No installer runs and no original executable or installed DLL is replaced.
Launcher startup information requests hidden windows; no visual parity is claimed.

| Controlled observation | Result | Scope |
| --- | --- | --- |
| Installed XLive override, default temporary settings | C0000005 | Stops after settings, before sound initialization. |
| Debug observation of that override | Write fault in xlive.dll+319049 to 10640F5E | First/second-chance AV registers, bytes, stack and loaded modules retained. First-chance invalid-handle events were continued for diagnosis only. |
| Verified genuine bundled XLive SDK, fresh temporary settings | Exit 4 | Sound initializes; fullscreen D3D9 CreateDevice fails with 8876086C in the current remote session. |
| Genuine SDK, loaded temporary Fullscreen 0, one tick | Exit 0 | Device created, initial Present skipped during reset. |
| Same ordinary windowed settings, three ticks | Exit 0 | 640x480, two Presents, one skipped Present, normal logged COM release. |

The installed override identifies itself as an XLiveLessNess adaptation for BSP.
Its fixed-address write into the reconstructed image is incompatible with this
executable; the current debugger evidence agrees with the existing fixed-patch
contract report. Its bytes remain unchanged. The selected replacement is the
genuine SDK previously extracted from the bundled gfwlivesetup package. All four
runtime files and the bundled redistributable hash were verified this turn.
The existing CLI selects their absolute paths and dependency files; there is no stub.

Windowed mode is an ordinary Native options-file setting in a separate temporary
directory. The real loader validates the supplied resolution to 640x480. No code
patch, forced renderer success, fake device-lost poll or copied renderer is used.
The command, process handles, terminal exit codes and complete logs are pinned
in the companion report. The initial unsupported --help request returned 2 and
is excluded from startup evidence.

## Fidelity and production receiver boundaries

The successful trace still reports unimplemented online, renderer-resource,
GUI node/model/layout, loading-queue and shutdown paths. Those trace labels and
the frontend bridge remain limitations; successful presentation cannot establish
Native object-model ownership, full startup parity or gameplay equivalence.
No screenshot, input/control, combat, model/animation or mission validation occurred.

The three independently reviewed receiver packets establish 1,008 Native bytes /
285 operations / 40 calls and eight profile-cell bytes. The actual Lua publisher
passes its unchanged incoming receiver as Ptr. Current app publishers instead
produce IDs or semantic records. The LandFort allocator/derived constructor
establish size and final profile/C4 stores, while base construction, descriptor,
world/publication and teardown remain separate ownership dependencies.

The self-table normal path also contains a real getter/assignment/destructor
refresh missing from the abstract Source sequence. A separate 40-byte fragment
can compose existing real providers; a semantic LuaObject copy cannot implement it.
The model-service audit identifies a concrete renderer layout/half-float import
composition, but actual unit model, raw atlas, model descriptor and fresh executable
node dispatch must still be produced and retained before numbering admission.

The goal remains active. This turn changes evidence and readiness, with no new
C++ implementation, build, complete Original ABI or gameplay admission.
