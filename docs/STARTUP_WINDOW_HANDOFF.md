# Native startup window handoff

Read-only audit on 2026-09-09, with project `bsp` and program
`/battlestationspacific.exe` verified before each live batch. This supplements
PLATFORM_LOOP, WINDOW_CLOSE, GAME_FRAME and D3D9_STARTUP. No C++ or Ghidra
annotations changed, and no window or power-setting calls were executed.

## Actual initialization call and ABI

Application initialization 0073d410 constructs platform 00becda0, later constructs
renderer 00b32410, then prepares a window-name string and calls platform vtable
+4h at 0073dc25. The callee is 00becee0. Its ABI is **cdecl with eleven stack
arguments, including an explicit platform pointer**: the caller pushes that
pointer at 0073dc21 and adds 2Ch to ESP at 0073dc2b; the callee has plain RET at
00bed289. Do not invoke it through a conventional thiscall adapter.

Arguments in increasing stack order are:

| Argument | Source / use |
| --- | --- |
| Platform | singleton0109cf04 |
| Title/class string object | Temporary length/data pair prepared by application |
| Fullscreen | Byte00f8899e zero-extended |
| Bit-depth/sync selector | Boolean of byte00f889e0; passed as fourth argument despite broader callee uses |
| X, Y | Globals00e1ae88,00e1ae8c |
| Width, height | Globals00f88994,00f88998 |
| Multisample | Global00f889d8 |
| Application | Saved application pointer, stored at platform+48h |
| HINSTANCE | Global00e1ae7c |

The routine registers an ANSI class with CS_GLOBALCLASS and 24 extra window
bytes, adjusts a WS_CAPTION rectangle and creates the HWND with platform in
lpParam. It then applies windowed/fullscreen styles, copies the title, computes
aspect fields, shows the window and invokes renderer initialization. It continues
through another singleton/buffer initialization and power/screensaver changes;
the renderer's existing device-creation prefix is not this whole initialization.

An earlier description that this post-renderer allocation necessarily initializes
timing is too strong: 00bec870 constructs a 14h-byte object with a critical section,
and 00bec3e0 is called on singleton0108fe88+4 with10000. Its export is truncated
after free. Its full role is unresolved; do not substitute the reconstructed
frame clock for it based on the numeric10000 argument.

## Message ABI and lifecycle caveats

Window thunk00bec3b0 is stdcall with the four Win32 arguments and RET10h. It loads
singleton0109cf04, pushes the original LPARAM, WPARAM, message, HWND and then
platform, and calls virtual+28h. That target00bed3b0 is **stdcall with five stack
arguments**, ending in RET14h. Neither callee receives platform in ECX.

WM_CREATE obtains CREATESTRUCT.lpCreateParams and stores it in window-extra
offset0, setting platform+40h. Other messages use GetWindowLongA(hwnd,0), while
some branches use the explicit singleton argument instead. These identities are
not interchangeable in an arbitrary multiwindow host. Full message handling has
XInput, audio, UI and renderer activation/reset dependencies; an ordinary default
window procedure cannot stand in for it when claiming native lifecycle parity.

WM_CLOSE sets +180h and returns0. As WINDOW_CLOSE establishes, confirmation/game
policy later sets application exit, which eventually sets +181h. WM_QUIT does
not directly terminate the reconstructed loop.

Platform virtual+8h00bebf70 restores a saved power scheme when index+4Ch is not
-1, clears frames-enabled+42h, and calls PostQuitMessage(0). It contains no
DestroyWindow, HWND clear, singleton unregister or assignment to exit+181h.
Its use by00becee0 when an old HWND exists must not be interpreted as proven
complete old-window teardown.

Virtual slot0 points to00bece30, absent from the current function inventory but
fully visible in raw bytes. ECX is platform; stack argument is a deleting-
destructor flag, RET4 and EAX returns the original pointer. It installs the
concrete vtable, calls SystemParametersInfoA(11h,1,null,0), destroys the text-input
list at+174h via00bec730, calls base destructor00be2b10, then frees the platform
when flags&1. The base frees the title string and unregisters the singleton.
There is no direct DestroyWindow or class unregister in these audited routines;
the complete native HWND release path remains to be established.

## Smallest independent implementation dependency: text-input queue

Beyond clock/PRNG, the platform constructor and message handler share a bounded
list dependency that can be recovered without inventing audio/render callbacks.
The list object is at platform+174h: sentinel pointer+4 and element count+8.
Each node is12 bytes: next+0, previous+4, two-byte event+8, unused final bytes.

* 00bec710 allocates12 bytes, sets next/previous to itself, returns node in EAX,
  plain RET. This is the sentinel builder used by platform constructor.
* 00bec7b0 takes three stack arguments (next,previous,event pointer), allocates12
  bytes and copies those fields, returns node in EAX, RET0Ch. ECX is not consumed.
* 00bed370 takes list in ECX and event pointer on stack, RET4. It constructs a
  node with next=sentinel and previous=sentinel.previous, invokes size increment
  00bed290(1), then links sentinel.previous and old-last.next to the new node.
* 00bed290 uses an unsigned `7fffffffh-count < increment` check and throws a
  native `std::length_error` with `list<T> too long` on overflow before changing
  count. The allocator/exception ABI needs an explicit typed-interface boundary.
* 00bec730 takes list in ECX, plain RET. Raw bytes00bec730..00bec777 reset the
  sentinel links/count, free every old node using saved next pointers, free the
  sentinel, then clear list+4. Its pseudocode incorrectly ends after the first
  free; use the raw loop, not that truncated export.

Message handler feeds this queue only when platform+170h is enabled. WM_CHAR
uses low WPARAM byte with event second byte0; Ctrl+V character16h additionally
sets+44h. Selected navigation/edit keys from WM_KEYDOWN use low virtual-key byte
with event second byte1. The dispatcher call at00bed664 passes ECX=platform+174h.
Do not treat these records as UTF-16 code points; the two bytes have distinct
roles. Queue consumers and enabling/clipboard behavior still need tracing before
connecting the queue to an actual text field.

This is a concrete constructor/message dependency suitable for a separate port.
It does not make native CreateWindowAndDevice or the application frame runnable.
The full window handler's activation dependencies and shutdown ownership remain
required before replacing the diagnostic HWND with a native lifecycle.

## Exact disk/live byte evidence

All inclusive ranges below match the installed executable and saved program.

| Range | SHA256 |
| --- | --- |
| 00bec3b0..00bec3d3 | 01e60e1316d293035548925c1dfa9a93708490eb43824567fcde534954951d7d |
| 00bebf70..00bebf94 | 8144c2a4b84d01cd8b096ef0d27ddc41750be60e0af5cf6921bf4120ee80dab3 |
| 00bece30..00bece6e | e6747a00ac6cded472183cafa5a10f70c0d638e381ad1279f7dcdf3836949bdd |
| 00bec710..00bec729 | aabd23785a96372f608cd6e5efd51ffa3e52bdef9293595289e5576b9bad17be |
| 00bec7b0..00bec7e4 | e589bb25ddabd90c27747866f9e2611b247a90ed4c5cbebd51edd2d0f10c1b6c |
| 00bed370..00bed3a1 | 41630ec9d826fd52f9e3226df16ce7890b7ddc00eb963367b85bbdb4045f59a8 |

This is static evidence only, with no new test or build. None of these findings
proves visible window behavior, input compatibility, shutdown or gameplay.
