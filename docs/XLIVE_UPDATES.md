# XLive update paths, conversion and process launch

Five normal function bodies are reconstructed with canonical `NativeString`
headers/storage and required SDK/Win32 operations. The wide conversion is a
value projection through the existing `startup_widen_path`, retaining the
existing `std::wstring` boundary instead of introducing another wide owner.
Descriptive game function names are hypotheses; existing names are preserved.

| Address | Original ABI | Recovered behavior |
| --- | --- | --- |
| 00a3ff20 | Output-header pointer on stack, RET4, AL bool; ECX unused | Query title-update information, narrow/copy path only for nonnegative result and type0 |
| 00a3fde0 | Output-header pointer on stack, RET4, AL bool; ECX unused | Query installation directory, then always append executable filename |
| 00a3e560 | Two C-string pointers on stack, RET8, EAX bool; ECX unused | Zero/fill 3Ch ShellExecuteExA request, return API nonzero |
| 00436630 | ECX narrow header, wide pointer on stack, RET4, EAX header | Fresh header; allocate full UTF16 length, copy low bytes until first low-byte zero |
| 0041e350 | ECX narrow header, C-string pointer on stack, RET4, EAX header | Nullable C-string assignment, resize without preservation, copy current header length |
| 004c5e60 | ECX wide header, C-string pointer on stack, RET4, EAX header | Unsigned-byte widening; reconstructed value semantics, native wide allocation/header ABI remains outside this packet |

`reports/xlive_updates.json` records inclusive ends, final instruction lengths,
code-span hashes and comparisons against the installed executable. The existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was queried only through
verified read-only `bsp.py` wrappers. No missing starts or omitted flow gaps were
found in these six bodies. No Ghidra mutation is part of this worker packet.

## Title-update information

00a3ff20 clears exactly 0x218 bytes at 00a3ff3b..00a3ff4c, writes cbSize=0x218,
and calls thunk 00a4d59c. That thunk reads IAT 00ce26e4, whose original PE import
is ordinal **5022**, `XLiveGetUpdateInformation`. It is a Win32 stdcall with one
output pointer and a signed HRESULT return. The SDK forwarding implementation
belongs to the integrator; `XLiveUpdateInformationHost` is a required interface.

The checked fields are cbSize +0, type +4, and a UTF16 path at +10 with 260 units.
DWORDs +8/+C remain opaque. Signed results greater than or equal to zero are
accepted, including positive results; type must also be zero. Failure or a
different type leaves the caller's existing output unchanged and returns false.
On acceptance, 00436630 constructs a temporary narrow string, the output is
resized with preservation enabled, and the current output length is copied.
The temporary is released afterward or if output resize/copy throws. The native
unwind state is armed only after the temporary constructor returns.

## Narrow and wide conversion

These routines perform no code-page or Unicode conversion. 004c5e60 zero-extends
each unsigned input byte. The existing startup widening implementation is reused
for this result; native pooled allocation, an empty null data pointer and the
8-byte wide header are not reproduced by a returned `std::wstring`.

00436630 zeroes the destination's two header words without releasing old storage,
then measures the full UTF16 string and calls canonical narrow resize(length,
preserve=true). It copies only each unit's low byte, stopping immediately when
that byte is zero. For `A U+0100 B C`, native length remains four, bytes0/1 become
`A,0`, bytes2/3 retain the allocator's prior contents, and byte4 is the resize
terminator. The title helper copies that full four-byte span. The implementation
preserves those untouched bytes instead of truncating the length or filling them.
This constructor requires a fresh destination unless the caller intentionally
accepts native overwrite/leak behavior. Inputs must be terminated and valid.

0041e350 differs from the constructor: it accepts a null source as length zero,
resizes the existing destination with preserve=false, then copies the current
length without copying a terminator. The canonical resize writes the terminator
only when it changes the allocation; equal-length assignment preserves the
native no-op resize behavior. The system-update helper needs this assignment,
not the header-resetting 0041e870 constructor.

## System path and launch

00a3fde0 opens `HKLM\SOFTWARE\Eidos\Battlestations Pacific` using ANSI
`RegOpenKeyExA`, options0 and access0x20019. On successful open, it queries
`ApplicationDir` with a 1024-byte buffer and accepts only API success plus REG_SZ.
The returned byte count is not used to trim or terminate the string. Successful
assignment sets the result flag, and `RegCloseKey` is called afterward regardless
of query failure/type mismatch. Its return status is ignored.

There is no native key owner in the unwind table: if string assignment throws,
the explicit close is bypassed. This behavior is preserved and verified using a
recording host, not an actual leaked registry handle. After a normal query path,
even an open/query failure, a temporary `BattlestationsPacific.exe` (25 bytes)
is appended to the current output without adding a separator. Its terminating
NUL is copied when constructing the temporary; append copies only its 25 data
bytes. The temporary is released if append allocation throws. The result says
whether the directory was assigned; it does not describe the final path's validity.

00a3e560 returns false immediately for a null executable. Otherwise it zeros the
complete 0x3C-byte `SHELLEXECUTEINFOA`, sets cbSize=0x3C, fMask=0x400, verb=`open`,
file and parameter pointers from the caller, and nShow=5. All other fields remain
zero. It calls `ShellExecuteExA` once and returns whether the BOOL is nonzero.
No quoting, directory substitution, process wait, sleep or exit is added. The
notification dispatcher owns subsequent sleep/exit behavior. The concrete
`Win32XLiveUpdatePlatform` directly forwards the registry and shell API calls;
construction alone performs no work.

## Validation and limits

The Win32 `/W4 /WX /fp:strict` build, both existing CTests and the focused local
fixture passed. The fixture supplies poisoned storage,
recorded registry/ShellExecute responses and an update-info provider; it checks
early low-byte termination, nonnegative/type gates, untouched failure output,
temporary cleanup, unconditional append and the exact launch request. It never
calls an actual SDK, registry API or process launcher. Actual Win32 forwarding
is compiled, not runtime-executed. No update was launched, no installed file
was changed, and SDK/game behavior remains unvalidated. Malformed unterminated
SDK/registry strings and allocation failure machine faults are outside the
valid-input C++ projection; native narrow tail bytes remain allocator-dependent.
