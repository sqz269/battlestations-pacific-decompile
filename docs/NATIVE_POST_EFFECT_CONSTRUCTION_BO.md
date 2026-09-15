# Native post-effect construction BO

Addresses: `00B4E840`, `00B3CD10`, `00B3CD20`. Descriptive names are hypotheses.

`construct_native_post_effect_00b4e840` reconstructs the complete 948-byte,
312-instruction normal body in an explicit source admission domain. It composes
the existing raw creators and concrete owners, including the raw-name model
extension and the 40-byte draw record. It also represents the twelve-state
cleanup map as ordinary C++ transitions. This is source reconstruction, not a
drop-in binary ABI, native FH3, renderer initialization, GPU or game result.

The branch starts at `077cf16e67e65115700793e4adc9aad5b1225ffe`, with BN
`ac498c87d80598ba7dc682e22b3b52b0be7a044a` cherry-picked as `0f0baa86`, then
BP `54c34b9d59e29460d335649d76c76c89f03ffea7` and
`8f2c9521e5c62f314f8fed1e3a729f569d404565` cherry-picked as `bbed04a5` and
`dfbd9105`. The second BP change is required for direct raw-name destruction.
The old VFS linker-options block removed by main remains removed.

| Routine | Complete native range | Original interface | Source coverage |
| --- | --- | --- | --- |
| B4E840 | B4E840..B4EBF3, 948 bytes | ECX raw24h/36-byte receiver; stack name header, primitive count, optional size input; EAX receiver; RET0Ch | Full normal body and logical cleanup map, within the provider boundaries below |
| B3CD10 | B3CD10..B3CD13, 4 bytes | ECX surface; EAX current DWORD+1C; RET | Exact width projection |
| B3CD20 | B3CD20..B3CD23, 4 bytes | ECX surface; EAX current DWORD+20; RET | Exact height projection |

The source receives a caller-owned, immovable `NativePostEffectConstructionBlock`
prepared before native effects. It reserves the camera scene/lifetime bindings
and two viewport records. Fixed optional storage holds all owner/reference
companions. Other canonical registrations may allocate metadata. Each concrete
reference borrows the original live atomic at actual+4 and adds no retain.
The block, its contexts, the original cells/tables, pools, registries and all
semantic views must outlive every surviving child, including failed attempts.
Preparation checks the shared CE4970/CE4ADC/D7A24C cell identities, renderer
publication, stream/declaration type-size table and current profile views.
The cache's string bridge and layout service must have been constructed with
these same canonical domains; their private bindings are a caller precondition.

## Native writes and acquisitions

ESI captures ECX at B4E85B; EBX becomes zero at B4E85D. B4E863 installs CEB130.
The source begins the actual `atomic<int32_t>` lifetime with count1 only at
B4E86E. B4E87F installs D61EC8, then clears +0C/+10/+14 and stores the original
count at +20. It does not initialize the complete allocation: +08/+18/+1C
retain their preimages until their individual publications.

| Publication or retained relation | Native creator / operation | Concrete source provider |
| --- | --- | --- |
| +08 at B4E8BD | CRT40h B4E891, B1FBB0 | Native frame target constructor; direct existing terminal context |
| Declaration local | `pf44uf42cc.mvfm`, renderer CURRENT+38 at B4E900 -> B317E0 | Raw string header and declaration cache; canonical declaration registration/reuse |
| +18 at B4E947 | Fresh renderer CURRENT+5C at B4E941 -> B287C0(count*3,1000h,declaration) | Registered logical vertex creator and `NativeLogicalVertexReference` |
| Captured mesh | B73B60/B73D70; B73BB0(mesh,0,CURRENT+18) at B4E993 | Raw canonical mesh slot and `NativeMeshReference` |
| +14 at B4E9A5 | 535320(actual effect-name header); B18A40(material,receiver,0) | Raw material factory and `NativeMaterialReference`; owner assignment adds no retain |
| Captured section | 533FA0; +08=4,+0C=0,+10=count*3,+14=0,+18=count | Raw section factory and `NativeMeshSectionReference` |
| Section/mesh relations | B864C0(CURRENT+14), B865A0(mesh), B73C60 | Concrete canonical owners and existing `GuiTextNativeLayoutServices` |
| +10 at B4EA33 | B74EB0/B75030(`PostEffectSysObj`) | Persistent model owner/reference; actual-header raw node name path |
| Model geometry | B75170(CURRENT+10,0,captured mesh,s,s) | Existing retained model geometry assignment |
| +0C at B4EAC5 | B71930/B71A80(prefix43C130(`PostEffectSysCam - `,effect name)) | Persistent camera owner/reference, raw name and admitted initial viewport |
| Replacement viewport local | CRT34h/B1F850; optional dimensions; B71990(CURRENT+0C,viewport) | Existing viewport owner and second persistent registry record |
| +1C at B4EBAB | CRT28h/B51BD0(leading+0,section,mesh,CURRENT model,CURRENT camera,visibility1) | BN raw draw-record wrapper and actual B51A20 access services |

Each creator's identity is recorded separately from current parent fields.
Current model/camera fields are resolved through the same canonical node runtime
at the actual use sites, so a callback replacing a published field is not hidden
by an earlier companion. The native mesh and section locals remain captured.
The raw 28h record has no new count or reference owner; +20/+24 remain untouched.

The x87 sequence at B4EA60 loads D7A260 once, reads the current model, then FSTs
the +17C argument and FSTPs the +178 argument. The source preserves this order
with inline x87 instructions. Draw-record staging likewise preserves FLD1,
current model read, FSTP visibility, current camera read, FLDZ/FSTP leading.
The BN wrapper preserves its own native x87 argument copies. Masked exceptions
and the existing x87 control domain are required; this is not fault-time proof.

Declaration decrement reads current CE2220 at B4E94A. The replacement viewport
reads it independently at B4EB5C. B4EBB5 captures it once for the final section
and mesh creator releases. On zero, the current profile and virtual0/deleting
slots are checked, then the canonical companion performs the established
terminal without a second decrement. Supported profiles are D61D1C/B48CA0,
D63194/B86690, D62D60/B74280 and viewport D5E5F8/B1F8F0, all through BD30E0.

## Optional dimensions

The complete optional branch calls current virtual+20 first, then reloads the
input/current table and calls +1C. D619A0 has B3CD20/B3CD10 in these slots, and
their complete four-byte bodies return current +20/+1C respectively. The source
admits that actual surface profile and preserves the height-then-width order.
It does not assume the D61948 texture profile, whose same offsets mean something
else. Other nonnull profiles remain an explicit unsupported input domain.

All four known call sites pass null: B528D2 in B52860 uses literal zero;
B52B2A in B529A0 uses PUSH0; B52DC1 uses EDI zeroed at B52D5A; B52F39 uses EDI
zeroed at B52EAB, with no intervening writes before their pushes. This caller
evidence does not justify removing the nonnull branch or claiming an actual
native nonnull caller. B529A0 was inspected for these argument paths only.

## Cleanup and host preservation

CBFC3B..CBFC44 is the raw ten-byte FH3 route `MOV EAX,DF8758; JMP BF6B43`.
DF8758 declares maxState12 and map DF877C. The raw funclet closure is
CBFBC0..CBFC3A (123 bytes), including the POP/RET tails omitted from some saved
Ghidra body bounds. No worker definitions, repairs, annotations or saves occur.

| State | Next | Funclet | Action |
| --- | --- | --- | --- |
| 0 | -1 | CBFBC0 | BD30F0 base profile restoration |
| 1 | 0 | CBFBC8 | Free captured raw40h frame allocation |
| 2 | 0 | CBFBD3 | 41DD20 declaration temporary |
| 3 | 0 | CBFBDB | B72F70 failed raw mesh slot |
| 4 | 0 | CBFBE3 | B748C0 failed raw model slot |
| 5 | 4 | CBFBEB | Clear mask1, conditionally destroy model name |
| 6 | 0 | CBFBEB | Same conditional cleanup |
| 7 | 0 | CBFC04 | B71350 failed raw camera slot |
| 8 | 7 | CBFC0C | Clear mask2, conditionally destroy camera name |
| 9 | 0 | CBFC0C | Same conditional cleanup |
| 10 | 0 | CBFC25 | Free captured raw34h viewport allocation |
| 11 | 0 | CBFC30 | Free captured raw28h draw allocation |

The body writes states0/1/2/3/4/5/7/8/10/11; 6/9 exist in the map but are not
invented as body visits. The source consumes each transition/raw-return or
release obligation before calling its action. A cleanup exception continues
the remaining ordinary C++ schedule without retrying the consumed action.
If a cleanup itself throws while another exception is being handled, this
explicit source policy finishes the remaining actions and propagates the newest
cleanup exception, replacing the earlier exception. This preserves the consumed
ownership schedule; it is not the terminate-on-double-failure policy used by
some other source providers and is excluded from native FH3 equivalence.
The normal model temporary clears mask1 before cleanup. The normal camera
temporary does not clear mask2, matching B4EAC8 onward. Outer temporary cleanup
uses the actual raw pool getter and can throw; persistent headers and captured
return diagnostics remain identifiable after failure.

Only a failed native child constructor returns its raw pool/CRT allocation.
Completed creators survive later native or host binding failures when native
states have disarmed. GUI `create_mesh/create_section/create_material` are
deliberately not used because their host metadata failure rollback would destroy
those completed objects. The acquired-before-registration stream output records
a completed stream even if its native renderer-array registration throws.

`native_completed` is a HOST-only fact set after the final two native releases.
An unexpected post-success owner/reference binding failure must not run base
cleanup, free the constructed receiver, or reclaim the block. A caller must
honor that marker before applying any failed-constructor raw free. The new
reference is bound without retain before a successful source return permits
caller publication. No B52860/B529A0 source integration is claimed here.

Retirement callbacks unbind using captured identities, without reading returned
native storage, and retain the optional companions until explicit external
quiescence. Reset performs no native cleanup. A failed camera can leave its
initial viewport alive after the camera raw slot is returned; its persistent
record remains live. This block does not invent orphan recovery, a cleanup
queue or a late destructor. Unbound completed-owner diagnostics may require a
separate disposition path before the block can ever be reset.
The acquired diagnostics expose borrowed pointers to every successfully
constructed reference before canonical binding begins. A registration failure
therefore leaves the exact unregistered companion available for explicit
external disposition, rather than an unreachable private live reference. These
pointers do not grant permission to replay an already consumed native release.

## Remaining provider and validation limits

* 535320 calls renderer actual+48 as a callable target. This must be a real
  acquisition implementation in the same admitted D5F0A8/current-table domain,
  supplied by the caller's existing bindings. Numeric addresses alone are not
  callable rebuilt source. Renderer construction/binding closure is unproven.
* If 535320's final effect release throws after material construction, that
  existing provider preserves the completed material but does not expose an
  acquired-before-release identity to BO. BO does not fabricate one or free it.
* The existing 43C130 prefix overload uses `ActualNativeStringPoolStorage` and
  retains its nonthrowing nested cleanup boundary. BO-owned normal/unwind
  temporaries use `NativeStringRawPoolContext`; this does not broaden 43C130.
* Declaration-cache, material, layout, node, camera, stream and draw providers
  retain their documented service, raw-storage materialization and exception
  domains. Canonical terminal callbacks inherit `RenderCommandReference`'s
  noexcept boundary. No native exception identity or hardware unwind claim.
* Null allocation branches are preserved. Where native code subsequently
  dereferences null, source diagnostics delimit the supported valid-storage
  domain; they do not reproduce access violations. Pools, current profiles and
  aliased cells must be valid for the original accesses; no fabricated defaults.

The accompanying report carries exact PE/live span hashes, call-site checks,
source/dependency pins and final build/object evidence. No new native fixture,
game execution or tests are introduced for this packet.

The final Win32 build and its one configured existing test, `reconstructed_math`,
pass. The mechanical call audit checks 47 direct/tail rows with zero failures;
twelve actual indirect rows remain explicitly labeled and are justified by the
listing, table and provider evidence. Static generated-code review follows the
receiver identity through the first allocation: CEB130, one count1 store,
D61EC8, +0C/+10/+14 zero, +20 original count, with no count0 store. The generated
seven x87 operations preserve the load/spill and intervening field-read order.
This evidence does not execute the new constructor or prove the failure paths.
