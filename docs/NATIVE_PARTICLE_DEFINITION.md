# Native particle definition construction and lifetime

Addresses: 00AFE0A0, 00AFA280, 00AF9FB0, 00B03940, 00B02B90,
00B01CB0, 00AFFDF0, 00B00090, 00AFA100, 00B039F0, 00B02C40,
00B01D60, 00AFA350, 00B03B40, 00B02FB0, 00B01EA0.

Descriptive names are hypotheses, not recovered symbols. The report records
complete installed/live byte spans, original ABI, call sites and uncertainties.

AFE0A0 produces the actual sparse 108h record allocated by the three definition
virtual08 methods. Its one value is captured once by MOVSS before four stores;
time bits and zero stores retain SSE behavior, with all unmentioned bytes left
untouched. This differs from the four-argument AFE1A0 child initializer.

AFA280 writes reference count1, reference then base profile, a zero actual8h
NativeString header, counts4C/68, byte7C, and copies the supplied name through
the canonical string storage. It reloads source and destination fields after
resize. Its BF7680 copy includes backward overlap handling, represented here
by memmove. Self-alias sees the already-cleared header and skips copying. Fields10,
70 and the low byte14 come from the original four stack arguments. Fields30/38,
the embedded pointer rows, and derived parameters remain allocation preimage.
Derived constructors only replace the profile after successful base construction.

AF9FB0 compares the actual kind header case-insensitively using the existing CRT
and 425850 helper, allocates94h/8Ch/90h for ConeEmitter/SphereEmitter/
SmartAreaEmitter, invokes the corresponding constructor, and calls the current
captured virtual14 with the actual text object. Constructor exceptions free the
raw allocation; parser exceptions do not. Unknown kind uses word70 as the
object pointer, and null allocation still reaches dispatch. No successful
fallback or parser implementation is invented. The parser14 methods are
B03EC0/B02FD0/B02210; they are distinct from the zero-argument parameter member
preparation methods used by AF9F50.

AFFDF0 frees a parameter's current +04 payload only for unsigned16 type +0A 1
or2 and nonnull data, then clears +04 after the free. The store was hidden by
Ghidra's incorrect no-return annotation on BF6989. B00090 returns the captured
parameter through the same F8D344 physical pool and existing924420 generic slot
implementation. It does not allocate, construct or initialize another pool.

Base destruction visits parameters24/28/2C/20/30/34/38, captures each before
payload disposal and slot return, then clears the current owner field. It
releases actual intrusive counts in rows3C/count4C and54/count68 using Windows
InterlockedDecrement, reads the current profile/slot00 only at zero, and reloads
the signed count after each callback. It destroys the current name header and
restores the reference profile. Curve/terminal failure performs the observed
string and reference cleanup; derived cleanup failure invokes base destruction.
The derived parameter slots are80..90 for cone,80..88 for sphere,80..8C for
SmartArea. Only SmartArea88/8C are cleared after successful disposal; the other
derived pointers intentionally stay stale. Scalar wrappers return the original
address and free it only when the low flag bit is set after successful cleanup.

The bindings borrow actual relocated native profiles and canonical storage.
They do not turn original address words into callable host C++ vtables. Real
parser/member dispatch remains a required application boundary. These are new
C++ lifetime/factory APIs, not original binary replacements; host cleanup does
not prove original FH3 exception compatibility or gameplay.

## Validation

Strict MSVC Win32 object compilation passed. Full byte spans and direct call
rows are verified by the accompanying report; combined build and independent
focused native-byte replay are recorded at integration. No permanent tests were
added. Three missing derived matrix-generation bodies are separately owned and
defined by the primary integrator before worker call verification.

## Follow-up packets

- Recover definition parsers B03EC0/B02FD0/B02210 and their parameter producers;
  lease each first. Fresh construction alone does not initialize their curves.
- Compose the recovered virtual08/0C/10 generation implementations with the
  application's actual definition dispatch and live model/emitter ownership.
- B0CA40 emission state initialization is currently a separate orchestrator's
  address lease; coordinate its eventual concrete binding independently.

## AN combined integration

The combined strict MSVC Win32 build and both seeded CTests passed. Focused
original-byte probes were relinked to the current combined library. All26
reconstruction names/signatures and six analyzed FH3 dispatcher comments were
saved and read back, preserving prior annotations; affected exports refreshed.
Four reports verify132 direct call rows with zero failures. The integration
report records exact per-probe limits and supersedes earlier worker pending
notes; application composition, native throwing ABI and gameplay are unvalidated.
