# Actual-storage entry for the effect direction kernel

The existing `material_effect_plane.cpp` already contains the x87 arithmetic
kernel for the `normalize=false` path of `0042D0D0..0042D18A` (187 native
bytes including both branches). This packet exposes that same kernel through a
public raw-pointer entry. It changes linkage and the typed wrapper's call name;
it does not reconstruct a second copy of the arithmetic.

The source entry takes destination in ECX, source in EDX, one stacked matrix
pointer, returns destination in EAX, and uses RET4. The original function has
an additional normalize DWORD slot and RET8. The source interface selects the
false branch. `00B448ED` in the material geometry binder selects that branch
with a zero normalize argument and a `(0,0,1)` source vector.

The kernel preserves the native source y/x/z float spills, x87 multiplication
and addition order, extended intermediates, destination float spills, and final
MOVSS copies. All matrix arithmetic precedes the destination writes. The
actual-storage caller supplies readable source/matrix storage and writable
destination storage; pointer aliasing follows the unchanged native load/store
order. The kernel does not translate the vector or call a normalization service.

The report compares the entire new public COMDAT against the saved old private
kernel, including relocation counts, and against the corresponding original
instruction spans. It also records the exact committed Win32 build. No new
test is required for this linkage change. The old typed interface remains
available. No new native function or byte credit is claimed, and the full
`0042D0D0` function remains incomplete because its normalize branch reaches
`00419510`, `00419440`, and the unresolved raw CRT square-root path `00BF7030`.
This interface permits composition of the proven false-branch contract; it
does not establish complete material submission, binary ABI substitution,
new native execution, rendering, or gameplay validation.
