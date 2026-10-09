# Self refresh, renderer services and category-header Source review

The primary review admits one complete 13-byte constructor, one qualified
40-byte Lua refresh fragment, and the concrete renderer service composition.
The normal MSVC Win32 build passed all three existing checks. A focused real
Lua 5.1.1 case and a controlled D3D9 startup smoke pass. Whole unit receiver,
resource selection, model/atlas ownership, Native EH/ABI and gameplay remain held.

## Admitted changes

| Change | Evidence | Scope |
| --- | --- | --- |
| 00928B73..00928B9A self refresh | Actual getter returned pointer, tracked assignment, distinct temporary destruction; 130B/47-op emitted Core root and indexed C++ catch/rethrow/EH reviewed | Ordinary C++ fragment; not the full 634B parent or a production attach consumer |
| Renderer layout and half import | Retained existing layout service in the same geometry/string/declaration/hardware/owner domain; existing system D3DXFloat16To32Array binding | Two ready-only borrowed views; no model, atlas or numbering owner |
| Live layout retention | Pending acquisition or live canonical companions reject shared drain; successful live layouts remain reusable | Caller retirement required; no rollback or replay |
| 00952640 category-header constructor | Complete 13B/six-op emitted body equals Original bytes, zero calls/relocations, unique Core definition | Actual three-DWORD storage; no array iterator/destructor or whole receiver |

The Lua case retains old self, replaces thisTable[key], forces destination
release to move the fresh tracked index, then verifies the current persistent
address/count, opaque bytes, replacement identity, actual Ptr setter and cleanup.
Its embedded owner/world placement is explicitly fixture storage. No exception,
Lua longjmp, hardware fault, Original handler or Original body was executed.

The fragment's emitted main body calls getter, assignment and normal destructor
in order. Its separate C++ catch calls the real destructor then rethrows; normal
destruction is outside that catch. Indexed xdata records retain one catch-all try
region and two states with no separate unwind cleanup. This is the qualified new
C++ error contract, not recovered Native FH3 equivalence.

The renderer constructor binds both retained services after their dependencies.
Normal reverse destruction retires the layout service before geometry, streams,
hardware and DeviceGraph. D3DX stays loaded while the half import is alive.
Emitted accessors check ready plus empty/transferred acquisition. Retention also
checks the real service's companion list. All 204 preexisting layout-provider
functions keep their code and resolved relocation targets; only the 11B/four-op
live-companion query is new. Nonempty/failure layout paths and float conversion
were inspected statically; the smoke exercises construction and empty drain.

## Build and physical evidence

The latest build pins 509 selected project inputs: Source121 plus recursively
included project files for eight explicit changed/new/provider translation units.
This is not a complete application dependency manifest. Four artifacts are pinned.
There are 43 selected whole Core objects plus one application object and 51
positive Core definitions. All 37 previously selected objects are byte-identical;
the five added provider objects and application object from the preceding
Source507 build are also unchanged after adding the category constructor.

Whole section bytes, physical symbol-table indices, archive member payloads,
relocations and reachable C++ EH records are retained locally. The existing UV
reader's executable section has an unclassified four-byte tail (05050504): its
whole bytes/relocations are captured, but no complete disassembly is claimed for
that tail. Every admitted root and the application object decode completely.
The new refresh and category constructor roots are absent from the app map;
the refresh is exercised by the focused probe. Real layout/import constructors,
acquired/live queries and normal retention are linked in the application.

Source121 and Source507 inputs/artifacts are frozen separately. Old receipts are
historical; they must not be replayed against now-current artifact paths.

## Controlled startup and remaining proof

The latest executable used the genuine bundled XLive SDK from the retained
private extraction, plus an isolated ordinary windowed options file. It created
a 640x480 window/device, ran three ticks, presented two frames, skipped the initial
reset Present, completed the loop and exited zero with normal final COM releases.
It stayed at PressStartPoll, injected no input and ran zero mission frames.
No screenshot or visual/gameplay validation was performed. The installed fixed
address XLive override and default fullscreen conditions remain separate earlier
failure observations; no original executable or installed DLL was replaced.

Locked GPR annotation preserves prior parent names/comments and Original bodies.
The previously missing 00952640 function was defined over exactly its 13 bytes,
named provisionally and saved; total/internal functions become 64730/64288.
The snapshot/index and affected exports were refreshed. No recovered-symbol or
whole-parent ABI claim follows from that annotation.

The separate readiness packets retain the shared-base array/destructor/FP/EH
frontier, genuine class-resource selection before unit+360 publication, and the
model descriptor's exact original initialization placement. None gains new Source
or gameplay credit from this build. See the companion primary JSON receipt.
