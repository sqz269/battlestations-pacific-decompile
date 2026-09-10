# Actual-header pooled string copy construction and substring

This packet reconstructs full `00426060` and `00469840` against actual eight-byte
headers (`uint32 length` at `+0`, Win32 `char* data` at `+4`). It closes the copy
and substring prerequisites for the separate `00BEE690`/`00BEE780` name-normalizing
owners. Those owners and renderer-container removal are not implemented here.

The new interfaces in `include/bsp/native_pooled_string_substring.hpp` return the
actual destination/output address. They compose the existing actual-header
`0041DD40` resize and `0041DD20` destruction bodies through `NativeStringStorage`.
They do not overlay `NativeString`, construct another owning abstraction, release
an existing output, or introduce a null-header guard.

| Original entry | Original ABI | Full checked span | C++ interface |
| --- | --- | --- | --- |
| `00426060` | ECX destination, stack source, EAX destination, RET 4 | `00426060..004260A2` exclusive, 66 bytes | `copy_construct_native_string_header_00426060` |
| `00469840` | ECX source, stack output/start/count, EAX output, RET 0Ch | `00469840..00469950` exclusive, 272 bytes | `construct_native_string_substring_00469840` |

These are source interfaces for MSVC Win32, not drop-in implementations of the
original calling conventions. The descriptive names are hypotheses.

## Copy constructor `00426060`

`00426068` compares the header addresses before the two zero stores at
`0042606A/70`. The identity branch follows those stores: exact self-copy clears
both fields and abandons the previous buffer. Distinct headers capture the
current source length, call actual resize with preserve enabled, then reread the
source length at `00426083` to decide whether to copy. The copy reads current
destination length, current source data, and current destination data in that
order. Its count is the current destination length, even if allocation changed
the source length. There is no destination-data guard and no local EH cleanup.
An exception during initial allocation leaves any callback-written destination
fields intact. This is a constructor body; it is not string assignment.

## Substring `00469840`

The local length/data header and output-constructed flag start at zero. EH state
1 is armed before the first source reads. A negative signed start adds its DWORD
representation to count with unsigned wrap and resets start to zero. Otherwise
start is unsigned-clamped to the source length. Source data is captured next;
null data skips the later source-length read. A zero length or start at/after the
current length takes the empty branch.

The nonempty branch compares wrapped `start + count` against the captured source
length. Only a greater result reduces count to `length - start`; it adds no range
error or overflow rejection. It captures the source address plus start, with
Win32 address wrap, before allocating the temporary with actual resize. It then
captures the temporary data pointer at `004698C1`. Nonzero count calls actual
`strncpy`, so an embedded NUL pads the remaining counted bytes with zeros while
retaining the requested string length. Replacing this operation with `memcpy` or
`strncpy_s` changes the recovered behavior. The source uses a narrow MSVC C4996
suppression around this call only.

The output is constructed from the actual temporary header through full
`00426060`. Only after that returns does `004698E4` set the output flag and
`004698EC` change the unwind state to 0. Normal temporary return uses the earlier
data capture, but **current temporary length plus one** after output construction.
An allocation callback can therefore replace the temporary header used by the
constructor and change the release size without changing the normal release
pointer. Source/output identity is supported through the temporary chronology.

The empty branch compares the output address with the local header address,
clears the output length and data, then calls resize-to-zero on distinct headers.
That resize is normally an equal-length no-op. No previous output is released.
The nonempty branch with count zero still follows temporary copy construction;
it does not call `strncpy` or allocate for the zero-length headers.

## Exception evidence and boundary

The original handler is `00C61B71` (`MOV EAX,D891E0; JMP BF6B43`). The complete
36-byte FuncInfo at `00D891E0` has magic `19930522`, maximum state 2, unwind map
`00D891D0`, null exception-specification list at `+1C`, and `EHFlags=1` at `+20`.
Reading only its first 28 bytes is insufficient for the host FH3 fixture.

| State transition | Original action | Established behavior |
| --- | --- | --- |
| 1 to 0 | `00C61B50`, 8 bytes | Pass current local header `[frame-14h]` to actual `0041DD20`. |
| 0 to -1 | `00C61B58`, 25 bytes | Test bit 0 of `[frame-18h]`; clear it; destroy actual output `[frame+4]` only when armed. |

The C++ implementation retains the state/flag structure. Temporary allocation
failure and output allocation failure both unwind the current temporary header.
The latter does not destroy or reset a partially constructed output, because its
flag has not been set. The original has no rollback of source or output fields.

The host receives an already-bound storage object; it does not perform native
singleton lookup on every operation. `NativeStringStorage::release` is `noexcept`.
Consequently the native post-construction singleton-getter/release exception
path is not exposed as a matched host behavior. A separate native-only fixture
injects a getter exception at this point and executes the original flagged
output cleanup. It verifies the original contract, not a throwing host release.
The normal host path cannot enter that catch through its `noexcept` release.
As with existing actual-header helpers, a zero-byte native `memcpy` is omitted
to avoid passing null pointers to the standard C++ operation. Allocations must
return nonnull; existing CRT/pool allocation policies remain unchanged.

## Verification

All 14 selected body, EH-data/funclet and hook-preimage spans were freshly checked
against the installed PE and live `/battlestationspacific.exe` in
`C:/Users/sqz269/bsp.gpr`. Each live read used the guarded `bsp.py ghidra` CLI.
The audit records exact bytes, SHA-256 hashes, old names/prototypes/comments,
source dependencies, and the private fixture artifacts.

One private Win32 fixture executes both complete original caller bodies, their
actual two unwind funclets, and the original relocated unwind map. It checks
every loaded span against the installed bytes before applying five recorded
relocations, a handler-entry bridge into the fixture image, and seven explicit
helper/CRT hooks. The handler bridge invokes the actual host
`__CxxFrameHandler3`; the constructor/substr control flow is not simulated.
Full original `00426060` remains the substring's copy constructor.

Both caller versions compose the same existing actual resize/destruction C++
bodies. Only the resize symbol is renamed in a separate fixture object so a
wrapper can expose the actual temporary header to controlled allocation
callbacks; the helper body is unchanged. `memcpy` and `strncpy` hooks call the
real host CRT operations. Controlled fixed buffers supply allocation/release
observations; this fixture does not execute the native sized allocator.

Eighteen focused matched scenarios cover counted copy after embedded NUL, current
source guard/data/length after allocation, self-copy abandonment, constructor
failure without output cleanup, substring padding, negative start, start clamp,
zero count, preallocation source capture, current temporary mutation during
output construction, source/output aliasing, initial/second allocation failure,
current-local cleanup, null/zero source, and a wrapped range reaching allocation
with the original count. The check passed **3,212 normalized words** (44 events),
with seven native copy calls, six native `strncpy` calls, and four original local
unwind executions. The separate post-construction getter-throw check passed the
original flagged output cleanup. Its trace is explicitly outside host parity.

The changed source and fixture compiled with `/std:c++17 /O2 /Oy- /EHsc /fp:strict
/MD /W4 /WX` for x86. `verify-seeds` passed all eight seeds; the final
`scripts/build.ps1` run passed `reconstructed_math` and `native_math_differential`
(2/2). Private CMake source registration was used only for this worker build.
No shared build registry, ledger, export, or Ghidra state was changed by the
worker. No tracked test suite was added. This is reconstructed, build-tested and
fixture-tested; it is neither native ABI-compatible nor game-validated.
