# Raw platform construction and base cleanup

The complete reconstructed `BECDA0` now constructs the actual 184h platform
owner before the existing renderer/device/installed-effect composition. It
publishes and registers through the application's raw singleton manager,
initializes the original window-base fields, and allocates a real 0Ch text-queue
sentinel. The previous typed `Win32PlatformState` fragment remains a separate
projection. Evidence and precise coverage: `reports/native_platform_construction.json`.

| Native body | Source operation | Original ABI |
| --- | --- | --- |
| BE2960..BE29F0, 145 bytes | Publish/register platform base | ECX owner; EAX original owner; RET |
| BE2A00..BE2A98, 153 bytes | Unregister platform base | ECX owner; no specified result; RET |
| BE2AC0..BE2B09, 74 bytes | Construct window base | ECX owner; EAX original owner; RET |
| BE2B10..BE2B76, 103 bytes | Destroy window base | ECX owner; no specified result; RET |
| BEC710..BEC729, 26 bytes | Allocate/self-link queue sentinel | ECX unused; EAX node; RET |
| BECDA0..BECE23, 132 bytes | Construct derived Win32 platform | ECX owner; EAX original owner; RET |

All six bodies consume no native stack arguments. New source context parameters
are additional interfaces, not original ABI replacements. Names describe behavior
and are hypotheses rather than recovered symbols. All 633 body bytes were checked
against both live Ghidra and the unchanged installed PE; 13 direct calls passed the
call-site verifier. Four indirect calls are the actual Enter/LeaveCriticalSection
imports. The executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Publication and cleanup ordering

`BE2960` arms generic-base cleanup before storing profile D68604. It resolves the
actual manager, captures its section at +10h, enters it, and increments the extra
DWORD depth at section+18h. Only then does it arm guard cleanup and publish the
original owner to 109CF04. The second manager getter precedes the current
publication read used by `BD0C30`. Normal decrement/Leave uses the first captured
section; guard cleanup stays armed through Leave. Return remains the original
receiver, even if the current publication has changed.

`BE2A00` writes D68604 before arming generic-base cleanup. Under the same captured
section schedule, it calls `BCFCA0` with the current publication and clears the
publication only after removal returns. It resets the original receiver to
CE3818. Neither operation substitutes a cached manager or semantic domain.

The reviewed FH3 data are E01270/E01260 and E012A4/E01294. In both bodies state1
unwinds the actual eight-byte guard through `411EE0`, then state0 resets the
original receiver through `412430`. A first-enter failure reaches only state0.
Normal Leave is still inside state1. The implementation preserves these C++
cleanup states and terminates if a second C++ exception escapes cleanup.

`BE2B10` writes D68608, captures title data at +8h, then arms base cleanup. A
nonnull title captures length+1 with DWORD wrapping before the actual `419CC0`
getter and `BD1510` return. The title header remains stale, including changes a
provider may have made. Getter exceptions can escape; this path does not pass
through the pre-existing noexcept semantic string release interface. Normal
`BE2A00` runs after disarming this cleanup, so it is not retried on failure.
E012D0/E012C8 and CC6B00 establish that cleanup edge.

## Incoming aspect bytes and derived stores

`BE2AC0` first completes base publication. It clears the native string header,
then executes **FILD signed DWORD +24h / FIDIV signed DWORD +28h**. The stores of
640 and 480 occur afterward, before FSTP float +10h. Thus default dimensions are
not the aspect operands. The implementation retains the original interleaved
stores and the caller's x87 environment. It introduces no float operand rounding,
default-aspect correction or divide-by-zero recovery.

`BECDA0` arms window-base cleanup after that base constructor returns. It writes
D68CC4, calls `BEC710`, stores the sentinel at +178h and zero count at +17Ch,
then performs the listed byte/DWORD clears in original order. Unassigned bytes
remain untouched, including queue +174h, requested dimensions, power-policy
storage and trailing +182h/+183h. E01F94/E01F8C and CC7460 establish the failure
edge to `BE2B10` after sentinel allocation throws.

The sentinel allocator uses the actual 0Ch object allocation contract, self-links
next/previous at +0/+4 and leaves +8..+Bh untouched. Both original pointer tests
and 32-bit wrapping are retained. A null allocator result does not become a valid
empty queue: the original attempted address-4 store is preserved.

## Validation and limits

`scripts/build.ps1` passed after source registration; both existing CTests passed.
No permanent tests were added. A fresh copy of the closed device/effect fixture
compiled 24 consistent source units, linked 23, and selected the new platform
object from the current production library.

One focused signed-input case (-2147483647 / 16777217), under x87 CW 0C7F
(24-bit precision, truncation, masked exceptions), compared all 184h bytes and
the control/status words with the original BE2AC8..BE2B07 instruction fragment.
Both produced aspect C2FFFFFE and status 0020. The 64 original bytes were copied
unchanged with only a receiver prelude and return suffix; this is fragment
differential coverage, not original whole-constructor execution. The same 64-byte
instruction sequence occurs in both linked production constructors because the
compiler also inlined it into the derived constructor; both occurrences were
attributed through the link map.

The temporary window base returned a nonnull title through the shared actual
string pool, preserved the stale header, cleared publication, and yielded the
same pointer on the next exact-size allocation. Full derived construction then
matched all 388 expected bytes, actual publication/registration and sentinel
self-links. The fixture's incoming 1920/1080 bytes produce aspect 3FE38E39 while
the constructor writes 640/480 to the dimension fields.

The same constructed platform subsequently participated in complete source
B32410 renderer construction, B2AEB0 device startup, a real focused B2ABD0 Reset,
and cold installed DDS/effect loading. Restored 16MiB vertex / 1MiB index buffers,
actual default surfaces, eight exact HAL shader bytecodes, primary4/secondary1,
canonical28 and cache cursor164 were retained. Installed files were hash-checked
before/after. The actual control thread joined with exit0, focus was restored,
and the timer request was balanced.

No original FH3/SEH ABI, injected allocation/registration/Leave failure, hardware
fault unwind, invalid-pointer domain, queue destruction or complete shutdown is
validated here. The fixture still creates its own small STATIC window and writes
HWND/activity; `BECEE0`, its native WndProc, timing owner and power/screensaver
behavior remain separate work. Renderer/raw scratch preimages, asset policy,
PC/USA and loose-file mount are still fixture inputs. There is no active frame,
draw or gameplay validation. The retained failed compile was a missing copied
fixture header; evidence-tool retries corrected a premature map read and an
incorrect assumption that the compiled x87 sequence could occur only once.
No production algorithm was changed for those retries.

Immutable closure: `local/checkpoints/78f2d717/native-platform-construction/validation.json`, SHA-256 `b8f69ab52e400559faf40c56d76a958c65458ce9e86d96740be1bb55489c5a50`. It contains 4959 artifacts, 81 physical mapped Win32 modules and 305 selected production source providers. The complete build/source inputs, compiled objects, original bytes, installed assets, results and prior Ghidra comments are retained; the captured docs/report precede this closure-pointer addition.
