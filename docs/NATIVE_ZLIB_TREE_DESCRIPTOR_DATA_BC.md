# GOAL16AZ: repair of the unpublished AT DATA candidate

Current status: `QualifiedRegisteredDATAReconstructedBuildAndIndependentlyReviewed`.
Root accepted2676 physical DATA bytes with the qualifications below; function and
instruction credit remain zero. The cold producer originally sealed status
`StrictProducerReady_ActualRegisteredProviderPending`. This review-derived
revision supersedes the unaccepted AT candidate39b5afe38f25488c9a9b19ea2bca7ee75e4ceeeb.
It defines one authentic2616B const RO region and retains three mutable20B
descriptors:2676 candidate Native DATA bytes, zero functions/instructions.
The cold producer assigned zero acceptance credit and left normal registration,
build/checks and independent actual registered/Core acceptance to Root; the
completed Root review is recorded below.

## Concrete repair and coverage

AT's separate arrays matched their individual contents but did not retain the
Native layout. AU's real scratch/output alias produces raw index40: Native
extra_lbits+160 reaches extra_dbits[6]=2; old AT reaches extra_dbits[10]=4.
The stock public maps also place length_code before dist_code, so raw distance
index513 reads0 instead of Native length_code[1]=1. AX established the complete
authentic region and these known nonfault differences. This revision preserves
the real relative data instead of qualifying away the changed values.

| Owned Native inclusive range | Bytes | Actual Source storage |
|---|---:|---|
| 00D677C0..00D681F7 | 2616 | one `bsp_native_zlib_tree_ro_region_00d677c0` const object |
| 00E1470C..00E1471F | 20 | mutable `bsp_native_zlib_static_l_desc_00e1470c` |
| 00E14720..00E14733 | 20 | mutable `bsp_native_zlib_static_d_desc_00e14720` |
| 00E14734..00E14747 | 20 | mutable `bsp_native_zlib_static_bl_desc_00e14734` |

Coverage2676 = previous unaccepted1644 + four additional tables1004 + intervening
spans28. The28 comprise the complete named bl_order19 and nine observed spacing
bytes. No previously accepted1584B is duplicated. The old five standalone array
declarations/definitions are removed, with no aliases, fake externs or duplicate
storage. All arrays are real typed subobjects of the public aggregate.

## Grounded type and static initialization

`NativeZlibTreeReadOnlyRegion` is standard-layout, size2616/alignment4. Its members
are extra_lbits int32[29]@0; bl_order uint8[19]@116; observed spacing[1]@135;
extra_dbits int32[30]@136; extra_blbits int32[19]@256; observed spacing[4]@332;
static_ltree CodeWords[288]@336; static_dtree CodeWords[30]@1488;
dist_code uint8[512]@1608; length_code uint8[256]@2120;
base_length int32[29]@2376; observed spacing[4]@2492; base_dist int32[30]@2496.
All nine spacing bytes are explicitly initialized observed zeros. The former
20B span is bl_order19 plus one zero, not invented20B padding.

CodeWords has uint16 code/length at0/2, size4/alignment2. The unchanged descriptive
descriptor has two4B pointers at0/4 and three int32 fields8/12/16, size20/align4.
The header asserts standard layout, every field offset, every complete member
size, all aggregate/descriptor/codeword sizes and alignments, and Win32pointer4.
No packing directive or implicit object padding supplies missing Native bytes.
These are grounded new Source types, not recovered Original public types/version.

The real const owner uses full literal initializers from authenticated static
bytes, with explicit external C linkage. Descriptors remain actual mutable
C-linkage objects; pointer fields stay mutable pointers to const subobjects.
They initialize directly to aggregate static_ltree/extra_lbits, static_dtree/
extra_dbits and null/extra_blbits. Their tails stay257/286/15,0/30/15,0/19/7.
There are no image-pointer literals, runtime copies/snapshots, constructors,
getters, opaque buffers, imports, weak/selectany aliases or initialization helpers.

## Complete measured strict object

The revised variant passed its first MSVC Win32 compile:
`/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy /c`,1.7736591s including environment
setup. No Source/compiler rejection, Source correction after compile or recompile.
Whole object3687B SHA256:
`07aada03c9aa3b72e022e12db6f321ead8d93ced7f160bc3ec1a15ba038da055`.

Four strong external DATA symbols are the actual RO owner and three descriptors.
RO .rdata is exactly2616B at symbol value0, byte-equal to all Native bytes,
without extra section padding. Actual RO section alignment8 does not change the
asserted language type alignment4. RW .data is exactly60B: descriptors at0/20/40
in Native order without padding. Separate globals still lack a C++ adjacency
guarantee; this is a measured strict-object result and needs registered recheck.

Five real I386_DIR32 fixups at .data offsets0/4/20/24/44 target the whole defined
aggregate with actual addends336/0/1488/136/256. Combined40 is true null without
relocation. Substituting only Native root+addend in a comparison copy makes all60
descriptor bytes equal; Source does not initialize Original image pointers.
Static raw reads at RO+160/+2121/+2536 produce2/1/32, as Native requires.

The other sections are .drectve47B default-library metadata, .debug$S200B complete
object/compiler CodeView records and padding, and .chks6440B compiler metadata.
Full header/section table, all raw bytes, relocation tables, primary/auxiliary
symbols, string table and padding are accounted without overlapping regions.
There are zero executable bytes/functions/undefineds/weak symbols/COMDATs/.CRT
initializers/helpers. The proof directly checks relocation target primary records.

## Evidence, limits and pending integration

Accepted AX design Root SHAa6022077acbf391707e12cbdc8afa8b1007223c4b0898950e6904570012550b9
and AX receipt29f4b574030f9202532c34a9bf1b6699309eb7955ff36eba3dd0d9a15dd4671f
ground this repair. Fresh AX whole live/PE RO SHA2c08f5a908713badb206a5c98c07b95231feb8a4b751641fb2eb1666958d7b6b;
descriptors SHAdad11b10aed45032960b25fbd26be65ba5c50804a1a2efa3d926603b0af7b415.
All ten named arrays match the actual frozen stock DATA. The PE initially
declares RO/nonW/nonX .rdata40000040 and RW/nonX .dataC0000040. These are initial
declarations, not live protection measurements or an Original zlib-version claim.

Ordinary C++ out-of-array indexing remains UB. Later raw consumers must use real
owner storage and grounded effective-address offsets, without fabricated guards
or clamps. Original absolute addresses/pointer bits, out-of-region arbitrary
scaled indexes, malformed raw aliases/faults, writes to const storage, protection,
placement, full ABI, parent adoption and game behavior remain qualified. This DATA
repair does not close AU, AN, _tr_init, any caller or runtime behavior.

This cold producer changes only these four files and its new ignored output.
No Main refresh, CMake/ledger/Ghidra/cache edit, normal build, test, probe or
Native execution occurred. Root will register/build in its genuine warm target,
run existing checks, authenticate exact commands/Source and whole actual COFF,
and independently prove unique Core member/all4 providers/both complete archive
dictionaries/placement and prior preservation before credit/publication.
Old AP/AT/AX seals remain intact. The old AT first strict1.4320417s/2911B object
is historical only, not current acceptance evidence or an added timing cost.
The new bundle local/goal16az_region_revision seals selected small AX refs,
current Source/header/Git blobs/strict object/full proof; no old Core/history copy.
Its receipt records all observed setup/repair/compile/proof/helper refinement/
documentation/commit/seal/release overhead from15:17:06UTC. No speedup claim.

## Root registered provider acceptance

Exact producer Source/header were registered at `22ba4e74cb5422f78f78f6fc888a2da286c73a96`, incorporating Main `5ff85bbf4e60166660566208a8335490dade08c4`. Normal Win32 build passed in18.6132903s; all three existing checks passed in7.93s. The actual whole3699B object SHA256 `b40c2ecbbec1a633f13cc43abe4ac14970a43a4a23092381330487594b97d8da` has the same owned DATA and five genuine whole-aggregate fixups as the distinct strict3687B object. Both complete83083-binding Core dictionaries resolve four unique strong DATA definitions to the genuine full object. Root checked all2505 COFF symbol tables, both full dictionary multisets, and all23 previous AR whole objects unchanged.

Independent final review is authenticated by Root receipt `5859f4c7ef761c020e1c16aa09ad6d0e72360419f4d9a7f812a8d5ea05754072`. Actual proof `730073f1abde6e6342e42917c18afe030ad166b1e40e8deb98aadbde3c5a2474` and compiler provenance `1db15055903bbc608ca79daae8eed674bbf07a3943e77915038849adf19200b0` retain the exact current build evidence. Acceptance covers2616 RO bytes plus60 mutable descriptor bytes using symbolic pointer relocation. It does not establish Original absolute addresses, global placement, out-of-region accesses, fault/protection behavior, full ABI, parent adoption or game behavior. No selected-body execution, new tests/probes or measured speedup claim. Earlier producer-stage statements and sealed artifacts describe that historical phase.
