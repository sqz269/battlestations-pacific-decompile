# Native material parameter source ownership

`set_native_material_parameter_owner_00b18a40` applies the established
`00B18A40` behavior to the actual `NativeMaterialStorage` fields. The earlier
`MaterialCloneState` version remains a semantic consumer; this function adds no
second material, owner registry, widget token, or reference counter.

The complete 91-byte function spans `00B18A40..00B18A9A`. Its original ABI is
ECX=material, stack=owner pointer and low byte of the retention argument, RET8.
The descriptive name `BSP_Material_SetParameterOwner` is a reconstruction name.

Assembly reads byte `+10D` before loading the old pointer `+0C`. If that byte
and pointer are nonzero, it decrements old-owner `+04` through the native
InterlockedDecrement import (`CE2220`). Zero dispatches the old owner's current
virtual slot zero while material `+0C` and `+10D` still hold their old values.
The pointer is cleared after that callback. The function then writes the
incoming pointer and exact flag byte, and only then increments nonnull incoming
owner `+04` when the flag is nonzero (`CE221C`). A borrowed old owner is not
released. Equal pointers still undergo release and retain; the incoming owner
must survive the release independently.

The C++ entry uses the existing actual owner domain only for a terminal zero
count. This requires the real owner atomic and a canonical companion whose
current native profile dispatches its destruction. There is no fallback for
an unknown owner. As with neighboring actual owner functions, valid lifetimes
and nonthrowing terminal dispatch are preconditions. This interface is not a
drop-in implementation of the original calling convention.

Validation and exact source/artifact hashes are recorded in
`reports/native_material_parameter_owner.json`. MSVC Win32 and both existing
CTest cases pass. One ignored differential fixture executes the complete
original setter through eight transitions, comparing the full 114h material
slot (normalizing the two live owner identities), reference counts, and terminal
events. Both paths physically delete two actual native render contexts. Their
terminal callbacks observe the old pointer/flag and incoming count before
publication, then deliberately mutate material fields to check overwrite order.
The original setter's two imports use host Windows Interlocked functions; its
relocated virtual-zero boundary dispatches the same reconstructed canonical
context destruction as the C++ path. This does not compare the original
context destructor, allocator, or generic virtual-zero thunk. Scratch materials
use flags-zero destruction and contain no parameters or nonnull effects.

GUI widget/page ownership,
nonnull effect lifetime, source packing, drawing, and gameplay remain separate
integration work; this setter alone does not establish those paths.
