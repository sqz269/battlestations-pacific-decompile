# World base method-table readiness (CC12)

The World still lacks a real current Source method table. Its **four directly
evidenced entries** require genuine implementations of scalar deletion, entity
draining, the constant-byte leaf and entity updating. The first independent
implementation candidate is the complete **3-byte /2-instruction `009035D0`**
leaf. Implementing that leaf alone would not make the table or World owner
ready. This packet changes only this document/report and adds no Source credit.

The newly merged Header13 report now records Source1 with its fresh-helper and
Root validation holds cleared. Header initialization/cleanup, the matrix
sentinel and post-base chain headers are handled by separate packets; this
review does not reopen those storage contracts. The integrator owns the active
`004CB030` lease, so constructor publication is treated as the existing
verified external contract rather than a competing analysis/edit packet.

## Physical prefix and publication

Current target-verified live bytes at CE7784 are
`B0 B0 4C 00 90 43 90 00 D0 35 90 00 F0 4B 90 00`:

| Byte offset | Native target | Complete body | Native call contract |
|---:|---|---:|---|
| +0 | `004CB0B0` | 30 bytes /11 instructions | ECX=World, one stacked flags word, RET4; EAX returns the original World pointer. |
| +4 | `00904390` | 8 bytes /2 instructions | ECX=World, no stacked argument; replace ECX with `[World+4]` and tail to `009041A0`. |
| +8 | `009035D0` | 3 bytes /2 instructions | No input read, no stack arguments; AL=1, upper EAX and other registers unchanged, plain RET. |
| +0C | `00904BF0` | 69 bytes /28 instructions | ECX=World, stacked float delta, RET4; actual entity-chain walk followed by `00904600`. |

`004CB04C` installs CE7784 at World+0 before root/array construction; normal
destruction reinstalls it at `00904C5F`. These are the only current live data
xrefs to the table address. A future Source owner must install a real current
table pointer at the same publication phase, not the integer CE7784 token from
`WorldObjectLayout`. A Source table must have lifetime covering every owner and
hold callable current Source entries with explicit matching receiver/stack
profiles. No table is created or bound here.

The preceding DWORD at CE7780 is zero; it supplies no recovered RTTI locator.
The adjacent DWORD at CE7794 is `004CB370`, but CE7794 is independently installed
as the participant table by Native stores at `004D6BBE` and `004CB30D`. Current
participant Source likewise uses CE7794. Therefore the legacy **five-slot World**
comment in `world_deferred_destroy.hpp` does not admit a fifth World method.
This report binds the four evidenced entries only; code-pointer adjacency is
not proof of another entry or complete C++ class/RTTI layout.

## Required real methods and remaining edges

**+0: scalar deletion.** The complete `004CB0B0` first invokes actual normal
destruction `00904C40`, regardless of flags. It then tests low flags bit0 and
conditionally frees the same owner through `00BF6989`; after the free returns
it still returns that original pointer. There is no accepted current Source
entry for this method. Its matching allocation/free domain is already known,
but its required `00904C40` lifetime body remains unready. A raw-free-only entry,
the new parent-header cleanup composition or an empty method would be wrong.
This packet does not expand or repair the normal destructor.

**+4: actual chain destruction.** `00904390` is only the receiver adapter. Its
required `009041A0` body is 100 bytes /43 instructions and consumes the actual 12-byte
`{first,last,count}` header at World+4. It repeatedly reads the live head while
count is nonzero, conditionally unlinks using entity+34/+38, then calls that
actual entity's vslot0 with flags1. Each destructor may change the header; the
native loop rereads it. This is entity/class destruction, not the category
node-free domain of raw clear70.

Current `drain_entity_chain_009041a0` uses a separate `DeferredDestroyChain`
projection and `WorldDeferredDestroyHost` accessor/deletion callbacks. It also
adds a non-native `stalled` exit. `00904390` itself is documented as a call
contract in a header, without a physical actual-World Source entry. The exact
missing edge is actual header/entity storage bound to real entity scalar
deleting methods with their flag1 lifetime contract. A thin adapter around the
projected host API would not close it, and the absence of a known direct static
dispatch does not make this slot optional.

**+8: constant-byte method.** The target bytes are exactly `B0 01 C3`:
`MOV AL,1; RET`. This is a genuine implemented Native behavior, not a proposed
placeholder. It neither reads World nor consults its active/ready bytes. The
only current xref is the table DWORD at CE778C; Ghidra currently has no function
starting at this address. The table's actual code target and complete return
instruction establish the bounded body independently of that missing listing
definition. No listing/function/name change is made.

No matching Source entry was found in the current `src`/`include` search.
The first useful next packet is therefore a dedicated constant-byte Source
method at `009035D0`, with no callees, allocation, table or class ownership.
Its semantic contract is an ignored receiver and byte result1. A name such as
`world_base_slot8_value_009035d0` would be descriptive; an `is_active` name is
not established. A normal C++ byte-return interface can be qualified separately
from native ABI fidelity. A future native-compatible adapter must additionally
preserve plain RET/no stacked arguments, upper EAX, flags and the unchanged
register state; returning a 32-bit integer1 is not the same machine contract.
The semantic reason for the slot and any dynamic caller's return-type use
remain unknown. This is a bounded recommendation, not a Source/table edit.

**+0C: actual entity updating.** The complete native body dereferences
`[[World+4]]`, walks next at entity+38, tests byte+5C, and calls entity vslot+DC
with the supplied float for each active entity. It reads the next link **after**
the virtual call, then always calls `00904600`, even for an empty chain. Its x87
FLD/FSTP argument transfers are preserved as native evidence; no numeric or
binary-ABI equivalence is claimed from the projected implementation.

Current `update_world_entities_00904bf0` takes `vector<bool>` and an abstract
index-based `WorldEntityUpdateHost`. `GameWorldHost` fills that vector from unit
indices; its matrix pass likewise uses `vector<MatrixInterpolatorRecord>` and
abstract entity operations. These do not provide a method-table callable
actual-World body. Minimum real dependencies are the actual chain/entity
storage and entity vslot+DC binding, plus actual `00904600` storage/lifetime
behavior. The sentinel producer alone does not supply that pass. Its deeper
math/entity dependencies are deliberately not expanded in this packet.

## Table and lifetime admission boundary

All four entries must be real before publishing the corresponding Source table;
null entries, thrown `not implemented` placeholders, address tokens and no-op
methods cannot stand in for the three lifetime-bearing methods. The constant
leaf is sufficient only for its own slot. Method implementation and receiver
profile/ABI binding are separate acceptance steps.

The table is installed before post-base headers exist. That does not authorize
calling the normal destructor, drain or update on base-only storage: the latter
two dereference World+4, and `00904C40` dereferences World+8 before a null check.
Constructor-failure cleanup remains the specific array/root protocol, not
vslot0. Neither table publication nor the new raw storage fragments establish
the complete World class lifetime, production caller, startup or gameplay.

The [report](../reports/cc12_world_base_method_table_readiness.json) pins the
Native table/method spans, current Source interfaces and dependency-status
snapshot. Existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was
verified by each live CLI request. No Source/table, build, test/probe,
Ghidra/shared metadata or destructor-listing changes occurred.
