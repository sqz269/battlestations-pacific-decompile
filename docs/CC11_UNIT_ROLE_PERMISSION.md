# Complete raw role-permission reader at 0059BBD0

This packet reconstructs the whole 35-byte routine at
`0059BBD0..0059BBF2`. Native ECX is the actual unit, entry stack +4 is the role,
and stack +8 is the candidate slot. It returns full EAX exactly zero or one and
uses `RET8`. The new C++ function returns `std::uint32_t` and takes an explicit
borrowed unit pointer; its calling convention is not a binary replacement.

For a valid role 0..8, the function reads the actual DWORD at
`unit+188h+4*role` once. It returns one if that policy is 9 or equals the full
candidate DWORD, otherwise zero. There are no calls, globals, virtual dispatch,
ownership changes, guards, default values, or extra interpretation of slot 8.
The source adds none. Policy `109h` is not policy 9, and equal high-bit DWORDs
are accepted without truncation.

The actual `00780120` caller sets EBX to role 1 at `007801F3`, loads the candidate
from message+28h at `00780204`, pushes candidate then role at `00780207/208`,
sets ECX to the unit, and calls `0059BBD0` at `0078020B`. It tests AL before the
pilot-role flag store at unit+184h. This packet supplies that exact leaf; it does
not close or bind the surrounding role-message dispatcher.

Structural producer evidence establishes nine separate policy fields:

- `00928630` publishes primary profile `D192E0`, then pushes value 9 and mask
  `1FFh` before virtual +148h at `00928711`.
- Live `D19428` (`D192E0+148h`) contains `00927D20`. That setter starts its
  cursor at unit+1ACh, counts nine iterations, writes `[cursor-24h]`, and advances
  four bytes each iteration. Thus policies occupy +188h through +1A8h.
- The distinct current-role fields +1ACh through +1CCh are explicitly initialized
  to 8 by `00928718..00928748`. They are not the permission table.

These are structural receipts from pinned native bytes. Neither the complete
constructor nor setter executes in this fixture, and neither is claimed closed.
The setter still calls real virtual providers, message routing and observer
notification. Existing semantic lambdas in `game_hosts_units.cpp` and
`game_hosts_hud.cpp` remain unchanged; the existing name ledger was provisional
and no raw reconstruction entry covered this routine before this packet.

One focused fixture supplies valid borrowed prefix storage containing all nine
policy DWORDs. Eighteen role/candidate pairs produce 36 original/source full-EAX
comparisons, covering wildcard 9, exact matches, mismatches, slot 8, and high-bit
values. All match explicit fixture expectations and all prefix bytes remain
unchanged. The complete original 35-byte body executes without any relocation,
patch, provider bridge, or replacement callback. Its bytes match the installed
PE and live Ghidra and are verified unchanged before and after execution.

The strict MSVC Win32 build passed `/O2 /W4 /WX /fp:strict` with an embedded
`asInvoker` manifest. Four current source/header/fixture inputs are frozen and
verified against the compiler's include trace, with 173 host-header hashes.
No repository support library is linked; the six searched CRT/Win32 toolchain
libraries have recorded hashes. Source COFF is 35 bytes/11 instructions, with
zero calls or relocations; its full review verifies the actual indexed load and
both full-EAX returns. The 26-byte native invoker supplies native ECX and stack
arguments, retains the original `RET8`, and captures the complete EAX result.

Result: 30 checks passed, zero failures. Five earlier reports and 728 recorded
artifacts preserve their before/after hashes. The report records the complete
evidence under `local/cc11_unit_role_permission_20261007_a`. Shared registration
and the full project build belong to the primary integrator. This packet makes
no complete entity, setter, game-world, dispatcher, or game-validation claim.
