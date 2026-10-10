# Locale owner entry windows and target readiness

The admitted 64-byte windows at `00CC8D30` and `00CD9410` identify two bounded
next Native body candidates: `00436230` and `00435A50`. Both entry sites still
have **no Ghidra function at or containing the queried address**. The recovered
prefix schedules below are instruction evidence, not established complete
routines or proof of owner initialization, destruction, registration or lifetime.
**Raw-map composition C++ GO and application GO remain false.**

This packet makes six read-only `bsp.py ghidra` query commands: one typed query
for both entry sites, two exact 64-byte reads, one prototype/size query for the
two immediate targets, one typed query for those targets, and one zero-context
listing query for eight prefix instructions. Target bodies are not exported,
decompiled or byte-read. No predecessor, sibling or third target is queried.
The existing `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` are verified
by the typed client. Retained complete typed responses are frozen at
modification 67 before and after. Later primary annotations do not change the
epoch of this evidence. No Ghidra, ledger, Source or C++ mutation occurs here.

Fresh bytes are exactly `00CC8D30..00CC8D6F` and `00CD9410..00CD944F`: 128 bytes.
They match the reused original PE copy byte-for-byte. Its SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`, length 12,223,752.
The installed game file is not reopened in this packet. A full frozen PE copy
is retained for independent hash/range replay; that does not admit target-body
analysis. The preceding provider/map/node evidence contributes 602 separately
reused bytes, not fresh reads.

The exact prefix at `00CC8D30` ends at its first `RET`, 22 bytes/six instructions:

```text
00CC8D30  MOV ECX,00E1768C
00CC8D35  CALL 00436230
00CC8D3A  PUSH 00CD9410
00CC8D3F  CALL 00BF6FF5
00CC8D44  POP ECX
00CC8D45  RET
```

The exact prefix at `00CD9410` ends at its first direct `JMP`, 10 bytes/two
instructions:

```text
00CD9410  MOV ECX,00E1768C
00CD9415  JMP 00435A50
```

Copied-byte x86 decoding and the existing Ghidra instruction listing agree on
all eight instructions. The first prefix forces ECX to the actual pool base
before `CALL 00436230`. Only if that call returns along its fallthrough does it
push the second entry address and call `00BF6FF5`; normal fallthrough from that
call reaches `POP ECX; RET`. Callee stack contracts and actual return behavior
are not established. The second prefix forces the same ECX value and directly
tail-jumps to `00435A50`. Its target is not `00436A50`.

Pushing the second entry address before the other call is a provisional
callback-registration/atexit-shaped clue. `00BF6FF5` receives no metadata or
body query here, and its semantics remain unclassified. No FS/EH setup appears
in the 32 prefix bytes; this says nothing about target bodies, complete routines
or owner exception/lifetime behavior. The remaining 42/54 bytes are retained and
mechanically decoded inside the fixed windows, with no routine ownership or
neighbor behavior assigned. No routine extent is inferred from padding.

Both candidate prototypes are stored `undefined FUN_...(void)`, and both have
stored `no_return=false`. Neither item proves the actual register ABI, normal
return behavior or semantic role. Typed bodies are complete address sets with
gaps; the prototype tool's min/max spans are not contiguous body sizes:

| Candidate | Exact typed body ranges, inclusive | Owned bytes | Min/max span |
| --- | --- | ---: | ---: |
| `00436230` | `00436230..004362E8` (185); `004362EC..00436302` (23) | 208 | 211 |
| `00435A50` | `00435A50..00435A6C` (29); `00435A78..00435A84` (13); `00435A88..00435AD9` (82) | 124 | 138 |

These exact ranges are the proposed next bounded body-review candidates.
Reading those bodies requires a new packet and ownership claim. This packet
does not add the gaps to their bodies, repair listing/flow/signatures, infer
constructor/destructor semantics, or close allocation extent, field
initialization, critical-section lifetime, table ownership or cleanup ordering.

The Source capture is commit `f8f7b1ca12b2a089fc67ebd3a16729eb1d0bac21`: 4,215
inputs comprising the selected 3,354 cohort and 4,208 tracked include/src paths
with overlap removed. All 4,215 actual committed blobs are retained and verified
against Git object IDs and captured files, with ten actual dependency blobs.
Compared with the preceding provider capture at `1232117fc`, the particle
lifetime header/source changed. Compared with the earlier compiled worker
Source cohort, those two files plus the four canonical-game-free lifetime files
changed. No current build or tests are run, and earlier compiled artifacts are
not proof that these six current files compile. A finite address-text search of
the 4,208 tracked include/src files finds no entry/target/pool literals; absence
of those literals alone is not proof that no semantically equivalent code exists.

The local evidence package is
`local/cc12_locale_owner_entry_target_readiness/evidence.zip`: 20,021,279 bytes,
SHA-256 `248f62df344990503f2aecbacef21853ca1c1f9e29eaa39a078d3410c8931d2d`.
It has 4,266 hashed payload files plus its manifest. Guarded replay passes for
the original ZIP and a physically copied ZIP with identical hash and result.
After loading its verifier, replay permits only read-only access to that one
ZIP; outside file reads, directory access, processes, network and dynamic loads
are denied. It verifies source cohorts/Git objects, dependency objects, original
PE hash/section mapping, both fixed windows, exact prefix branch targets,
listing agreement, null entry bodies and exact candidate body ranges. This is
finite evidence validation, not Native execution, fixture/build, ABI or gameplay
validation. The executable, saved analysis, OS and earlier proof packages remain
unchanged by this packet.
