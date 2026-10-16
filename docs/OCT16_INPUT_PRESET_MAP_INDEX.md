# Actual preset map indexing

Whole `006A1E70..006A1F4A` is 219 bytes / 75 Native instructions. The owned
logical completed-key action `00C7F100..00C7F107` adds 8 bytes / 2 instructions.
The 10-byte handler at `00C7F108` and 44 bytes at `00DAC190` are read-only
support. Established `STL_Tree_MapIndex_StrKeyCI_N2C_006a1e70` and
`Unwind@00c7f100` names remain. The existing four-argument
NativeStringStorage/AV overload and all previous records remain unchanged.

The new raw overload is `index_native_input_preset_tree_006a1e70` in
`native_input_preset_map_index.hpp`. Native ECX is the tree, EDX is unconsumed,
and one stacked key-header pointer is removed by RET4. EAX returns the retained
node plus 14h, the mutable 20-byte mapped representation. Source additionally
receives the explicit default fifth-word preimage, SAME RawCtx, actual
0109EEA4/0109DE1C references and existing NativeCrtStricmpCallees: six stack
DWORDs, actual RET18h. This is a new C++ interface, not Original binary ABI.

Actual published lower bound returns a NODE before this body's first owner
diagnostic. A returning diagnostic does not repair the owner. Compare the
retained node against CURRENT owner.head4; only a non-end node reaches the
supplied header comparator, original key versus node key at +Ch. False is a
hit. Both comparator paths retain the same actual locale cell and required
provided CRT parent services, including their empty/high-bit/errno/invalid/
locale branches. The old two-argument CurrentCRT comparator is not used.

On a miss the five private mapped DWORD representations are
`FFFFFFFF,00000000,00000000,00000000,(preimage & FFFFFF00)`.
Native writes only the low byte of the fifth word; its other three bytes are
copied by the actual constructor. The explicit uint32 preimage supplies these
bits without reading an indeterminate C++ DWORD or choosing zero implicitly.
It is required representation input, not a semantic float or an automatic
recovery of the Original private stack. Private 20-byte mapped and aligned
28-byte pair storage introduce no host map, node overlay or container model.

Construct the pair before arming cleanup. Pass the constructor's ACTUAL
returned pointer to whole hinted insertion, with a real private 8-byte output
and by-value iterator containing retained owner/node. Read the actual returned
iterator OWNER before NODE, replacing both retained values. The guard borrows
the physical temporary pair, independently of the constructor return value.

Capture CURRENT temporary DATA4 while the completed-key guard is armed, then
disarm and end that C++ EH scope. Null DATA skips length and pool work. Otherwise
capture CURRENT temporary LENGTH0 and use unsigned length+1 wrap, call the real
raw getter before every return, and return the captured DATA through actual
pool return with the live small-return gate. No header reset or retry is added.
Normal cleanup does not call whole69E090, which would recapture DATA too late.
Logical unwind cleanup delegates actual raw69E090 on the completed key header
and SAME RawCtx, so its DATA/length reads occur when unwinding. No EBP reader,
extra key offset, fake frame or normal-payload guard is introduced.

After either path, diagnose the retained returned owner if null, then read its
CURRENT head and diagnose equality with the retained node. Diagnostics may
return, throw or terminate according to the actual SDK domain; returning does
not trigger repair or another validation. Return the retained node+14h even
after returning diagnostics. All reached child/provider progress and alias
effects remain inherited; this body promises no whole-call entry-preimage
preservation for overlapping storage.

| Actual Native instruction | Ownership and actual provider |
| --- | --- |
| `006A1E93 CALL 0069CDB0` | Owned whole body; actual nil29 lower bound |
| `006A1EA0 CALL 00BF6713` | Owned whole body; initial SDK diagnostic |
| `006A1EB1 CALL 00443D00` | Owned whole body; supplied-cell/callee comparator |
| `006A1EDC CALL 0069E6E0` | Owned whole body; actual raw pair constructor |
| `006A1EEF CALL 006A15E0` | Owned whole body; actual whole hinted insertion |
| `006A1F14 CALL 00419CC0` | Owned whole body; actual raw pool getter |
| `006A1F1B CALL 00BD1510` | Owned whole body; captured-data pool return |
| `006A1F24 CALL 00BF6713` | Owned whole body; final owner diagnostic |
| `006A1F2E CALL 00BF6713` | Owned whole body; retained end diagnostic |
| `00C7F103 JMP 0069E090` | Owned logical completed-key effect only |
| `00C7F10D JMP 00BF6B43` | Read-only handler; no extra Source promotion |

The Original main pushes handler C7F108. That handler loads FuncInfo DAC198,
whose sole unwind pair at DAC190 is `(-1,C7F100)`; no local try/catch exists.
FuncInfo words are `19930522,1,DAC190,0,0,0,0,0,1`. The action's runtime EBP-28h
is a Native EH convention, distinct from the normal body's EBP key register.
Source keeps compiler EBP and a real completed-key guard, armed after actual
constructor success, with a destructor explicitly noexcept(false). A second
cleanup exception during C++ unwinding reaches the language termination
boundary; this does not establish Original FH3/SEH/RTTI/fault behavior.
Existing Ghidra CALL_RETURN overrides at the two handler/action tails remain
untouched; exact E9 bytes establish physical JMPs.

The normal Win32 script passed all three existing suites in 7.80s, exit0.
No tests/probes were added and no owned Native body was executed. Fresh static
proof covers owned227B77, read-only10B2, all88 current contexts, 17 physical
branch/call sites, 44 EH bytes and 26 provider LF hashes matching published Main
28d884b161299605948b6d57486a206322a6a0cf. This includes the unchanged old
four-argument API. Source51 was authorized after whole hint and unique publication.

The actual CMake object contains 469 CODE bytes / 170 instructions across eight
CODE sections, 17 sections total and 24 relocations. Public body is 351B120,
logical cleanup 9B4, destructor 19B7, EH text 52B19, and four trivial helpers
38B20. Five INT3 bytes are padding; two NOPs are handler-entry instructions.
Excluding all seven gives 462B163. All 24 relocations are accounted for: 19 in
CODE (16 REL32 and three DIR32), three debug DIR32NB and two EH-data DIR32.
SAFESEH's symbol index48 resolves to the generated handler at text$x+Dh.
Data includes the one-state44B EH table, 4B SAFESEH, three16B debugF records,
415B directives, 208B compiler/object debug record, 136B checksums, and the
unreferenced 4B MSVC header weak `__Avx2WmemEnabledWeakValue` BSS datum; Source
introduces no global provider/state. No literal table, local try or typeinfo.

The compiler writes Source EH state-1 at public EDh immediately after hint
return, before raw OWNER/NODE/DATA loads F4h/F6h/F9h. Source guard semantics
extend through those captures, but these nonthrowing loads are optimized out
of the EHsc range; Original raw-fault cleanup scope is unproved. Captured length
load106h precedes getter10Eh; its pure +1 LEA119h follows the getter. Private
fifth-word initialization uses an explicit masked DWORD, with private store
scheduling distinct from Native's BYTE store. Captures and copied bits remain
correct in the qualified Source domain; private frame, flags, asynchronous
faults and Original instruction identity are not claimed.

Before the default build, both old unique objects were copied into read-only
content-addressed archives. The prebuild mapping receipt41959df6 preserves the
old f963 and alternative4338 hashes. The default build replaced the original
path with a62e607a; the historical f963 hash now resolves through its immutable
copy. Original receipts were not rewritten. All31 unique artifacts, old Root
seals and unrelated lazy-lock940f evidence are retained with that explicit
mapping. See the packet report and separate precommit/frozen-pin receipts.

RawCtx pool/currentCRT policy, borrowed feature/DF/SSE2, actual locale/errno/TLS
and SDK domains remain those of the published providers. This is reconstructed
and build-tested Source with static evidence, not Original heap/ABI/private
frame/FH3/SEH/fault identity, complete CRT ownership or game/runtime validation.
