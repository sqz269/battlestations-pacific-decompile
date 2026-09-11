# Native registry base leaves

Two complete entries are reconstructed: B197E0[9] and B197F0[41]. The first
stores original profile D5E550 and returns its incoming owner. It writes no
tree or publication state. The second tests bit0 of the current flags byte
**before** clearing F8D41C or installing CE3818, then conditionally frees the
captured owner and returns its pointer bits with RET4. It does not destroy a
registry tree. These base leaves differ from B1B660/B1B710, which call the full
registry destructor and then read their flags.

The new constructor's fastcall declaration retains ECX owner and plain RET.
The scalar adds EDX pointing directly to the actual current publication cell;
its current flags slot remains at entry ESP+4. One ten-byte MOV is rebound
from absolute F8D41C to EDX+disp32(0), retaining instruction length and EFLAGS.
The TEST condition survives the intervening PUSH/MOV instructions through JZ.
The only other changed bytes are the direct call operand to the existing
actual `singleton_lifetime_free`. No proxy pointer, callback, flag snapshot,
null guard, tree cleanup or source EH frame is introduced.

Fresh guarded reads matched seven disk-backed spans totaling77 bytes: both
full bodies, adjacent seven-byte CC padding, original free thunk and actual
profile slots. The actual F8D41C publication is separately qualified as four
saved-image virtual-zero bytes, not a current runtime value. Its pointer is
an explicit borrowed source binding. An independent worker reviewed the
complete source/header against the original instruction streams.

The strict main Win32 build passed both existing CTests and eight original
seed checks. The frozen build retains14 unchanged prebuild inputs and two
exact archive members. The finite proof checks all50 original bytes and15
instructions, the publication addressing adaptation, the exact current free
symbol relocation, and all154 COFF sections across the two actual objects.
The lifetime object matches the previous successful main archive exactly.

Saved Ghidra analysis initially had no functions at these entries. Both full
bodies are now defined, including the three-byte ADD ESP,4 after the returning
free call. Existing neighboring function/free metadata and address comments
are preserved. The repair journal retains the original missing-function state.

No new runtime fixture was needed. Static byte/provider proof does not execute
either new path or prove linked addresses, original CRT byte identity, native
SEH, incidental provider volatile-register/fault behavior or gameplay. The
returned freed pointer is never dereferenced. The source and actual free
provider retain their documented allocation-domain and normal-return limits.


Both complete entries are registered, their reviewed names and evidence comments saved, and affected exports forcibly refreshed. Immutable primary evidence: `local/registry_base/`; actual main build inputs and two archive members: `local/registry_base_build_frozen/`.
