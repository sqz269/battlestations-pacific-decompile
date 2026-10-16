# Preset temporary-key release

| Entry | Coverage | Original interface |
| --- | --- | --- |
| 0069E090 | Complete29B/12 instructions through0069E0AC | ECX actual header, EDX unused, no stacked input, plainRET |

The new Source wrapper directly calls the published real RawCtx overload of
whole0041DD20. Complete29-byte candidate/reference bodies match after masking
only two independently resolved E8 operands:8 bytes masked,21 unchanged.
Both bodies call the same actual getter and return providers. The existing
`BSP_NativeString_ReleaseStorage_0069e090` identity remains unchanged; the new
Source name is descriptive and provisional.

Capture data at header+4 before reading length. Null data skips length and
all provider/context-field accesses. Otherwise capture CURRENT length at+0,
add1 with DWORD wrapping and resolve the actual getter before every return,
including large blocks, disabled small returns and wrapped size0. Return the
captured data/size without resetting the header or rereading it after getter
work. No guard, adapter, callback, new context/global/cache, retry, duplicate
release, free, rollback or local catch is introduced.

| Scope | Native site and actual target |
| --- | --- |
| Owned getter | 0069e0a0 CALL 00419cc0 |
| Owned return | 0069e0a7 CALL 00bd1510 |
| Readonly reference getter | 0041dd30 CALL 00419cc0 |
| Readonly reference return | 0041dd37 CALL 00bd1510 |
| Readonly Original action | 00c7f103 JMP 0069e090 |

The context borrows the actual pool publication01090AA8, small-return gate
01090AA4 and raw-manager publication01090AA0. It creates no substitute pool
or lifetime model. Getter failure escapes this ordinary Source wrapper;
there is no `noexcept` boundary or exception conversion. Existing getter
progress is preserved and the caller owns secondary-cleanup exception policy.
The published return provider retains its current-CRT/critical-section domain.

The older `release_pooled_string_at0_0069e090` API still takes
`NativeStringStorage&` and remains `noexcept`. Its Source already reads the
correct fields. Its virtual-storage contract and historical report remain
separate from the new raw getter-failure transport.

Earlier name-ledger text said: "Returns the string buffer at +0 (length +4)".
That prose reversed the fields. Both complete Native bodies and both Source
implementations establish LENGTH0/DATA4. The old text is retained as a
preimage with an explicit correction; no older Source contract is rewritten.

Native null-data EAX is ambient, while nonnull EAX is provider residue. Source
void does not invent a return value. Added RawCtx gives SourceRET4 versus
Native plainRET. Original private argument/frame aliases, registers/flags,
CRT/heap, FH3/SEH/RTTI/faults, binary ABI and game behavior remain unproved.
C7F100's runtimeEBP action is readonly origin evidence, with no logical-action
Source, public EBP reader, map-EH, pair, hint or map-index promotion.

Validation and exact hashes are in the companion report. No new tests,
probes or Native execution are added; frozen-pin primary review is pending.

Default MSVC Win32 build and all three existing suites pass in7.42seconds.
These existing suites do not exercise this entry. The entire actual CMake
object has one19B/8-instruction function and one REL32 to the REAL RawCtx
overload. It forwards the context/header, consumes the cdecl arguments and
uses RET4 without local EH, helpers or code padding. The unreferenced4B
compiler BSS is uninitialized storage, not file bytes or application state.
The Source context argument is still forwarded when data is null; its object
must be valid even though the callee skips context-field accesses.
