# Raw DWORD-vector insertion

Complete ordinary Source for 0046AFB0 (436B/181 instructions) and 0046B4A0
(137B/56 instructions). Independent full-listing review passed and every real
provider is integrated. The default Win32 build gate remains pending.
The descriptive vector/scene names are hypotheses, not recovered class symbols.
The original STL names are retained. Vector storage is the observed 16B raw
receiver: +4 begin, +8 end, +Ch capacity, with +0 untouched directly.

N-insert is ECX receiver, four stacked words, RET10h. Its sole caller ignores
incidental EAX; the ordinary Source interface has no semantic result. Actual
entry P+4/P+8/P+Ch/P+10h binds unread owner, position, count, and value pointer.
It reads the pointed DWORD and writes it BACK into the value-pointer argument
BEFORE reading begin/capacity/count, including count0. The unused owner and
unreached position need no reads. On an in-place path the original count cell
is replaced by count*4, then reread after child calls at the original points.
The value cell's actual address is passed to progressive fill providers.

Capacity and size use exact modulo DWORD subtraction and arithmetic SAR2 bits.
The unsigned 3FFFFFFFh overflow comparison and 1.5x growth are preserved,
including repeated current-end reads relative to the same captured begin.
Growth allocates first, then copies prefix, fills, copies suffix, captures
current old begin/size, adds captured count before free, frees the captured old
buffer if nonzero, then stores new begin, capacity, and end in that order.
There is no local native EH record and no invented new-buffer rollback.

The short-tail in-place branch moves the suffix, recomputes remaining count
from CURRENT end and captured position, fills the uninitialized tail, rereads
the actual byte-count cell, updates CURRENT end, rereads it, then fills the
original gap. The other in-place branch moves the old tail, publishes the
move's captured result to end BEFORE copy-backward, then rereads the actual
byte-count cell and fills. Captured operands and fresh field/cell reads are
kept distinct. No valid-only range, count, order or alignment shortcut is used.

Single-insert is ECX receiver, output/owner/position/value-pointer, RET10h/EAX
current output. It captures owner first and captures begin once. Null begin or
zero SAR2 size skips initial owner diagnostics and uses index0. Otherwise it
reports begin>end and captured owner mismatch, retaining captured begin across
returning diagnostics before computing the index from CURRENT position.
It reads current value pointer BEFORE current position, then prepares child
words in value/count1/position/captured-owner push order and calls real N-insert.
Afterward it captures new begin and validates it against current end; writes
captured new begin to the actual position cell; computes the result from
captured index; checks current end then current begin (short circuit); then
reads current output and publishes NODE before OWNER. Diagnostics may return.

For single-insert entry Q, outer cells are Q+4..Q+13h. The unused Q-4 private
word is not bound. Child N-insert entry is Q-28h, its four outgoing words occupy
Q-24h..Q-15h, and its scalar-value cell is Q-18h. New stable metadata is
disjoint from all described cells/payload; actual reached backing is explicit.
Child metadata is inspected only after the reached initial diagnostics and
value/position captures. Original physical stack/argument-home coincidence is
not supplied by these interfaces.

Actual required providers: 00466CE0 allocation, 0046A400 owning length error,
00468F60 nonzero-distance move (INCLUDING negative distance), 00469950 count
fill (INCLUDING high-bit count), 00468460 signed-positive copy backward,
00468440 progressive fill, actual 00BF65AC free and 00BF6713 returning CRT
diagnostic. No callbacks or unresolved stand-ins are introduced. Their complete
ordinary Source must be delivered before this packet earns Source-ready/build
credit. Their actual Source implementations are now integrated.
Current-CRT/owning C++ transport does not prove original heap/staticCRT/
RTTI/FH3/register/private-stack/fault ABI, new-body native differential behavior,
application adoption or game validation.
