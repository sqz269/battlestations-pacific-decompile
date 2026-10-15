# Native platform host adoption 34

Normal startup now allocates an actual 184h-byte platform through the canonical
Source allocation boundary and calls the existing complete `BECDA0` constructor.
The retained creation context is also the canonical `D68CC4 -> BECE30` deletion
binding. It borrows the VFS's same raw string/manager cells and owns one
`0109CF04` Source publication cell. Renderer construction borrows this exact
cell; the GUI effect probe no longer requests the legacy focus-only publication.

The supported caller site is `73D8D1..73D8F3`: allocate 184h, retain the result,
skip construction on null, otherwise enter `BECDA0` on that allocation. This
Source composition does not establish the whole `73D410` private/FH3 frame or
its failure unwind. An entered failure retains the graph and prohibits replay.

## Storage and producers

`NativePlatformObjectStorage` describes the known native offsets and explicitly
reserved untouched ranges. It has no constructor, destructor or host vtable.
Actual allocation bytes are never value-initialized. The host captures those
bytes, prepares its metadata views and verifies that metadata preparation did
not change any incoming byte before entering the native Source constructor.
In particular, the signed incoming `+24/+28` x87 operands are not replaced by
the later 640/480 stores. The original x87 Source body is unchanged.

The `Win32PlatformState` facade references the actual storage's same fields.
Window configuration and messages therefore publish height at `+28`, HWND at
`+30`, focus at `+41`, loop flags at `+42/+43/+181` and the other recovered
scalar fields directly into that owner. Native title resize/copy uses its
actual `+4/+8` header and raw pool. `BE2B10` owns its sole native return; the
Source view's destructor does not release that header.

Standalone compatibility views keep independent unregistered projection bytes.
Their copy/move behavior remains value-based; actual-owner views reject copies
and retargeting. The compatibility storage is never a second constructed,
registered or published native owner.

## Text queue

The host prepares a borrowing `PlatformTextInput` facade, with its enable and
clipboard references bound to `+170/+44` and its queue bound to the same
`+174/+178/+17C` header. No private sentinel is allocated for the actual host.
Append uses existing complete raw `BED370`. Valid-node pop uses existing
`BEC990` with the real process invalid-parameter provider and canonical free
domain. Enable/clear preserves `A965A0`'s store, detach/count-zero and current
sentinel reload order. Native `BECA00` owns queue/sentinel retirement.

The typed pop API still returns false on empty rather than invoking the
Original CRT invalid-parameter handler. Checked-iterator/register ABI and
invalid-storage behavior are not claimed. The standalone compatibility queue
retains its earlier private-node behavior and its documented failure boundary.

`BSP_NATIVE_PLATFORM_QUEUE_PROBE=1` exercises one focused actual-owner case:
enable, append two synthetic Source events, pop one, same-state enable/clear,
then disable. It verifies the actual header count and unchanged sentinel and
restores disabled/empty state through those Source operations. It does not
inject Windows input or test a real text editor.

## Native drain and window cleanup

Before either normal or fallback shared drain, the host captures its HWND and
disarms its message adapters. Canonical scalar deletion can then free the raw
platform while the Source facade and callback/window-extra metadata stay alive.
After the real manager and platform publications are null, the host closes the
binding without reading retired raw fields. Final window destruction uses the
captured handle, followed by facade metadata destruction. No extra native
platform deletion, title return, queue clear or owner free is introduced.

The power-policy producer remains at its existing explicit activation boundary.
`BECA00` performs its established unconditional screensaver-enable call; it
does not invent a power-stop/restore or HWND-destruction schedule.

## Verification and boundaries

The strict Win32 build and three existing suites pass. The existing platform
probe also passes, including one new compatibility copy/move independence case.
The complete 96-byte sampler bridge remains identical in its frozen probe,
current object and linked executable, with no padding or relocations.
The fresh windowed application run on Source
`e87607619542493b7d6185968f48a1c33af2b970` exits zero with the actual queue
probe, GUI acquisitions and POST cold/warm acquisition enabled. Its frozen
build contains 139 application objects and the normal complete libraries.
The machine-readable receipt is
[`cc12_native_platform_host_adoption_34.json`](../reports/cc12_native_platform_host_adoption_34.json).

The one observed allocation produces profile `D68CC4`, dimensions 640x480 and
one self-linked sentinel with count zero. Its captured incoming `+24/+28`
words are both zero, unchanged by metadata preparation. The unchanged x87
constructor stores aspect bits `FFC00000`; zero inputs were observed, not
seeded or required. Window configuration then produces height 600 and a live
22-byte native title on that same owner. Height/handle/text/loop alias checks
and the renderer's exact publication-cell identity check pass.

The actual-owner queue case passes append2/pop1/same-state clear/disable while
retaining its sentinel. Three qualified GUI/POST compiler observations retain
the previous shader2-to1 and device23/23,25/25,27/27 checks. POST warm lookup
uses the same effect with no fourth compiler observation. Caller references
retire normally; the shared native drain clears platform and manager
publications, joins the renderer worker and reaches final device/API COM0/0.
The two-iteration loop presents once and skips once. The closure log's
`raw_reads=0` is a marker for the inspected metadata-only path, not a memory
instrumentation counter. The runtime still reports 43 unimplemented host
methods, including native Text buffer and section production.

The window procedure still uses its Source C++ view transport, and platform
`+48` is the existing Source application callback owner. This does not admit
the Original complete window procedure, application vtable or binary class ABI.
Source window creation still has its documented API/failure boundaries. Full
`B24FB0`, native Text/font/widget host activation, drawing, visual parity and
gameplay remain separate requirements. The new real height publication removes
the focus-only receiver prerequisite; it does not itself execute Text selection.
