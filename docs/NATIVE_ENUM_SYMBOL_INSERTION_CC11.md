# Native enum symbol insertion

Addresses: `008F2850`, `008F4DD0`.

This reconstructs the complete ordinary initializer and symbol-dictionary insertion
through new Win32 interfaces. Genuine initialized owner storage now feeds the existing
node readers, word getter and membership getter. It does not change `PropertyLibrary`,
bind native enum declarations/namespaces, or provide CEnum virtual destruction.

| Entry | Inclusive end | End exclusive | Bytes | Coverage |
| --- | --- | --- | ---: | --- |
| `008F2850` | `008F28E2` | `008F28E3` | 147 | complete ordinary insertion, 55 instructions |
| `008F4DD0` | `008F4E6C` | `008F4E6D` | 157 | complete ordinary initializer, 48 instructions |

All 304 original bytes match live Ghidra and the installed PE. Full assembly,
prologues and terminal returns are pinned under ignored `run02/inputs/`; original
bodies were not executed. The report records all six direct calls:

| Site | Callee | Containing body / path |
| --- | --- | --- |
| `008F285E` | `0048D480` | insertion / unconditional symbol finder |
| `008F287C` | `004E7C00` | insertion / miss, actual `00E17578` pool |
| `008F28A3` | `0041DD40` | insertion / fresh-header owning resize, preserve=1 |
| `008F28B8` | `00BF7680` | insertion / current source length nonzero |
| `008F4E36` | `0041DD40` | initializer / type header, length=1/preserve=0 |
| `008F4E4B` | `00BF7680` | initializer / current type data nonnull |

Historical `00BF7680` keeps its `_memcpy` name. Its admitted copy permits overlap;
Source uses current `std::memmove`, with the existing raw-string resize provider.
This is not historical CRT/fault parity or an original calling-convention binding.

The raw owner is borrowed from an actual aligned `19Ch` allocation, as witnessed by
the library's `008F69A7 PUSH19Ch -> 008F69AC allocation -> 008F69C9 constructor` and
the parser's `008F5F29 PUSH19Ch -> 008F5F2E allocation -> 008F5F4B constructor`.
These are allocation extents, not a recovered C++ class ABI. Its physical view is:

| Owner offset | Recovered initialization |
| --- | --- |
| `0` | opaque native profile `00D16508` |
| `4` | nested map profile `00D162C0`; actual insertion receiver is owner+4 |
| `8` | map count zero |
| `0C..108` | 64 null DWORD bucket heads |
| `10C / 110` | observed words `9999 / 0`, without invented range/default policy |
| `114 / 118` | genuine owning eight-byte string header; length=1, data contains `E` |
| `11C..19B` | inline authored-name area; **only first byte** is zeroed |

The type source is the verified `00D162D4` binding, bytes `45 00 00 00`. No fake map
constructor, type provider, virtual profile callback or enum metadata is created.
Map initialization is inline in this complete initializer, not a separate native
constructor. Unknown name-tail bytes remain unchanged.

Insertion captures the finder's bucket output. A hit stores only the opaque word
at node `+8` and reloads the current count. A miss obtains a genuine `14h` slot from
the completed enum node pool, initializes only its owning string header, preserves
the identity guard, resizes from current query length, then reloads source/destination
fields for the copy. It stores mapped word, current bucket head into node `+C`,
publishes the node, increments count and reloads it for return. Allocator-owned
node `+10` is untouched. No CString rescan, shared key pointer, default, reset or
rollback is added.

The sole actual caller `008F2E40` passes owner+4 at `008F2F27` and independently
rejects case-insensitive duplicates before this low-level insertion. Its first-symbol
retention remains separate from the low-level overwrite branch. The previously
bound semantic library policy is unchanged. The higher wrapper's remaining full
ordinary/EH/metadata binding is not claimed here.

The focused fixture uses two real current-CRT `19Ch` allocations initialized through
the new body, a genuinely constructed enum node pool, and real raw-string headers.
Installed `global.enums` supplies 22 `LandVehicleClasses` and six `SoldierTypes`
symbols. All survive 28 query-temporary destructions; five actual bucket collisions
exercise prepend/chain ordering. A case-variant replacement changes only node `+8`,
preserving key/header/chain/slot ID/count/owner and allocating nothing. One empty-key
miss retains the exact zero-length/null header. Overall, 30 queries are explicitly
destroyed, 57 word reads and 57 memberships pass, and 114 complete primed temporary
string-pool prefix restoration checks pass. Both owning type headers are distinct;
only the first authored-name byte changes from poisoned raw storage.

Manual fixture cleanup removes exposed heads, explicitly destroys each key and type
header, returns real slots, trims through the exact `CE37A4 -> 00410CD0` binding,
shuts down/unlinks the pool and releases storage. This is **not native CEnum
destruction**. Original global/static pool construction (`CC8A50`, `E17578`), CEnum
cleanup/deleting graph (`008F4E70`, `008F4B60`, `008F59C0`), private FH3 handlers,
declaration identity, namespace ownership and traffic registration remain external.

Successful immutable receipts are `local/cc11_scene_enum_symbol_insertion/run02/`,
with recipe `run_probe.py --out <fresh-directory>`. All 12 fresh TUs pass strict
MSVC Win32 `/W4 /WX /fp:strict`; link, COFF/provider checks, manifested Source probe
and embedded `asInvoker` extraction pass. Actual inventory is 41 project headers
plus 11 production CPPs: 52 production inputs, 54 including fixture and recipe.
Current primary `488abdcb0` full-build supports, same Source through baseline
`8cdccbcbd`, are frozen with equal original pre/copy/post and pinned pre/post hashes;
core SHA is `f1863c68b9064abcfc0b50c624b751e80a4c16b89ffee02ee7a1a531ead82ae1`.
All 458 historical receipt/support files, including the prior 307 receipts and pool
`run01`, remain unchanged. Failed compile-only `run01` is retained: an MSVC reserved
inline-assembly parameter name was corrected to `byte_offset` before clean `run02`.

Admission is successful coherent nonoverflowing, stable/disjoint storage, finite
consistent chains, closed NUL-free ASCII/C-locale queries, and null data only for
empty keys. Allocator failures, aliases/reentry/concurrency, malformed/fault paths,
historical CRT and original EH/SEH/ABI are unbound. No whole native wrapper replay,
CEnum destruction/namespace/enum identity, traffic or gameplay claim is made.
Main CMake registration/full build remain the primary's integration checks.

Primary integration at `d2d2fa396457f95ac46bb709f4a62fc9adf1c7b4` passed the full MSVC Win32 build and all three existing CTests. Independent primary validation freshly compiled12 actual TUs,53 Source/header/fixture pins,41 actual project compiler includes, the current linked BSP libraries (none for group storage), original PE and622 historical worker inputs. Complete Source COFF bytes match the worker. Complete ordinary147B55inst008F2850 and157B48inst008F4DD0 total304nativeB. Truealigned19Ch owner; outerrawD16508/map4rawD162C0/count8=0/64nullbuckets0C..108/10C9999/1100; genuineowningNativeString114/118 realresize(length1,preserve0) then current-data-conditioned current-length copyfromverifiedD162D4 Ebytes; ONLYfirst authoredname11Cclear/tailpreserved. Trueactual14hpool/RawString/lookup providers; insertionmapreceiverowner+4, capturedhashbucket; hitchangesonlynode8/countreturn, missownskey/currentpost-resize fields/chainC→bucketpublication→countincrement/reload;slot10preserved. Currentmemmove underexistingadmittedsuccessfulcopy domain, BF7680 historical_memcpy name retained/CRTparity unproved. Oneconnectedactual2Ctor/28installedsymbol/5collision/30querycleanup/57word57membership/114wholetemporaryprefixchecks/caseword-onlyreplacement+emptykey fixture; explicitactualkey/typeheadercleanup+slotreturns/trim/unlink NOTCEnumdestructor. Original2functionsbodies NOTexecuted; no inventedMap/virtual/defaultservices. ASCII-C-locale/stable-disjoint/coherent nonoverflow successful domain; originalglobalstartup/wholehigherwrapper/namespace/declaration/EH/failure/originalABI/gameunbound.
