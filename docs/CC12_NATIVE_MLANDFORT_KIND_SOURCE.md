Primary compiled review: Root read all 54 emitted bytes / 19 operations and verified an exact byte-for-byte match to the complete Native body, both RET4 exits, every local branch and the unique physical public definition. There are zero indexed public edges, calls or new EH records.

Combined MSVC Win32 build 2026-10-09T20:46:26.766022+00:00 to 2026-10-09T20:46:43.654990+00:00 passed all three existing checks. Both leaves total 113 bytes / 40 operations; all 35 previously reviewed objects remain byte-identical including code, EH and indexed relocations. The current receipt pins 121 inputs and four artifacts, with 37 whole objects and 41 selected positive Core definitions. The two roots are absent from the application map.

Root corrected the candidate documentation and header to describe a raw naked x86 load, without imposing an extra aligned C++ DWORD-object lifetime restriction. Fixed matches need no receiver data backing; a reached fallback needs actual readable receiver+C4 storage. C++ implementation is unchanged. The worker Source117 snapshot and its original header/document pins below remain historical evidence. Original whole-call ABI, production receiver ownership/dispatch, startup and gameplay are unproved.

# MLandFort kind query Source candidate

`006F5890..006F58C5` is a complete 54-byte, 19-operation leaf. Its existing
provisional Ghidra name `BSP_MLandFort_IsKindOf` is retained. Root approved
its complete body, PE/live/saved-start agreement and actual slot evidence in
`reports/cc12_native_unit_numbering_update_virtual_query_readiness.json`.
This packet reads only the owned query record; it does not re-audit the
numbering caller, other query bodies or the broader profile catalogue.

The entry loads the full tag from `[ESP+4]`, checks `1Bh,5,4,2,1,0` in that
order, then performs exactly one late `CMP EAX,[ECX+C4]` if every fixed check
failed. A match reaches `MOV EAX,1; RET4`; a mismatch reaches
`XOR EAX,EAX; RET4`. All 19 operations, seven short-branch targets and both
returns are represented directly. The zero check remains `TEST EAX,EAX`.

The Native SHA-256 is
`d487512e038c5230747b5b56380f0cfa842e5ddeeacce112e2daa048769953d0`.
The actual data slot at `00CFF3F8+5Ch = 00CFF454` contains `006F5890`.
The body and this four-byte slot were replayed against the installed PE;
the saved listing hash and all 19 instruction starts also match. This slot
observation does not establish a production receiver or dispatch binding.

## Source interface

```cpp
std::uint32_t __fastcall native_mlandfort_is_kind_006f5890(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);
```

The naked Win32 definition follows the actual `006DFE50` entry convention in
`native_ship_kind.hpp/.cpp`: receiver in ECX, a placement-only unused EDX
formal, and the raw tag at `[ESP+4]`. No semantic EDX input is invented.
Only that existing entry/interface was studied; the files are context pins,
not child providers. This new leaf contains no calls or relocations in its
intended body, and its emitted 54-byte size/encoding remains unverified until
Root builds it. The Source result exposes the actual full DWORD 0 or 1.

Fixed matches do not read receiver memory. On fallback, an ordinary caller
supplies the actual receiver with readable four-byte backing at +C4,
plus valid stack argument and return backing. The naked load retains Native
x86 access/alignment behavior and adds no C++ typed-view lifetime requirement. No null
test, eager id copy, field snapshot, class layout, global, dispatcher,
callback, guard, owner, `noexcept` or consumer is introduced. ECX, EDX and
nonvolatile registers are unchanged by the Native body. Raw fault, private
EH, whole Original class ABI, lifetime, startup and gameplay remain unproved.

This concrete leaf does not resolve `008761E0`'s actual receiver, profile,
current target or numbering services. In particular, query `1Bh` does not
select a profile. The existing source-held numbering decision is unchanged.

## Baseline and handoff

The Source117 requeue and byte-return primary receipts share the integrator
build `2026-10-09T20:20:30.934115+00:00` to
`2026-10-09T20:20:47.377222+00:00`: 117 inputs, four artifacts, three checks,
35 whole objects and 39 selected positive Core roots. The report records
every Root raw input/artifact check and normalized worker input check, with
raw worker matches and CRLF/LF-only differences distinguished. Claims apply
to the recorded worker-time snapshot; older Source109/113 builds are not
replayed as current evidence. Ship context files are pinned separately.

Only `native_mlandfort_kind.hpp/.cpp`, this document and
`reports/cc12_native_mlandfort_kind_source.json` are owned. The complete
candidate/document and the four-file diff were reviewed. No build, test,
probe, CMake, ledger, GPR, Ghidra or shared metadata edit was made. Root owns
registration, exact emitted-byte/branch and indexed Core review, and admission.
Source admission, Original ABI, startup and gameplay credit remain zero.
