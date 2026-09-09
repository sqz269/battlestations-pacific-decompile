# Camera inverse and multiplication contracts

This read-only pass verified `bsp` and `/battlestationspacific.exe` before each
analysis batch and inspected original assembly for 00b63b30 and 00413920.
Both complete bodies match the installed executable. No C++, Ghidra metadata,
annotations, saved project state or runtime fixtures changed.

## 00b63b30: inverse of an orthogonal scaled affine transform

ABI: ECX=destination float[16], EDX=source float[16], plain RET. EAX remains
the destination pointer on return; callers consume it despite the decompiler's
void signature. ESI and EDI are saved/restored. The initial REP MOVSD copies
16 DWORDs forward, assuming the normal ABI's clear direction flag.

With row-major storage, source row vectors `a_r=(S[4r],S[4r+1],S[4r+2])` have
squared lengths `n_r`. The mathematical operation is:

```
D[4*c+r] = S[4*r+c] / n_r             r,c in 0..2
D[12+c] = -sum(S[12+r] * D[4*r+c])    r in 0..2
```

Indices 3,7,11,15 remain the bitwise values copied from the input. It does not
force an affine last column. This is an inverse when the upper three source
rows are mutually orthogonal with nonzero lengths and the matrix has affine
last column (0,0,0,1). It is not a general inverse for shear or perspective.
There is no orthogonality test, singularity test, epsilon, error return or
zero-denominator fallback. Nonuniform scale and reflected orthogonal axes are
compatible with the mathematical construction.

### Alias restrictions

Do not treat the leading copy as an alias-safe input snapshot. Subsequent
divisions still read source through EDX. For example, at00b63c6f it stores
destination[4], then at00b63c78 loads source[4]. If destination==source, that
source element has already changed. More source elements are overwritten before
later reads. Exact in-place operation is therefore not a supported general
inverse operation. Partial overlap also has both forward-copy and subsequent
read/write hazards. A typed mathematical interface should require disjoint
64-byte source/destination ranges, or clearly expose changed alias semantics.

### Precision and signed zero

Squared components are individually evaluated with x87 and spilled to float32.
For each row, the first two squared float32 values are added in x87; the third
squared float32 value is added next, then the norm is spilled to float32.
Divisions use those stored norms and each output coefficient is stored as
float32. Rewriting the operation as an unspilled generic dot product changes
where rounding happens. The third diagonal result is explicitly spilled,
reloaded and stored before use in translation.

Translation computes each three-product sum with x87, spills the sum to
float32, then applies SSE SUBSS using **negative zero**, bits80000000, from
global00d7a208 as the left operand. This is `float32(-0.0f - stored_dot)`,
not an assumed positive-zero subtraction. The three translation components
are written after preserving the input translation values. Exact rounding,
NaN conversion, denormal and exception behavior depends on the caller's x87
control word and MXCSR; the routine does not configure either environment.

## 00413920: matrix product with separate result argument

ABI: ECX=left float[16], first stack argument=destination float[16], second
stack argument=right float[16], RET8. EAX is loaded from the destination stack
argument at0041392a and remains the returned destination pointer. Under row-major
storage it writes `destination = left * right`:

```
D[4*r+c] = L[4*r] * R[c] + L[4*r+1] * R[4+c]
         + L[4*r+2] * R[8+c] + L[4*r+3] * R[12+c]
```

The output is written in ascending index order. Each left row is cached before
its outputs are written. During the first output row, right-hand columns are
loaded and retained in x87 registers or stack temporaries before the respective
output write; by the end of that row, all right matrix components are cached.

Consequently exact destination==left and exact destination==right are supported
by the observed load/store ordering, including all three pointers equal.
This is an assembly-derived alias conclusion, not a new native fixture result.
It does not imply arbitrary partial-overlap safety: destination=left+4 floats
can destroy the next left row; destination=right+1 float can overwrite a right
column before that column is cached. Distinct source matrices may alias each
other safely when the output does not overwrite future uncached source data.

Input caches include FLD/FSTP float32 pairs, which can convert signaling NaNs.
Products and sequential sums use x87 registers, without float32 rounding after
each multiply or addition; results round on their final FSTP output. There is
no SSE multiply/add or fused multiply-add instruction. A simple scalar C++
float dot product, compiler-reassociated expression, or SIMD implementation
cannot be called bit-identical without a focused reference check. Even the
mathematical commutativity of the first pair of terms is insufficient for
claiming identical NaN payload propagation. Preserve instruction order for
strict numerical/native exceptional-value parity.

## Actual camera callers

`00b6fcb0` tests camera flags+5Ch mask08h, refreshes the source transform if mask02h
is absent, then calls00b63b30 with source=camera+F0h and disjoint stack output.
It copies the result to camera+60h via004134f0, sets mask08h and returns+60h.
This confirms the return-pointer contract and shows that the camera caller
does not rely on in-place inverse behavior.

`00b70490` checks camera+2F0h mask10h. It ensures the inverse/view cache at+60h,
obtains projection through00b6fcf0, and calls00413920 with ECX=camera+60h,
stack destination=temporary and right=projection. It copies the result to
camera+220h, sets mask10h and returns+220h. The resulting order is view times
projection under the matrix storage convention above, not projection times
view. Constructor conventions, dirty-flag writers and full camera lifetime
remain separate reconstruction work.

## Byte evidence

| Address | Length | SHA-256 |
|---|---:|---|
| 00b63b30 | 531 | 619b87a228444a7f6e3e78d8d26e1cbd2851baa6bbcbc3362ea9b39c2ff06d5c |
| 00413920 | 874 | a2ae101c6235ce95cd7cffdc28deb5797b9d844dbc809e729db5ac5b134e29d9 |
| 00d7a208 | 4 | 6d58692645c9d1cfaf13541cbd258f86193ef63c2f1d38f6bbca9617372d7bd6 |

All listed ranges matched original disk and saved image; the last range is
little-endian `00 00 00 80`, negative float32 zero. No whole-image identity,
compiled replacement, fixture equivalence or game validation is claimed here.
