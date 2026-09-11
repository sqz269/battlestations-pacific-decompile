# Three geometry helpers

`geometry_helpers.cpp` reconstructs the complete game bodies at 00414EB0,
00414F50 and 00415010. Names are descriptive hypotheses. Generic standard arrays
and a borrowed float pointer describe only the observed coordinate storage;
they do not assert a native class layout. Existing GUI UV rectangles and 3D
vector/model bounds types carry different domain contracts, so none is repurposed.

## Heading angle, 00414EB0

Native ABI is ECX pointing to two floats, no stack arguments, RET, float in ST0.
The helper loads the second float, then the first, and calls 00BF701A with
ST0=x/ST1=y. That CRT entry loads descriptor 00E15C20 and jumps to
`__cintrindisp2`. The descriptor names `atan2`; the sign dispatcher examines
both stack values and restores their order, and the finite leaf uses FPATAN.
The recovered operation is therefore `atan2(y,x)`, followed by this game logic:

1. Store the CRT result to binary32 and reload it.
2. Subtract it from the double at 00CE3830, then store/reload binary32.
3. If the result is ordered-negative, add the double at 00CE3828 once and
   store/reload binary32. Equal zero and unordered NaN skip the addition.

The constants are promoted binary32 values, not full-precision pi literals:

| Address | Double bytes, little endian | Exact value |
| --- | --- | --- |
| 00CE3830 | `00000060fb21f93f` | 1.57079637050628662109375 |
| 00CE3828 | `00000060fb211940` | 6.283185482025146484375 |

The angle is zero along the positive second component and increases toward the
positive first component. No world-axis or orientation-class meaning is proven.
There is no normalization loop, modulo, or clamp. For example, direction
`{-1.1e-7f,1}` reaches a small negative intermediate and the adjustment rounds
to the full-turn constant itself. A strict upper-exclusive range promise would
be incorrect.

Production calls the actual host `_CIatan2` entry from the Win32 UCRT import
library, retaining the hidden x87 input ABI. The linked fixture imports that
name from `api-ms-win-crt-math-l1-1-0.dll`. No CRT implementation is copied or
reconstructed. Legacy CRT errno, matherr, exceptional values and diagnostics can
differ from the current host CRT. This packet's numerical differential isolates
the game helper by binding both sides to the same host CRT boundary.

The original CRT finite leaf at 00C07E4C has no saved function start; its verified
terminal RET is 00C07E6B, length 1, inclusive end 00C07E6B. This is library evidence,
not an incomplete game helper or a request to port that CRT leaf. All three owned
game starts and complete exported flows already exist. No Ghidra write occurred.

## Half-open containment, 00414F50

Native ABI is ECX=bounds, one stack point pointer, RET4, EAX0/1. Bounds are four
floats at offsets 0/4/8/12: minimum X/Y, maximum X/Y. The point's X is loaded and
spilled to float before any bound comparison. It rejects X below the lower bound
or unordered, then rejects X at/above the upper bound or unordered. Only if both
X checks pass does it load pointY and perform the corresponding Y checks.

Thus ordinary finite behavior is lower-inclusive/upper-exclusive. Any unordered
comparison encountered rejects. Later components are not eagerly read. The x87
loads/spills, FCOMI/FCOMIP and stack pops are retained, including masked signaling-
NaN behavior. No empty-rectangle special case, bound reorder or null validation
is added.

## Sequential expansion, 00415010

Native inputs match containment; RET4, with no consumed return value. It tests
and conditionally writes minimumX, maximumX, minimumY, maximumY in that order.
The relevant point component is freshly loaded/spilled before **each** test.
Only an ordered strict comparison writes a bound, so equality preserves original
bits, including signed zeros. An unordered comparison skips that write and
continues. `fmin`/`fmax` would not preserve this NaN behavior.

The point may alias bounds. For bounds `{0,10,-5,20}` and point=`bounds.data()+1`,
updating maximumX changes the point's later Y source. The final bounds are
`{0,10,10,20}`. Capturing the point once would incorrectly store -5 into minimumY.
The raw borrowed point pointer permits this without inventing an overlapping
`std::array<float,2>` object lifetime.

## Verification and limits

All three complete native spans, both constants, the CRT descriptor and finite
library evidence matched live saved analysis and the installed PE. The report
contains raw hashes, assembly, ABI, prior names, exact terminal addresses and
uncertainties. No name change is made to a correctly identified library operation.

Strict MSVC Win32 Release build and both existing CTests passed; eight native
math seeds matched. One ignored focused fixture compares relocated original
game bytes against the new interfaces: 45 angle, 303 containment and 330 expansion
cases across x87 control words 007F/027F/037F. It covers lower/upper edges, NaNs
including signaling encodings, infinities, signed zeros, unusual bounds and
overlapping point/bounds. Output bits and masked x87 exception-status bits match.
Only the angle's CRT call and two constant addresses are relocated; the two
bounds helpers execute their original complete bytes without call substitutions.

These are build/fixture-validated typed interfaces, not drop-in native ABI
replacements. Original CRT diagnostic parity, unmasked floating exception
handlers, other floating environments and game-runtime behavior remain unverified.
