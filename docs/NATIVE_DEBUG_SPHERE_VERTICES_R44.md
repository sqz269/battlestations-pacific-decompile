# Native debug sphere vertex interior

`native_debug_sphere_vertices.cpp` adopts only the B2BF60..B2C142 interior
of B2BB90. It is a source interface for a fragment, not a new original function
or a callable replacement for the parent renderer.

The historical function body comes from `a6b2884f2` and is unchanged. Its 123
annotated instruction rows preserve the three rings: 13 XY vertices, 12 YZ
vertices beginning at index 1, and 13 XZ vertices. Each vertex contains three
float words and the supplied color word, for 608 output bytes at stride 16.

## Source contract

The new function receives output in ECX and four center/radius DWORDs in EDX.
The color DWORD and a borrowed pointer to the actual CEC730 double are stacked;
the wrapper returns with RET8. It snapshots the four input words into its own
scratch frame before writing output. Each angle calculation reads the borrowed
double at its original FMUL64 point.

The kernel keeps every FILD, float spill, FCOS/FSIN, radius multiply, addition,
ordered temporary write and loop branch. It uses the caller's current x87
control state. No trigonometric library call, SSE arithmetic replacement,
default scratch initialization, or owner allocation is introduced.

The original parent supplies EAX output, ECX color, and its private stack
locals. The added wrapper is a different source ABI. Original private-stack
aliasing, incidental register state, ambient x87 stack faults and hardware
exception identity remain outside the proved interface. The actual input,
output and angle-step storage must remain valid for their reached accesses.

## Validation

The report records a fresh live-Ghidra versus installed-PE comparison of all
483 interior bytes and the eight-byte angle step. No Ghidra function was
created or changed. Parent integration owns the fragment ledger entry.

The ignored fixture is the vertex-only portion of the historical debug24
fixture. It copies the real 483-byte interior, relocates only its three CEC730
operands to the same supplied double, appends RET, and uses the historical
stack shim. Source and original execute the same two input records under all
four x87 rounding modes. Each comparison includes all 608 output bytes and
both surrounding canaries. Inputs include signed zero, a large finite center,
and a negative-zero radius. Every emitted color word is checked.

Strict Win32 build and the three existing CTests are required before the
packet closes. The report identifies the frozen library, sphere object,
probe, observed compilation/link dependencies and runtime modules. These
checks establish scoped fragment behavior; active/cold B2BB90, complete
renderer composition, original FH3/SEH and game rendering remain unvalidated.
