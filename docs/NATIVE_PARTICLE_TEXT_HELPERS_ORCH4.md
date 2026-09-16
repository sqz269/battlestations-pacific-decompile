# Particle parser token helpers

`src/native_particle_text_helpers.cpp` reconstructs two complete bodies. Their
names are hypotheses. Both receive an actual cell containing a text pointer;
this is not the length/data layout of an eight-byte pooled string.

| Address | Bytes | Original ABI | Behavior |
| --- | ---: | --- | --- |
| AEDF60 | 21 | ECX pointer cell, RET, ST0 result | CRT atof followed by x87 binary32 store/reload |
| AF3E90 | 95 | ECX pointer cell, RET, EAX count | Count ASCII space runs before a nonprintable signed byte |

## Exact behavior

AEDF60 calls BFA66C (`_atof`), then FSTP32 and FLD32. The source invokes the
current CRT through its ST0-returning ABI, avoiding an added C++ binary64
temporary. The compiled Win32 object has CALL at +19, FSTP DWORD at +1C and
FLD DWORD at +22, with no intervening floating-point operation. This preserves
the conversion's arithmetic schedule, not the original stack-frame ABI.
The current CRT is an explicit provider boundary: original CRT parsing,
locale, and error-state equivalence have not been established.

AF3E90 returns zero for a null text pointer. Otherwise it first scans through
the full NUL-terminated string, then returns one plus the number of runs of
ASCII spaces before a signed byte below 32 or byte 127. Leading and trailing
space runs count; an empty string returns one. Tabs and high-bit bytes stop
the count rather than separating words. Signed index/length comparisons and
the captured byte at the end of a space run are preserved. It adds no bounds
checks or Unicode rules.

## Validation

Full original PE/live spans match; the sole direct call row is verified.
The strict Win32 build and three existing CTests pass. A temporary probe links
the actual built library and executes both copied original bodies. Thirteen
field-count inputs exercise null, empty, leading/trailing/repeated spaces,
controls, DEL and high-bit bytes. Seven float inputs under four x87 rounding
modes match result bits and exception flags, including halfway values,
overflow, underflow, negative zero, NaN and trailing text. Both float versions
use the same current CRT provider.

An independent review found an extra volatile read after a space run; the final
source reuses the terminating byte as the native comparison does. Object
inspection confirms the FSTP32/FLD32 schedule. The report preserves original
metadata and hashes the source, object, disassembly, probe and build receipt.
No native ABI replacement, original CRT equivalence or gameplay is claimed.
