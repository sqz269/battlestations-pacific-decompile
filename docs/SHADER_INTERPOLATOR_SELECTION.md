# Interpolator selection

`append_selected_interpolators_00b36800` projects the native field-selection
routine into owning C++ records. It appends ScreenSpacePos (float4, POSITION0,
mask zero), then visits base and effect descriptor fields in order. Existing
output is preserved; repeated fields are neither cleared nor deduplicated.

With both usage pointers null, every field is copied and its mask is regenerated
from its component count. Zero components produce zero; counts at least32 produce
all32 bits, matching the native repeated x86 shifts.

With either usage pointer supplied, only TEXCOORD or COLOR fields whose matching
buffer exists are eligible. Each semantic has its own flattened component offset,
carried across both descriptors. A usage DWORD holds four low-bit component flags:
component `i` reads word `(offset+i)/4`, bit `(offset+i)%4`. The native descending
scan counts selected components and preserves their original indices in the mask.
Offsets advance by the original field width, including fields with no selected
components; those empty fields are omitted. Other semantics are skipped. This is
component packing order, independent of the field's declared semantic index.

The native ABI is ECX builder with three stack arguments and RET0Ch. Its owned
pointer lists, string allocation and null-entry failure paths are not reproduced.
The typed interface rejects undersized usage buffers and cumulative DWORD offset
overflow before changing output. Native malformed-input memory access is not an
interface contract. Successful output appends only after collecting all fields.

Assembly and the full original/saved body bytes from00b36800 through00b36e2b
support the port; the hash is in `reports/shader_interpolator_selection.json`.
The existing generated debug/dummy shader fixture now obtains its output list
through the unfiltered operation. Win32 Release build, both existing CTests and
the D3D9 probe passed, including VS3/PS3 compilation and centerFF407FBF /
outsideFF000000 readback. No tests were added. The filtered branch has assembly
evidence only; reflection-producer integration and native differential validation
remain pending. This does not establish a runnable game or gameplay parity.

Follow-up: the existing draw now exercises filtering from compiled pixel-shader
disassembly with COLOR0=15. See `SHADER_REFLECTION_ANALYSIS.md`; sparse masks and
native differential validation remain pending.
