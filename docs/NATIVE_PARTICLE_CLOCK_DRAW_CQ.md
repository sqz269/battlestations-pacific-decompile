# Raw particle-clock time and sink dispatch (CQ)

B19A10..B19A56 is the complete 71-byte update of the actual particle-clock
owner returned by 004DE4B0. ECX is that 1Ch owner and the original single
float32 stack word is consumed by RET4. The raw fastcall interface reserves
unused EDX, keeping the time value in the original argument slot. Its Win32
assembly retains the complete instruction sequence, including the x87 load
and spill rather than substituting a C++ float callback.

The entry copies the input's raw bits through MOVSS, captures the record-array
start at +8, then stores those original bits at owner+18. Initial count at +Ch
forms the wrapped end using a 2Ch-byte stride. No record means no x87 operation.
For each record, +28 contains the actual sink object. The code loads the current
public argument with FLD, resolves the sink's current vtable+18 entry, spills
the value with FSTP into its stack argument and calls through EAX with the sink
in ECX. Thus the owner receives the original bit pattern, while each sink
receives the x87 load/spill result under the current control/status state.
Signaling-NaN and exception behavior must not be replaced with a bitwise copy.

After every callback, the code reloads count and array start to compute the
current end, then advances its captured cursor by 2Ch and compares for equality.
It does not restart from a changed base, clamp a count or repair an invalid
traversal. A callback may change the public argument used by later iterations;
owner+18 remains the original entry value. ESI and EDI are preserved, and the
sink call must honor the original thiscall/RET4 contract.

The existing `render_tail` implementation remains a typed projection. CQ adds
a separate raw-storage fragment with zero additional native body credit.
004E53AD is the original caller; the preceding clock getter supplies ECX and
the caller supplies its float32 time value. The report pins original PE/live
bytes, the indirect sink call, that direct caller and the generated object.
Whole-function byte identity is reported separately from compilation, source
probe execution and gameplay. CP supplies the raw singleton owner; its CRT,
publication and private FH3 limits remain documented in that packet.
