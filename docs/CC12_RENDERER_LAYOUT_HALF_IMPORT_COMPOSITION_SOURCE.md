# Renderer layout and half-float service composition

Root admitted the bounded Source change after the fresh 509-input Win32 build and emitted review. Current validation and limits are in docs/CC12_SELF_REFRESH_RENDERER_SOURCE_PRIMARY_REVIEW.md and its JSON receipt. The worker candidate statements below describe its earlier unregistered/unbuilt capture. Whole receiver/model, Native ABI/EH and gameplay admission remain separate.

The renderer application now retains the existing concrete section-layout service
and half-float reader in the same ownership graph as its live renderer, geometry,
declarations, pools and strings. Ready-only accessors borrow those persistent
members. Interrupted acquisitions and surviving layout companions both prevent
the existing normal singleton-drain path.

This is a Source candidate at base a44121db9dc9a936b323dc0004ad3ffaff0987e5.
It changes four existing C++/header files plus this document and the companion
report. No separate provider-view header or .inc change was needed. No Native
body/address, callback, registry, ID, fixture resource or original ABI is added.

## Actual construction and aliases

| Retained member | Concrete existing arguments | Lifetime |
| --- | --- | --- |
| NativeD3dx9Float16Import vertex_half | devices.d3dx | The existing DeviceGraph system d3dx9_40.dll stays loaded through every import use. The wrapper resolves D3DXFloat16To32Array, not the particle reverse conversion. |
| GuiTextNativeLayoutServices section_layouts | texture_loading.geometry, graph.vertex, declaration_loading, devices.hardware, profiles(0x00d62af4) | The same renderer cell, geometry/owner registry, declaration pool/profile/type tables, strings and hardware tree survive the service and every native layout borrower. |

The members follow GameGridGraph in Impl, after every direct provider. Their
initializers are adjacent to that graph at
src/game_native_renderer_application.cpp:332-334. The existing layout constructor
validates same-domain identities (src/gui_text_native_layout.cpp:61-75), including
renderer publication alias, actual owner registry, declaration/type tables and
the same string wrapper. No copied publication or substitute service is supplied.

The new section_layout_services() and vertex_half_import() accessors return
references to these members. They require renderer phase ready and layout
acquisition phase empty or transferred. They only check and borrow; they do not
create a temporary wrapper, acquire a native layout or rewrite operation state.
The existing game_grid_context().graphics/strings, publication_00f8d394() and
actual_owners() expose the other already shared renderer prerequisites.

Borrow checks occur at accessor entry. They do not lock the renderer or revoke a
reference already handed out. Callers must serialize use against startup, frames,
device recreation and drain, and retain the application plus every borrowed VFS,
string, singleton/support, pool and data owner. Callers must retire their actual
layout references before the shared renderer drain.

No copy or move is introduced. The application and half-import ownership retain
their existing deleted-copy/nonmoving restrictions; the layout service remains a
unique_ptr owner with a user-declared destructor. A reference is not ownership and
must never outlive the application's stable Impl.

## Failed work and surviving successful layouts

The layout service already records factory and registration phases before the
corresponding operations. Its existing destructor rejects interrupted acquisition
or a nonempty companion list. A completed transfer can still leave live native
references, so checking the acquired phase alone is insufficient.

The new const noexcept has_live_companions() query returns only
!impl_->entries.empty(). It performs no allocation, retirement, native refcount
inspection or state reset. It also avoids dereferencing acquired.companion: that
bookkeeping pointer can outlive actual companion retirement after a transfer.

requires_process_retention() now combines its existing phase/shader/shadow reasons
with both incomplete layout acquisition and any surviving owned layout companion.
The existing drain_singletons() precheck rejects before singleton shutdown.
GameStartupHost's existing retention policy sees the same result and exits the
process while retaining the graph instead of invoking ordinary destruction.
No rollback, failed-frame replay, replacement service or synthetic release is
added. Existing destructor invariants still apply to callers that bypass the
retention/drain contract.

| Renderer/layout state | New accessor result | Layout reason to retain process |
| --- | --- | --- |
| Ready; empty acquisition; no companion | Borrow | No |
| Ready; transferred; no companion | Borrow | No |
| Ready; transferred; live companion | Borrow; existing successful layouts remain reusable | Yes |
| Ready; factory/registration; either companion state | Reject | Yes |
| Any renderer phase other than ready | Reject | Existing application phase rules and layout guards apply independently |

Callers remain responsible for actual reference retirement/unbinding until the
canonical zero-reference disposal removes every entry. Retention protects a missed
retirement; it does not perform that retirement. The accessors deliberately do not
reject a successful transferred acquisition solely because companions are live,
so the existing service can be reused while valid layouts remain in use.

After every native borrower is retired, reverse member destruction destroys
section_layouts and vertex_half before texture geometry, streams, DeviceGraph,
hardware/declaration providers and the canonical registry. DeviceGraph unloads its
D3DX module after vertex_half is gone. The new constructors bind an existing import
and allocate/validate Source layout metadata only; they do not start native layout
acquisition, so provider members also outlive constructor unwinding.

## Evidence and remaining admission

Before edits, the published Source121 authority was replayed at
2026-10-09T21:26:49.319126Z: all 121 selected Root raw inputs and four artifacts
matched, and all worker inputs matched after CRLF/LF normalization (119 raw matches;
two older report newline differences). The receipt records 37 selected whole
objects, 41 selected Core roots and three existing checks. Thirty-seven is not the
number of all Core members.

All four changed C++/header files are outside that selected 121 manifest.
A continuing 121-pin match therefore cannot validate this new composition. The
companion report separately pins all four changed source files, their base-commit
preimages, bounded dependency pages, the authority receipt and replay capture.
The previous binary/build evidence gives this candidate no compilation or runtime
credit. Root must run the normal Win32 build and inspect the changed application
and layout-provider emission/linkage, including any newly linked existing Core
dependencies.

The published controlled Source121 startup receipt records three ticks, two
Presents, exit 0, zero mission frames and PressStartPoll, using genuine bundled
XLive with isolated 640x480 windowed personal settings. Its separate installed
fixed-address XLive override and default-fullscreen runs failed. This packet did
not rerun those observations; they are bounded prior startup evidence, not this
candidate's runtime proof, faithful startup completion or gameplay validation.

Complete owned headers/C++ and every changed line were reviewed; member order,
actual domain arguments, ready checks and both pending/live-companion retention
paths were inspected. No worker build, tests, probe, runtime, object recapture,
CMake, ledger or GPR work was performed. Root owns the normal build, emitted review
and bounded smoke.

Actual 711BE0 binding remains unadmitted. This change creates no unit model_360,
model+160/holder+0C hierarchy, raw atlas, model descriptor, executable current-node
callback or numbering-operation owner. It supplies genuine retained renderer
prerequisites for those later producers, with no original ABI/FH3, faithful
startup or gameplay/visual-numbering credit.
