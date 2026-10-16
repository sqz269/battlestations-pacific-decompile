# GOAL16AT: zlib tree descriptor DATA producer

Status: `StrictProducerReady_ActualRegisteredProviderPending`. This packet supplies
eight complete Source DATA definitions corresponding to 1,644 Native bytes and
zero functions or instructions. Accepted DATA credit is zero. Root must register
the translation unit, run the normal warm build and existing checks, and review
the actual registered object/Core providers before acceptance or publication.

The producer started from published Main
`dcea22c446e3537a757f6bbfa3c9cf854f8e6772` in its clean isolated worktree.
It changes only the Source/header, this document and the matching report. It adds
no CMake registration, ledger record, Ghidra annotation, parent function or test.
There was no normal build in this cold worktree; that required gate belongs to
Root's explicitly coordinated warm integration.

## Owned data and initial representation

| Native inclusive range | Bytes | Complete Source definition |
|---|---:|---|
| 00E1470C..00E1471F | 20 | `bsp_native_zlib_static_l_desc_00e1470c` |
| 00E14720..00E14733 | 20 | `bsp_native_zlib_static_d_desc_00e14720` |
| 00E14734..00E14747 | 20 | `bsp_native_zlib_static_bl_desc_00e14734` |
| 00D677C0..00D67833 | 116 | `bsp_native_zlib_extra_lbits_00d677c0[29]` |
| 00D67848..00D678BF | 120 | `bsp_native_zlib_extra_dbits_00d67848[30]` |
| 00D678C0..00D6790B | 76 | `bsp_native_zlib_extra_blbits_00d678c0[19]` |
| 00D67910..00D67D8F | 1152 | `bsp_native_zlib_static_ltree_00d67910[288]` |
| 00D67D90..00D67E07 | 120 | `bsp_native_zlib_static_dtree_00d67d90[30]` |

The header grounds a naturally aligned 20-byte Win32 descriptor: two 4-byte
pointers at offsets 0/4, then three signed 32-bit integers at 8/12/16; alignment 4.
Each code/length element has two unsigned 16-bit words at offsets 0/2, size 4,
alignment 2. Standard layout, all offsets, sizes and alignments are compile-time
assertions. These are descriptive Source types, not recovered Original public
types, a deflate-state layout claim, or a claim about the Original zlib version.

All three descriptors are actual mutable C-linkage objects. Their pointer fields
remain mutable; only the pointed-to rooted tables are const. The literal descriptor
refers to the complete 288-pair tree and 29-int extra-bits array, followed by
`257,286,15`; the distance descriptor refers to the complete 30-pair tree and
30-int array, followed by `0,30,15`; the bit-length descriptor has a real null tree
pointer and the complete 19-int array, followed by `0,19,7`.

The five arrays have actual external const definitions and full typed literal
initializers. No file-local stock symbol is declared as an invented provider.
There are no image-pointer literals, external imports, weak aliases, getters,
constructors, snapshots, runtime copies or initialization helpers. Native initial
PE declarations are RW/non-executable `.data` for descriptors (`0xC0000040`) and
RO/non-executable `.rdata` for tables (`0x40000040`). These observations and the
stock rooted const declarations ground the Source mutability choices; they do
not measure live page protection or prevent later Native protection changes.

## Measured strict producer

The first compile passed: MSVC Win32 `/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy /c`,
1.4320417 seconds including compiler-environment setup. The complete 2,911-byte
COFF object has SHA-256
`4df40a309d2b9c6fe5b9b439fe001d80b0883fa19d5162666499a8958f0f6c2f`.
All eight public definitions are strong external DATA symbols. The object has
zero executable bytes, function symbols, undefined or weak externals, COMDATs,
`.CRT` sections or executable helpers.

Its 60-byte RW `.data` contains the three descriptors in Native order at offsets
0/20/40, without padding. Five real `IMAGE_REL_I386_DIR32` relocations at offsets
0/4/20/24/44 name the complete defined arrays with zero addends. The null cell at
40 has no relocation; all integer tails match. Restoring only the five evidence
Native pointer values to a comparison copy makes all 60 bytes equal. Source never
uses those Original image pointers as initializer values.

Its RO `.rdata` contains the five tables at offsets 0/120/240/320/1472. Every one
of the 1,584 table bytes matches the authenticated Native bytes. The section is
1,592 bytes because of four zero alignment bytes at each of offsets 116 and 316;
those eight bytes receive no Native DATA credit. The actual section alignment is
8 for `.rdata` and 4 for `.data`; this does not change the asserted language type
alignments. Native intervening gaps are not owned or reproduced as data.

The remaining sections are `.drectve` (47 bytes of default-library metadata),
`.debug$S` (224 bytes: object/compiler CodeView records and padding), and `.chks64`
(40 bytes of compiler metadata). The complete header/section table, all raw
sections, five relocation records, all primary/auxiliary symbols, complete string
table and padding are recorded in the ignored strict proof. No bytes are omitted.
One local proof assertion was corrected to use the parser's numeric header
machine field; no Source, compiler command or object changed after the compile.

## Evidence and remaining gates

AP's accepted design/Native data/stock Source proof is the evidence basis:
Root AP receipt `898cfc7c93c1ed4cd7adb22e67def355b3cefe4c41bfc559e2591524a2ba69c9`;
AP manifest `b06827ff7c34bc9e9f23145f45acce5a6bbc22445c3888ef935df0029e414e69`.
The ignored `local/zlib_tree_descriptor_data_bc` bundle freezes only selected AP
inputs, Source/header, exact compile command/log/result, the whole strict object,
full COFF/data proof and this packet's handoff/seals. It does not recopy old Core
archives or recursively freeze review histories. Its final receipt records full
observed producer time through sealing/release, including setup and corrections.

Separate C++ globals have no language adjacency guarantee. The measured strict
object preserves the Native 60-byte order, but actual registered/link placement
must be rechecked. Source symbol addresses and pointer bits differ from the game;
no Original storage identity or out-of-object adjacency is promised. Arbitrary
raw descriptor aliases, invalid pointer representations, malformed accesses,
out-of-range indexing and their faults are not proven equivalent by typed DATA
initializers. No repair, reset or protection mechanism is added.

Normal target registration, actual C++17 project/command provenance, full actual
COFF, unique Core member/eight strong providers in both complete archive
dictionaries, placement and prior-provider preservation remain Root gates.
AN's `init_block` and the 98-byte `_tr_init` parent are separate packets. There is
no parent Source adoption, Original ABI, runtime, game or performance claim here.
