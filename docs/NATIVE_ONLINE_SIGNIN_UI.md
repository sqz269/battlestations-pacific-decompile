# Native online sign-in UI over the actual manager

The C++ entry points in `native_online_signin_ui.hpp` operate on the existing
`NativeOnlineManagerStorage` 3F0h image. They expose semantic Win32 C++ calls,
not native register/SEH entry points. The underlying installed executable is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`;
analysis used `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
The installed image, function bounds and call sites are recorded in
`reports/native_online_signin_ui.json`. No game sign-in UI or account request
was invoked for verification.

| Native address | Bound/ABI | Coverage |
| --- | --- | --- |
| `00A40510` | `00A40510..00A4099F`, ECX=captured manager, RET | Complete six-case normal state-machine body, including all exits and local cleanup; native SEH and arbitrary fault equivalence not claimed. |
| `00A40020` | `00A40020..00A4010C`, ECX=manager, RET | Complete normal reset body; malformed-vector CRT failure route represented by an exception. |
| `00A3E700` | `00A3E700..00A3E71B`, ECX=manager, RET | Complete three-store body. |

The manager's `+3B0` word selects states 1..6; other values return. State 1
first checks live `+2C`. If set, it writes `+3B0=0`, `+3B4=1`, `+3B8=1`.
Otherwise it constructs four eight-byte native wide output headers, all
initially `{length=0,pointer=null}`, and resolves, in order:

1. `FE_xbox.xsm_SignIn_Title`
2. `FE_xbox.xsm_SignIn_Question`
3. `FE_xbox.xsm_signin_signinuser_pc`
4. `FE_xbox.xsm_signin_continuewithoutsigningin_pc`

Each native eight-byte key is constructed through `0041E870`, passed to the
current `00A9FAD0` locale resolver, and destroyed through `0041DD20` before
the next key. The four wide outputs are destroyed in reverse order through
`00436430`, even when the message-box call returns pending. The supplied
`NativeStringRawPoolContext` uses the canonical published pool and current
shutdown gate; it introduces no second allocator. The concrete locale adapter
borrows the caller's current `LocaleTextResolver`, which in turn must bind the
current locale tables and Lua context. It appends to raw eight-byte output
headers using the existing `004C53E0` resize and copies UTF-16 code units.
That resolver's documented invalid-input/overflow exceptions are a host
boundary, not a claim of native SEH parity.

After all four resolutions, state 1 clears *seven* DWORDs at `+3C0..+3D8`.
It does not clear choice `+3DC`, `+3E0`, or `+3E4`. It captures title/question
and two button pointers, using a shared zero wide character for null data;
then reads current `+3E8`. If zero, it calls `XShowMessageBoxUI` with current
`+3B4`, two buttons, focus 0, flags 1, choice pointer `manager+3DC` and
overlap pointer `manager+3C0`. The text and pointer-array are borrowed only
for the synchronous SDK call; the SDK must have consumed/copied them before
return. Pending (`0x3E5`) writes state 2, after which the local text buffers
are still destroyed. A non-pending return leaves state 1 for a retry. The
overlap and choice buffers remain manager-owned across frames and SDK callbacks.

| State | Current reads and outcomes |
| --- | --- |
| 2 | If `+2C` is set, pending `+3C0` waits; otherwise reset slots. If `+2C` is clear, pending waits; completed `XGetOverlappedResult(overlap,null,1)` failure invokes `A40020`. Success choice `+3DC=0` enters 3; choice 1 copies `+3B4` to `+3B8`, clears `+119/+11A/+120`, sets `+11C=1`, `+124=-1`, enters 5. Any other choice stays in 2. |
| 3 | With `+2C` clear, visible UI at `+3E8` waits, as does a nonzero `XShowSigninUI(1,0)` return. Otherwise `A40020` runs. A set `+2C` also runs `A40020`. |
| 4 | Set `+2C` resets slots. Otherwise copies `+3B4` to `+3B8` and `+11C`, clears `+3E4`, sets `+3BC/+119/+120`, sets `+124=0`, enters 5. It does not write `+11A`. |
| 5 | Set `+2C` resets slots. If `+119`, passes an uninitialized 28h-byte stack output to `XUserGetSigninInfo(+11C,1)`. Only success reads byte +8 and stores its low bit to `+11A`; failure or clear `+119` writes `+11A=0`. On success, a current `manager+8C+4*(+11C)` value of 2 calls the integrated raw `A3ED10` path builder, then enters 6. |
| 6 | Set `+2C` resets slots. Otherwise clears `+3BD`, writes `+28=2`, enters 7. |

All reads after external calls are from the captured manager, including the
choice after overlap completion, `+3E8/+3B4` after localization and `+11C`
after sign-in-info. Valid manager lifetime and valid indexed `+8C+4*user`
storage are caller requirements, matching the native unchecked access. No
projected `OnlineSystemState`, `XLiveSystemPumpState` or cached manager is used.

`A40020` captures callback `+20`, checks current `+8C`, and invokes the
callback with ECX=0 only when both are nonzero. It then rereads `+3BD`; when
zero it clears `+119`, sets `+11C=1`, then clears `+11A`. It clears bytes
`+2F/+2D/+2E/+120`, sets `+124=-1`, clears `+28/+2C/+35C/+358`, validates
achievement-vector begin `+364` against end `+368` twice, and moves end back
to begin if they differ. In the ordinary body, its computed memmove length is
zero because current end equals captured end. It writes `+3B0=0`, `+3B4=1`,
`+3B8=1` last. `004254B0` is an empty diagnostic callee. `A3E700` only writes
those final three words; it does not run the callback or clear other fields.

`NativeOnlineSigninUiRuntime` resolves the game's imported XLive ordinals
5266 (message box), 1083 (overlap result), 5260 (sign-in UI), and 5267
(sign-in info) from the same borrowed `XLiveLibrary` module. The state-5
storage call resolves ordinal 5344 and executes the integrated
`build_native_online_storage_path_00a3ed10` against the same captured manager.
The reset callback is the already reconstructed `NativeOnlineSigninCalls`
binding. Missing ordinals and absent library/invalid host headers fail rather
than substituting SDK success. No Ghidra annotations were written by this
read-only worker; root should update the three function comments from the
earlier projected status to the evidence and current scope above.
