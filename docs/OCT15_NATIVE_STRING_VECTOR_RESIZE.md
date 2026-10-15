# Actual pooled-string vector resize with owned fill parameter

Addresses: `0049E050`, `00C63540`.

The complete normal resize body and owned-parameter unwind are reconstructed
in `src/native_string_vector_resize.cpp`, using accepted full actual InsertCount,
erase and raw-pool destruction. The established library name
`STL_CheckedNativeStringVector_Resize_0049E050` and generated unwind name
`Unwind@00c63540` are retained. This is a new Source interface with an explicit
`NativeStringRawPoolContext`; original binary ABI/EH identity is not claimed.

| Address | Original ABI | Coverage |
| --- | --- | --- |
| `0049E050` | fastcall `ECX` receiver, `EDX` unused; three stacked DWORDs requested count/owned fill length/owned fill data; `RET0Ch`; no semantic result | complete actual Source resize, all normal branches and owner cleanup |
| `00C63540` | unwind-funclet `EBP+8` selects owned8h parameter; tail transfer to actual destructor | complete Source owner-unwind fragment, not a standalone native-frame API |

The Source API is `resize_native_string_header_vector_0049e050`. Its two fill
words transfer an already prepared owning argument, rather than borrow a fill
prototype. Source places the original bits into a canonical private eight-byte
`NativeString`, which has no implicit storage destructor. No extra allocation,
copy construction, generic value model or new allocator/copy callback is added.
A caller retaining a prototype must prepare an independent owned copy before
transferring the words. The data allocation is consumed even for shrink,
unchanged size or an empty vector. Nonempty values are supported even though
both known direct callers supply literal empty owned headers. The data/size
must satisfy the accepted real raw-pool/context return contract.

The caller stack was checked, including its cleanup: entry stack words are
return address, requested count, fill length, fill data. After the three EH
pushes, eight local bytes and four saved registers, normal `ESP=entryESP-24h`;
the parameter words are at `ESP+28h/+2Ch/+30h`, and state is at `ESP+20h`.
The callee's `RET0Ch` consumes the three parameter words. Native normal `EBP`
is used as a captured vector-end register. The unwind funclet's supplied `EBP`
is a distinct runtime frame basis; its `+8` selects the owned parameter header.
The Source canonical header/context/private frame are not an original stack
overlay and do not reproduce private argument-slot aliases.

The canonical10h vector uses begin+4/end+8/capacity+C and actual8h pooled-string
elements; preceding actual constructor/Buy producers establish those offsets.
Resize captures begin before arming state0 ownership cleanup. Initial size is
zero for null begin, without an end read, or `SAR32(CURRENT end-captured begin,3)`
with DWORD wrapping subtraction. It compares that size with requested count.

For growth it recomputes size against the SAME captured begin and a fresh end,
then captures another CURRENT end. Captured begin>end triggers the existing
current-CRT noinfo report. A returning report preserves captured size/end.
Delegate full actual InsertCount with owner=receiver, position=captured end,
count=requested-recomputed size with DWORD wrap, and the owned parameter header.
The child makes its own alias-protecting value copy; the parameter remains
owned by Resize. No extra owner validation, length clamp or append substitute
is introduced.

Otherwise null captured begin skips shrink entirely. With nonnull begin, read
another end and recompute size; requested>=fresh size also skips shrink.
For a shrink, retain this captured end through all later reports. First report
captured begin>captured end. Then capture CURRENT begin and report it>fresh end.
Compute target=captured-post-report begin+requested*8, preserving wrapping.
Report target>fresh end OR target<fresh begin, using short-circuit read order;
an above-end target skips the lower-bound read. Neither target nor old end is
recomputed after a returning handler. Delegate actual erase with both owners
equal receiver, target first, retained end last, and a private two-DWORD result.
The result is not read. No local rollback or additional validity check is added.

On any supported C++ failure after state0, Source destroys the CURRENT owned
parameter header through accepted actual `0041DD20`, then rethrows. The native
normal exit captures parameter data before disabling its cleanup state; null
data skips length/getter/return. Otherwise it captures length+1 with DWORD wrap
and flag1, resolves the actual current raw pool and returns the captured block.
Source reuses that same actual destructor outside its algorithm catch, so a
normal destructor exception is not retried. The Source private-header load
schedule and C++ catchability do not reproduce native fault/unwind identity.
If an unwind cleanup throws, later cleanup stops and the newer Source exception
escapes; original FH3/SEH/double-exception personality remains unproved.

Native FuncInfo `00D8B768` has magic19930522h, maxState1, unwind map `00D8B760`,
zero try/catch records and one unwind entry `{-1,00C63540}`. Handler `00C63548`
loads that FuncInfo then transfers to original FH3. The owned cleanup action is
`LEA ECX,[EBP+8]; JMP0041DD20`. Its tail transfer is byte evidence, not an
ordinary CALL claim. State0 is armed after the first receiver-begin read;
normal state -1 is stored before the pool getter/return. No Ghidra mutation,
native-frame object, synthesized placement-delete or separate callback is used.

All44 bytes of `[00D8B760,00D8B78C)` match between disk and live byte queries;
SHA256 `55aacd9c8c693f22881df373bf0228d2ad379d18b54ad2e86d8fa5e2227a526a`.
The original `[0049E050,0049E144)` body is244 bytes, SHA256
`f0c46828c6cf6e1095c57a394e9c4fdff0565405f6b07254f68a2a10a3256005`.
All92 decoded instruction starts match the exported assembly. The JSON report
contains the raw EH hex and18-byte unwind/handler decode. Ignored receipts are
`local/oct15_native_string_vector_resize_disk.json` and
`local/oct15_native_string_vector_resize_callers.json`.

Both known direct argument setups were inspected. The constructor caller
zeros both eight-byte argument words using its preserved zero EBX, requests its
captured count and addresses its actual string vector. The settings-loader
caller writes both words0, requests2 and addresses a captured nested vector.
Neither caller owns that parameter allocation after return. These limited
caller values do not narrow the complete callee contract to empty fill only.
The following ten ordinary direct CALL claims include both caller sites:

```text
0049E0A2 CALL 00BF6713
0049E0B3 CALL 0044F410
0049E0D0 CALL 00BF6713
0049E0DD CALL 00BF6713
0049E0F3 CALL 00BF6713
0049E103 CALL 004954F0
0049E127 CALL 00419CC0
0049E12E CALL 00BD1510
004A3735 CALL 0049E050
006A8F68 CALL 0049E050
```

The prior fragment `resize_native_input_settings_strings_0049e050` in
`native_input_settings_vector_storage.cpp` remains unchanged, with its original
`NativeStringStorage`, qualified inner append and fixture provenance. No prior
record or result is promoted to validate this new actual-pool Source function.
Accepted actual children were obtained from root staging and published
main `6f83fab39` before build. `./scripts/build.ps1` passed all three existing
suites; the native differential suite covers prior math seeds. JSON and Markdown
verification each passed all ten ordinary CALL claims, with no unchecked claims
or warnings. Owned-ledger/index and diff checks passed; only the two leased
addresses changed, all prior name evidence/metadata was retained and all prior
reconstruction records remain unchanged. CMake registration was queued, claimed
on turn and released after the append at the primary's request. No new routine
tests are added.
No new-function native differential, process binding, binary ABI, original
CRT/FH3/SEH/private-frame/fault identity or gameplay validation is claimed.
