# Empty native material diagnostics record

`00B106C0..00B106FF` is the complete 64-byte initialization routine for the
300-byte (`12Ch`) record used by `00B16F80` material draw diagnostics. The source
keeps the original ECX input, EAX receiver result, saved ESI, RET, and exact
DWORD store order. The descriptive name is a hypothesis, not a recovered symbol.

The routine first clears `+11C`, `+120`, `+124`, and `+128`, then clears `+0`.
It next makes fourteen iterations. Each iteration clears one DWORD in each
array starting at `+4`, `+3C`, `+74`, `+AC`, and `+E4`, in that order, then
advances four bytes. Thus every DWORD of the record is zeroed once. It calls
no allocation, string, destructor, or reference-count helper. This entry creates
raw empty storage; it does not release resources from a live record.

The observed caller constructs a temporary diagnostic record before inserting
it into the diagnostics container. The parent `00B16F80` and vector helpers
`00B15610`, `00B13B00`, `00B150D0`, and their deeper dependencies remain
outside this packet. The initialization function is not a substitute for those
operations or for complete material-draw execution.

The matching report pins current Ghidra, the installed PE, the complete source
COMDAT, original caller evidence, and the exact committed Win32 build. No new
test is added. Full byte identity is static evidence; native execution, a
drop-in module, completed diagnostics, rendering, and gameplay remain unclaimed.
