# Segment 10: GGame helpers, singletons and scene-entity bodies (cc13-seg10)

Code: `src/seg10_cc13.cpp`, `include/bsp/seg10_cc13.hpp`, namespace `bsp::cc13_seg10`.
Every body listed here is reconstructed and build-tested. None is ABI-compatible or
game-validated. Names are hypotheses.

## Model

Same as segments 5 and 7 (`docs/CC13_SEG5.md`). Fields are read and written at the image's
offsets. Every call goes through a named `Seg10Host` method at the point the binary makes it.
Globals are read and written through the host. That includes the lazy singleton slots
00F8A71C, 00F8998C and 00E198D0, and the registry slots 00E1867C and 00E18678.

## Containers

| Address | Behaviour |
|---|---|
| 004D1720 / 004D38E0 | 004CF3F0, free head +4, +4 = 0 (byte-identical). |
| 004D1890 / 004D3A30 | Range release 004CC760(ECX first, EDX last, vector, vector), free, zero (byte-identical). |
| 004D28A0 | 004D05E0 on +8, free head +0Ch, release NativeString +0. |
| 004D4830 | +0/+4 = 0, list +8 head from 004C3020, +10h and +14h..+24h = 0. |
| 004D4DB0 | Zero two vector headers. |
| 004D9E60 | Pop the last 2Ch-byte alias record (004D45A0). |
| 004E7730 / 004E7820 | Counted block reserve (1Ch / 4-byte elements): a 16-byte-aligned data pointer inside an 18h-header block, copy, carry +4, free the old block. When the capacity is not raised it is only stored, even into a null block, as the binary does. |
| 004E77F0 | Free +0Ch, zero +4/+8. |

## Game world, records and singletons

| Address | Behaviour |
|---|---|
| 004D2B10 | World containers teardown: 007FFD60, 006EA1A0, 004CB220(game+7178h, 0), 00442170, 0095BA30. Then the vehicle registry index tables reset (skipped when the state queue front is 0Bh), then nine registry clears ending in a tail jump to 00900BB0. |
| 004D3040 | Enum registry (10Ch, vtables 00CE78BC/00CE7514) into [00E1867C]; universe library (008F5670) into [00E18678]; 008F7100(library, "universe\Library\"). |
| 004D4A20 / 004D4AB0 | Record 00CE78FC: zero eight vectors and run 009F89F0; the destructor frees them, clears [00F8A71C] and sets vtable 00CE3818. |
| 004D9D10 / 004D9D60 | Record 00CE7AAC: three ref-pointer vectors; the destructor releases each with 004CE600 / 004CE5C0 / 004CE580, clears [00F8998C] and sets vtable 00CE3818. |
| 004D6CE0 / 004DC200 / 004E1CA0 | Double-checked lazy singletons, identical listings apart from slot, size and constructor: (00F8A71C, 84h, 004D4A20), (00F8998C, 34h, 004D9D10), (00E198D0, A0h, 004E18E0). Created under the 00415350 manager's section and registered with 00BD0C30. |
| 004E18E0 | Award tracker constructor: vtable 00CE7F08, five trees (004C2A80, 00443E20, three 004C2AD0) and two pooled lists (004C3020). |
| 004D4BB0 | Session message E4h read: 8 bits +10h, 3-bit kind +18h. Kind 3 appends count records of 38h bytes to the list +1Ch; the first 8 bytes are never written, as in the binary. Other kinds read 4 bits into +19h. |
| 004D0FC0 | Player left: "leave" to the warning manager (00985920); name inserted into the set at 00E18A60 (004D0640); record reset; game+18F0h[slot] = 0. |
| 004D2A80 | Release the pending mission name (+10h) and the NativeString vector +4. |

## Prompts, state requests and presence

| Address | Behaviour |
|---|---|
| 004D7B10 / 004D7B50 | Prompt callbacks: [00F1B038] = 2, then 004D7970(game, 0) or the multiplayer menu's 00688C70 as listed. |
| 004DB240 / 004DB260 | Result 1: 004D8000 (004DB260 first broadcasts the close with 007728B0 when hosting). |
| 004D7920 | 004D3ED0(game+5D8h, &state). |
| 004D7940 | Enqueue 0Ah, tail jump to 00772610(game+1EF0h). |
| 004D7F90 | Drain (004C88A0); enqueue 10h and 2 with a mission, else +5D4h = 2; when 0067C8F0 is false, restart sign-in (00A3E6A0). |
| 004D8000 | XUserSetContext(00A3EAC0(), 8001h, 4) for a selected user. 00A3EAC0 takes no stack arguments: the 4 and 8001h pushed before it are the import's own arguments. Then presence flags and [00E19880], the pending-invite branch (movie stop, 00688B20, 0067D6E0, tail jump to 004BFC70), else a tail jump to 004D7F90 unless +5D4h is 2. |
| 004DCA80 | Passes ECX through to 004DC2C0 with the argument and a 60h-byte by-value block (+8/+0Ch zero). |

## Input

| Address | Behaviour |
|---|---|
| 004D9420 | Repeat gate (004D6900 map at game+5C8h vs time +64Ch, FCOMIP: passes when last < now or unordered); returns the action's axis float or 0.0. |
| 004D9480 | Same gate for pressed-not-held actions (+28h set, +20h clear); returns 1 when it fires. |
| 004D6410 | Assign a level's id vector (004D3740), 004C4300, find the top non-empty level (+5A4h downwards, minimum 1), and for level < 4 call 00A933F0(input, 1Dh, top). |
| 004D8AE0 / 004D8B70 | cdecl (game, id, ..., 0): build a vector (004D6840), 004D6410 with level 2 / 3, free. |
| 004DB030 | Pause toggle (single-player and multiplayer branches as in the listing). |

## Camera

| Address | Behaviour |
|---|---|
| 004DC940 | Store the fov index. Gating when the HUD is hidden: mode 8, or mode 9 / multiplayer / +61Ch, and the mission check through [[+A8h]+20h] vtable+38h. Then 00B6FBB0(float(cfg[index] * [00F889B4] * scale)). |
| 004DCDF0 | Camera display settings: aspect, clear flags, viewport size, far plane 20000.0f, then 004DC940 with the current fov ratio (FDIVP ST(1),ST(0), bytes checked). |

## Scene entities

| Address | Behaviour |
|---|---|
| 004E5850 / 004E58E0 | CameraPath constructor (00928630, 007B28F0 on +1E4h, five vtables, class id 4Ah) and destructor body (007B1BE0, 009287B0). |
| 004E5980 | NavPoint constructor (00928630, four vtables, class id 41h). |
| 004E5BB0 | 0053D700(ECX second argument), returns 0, RET 10h. |
| 004E5B20 | Command record copy, including the six copies of the +0Ch union. |
| 004E6540 | Collision node record: vtable, 004E6480, +4Ch. |
| 004E68A0 | 48h-byte zeroed record with 0.3f at +3Ch/+40h. |
| 004E78D0 / 004E78F0 / 004E7930 / 004E7950 / 004E7990 | World list register (00928560 + 00484540) and unregister (00928570 + chain walk + 004837D0) for lists +324h, +330h and +33Ch. |
| 004E60E0 | _vsprintf_s(dest, 40h, format, varargs). The C++ takes `...` and hands the host a va_list. |
| 004E6920 / 004E6970 / 004E69C0 / 004E6A10 | Existing: these IsKindOf bodies are the 4Ah / 41h / 42h / 43h rows of `kUnitKindClassBodies` in `src/unit_kind_query.cpp`. Bytes re-checked; a ledger record now points there. |

## Uncertainties

- Vtable stores are written as image addresses.
- 004E60E0's variadic C++ form is a host convenience. The binary passes the address of its
  stack arguments.
- 004D8AE0 / 004D8B70 take the zero-terminated id block as a pointer.

## Not reconstructed

These are too large or would need many more hooks than this pass allowed:
004D2BB0 (DestroyWorld), 004D32A0, 004D4720, 004D56E0, 004D5BD0, 004D62C0, 004D7BA0, 004D7D50,
004D80D0, 004D8620, 004DB920, 004DC020, 004DD7C0, 004E1A50, 004E2200, 004E27E0 (775 lines),
004E3AA0 (OnInit), 004E5540.

## Belongs to cc9

None. This range is game-state, UI and session code.
