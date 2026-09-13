# Native input binding slots

Addresses: 00A92790, 00A92820, 00A93750. Packet `orch4_input_slots_al`.
Names are hypotheses. Source: `native_input_binding_slots.hpp/.cpp`.

These three complete bodies operate on the same actual24h owner,30h actions,
34h bindings and14h modifier arrays produced by the native Inputs parser.
They supply the slot operations needed by the remaining settings-application
path in006AB820 and006AA640. The existing value projection in
`input_binding_install` remains a separate source interface.

| Entry | Native ABI | Inclusive end |
|---|---|---|
| A92790 | ECX owner; stack action, signed slot, descriptor14h output, float output; RET10h | A92819 |
| A92820 | ECX owner; stack action; EAX signed count; RET4 | A92833 |
| A93750 | ECX owner; stack action, signed slot, five descriptor words by value, float; RET20h | A937C1 |

The count is action+14, with action address owner+4 base plus action*30h.
The reader uses signed count>slot with no lower-bound or action check. A present
record supplies five ordered DWORD loads/stores from binding+4..+14, followed
by x87 FLD/FSTP from scale+30. Supported source/output overlap must preserve
that sequence; an overlapping store can affect the next source load.

A missing record writes {-1,0,0,-1,flag}, then positive-zero scale via MOVSS.
Only flag's low byte is initialized. A927CA clears a byte in the native local
at entryESP-4; A927CE reads that entire DWORD. Its high24 bits are incoming
stack contents, independent of the output preimage. The source reader takes
an explicit scratch preimage to represent those unspecified padding bytes.
It does not read uninitialized C++ storage or silently promise zero padding.

Installation captures the action address before checking signed count<=slot.
When needed it calls existing concrete A93500 with wrapped slot+1, then reloads
the binding base through the captured header. Five descriptor words arrive by
value before allocation. Stores replace class, index, cached pointer, code,
curve byte(class==2), the full flag DWORD and scale. Scale uses raw bits here,
so installing a signaling NaN does not perform the reader's x87 conversion.
Both modifier headers and untouched padding retain their current ownership.

The final call is the existing concrete A91E80 over the entire captured action.
It resolves current primary and modifier pointers through the supplied actual
backend lists, including bindings other than the replaced slot. ClassFFFFFFFF
retains its incoming cached pointer while clearing resolved. No device query,
polling, reference acquisition or extra rebind is added by these functions.

Validation: live Ghidra and installed PE bytes match all three complete bodies
(138,20,114 bytes;48,6,39 instructions). Stored flow has no gaps. Both outgoing
CALLs and all eight current incoming CALL sites were inspected and mechanically
verified. Strict Win32 build and both existing CTests passed.

One ignored manifested fixture executes the original three bodies in an isolated
process. Only the installer calls are relocated to the same concrete source
A93500 and A91E80 used by the reconstruction. Its189 compared words cover raw
descriptors, x87 scale cases, explicitly seeded missing-slot scratch, ordered
overlap, a valid interior-base negative-slot access, whole-action and modifier
rebinding, full flag/padding retention, growth and the live default scale.
The native installer makes one resize and three rebind calls. Backend pointer
tokens are copied into caches but never dereferenced as devices; hardware calls
are zero. Newly allocated unspecified padding is masked, while controlled
existing padding and the full installed flag word are asserted separately.

This is a source service ABI, not a binary replacement. Original FH3/SEH,
arbitrary invalid storage, concurrency and gameplay are unverified. No permanent
tests were added. The raw540h settings producer, tree/application graph and
remaining699AD8..699B7D full loader tail are still unfinished. The typed settings
objects cannot be cast to the native layout to close those dependencies.

Exact final combined-source build and fixture hashes are pinned in
`reports/native_input_binding_slots.json` before publication.
