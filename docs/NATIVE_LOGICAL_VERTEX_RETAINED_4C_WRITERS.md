# Logical vertex retained+4C writer discovery

This pass examined twelve previously unclassified decoded DWORD stores at
non-stack +4C addresses. None establishes a nonnull writer into the retained
field of the D61D6C logical vertex stream or its D62B68 base. The required
writer -> incoming profile -> current-zero terminal -> allocation chain
remains unresolved. No owner, B24840 binding or B24BF0 reset source is added.

## Selection and verification

The candidate input is the earlier 498-entry decoded-offset heuristic at
`local/logical_vertex_terminal/4c_candidates_prioritized.json` in worktree
`battlestations-pacific-decompile-logical_vertex_terminal_next`. Its 31
graph-prioritized sites were excluded. The prior owner, terminal, owner-escape
and alias-escape reports were checked for already examined sites; none of the
twelve has a prior recorded exclusion or identity review. B56BDB and B5E2D8
had appeared only in the earlier raw-pattern listing. The other ten sites
were absent from the earlier discovery evidence.

This selection favors unclassified renderer-adjacent constructors, record
initializers and possible pointer stores. Known Lua/CRT/physics candidates,
already excluded render-resource/stream-count stores and an active material
pass owner were not selected. The batch is twelve sites, below the authorized
limit of thirty; no further candidate batch is implied by these results.

Each selected site's live function query establishes actual containment rather
than the heuristic's nearest preceding entry. Its current Ghidra instruction
start, exact original bytes and independent Capstone decoding agree on a MOV
to a non-stack memory operand with displacement exactly 4C. All twelve full
function-range byte spans match the installed PE. Twelve additional bounded
caller/provider spans establish the relevant receiver and source dataflow.
The combined fresh evidence is 24 spans / 8,031 bytes, including 7,277 bytes of
candidate function ranges and 754 bytes of context. No analysis mutation was
needed to define or rename functions.

## Examined sites

| Store / containing function | Actual dataflow and scoped result |
| --- | --- |
| A3B1F2 / A3B150 | EAX is zero from A3B154. Transfer the old +4C list head to the supplied output list, then clear +4C and +58. A3B870 visits sixteen records with stride49C; A3BC60 selects manager+19C+index*49C. This is embedded list-record cleanup, not a recovered nonnull retained-stream writer. |
| A4ABDC / A4A890 | Copy the seventh stack DWORD to receiver+4C while updating XUser-property caches. Caller A4AC40 computes this argument as the maximum of scalar DWORDs from its state and E188A8-backed game data. Adjacent +40/+44/+48 and fourteen DWORDs at +50 are cached together. No intrusive pointer publication is established. |
| AE1E4F / AE1E10 | EBX is zero from AE1E32. Initialize the +44/+48/+4C triplet after two native string headers. AD8EC1 allocates70h bytes and passes that allocation through AD8F85; this constructor later requests the literal `BushGroup`. This is a distinct70h owner construction, not a nonnull logical-stream+4C assignment. |
| AFA2C7 / AFA280 | Zero +4C while constructing an owner whose installed profile is D5DBC4. The initial intrusive count is1, but its different profile and string/field layout do not identify the independent pointee released by B62010. |
| B0CC44 / B0CC10 | Zero one of forty DWORD counters. B0CCE0 initializes two such arrays; B32410 supplies renderer+1B78. B0CCB0 copies forty DWORDs to array+ A0 before clearing the first array, and B2D8E0 supplies the same renderer field. It is an embedded renderer counter, not a logical owner. |
| B0F785 / B0F6E0 | EBX is zero. Capture existing +4C, decrement its reference and dispatch current slot0 at zero, then clear it. The sole live direct caller B14F60 installs D5E480 at the same receiver immediately before this function. This is a different owner's cleanup; it does not supply a logical-stream+4C producer. |
| B35510 / B354D0 | Zero +4C in a constructor that installs D5F314 before initializing its fields. The direct caller is B3C3A0's material-program construction path. No nonnull retained pointer is stored here. |
| B56BDB / B56B90 | Zero +4C in a constructor that installs D621F8. This site was previously only a raw byte-pattern candidate; actual containment and the zero/profile dataflow are now checked. |
| B5E2D8 / B5E270 | Store the second BD1860 result. The complete provider allocates a1Ch tracked critical section. B32410 invokes this constructor with renderer+1D2C, so the store is renderer+1D78, after constructing five native strings at record+14. It is an embedded renderer lock, not the logical stream's independent retained object. |
| BA72E0 / BA71C0 | Clear a released intrusive +4C slot after installing D63E34 at this different receiver. As with B0F785, this establishes a cleanup site, not a nonnull logical-stream writer. |
| BB1001 / BB0E30 | Store literal4 at +4C, adjacent to literal32h at +50, after installing D640E4. It is constructor configuration in a different profile, not a retained-object publication. |
| BC0AB6 / BC0510 | Store the low DWORD of an x87 FISTP integer result at +4C, restoring the saved control word afterward. The surrounding path constructs/configures a dynamic light; 873C80 passes its +20 object as the receiver. No pointer ownership publication is established. |

The two release-and-clear sites resemble B62010's mechanics, but their enclosing
receiver identity is different. Similar destructor code does not type the
logical stream's independent +4C pointee. The one nonnull allocation result
in the batch is a tracked critical section inside the renderer's embedded
record; that positive identity exclusion likewise cannot be adopted as the
missing logical retained-owner contract.

## Remaining boundary

These are bounded instruction/dataflow findings under the ordinary distinct
native object/allocation domains. They do not prove that logical+4C stays null
after construction. Unreviewed decoded candidates, other opcodes, bulk copies,
adjusted or escaped pointers, indirect writers and callback reentry remain
outside this pass. The 498-entry heuristic contains reads, address-taking and
non-instruction possibilities as well as stores; subtracting this batch does
not produce a count of all remaining real writers.

Before implementing the logical owner, recover a concrete nonnull+4C adoption
path with the actual incoming profile and complete zero-count/deleting/
allocator chain, or establish a complete scoped lifetime proof for every
admitted escape. Existing retained-memory profiles must not be selected merely
because their destructors are available. No generic terminal callback or
constructor-null restriction is justified by these negatives.

The ignored immutable bundle is `local/retained_4c_writers/` in worktree
`J:/PROG/battlestations-pacific-decompile-native-logical-vertex-retained-4c-writers`.
It includes original candidate inputs, prior-report pins, live listings,
function/byte checks, contextual witnesses and this report. Only this document
and its audit JSON are committed. There are no C++/shared metadata/Ghidra
changes and no build, fixture, native ABI or gameplay claims.
