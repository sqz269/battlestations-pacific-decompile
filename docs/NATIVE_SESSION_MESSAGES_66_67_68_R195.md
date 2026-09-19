# Session messages 66, 67 and 68 (R195)

## Scope

Reconstruct eighteen complete normal game methods (2,132 bytes) and three
five-slot profiles. Existing bit, string, numeric, allocation and checked-DWORD
storage providers supply their dependencies. No new STL implementation or
unresolved-call fallback is introduced. Names are descriptive hypotheses.

| Type | Constructor | Predicate | Writer | Reader | Destructor | Scalar destructor | Profile / size |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 66 | 00766D00 | 00766D90 | 00943810 | 00947E40 | 00766DA0 | 00766E00 | 00D03740 / 5Ch |
| 67 | 00765760 | 007657C0 | 008E47E0 | 008E4850 | 007657D0 | 00765830 | 00D0366C / 30h |
| 68 | 00765850 | 007658B0 | 008E48C0 | 008E4990 | 007658C0 | 00765920 | 00D03680 / 44h |

Constructors inline the established base initialization: delivery=3, fields
+8/+Ch zero, base profile `00D02C68`, fixed type, then one captured current
game pointer and signed owner index at +18ECh. Index 0..7 selects +18CCh;
other indices produce null. They publish delivery=1 and the concrete profile.
Base padding and other uninitialized payload fields remain untouched.

All predicates compare the full DWORD query only against the fixed class
type. They do not read the receiver, including its mutable type byte. Each
profile's fifth slot is the existing always-true method.

## Type 66: three DWORD vectors

Storage contains signed fields +18h/+1Ch, WORD +20h and retained WORD +22h,
three 10h vector headers at +24h/+34h/+44h, optional WORD handle +54h,
Boolean +56h, retained byte +57h and DWORD +58h. A vector header contains
retained DWORD, begin, end and capacity-end. Construction zeros only the nine
pointer words. Its native EH frame remains at state -1 throughout; the map's
root and vector cleanup states are not activated.

Wire order is mutable type8, signed +18h/5 bits, WORD +20h/13 bits, signed
+1Ch/4 bits, Boolean +56h, optional handle/12 bits, then +58h/16 unsigned bits.
The writer reloads the Boolean after writing it. Each vector follows with a
4-bit count and signed values of widths 10, 4 and 4 respectively. The count
is truncated on the wire, but the loop still visits the full current vector.
Counts use wrapped 32-bit byte subtraction and arithmetic shift by two.
Loop bounds, validation and backing pointers are reloaded in native order;
the explicit invalid-parameter binding can return.

The reader's count is local; the original reuses its consumed stream-argument
stack slot. **A zero wire count skips resize and leaves the existing vector
unchanged.** A nonzero count calls the already reconstructed
`resize_native_input_settings_words_00492210(header,count,0)`, validates each
index, and reads signed elements. This is the same native dependency used by
the original reader, not a newly inferred substitute.

Destruction stamps the concrete profile, frees and clears vector headers in
reverse order (+44h, +34h, +24h), then stamps root `00CE4974`. Retained header
DWORDs remain unchanged. There is no native destructor EH frame here.

## Types 67 and 68: owned strings

Type 67 owns the string header at +1Ch/+20h. Its wire sequence is type8,
signed +18h/4, owned string, signed +24h/32, unsigned +28h/4, then WORDs
+2Ch/+2Eh with 12 bits each. Construction zeros only the owned header.

Type 68 owns the string header at +3Ch/+40h and retains WORD +1Eh. Its wire
order differs from storage order: type8, unsigned +18h/4, float +20h,
WORDs +24h/+26h with 12 bits each, signed +28h/32, unsigned +30h/3,
float +34h, unsigned +38h/4, WORD +1Ch/12, signed +2Ch/5, then owned string.
Each float write captures maximum `00D7A248` through x87 FLD/FSTP followed
by the value; each read captures that scale through x87. Numeric calls use
zero=0, signed=1 and width=32. The actual owned-string codec preserves its
low-eight-bit length and first-NUL reader behavior.

Both destructors stamp their concrete profile, return the owned string while
retaining its header bits, and stamp the root on normal exit or unwind.
Native EH state 0 maps to the root stamp. The source uses `__try/__finally`
to preserve that boundary. Handler/info/map addresses are respectively
`00C88B58/00DB877C/00DB8774` and `00C88B78/00DB87A8/00DB87A0`.
Type 66's inactive constructor map is `00DB89FC`, info `00DB8A14`, handler
`00C88CEE`.

All scalars call the complete destructor, free only for flags bit 0 and return
the captured object address. They do not free the object when destruction
escapes. Native writers/readers/scalars use ECX=this, one stack argument,
RET 4; readers use the cursor at +4 in the 18h stream wrapper. Constructors
return EAX=this. Explicit source contexts change the complete binary ABI.

## Evidence and validation

The existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
supplies live bytes checked against the original PE: 24,288 bytes, 86 owned
call edges and 693 fixture relocations. Three factory constructor calls are
recorded separately from owned edges. Six missing methods were defined;
returning-free gaps in the vector destructor and three scalars were repaired.
Names/comments are saved under the shared write lock with prior values and
refreshed exports retained.

The strict Win32 build and three existing CTests pass. The combined fixture
passes **9,707 original/source pairs and 20,028,573 matching bytes**, plus
three source-only cleanup faults. New coverage consists of 84 constructor
and destructor cases, 768 fixed-type predicate cases, 18 populated scalar
cases, 256 repeated vector cases and 2,176 repeated string/numeric cases.
The inherited 6,405 cases remain in the run.

Vector comparisons include growth/shrink, initial capacity, counts 0/1/3/7/
15/16/17/31, count truncation while walking all elements, and a second read
with all counts zero that must retain sizes, capacities and contents. String
cases cover lengths 0/1/148/149/150/255/256/511, embedded NULs and repeated
replacement. Records exercise all eight bit alignments; float cases exercise
edge values, four x87 rounding modes and both conversion selectors. Full
initialized storage, retained bytes, wire bytes, cursors, pool state and
masked x87/MXCSR exception flags are compared.

Two new source-only pool faults verify that types 67 and 68 stamp the root,
retain the header and do not return a string whose cleanup failed. The third
fault is inherited type-55 cleanup coverage. Original library comparison
dependencies add ten bodies (1,108 bytes); `00BF67A7` binds to the installed
CRT's actual `memmove_s`. The source reuses the existing checked-DWORD storage
contract; these results do not establish original STL/CRT exception identity.

The full factory, recorder, network/startup composition, gameplay, complete
binary ABI, original FH3 dispatch, unmasked FP faults, allocation failures,
invalid iterators, malformed ranges, arbitrary aliases and concurrent mutation
remain unproved. Source profiles borrow their context after the five native
slots; the caller must preserve all binding lifetimes.

Evidence and merged-build receipts: `reports/native_session_messages66_67_68_r195.json`.
