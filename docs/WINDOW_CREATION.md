# Native window creation fragment

`create_platform_window_00becee0_fragment` replaces the diagnostic probe's
handwritten class/creation setup with the recovered Win32 operations from
00becf29 through00bed087. It remains a fragment of the eleven-argument cdecl
CreateWindowAndDevice routine, not a reconstructed platform object or lifecycle.

The class uses CS_BYTEALIGNWINDOW (0x2000 stored at 00becf36; an earlier reading said CS_GLOBALCLASS, which is 0x4000), zero class-extra bytes, 24 window-extra bytes,
the supplied HINSTANCE and procedure, shared arrow cursor, and no icon, brush
or menu. The native procedure is00bec3b0. The new API requires an explicit
procedure; the probe explicitly supplies DefWindowProcA because the native
dispatcher still has unresolved activation/audio/UI/renderer dependencies.
The class name and initial title share the supplied ANSI name. Native null data
falls back to an empty byte at0109db8d; the port uses an empty literal.

The sequence adjusts `{0,0,width,height}` with WS_CAPTION and no menu, creates
a caption window with the requested initial X/Y and adjusted outer dimensions,
and forwards the supplied platform pointer as lpParam. Once CreateWindowExA
returns, the HWND output is stored before style changes, matching the native
store at00bed009. Width/height subtraction uses explicit 32-bit modular bits.

Windowed mode then sets WS_CAPTION and extended style0300h (WINDOWEDGE and
CLIENTEDGE), and calls SetWindowPos with null insert-after, X=Y=0, the adjusted
outer dimensions and SWP_FRAMECHANGED. It does not preserve requested X/Y at
this stage. Adjustment precedes extended styles; the port does not recompute
the outer rectangle to compensate for their client-edge effect. Fullscreen mode
sets WS_POPUP and TOPMOST, places at0,0 with unadjusted requested dimensions and
HWND_TOPMOST, also using SWP_FRAMECHANGED. No SHOWWINDOW flag is supplied.

## Ownership and incomplete initialization

Output HWND and class atom must initially be zero. The host guards those outputs
and requires a procedure. Native code ignores the Win32 results; the port reports
failures, retaining any created window/class for caller cleanup. It does not
reuse an already-registered class after registration fails. Callbacks must keep
their own lpParam object alive throughout any messages triggered by these calls.

The original routine first stores requested dimensions/application and invokes
its stop virtual when an old HWND exists. That branch is excluded; the host
supports a fresh window only. After this fragment, native code copies the title
into its string, calculates aspect and screen fields, sets frame enable, calls
ShowWindow, initializes the full renderer, and performs unresolved buffer and
power/screensaver setup. None of those are silently substituted by this API.
The probe's later device initialization is still its explicitly diagnostic route.

DestroyWindow and UnregisterClass remain explicit probe cleanup. They are not
claimed as the native destructor/stop policy: the audited native stop routine
does not destroy the HWND. The new fragment itself does not call ShowWindow;
the original visible-window step lies later in00becee0 and is not yet integrated.

## Evidence and validation

`reports/window_creation_audit.json` records a byte-for-byte installed-PE/saved-
Ghidra match for the351-byte fragment. Each analysis batch verified project
`bsp`, program `/battlestationspacific.exe`. The parent function's descriptive
name and evidence comment were updated, preserving previous comments and saving
the program; its export was refreshed.

The Win32 build and both existing CTests pass. The existing D3D9 probe queried
class style/extra bytes, window style, extended style, outer dimensions,0,0
placement and hidden state. All passed for the windowed path, followed by device
creation, isolated draw/readback checks, actual Lua material compilation/draw,
physical DDS loading and texture-byte comparison. The HWND and class were then
released by probe cleanup. No separate test target was added.

This validates the windowed setup on this machine with a diagnostic procedure;
it does not prove native message routing, fullscreen behavior, failure recovery,
visible-window lifecycle, shutdown or a runnable game.
