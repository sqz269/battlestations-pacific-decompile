# Instance-generator serial owner readiness primary review

Root accepts the bounded process-owner proposal from worker commit
`85b8bd14edffb2220f0cb5aaf8a5a9fbc6a21791`. The next Source packet may append
one DWORD to the existing function-local `GameNativeRendererScalarProcess`
and expose a stable volatile DWORD reference. Existing field offsets and
process identity must remain intact. The accessor performs no increment,
reset, allocation, callback, Native operation or fixed-address mapping.
Actual generator/context/application wiring remains a later packet.

Root independently replayed 1,659 pin occurrences, 539 complete Source/Git
inputs, the exact patterns and all 4,207 files of the Source census, and all
1,227 immutable ZIP payloads in 1,229 entries. Two exact serial references,
44 context references and six attachment references match the retained census.
The unavailable historical CK `probe.cpp` remains explicitly unavailable;
no original Artifact or execution result is reconstructed from that pointer.

Root freshly verified original-image identity before and after the exact
472-byte header replay, then independently parsed the DOS, PE/COFF, optional
and four section headers. The `.data` section starts at VA `00e08000`, has
virtual size `00297edc`, and has only `00010000` raw bytes, ending raw-backed
VA support at `00e18000`. The exact DWORD `[0108fd30,0108fd34)` lies wholly
inside the virtual zero-fill tail and the image extent. Its initial loader
preimage is zero; no file-backed target DWORD exists. No target value, image
body, import/resource contents or callee body was interpreted by this gate.

Typed metadata identifies exactly four defined data bytes with no instruction
or function ownership at modification 32. The containing-block script query
failed because `/run_script_inline` execution is disabled. Its query and raw
failure receipt are frozen; the attempted facade was reverted, and no opt-in
or fallback occurred. Live Ghidra block provenance remains unverified.
The reference endpoint returned the retained load at `00b452bb` and load/write
at `00b452c4`, without a completeness/count attestation. This does not prove
the absence of initializers, resets, dynamic writes or address escapes.

The proposed Source cell is distinct from the logical texture serial. It is
initialized once in the existing process owner and survives renderer/application
replacement. Native pre-first-use state, ABI and runtime equivalence are not
claimed. The later graph still needs actual generator/profile/geometry/string
and declaration identities, persistent acquisition/failure frames and payload
retirement before shared drain. This readiness review itself changes no C++,
build, tests, Ghidra, startup or gameplay state.
