# Actual platform window and configuration frontier

This is an evidence and readiness packet on `58f2320c8`, after the raw
388-byte construction/retirement adoption. It adds **no reconstructed source
body** and does not activate a window, Text or the application. The complete
BECEE0 body is 938 bytes; the existing projections and provider bodies do not
yet compose into its actual ownership contract. Machine-readable evidence is
in `reports/native_platform_window_configuration_frontier.json`.

Fresh live Ghidra and installed-image comparison covers 13 spans / 2,506 bytes,
including BECEE0, the 36-byte BEC3B0 thunk, the 946-byte BED3B0 handler, the
37-byte BEBF70 stop, caller/allocation spans, profile slots, message tables,
x87 constant and EH metadata. Four saved listings equal their live listings.
Of those bytes, 2,505 are file-backed; the one-byte empty-string fallback at
0109DB8D is the PE loader's zero-initialized virtual tail. Its live zero is not
an on-disk initialized byte. No Original code or system-setting API is executed.

## Eleven original stack arguments

The 0073DBC7..0073DC2D caller reads current 0109CF04, dispatches D68CC4+4 to
BECEE0, then adds 2Ch to ESP. BECEE0 ends with RET at BED289. It is a cdecl
entry with an explicit owner; ECX is not its receiver. Each row is a DWORD slot
at entry ESP plus the shown offset.

| Argument | Stack | Meaning and caller source |
| --- | --- | --- |
| 1 | +04 | Current actual platform from 0109CF04; same 184h-byte allocation |
| 2 | +08 | Pointer to the temporary native 8-byte title header, not `char*` |
| 3 | +0C | Zero-extended byte 00F8899E; branches consume low byte, renderer receives captured whole DWORD |
| 4 | +10 | Caller tests byte 00F889E0 and pushes SETNZ 0/1; fullscreen stores this word at owner+18 |
| 5 | +14 | x from 00E1AE88 |
| 6 | +18 | y from 00E1AE8C |
| 7 | +1C | width from 00F88994 |
| 8 | +20 | height from 00F88998 |
| 9 | +24 | renderer option from 00F889D8 |
| 10 | +28 | Application owner, captured by the caller into EBP |
| 11 | +2C | WinMain HINSTANCE from 00E1AE7C |

The body stores requested width/height at +34/+38 and application at +48
before testing HWND+30. If that HWND is nonzero, BECF27 calls the same owner's
slot+8, proven D68CCC -> BEBF70. That path conditionally restores the captured
scheme, clears +42 and posts WM_QUIT; it does not destroy the old window.
Construction's incoming +24/+28 bytes remain allocation preimage, not defaults.

## Window, title and x87/store order

BECEE0 registers style **2000h (CS_BYTEALIGNWINDOW)**, 24 extra window bytes and
BEC3B0 as WNDPROC. It uses the current argument-2 data pointer for class/title,
with the actual 0109DB8D empty fallback, and passes the actual platform as
CreateWindowExA lpParam. It writes the returned HWND to actual owner+30.
Registration, rectangle, creation and subsequent style/API results do not add
failure branches. The existing real-window helper has additional failure
returns and therefore is a reviewed fragment, not a full-body replacement.

At BED088 the destination title is the real 8-byte header at owner+4. Pointer
self-assignment skips work. Otherwise 41DD40 resizes from the current source
length with preserve=1; the body rereads source length, and if nonzero copies
the **current destination length** from current source data to current
destination data. This cannot be replaced by `std::string::assign(char*)`.
The raw pooled-string implementation already supports the required allocator
domain and header access without cloning the header.

The desktop RECT is not pre-zeroed by Original. GetClientRect failure leaves
the actual incoming stack bytes; the current projection zero-initializes it.
Desktop width/height subtraction wraps as DWORD, then the following x87 order
uses signed FILD/FIDIV with the current control word:

1. BED0D9 loads desktop width; BED0E7 divides by desktop height.
2. BED0F2/F5 store requested width/height into the same owner's +24/+28, and
   BED0F8 stores the low fullscreen byte at +0C, **before** the first FSTP.
3. BED0FB rounds to a float stack slot, BED0FF reloads it, and BED103 stores
   desktop aspect at +3C while retaining ST0.
4. Windowed mode pops that value, then FILD/FIDIV requested width/height.
   BED118 rounds the selected aspect into the original argument-8 stack slot,
   overwriting that slot; BED11F reloads it and BED126 stores +10.
5. BED129 loads actual D5BD98 bits `3FAAAAAB`, then FXCH/FCOMIP/FSTP and JBE.
   +0D is 1 only for ordered greater-than; unordered goes to zero. +14 records
   requested 1280x720. These are byte stores, not DWORD stores.

The existing `aspect_ratio_x87` helper expresses a double division and float
conversion. Its projection-level result is not proof of the whole instruction
schedule, stack alias, control/status effects or fault-visible store order.
No new aspect preimage or substitute arithmetic is supplied by this packet.

Fullscreen stores argument 4 at +18 and zeros +1C/+20; windowed mode calls
GetDC/GetDeviceCaps(BITSPIXEL)/ReleaseDC and stores original x/y at +1C/+20.
It sets +42 before ShowWindow. At BED1B7 it reads current F8D394 and its slot+4,
proven D5F0AC -> B2AEB0. The ten renderer slots, in order, are current HWND+30,
captured argument 3, current width+24, current height+28, 15h, 1, argument 9,
4Bh, argument-4-nonzero and 0. The height comes from the owner after the x87
code has overwritten the old height stack slot. B2AEB0 consumes RET28h.

## Providers that exist, and ownership still missing

| Domain | Current concrete evidence | Remaining requirement |
| --- | --- | --- |
| Platform lifetime | NativePlatformConstructionContext, actual184h construction and D68CC4 retirement | One canonical CF04 cell and this exact allocation across all consumers |
| Title | `resize_native_string_header_0041dd40` with NativeStringRawPoolContext | Borrow the same AA0/AA8/AA4 cells and actual owner+4; preserve live rereads |
| Renderer startup | Complete `initialize_native_renderer_device_00b2aeb0` and its actual device/reset/default-surface providers | Invoke at the original boundary on current F8D394; no already-started-device substitution |
| Render cache | Complete BED1E8 fragment with actual14h owner, FE88 publication, AA0 manager and D7A24C literal | Use that same context after device startup and before power changes |
| Win32 creation | Real Win32 helper and reviewed window fragments | Actual raw WNDPROC/handler ownership; preserve ignored failures and synchronous re-entry |
| Power | Correct typed import surface and reviewed projected producer/stop schedule | Raw +4C/+50/+E0 storage in the same184h owner; no projected policy owner or copied fields |
| Messages | Reviewed focus, activation, control and text projections | Full actual BED3B0 receiver/ABI and raw queue append domain; projections do not supply it |

`GameNativeRendererApplication::DeviceGraph` currently declares a separate
`const void* volatile platform_publication`, initialized from the constructor's
pointer value. Reset and loading-focus consumers borrow that local cell. It
does not alias a caller-owned canonical 0109CF04 reference. The current
44-byte focus storage recovers only +30 HWND and +41 active. Its opaque +28
is not present height and cannot serve Text AB8CE0 or the complete platform.

## Message receiver and stack frontier

BEC3B0 is a four-argument stdcall WNDPROC, RET10h. On **each call** it loads
current CF04, then forwards five stack arguments to slot+28: explicit active
platform, HWND, message, WPARAM, LPARAM. D68CEC identifies BED3B0, whose
epilogues use RET14h. The existing engine-services wrapper accepts a platform
value supplied by its caller and invokes a callable vtable; the raw Source
owner's numeric D68CC4 identity is not a callable host vtable.

BED3B0 separately captures `GetWindowLongA(HWND,0)` for messages other than
WM_CREATE. WM_CREATE reads CREATESTRUCTA.lpCreateParams, stores that pointer
at window-extra offset zero and sets that receiver's +40. WM_SIZE and
WM_ACTIVATE first use the window-extra receiver, then reload the explicit
active receiver for later HWND/focus work. Text, WM_CLOSE and WM_SETTINGCHANGE
arms consume the explicit receiver. Merging these receiver identities loses
Original behavior when publication or window ownership differs.

CreateWindowExA, SetWindowPos and ShowWindow can enter the WNDPROC before
BECEE0 has finished configuration/device/cache/power initialization. Full
message behavior therefore belongs to the creation dependency, not only to a
later loop. The activation arm still needs actual input, sound, GUI/media and
renderer/render-service providers. Raw text production must implement
BED370/BEC7B0/BED290 against platform+174h; the typed TextInputQueue currently
owns new/delete nodes and adds unlinked-node RAII on count failure. Raw queue
construction/retirement does not establish raw append.

## Cache EH and power tail

The original FuncInfo E01FC0 has one unwind state: E01FB8 maps it to CC7480,
which frees the captured allocation. BED1E8 allocates14h, arms that state after
allocation, conditionally constructs BEC870, reloads FE88 and disarms before
BEC3E0(10000). Existing raw BED1E8 Source supplies this bounded ownership
schedule; it does not add full-window rollback.

Only afterward BED223 sets actual +4C to FFFFFFFF and calls GetActivePwrScheme.
On success it calls ReadPwrScheme into actual +50, ignores that result, captures
the current scheme, copies 24h DWORDs to actual +E0, clears +11C then +118 and
calls SetActivePwrScheme. BED269..276 disables the screensaver even when the
get-active call fails. The constructor leaves these policy bytes untouched.
This conditional old-window stop and setup policy do not add a power restore
to normal D68CC4 retirement, whose separate proven schedule only enables the
screensaver before queue/base/free cleanup.

Next work is actual raw message/queue ownership and canonical-cell binding,
followed by composition of the real renderer/cache/power domains into BECEE0.
The current evidence establishes static/live bytes and dependency contracts;
it does not establish original ABI execution, whole-window startup, active
frame, Text activation or gameplay.
