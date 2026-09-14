# Actual compiler stream and pass providers

Addresses: `00BE4460`, `00BE40D0`, `00B1FF50`, `00B44B10`.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| BE4460..BE4497 | ECX stream; stack name8h, optional count; `RET8` | Complete normal raw D691B0 provider |
| BE40D0..BE40ED | ECX stream; stack DWORD, optional count; `RET8` | Complete normal raw D691B0 provider |
| B1FF50..B1FF56 | ECX renderer; EAX renderer+1B18; `RET` | Complete seven-byte leaf; no dereference/null guard |
| B44B10..B44BE5 | ECX actual88h pass; EAX same pass; `RET` | Complete normal numeric D5F0A8/B319B0 overload; failed source acquisitions retained |

BE4460 captures name length, dispatches current+54 to BE40D0 with that length,
reloads data from name+4 (null uses109DB64), then dispatches current+28 to the
real physical BF4F50 WriteFile body using the captured length. Both writes use
the same optional output pointer; the return is the second count. BE40D0 passes
the address of its actual incoming DWORD cell, size4 and the optional count to
current+28. No artificial result check, accumulation or semantic stream is used.
B38A70's producer publishes the physical cache stream at cache+8; D691B0+54,
+64 and+28 contain BE40D0, BE4460 and BF4F50. The source validates those actual
current entries and invokes their concrete implementations.

The B44B10 overload is distinct from the older callable-table constructor.
It calls actual B5F720, writes D61BE8/count6C/70/74/78/7C/80/84 in native order,
binds the same canonical state metadata, constructs white.tga, reloads current
renderer D5F0A8+64 and calls the real B319B0 cache with option0. Its persistent
cache acquisition is attached before the call. The returned actual owner is
transferred to84 without an extra retain, followed by native temporary string
pool return, leaving the raw header stale.

The original B44B10 handler CBF3D0 references descriptor DF7D24 and two entries
at DF7D14. State0 -> -1 invokes CBF3C0/B5F510 on the base; state1 ->0 invokes
CBF3C8/41DD20 on the temporary. Source records those states and preserves failed
children; it does not disarm the real cache child or simulate FH3 rollback.
Only BE40D0 required a missing-function repair. Explicit primary authorization
allowed the exact half-open range `[BE40D0,BE40EE)`; the official locked tool
saved before/after state in `native_material_compiler_providers_definition.json`
and refreshed exports. No other Ghidra writes were made by this packet.

All four complete byte spans, current profile operands, cleanup map/actions and
the original physical-write span match live Ghidra and the original installed
PE. B1FF50 and both stream leaves passed focused original/source comparisons
using real WriteFile. B44B10 is compiled and linked through the complete parent;
its texture acquisition and exception behavior were not runtime exercised.
Other stream/renderer profiles and private native stack aliases are explicit
source-interface boundaries, not synthetic fallback behavior.
