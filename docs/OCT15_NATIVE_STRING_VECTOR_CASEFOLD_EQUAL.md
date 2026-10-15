# Actual-header string-vector case-insensitive equality

Packet `oct15_native_string_vector_casefold_equal` reconstructs the complete
ordinary `008D48E0` body as `equal_native_string_vector_casefold_008d48e0`.
The historical `native_string_vector_equal_008d48e0` host projection remains
separately recorded and unchanged.

The live target was verified as project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 little endian, image base `00400000`.
Complete pseudocode and all50 assembly instructions were inspected. All109
body bytes match the installed executable and saved/live Ghidra byte response.
There are no listing gaps, local cleanup, exception handler or analysis repairs.

The native input is ECX=this0Ch vector header and one stacked other-header
pointer, with RET4 and boolean AL. Only AL is defined as the semantic result;
different exits leave different upperEAX values. The Source interface is a
separate cdecl function with two pointer arguments and a boolean result.

Both actual vectors have data at+0, signed count at+4 and unused capacity at+8.
The routine reads other count first and then this count. Unequal counts fail;
equal signed nonpositive counts succeed without examining either data field.
The positive loop reloads both data pointers for each eight-byte element.
All index scaling and address additions use DWORD wrapping arithmetic.

Each element is an actual eight-byte stored-length/text header. Zero left
length succeeds only for zero right length; neither text field is then read.
Nonzero left length requires nonzero right length. After loading right text
and then left text, the Source uses current CRT `_stricmp`. Unequal nonzero
stored lengths are allowed. Embedded NUL suffixes and stored-length/content
disagreement therefore retain native behavior; no host string length is used.
After a successful element comparison, the incremented index is compared
signed against the freshly loaded other count. Failure stops immediately.

The incoming settings site passes the two object+98h addresses and consumes
AL immediately. Its prolog and local argument/result sequence were inspected;
this is interface evidence rather than a complete settings reconstruction.

```text
008D4929 CALL 00BF7FBF
008D4D1B CALL 008D48E0
```

No guards, snapshot, allocation, callback or ownership change is introduced.
Taken raw header words must be readable and text buffers must satisfy current
CRT string/locale contracts. Concurrent mutation and aliases to private native
or Source frames are excluded. Raw memcpy loads do not reproduce original
fault width/address. The current CRT boundary does not establish the exact
original CRT locale, invalid-parameter or exceptional behavior. Source builds
and static evidence do not establish binary compatibility or live game behavior.

Validation and binary identity are recorded in
`reports/oct15_native_string_vector_casefold_equal.json`. Existing Win32 checks
are used; no new tracked tests or new-function differential claim is added.
