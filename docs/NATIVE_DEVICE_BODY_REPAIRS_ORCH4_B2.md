# Device-class virtual function and reserve listing repairs

The live `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
contained vtable targets `00731A50` and `00730CB0` without defined functions.
Their complete disk instruction graphs were reviewed before definition, and
the locked definition tool verified all proposed bytes against live memory.

| Entry | Inclusive end | Bytes | Live instructions after repair |
| --- | --- | ---: | ---: |
| `00731A50` | `00731AD2` | 131 | 52 |
| `00730CB0` | `00730D4A` | 155 | 52 |
| `0043FA60` | `0043FABE` | 95 | 38 |

`00731A50` ends at its `RET 4`, before INT3 padding and the next function
at `00731AE0`. `00730CB0` has reachable x87 cleanup at `00730D47..00730D4A`
after its earlier RET: the branch at `00730CEE` reaches this cleanup and
jumps back to `00730D30`. The body therefore includes that tail, before
padding and the next function at `00730D50`.

`0043FA60` already owned its complete range. Its false returning-free
override hid `0043FAB1..0043FAB9`, including new data/capacity publication.
The locked flow repair restored those nine bytes without changing global
callee no-return flags. All three functions now report zero listing gaps;
the project was saved, exports refreshed and the function snapshot rebuilt.
No original executable bytes changed. The two new definitions had no prior
function name, prototype or function plate comment to replace.

## Established behavior and naming

`BSP_GunClass_ActivateModelAndBulletClasses` is a descriptive hypothesis for
`00731A50`, native ECX class plus one stacked selector, `RET 4`. A nonzero
byte at class+44h bypasses work. Otherwise it forwards the selector to
`0043EBD0`, captures the class+74h array and +78h count, and walks 48h-byte
entries to a captured end. It skips a null entry or null +34h pointer,
reads that pointed object's +Ch identifier, and calls `006EAFE0` through
ECX output-holder/EDX identifier. It releases each returned holder through
InterlockedDecrement(+4), calling vslot0 only at zero, then clears the
holder. There is no native EH frame. The bullet acquire chain is still
source-open; this evidence does not claim a completed activation source.

`BSP_GunClass_RunRecoilLoopAndLoadModelResource` is a descriptive hypothesis
for `00730CB0`, native ECX class, no stacked arguments, AL=1 on return.
Reader `007327B0` associates +A8h/+ACh/+B0h with BackSpeedStart,
BackSpeedFact and DistMax. The routine performs an SSE comparison followed
by an x87 loop with explicit binary32 stores and unordered-sensitive
branches. It does not store computed values back to the class. The live
constants at D7A218/D7A258 are float/double positive zero. If class+38h is
nonzero, it calls `00879590(class, 0)`. Ordinary float arithmetic is not
established as an equivalent replacement for this instruction schedule.

These are metadata and evidence repairs, not reconstructed source, a
native ABI replacement, runtime equivalence or gameplay validation. The
seven factory unwind delete funclets' short stored tails remain a separate
known metadata limitation in the device factory audit.

Receipts: `reports/native_device_body_repairs_orch4_b2.json` and
`reports/native_device_body_flow_orch4_b2.json`.
