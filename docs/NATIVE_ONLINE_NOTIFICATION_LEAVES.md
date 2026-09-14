# Native online notification leaves

Addresses: `00A3E600`, `00A3FDE0`, `00A3FF20`, `00A3E560`. These are complete
normal bodies over the captured 3F0h manager and native eight-byte string
headers. The C++ entry points are explicit-source interfaces, not native ABI
replacements. `NativeOnlineNotificationLeafRuntime` binds a borrowed, already
loaded XLive module and actual Win32 calls. The fixture uses recording calls and
never launches an updater or makes an SDK/network request.

| Body | Coverage | Original ABI | Observed work |
| --- | --- | --- | --- |
| A3E600 | complete normal | ECX=captured manager, one mask DWORD, RET4 | Bit0 gates XUserGetName(0, scratch128, 128). Only success compares case-sensitive C strings. Difference copies all 128 bytes at manager+90, then current +11C=0 and current +18 target call with ECX=0. |
| A3FDE0 | complete normal | output native string pointer on stack, ECX unused, RET4, AL bool | RegOpenKeyExA HKLM, `SOFTWARE\\Eidos\\Battlestations Pacific`, access 20019h; query `ApplicationDir` into 400h bytes; only successful REG_SZ assigns the C string. Explicit RegCloseKey follows. An allocated 25-byte `BattlestationsPacific.exe` suffix appends unconditionally with no separator. Return reports directory assignment, not append success. |
| A3FF20 | complete normal | output native string pointer on stack, ECX unused, RET4, AL bool | Zero 218h SDK image, set cbSize, call ordinal 5022. Signed HRESULT>=0 and type+4=0 permit a 00436630 narrow temporary from UTF16 path+10; resize/copy to output, release temporary, return true. Other results leave output untouched by this body. |
| A3E560 | complete normal | two C-string stack pointers, ECX unused, RET8, EAX bool | Null executable returns false. Otherwise zero 3Ch SHELLEXECUTEINFOA, set cbSize=3Ch, fMask=400h, verb `open`, file/parameters, nShow=5, call ShellExecuteExA, return its boolean. No wait or exit in this helper. |

The production profile callback target is installed at `0073DC87`:
manager+18 is `00737D60`, which reads the selected native name from the
currently published manager and writes actual current-game profile headers and
the +1FF0 mirror. The runtime needs a borrowed
`NativeOnlineProfileCallbackContext` to execute it. `00735510` and `00735520`
are the separate manager+20 and +24 XUserSetContext slots from `00A40DF0`;
neither is the normal callback18. See `NATIVE_ONLINE_PROFILE_CALLBACK.md`.
The name SDK may mutate
the captured manager during its call; comparison, cache copy, state and callback
reads happen afterward. Query failure can leave scratch partly written but does
not change the manager. A callback exception happens after the cache copy.

The path output is the caller's actual `NativeString` descriptor; the caller
supplies its live `ActualNativeStringPoolStorage`. The title SDK image and
registry buffer begin with unspecified/zero preimages exactly where the native
does: SDK image is zeroed, name and registry scratch are not. On title failure
the SDK's partial writes remain only in its discarded local image. On registry
failure the previous output survives until the unconditional suffix append.
Successful REG_SZ must contain a readable ANSI NUL; successful title type0 must
contain a readable UTF16 NUL. Native `00436630` allocates by full UTF16 length
but copies only low bytes through the first low-byte zero, so the copied output
may include untouched allocation bytes when a nonzero UTF16 unit has low byte
zero. No Unicode conversion or sanitization is added. The raw dispatcher owns
the path comparison, `\\setup.exe` append, later `Sleep(200)` and `_exit` paths.

Direct call-site evidence is in `reports/native_online_notification_leaves.json`.
Notable cleanups: A3E687 RET4; A3FF18 and A40019 RET4; A3E572/A3E5C7 RET8;
A3FEAB/A3FED5 and A3FF4C/A3FFC2 ADD ESP,0Ch for the CRT calls. Imported
Win32 calls use their IAT slots, while the callback is `CALL EAX` at A3E67D.
Ghidra was read-only for this packet. Descriptive names remain hypotheses.

Validation status and exact artifact hashes are recorded in the report. Build
and fake-call fixture results do not establish game behavior, binary ABI, or
real network/update effects. Invalid pointers, unterminated SDK strings, native
SEH machine faults and C++ exception-domain equivalence remain outside this
normal-body source contract.

## Integrated library evidence

At `708e70c96b97e76e3eca1bde53e339246cb72ea1` all six wave sources are registered once in the default
Win32 target. `scripts/build.ps1` and both existing CTests passed. Four focused
fixture programs were compiled and run against that exact `bsp_core.lib`:
13 copied-original achievement cases, two copied-original reset cases and UI
preimage/cleanup checks, the leaf/profile fake-call fixture, and ten
copied-original pump cases plus dispatcher partial-output/cleanup checks.
Two isolated child runs use fake SDK/Shell calls and actual `_exit(0)`.
The six packet reports have115 checked direct call rows and zero failures.
Fourteen saved names/comments were read back with prior comments preserved;
exports were refreshed. No listing repair was required for these14 bodies.

Immutable checkpoint: `local/checkpoints/708e70c9/native-online-pump-default/validation.json`; SHA256 `e5ad416dc64065a43b0bfe74e4c511c1ffa19de01e71e9a9b983f824e0e03f46`;
3182 artifacts. It pins exact sources, build objects and
libraries, compiler include records, actually searched libraries, mapped
32-bit runtime modules, command/stdout/exit evidence, original byte spans and
three verified worker archives. The worker fixtures retain their original
C++20 harness requirement; the earlier C++17 harness invocation failure is
preserved separately. Production source was unchanged by that harness fix.

The full raw3F0 manager construction/destruction, real SDK asynchronous
lifetimes, client virtual behavior, original ABI/FH3/SEH and gameplay remain
open. The UI's locale and clock-provider boundaries remain as documented.
