# Native input device index

Target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. This source projects actual 32-bit native checked-tree storage. It has a new C++ service ABI; it is not a binary replacement or gameplay proof.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `0055C110` | ECX tree, stack pooled8h key, EAX node+14h, RET4 | Complete normal existing/missing paths for freshly constructed empty defaults; original SEH and exceptional unwind identity excluded |
| `00554980` | ECX tree, stack key, EAX lower-bound node | Complete normal search, empty and case-insensitive ordering |
| `0055B520` | ECX output record, EAX record | Reached fresh empty default, seven allocated heads and three empty vectors; opaque fields left untouched |
| `0055BA80` | ECX output pair, stack key and mapped record | Reached fresh-empty pair copy, owned key and seven fresh mapped heads; populated mapped copy excluded |
| `0055BF00` | ECX tree, stack output/owner/node/pair, EAX output, RET10h | Normal checked hint and fallback unique insert; malformed topology excluded |

`0055C110` uses `00554980` to find the first key not less than the input. Both comparisons use native case-insensitive string ordering: zero-length keys precede nonempty keys and nonempty buffers use `_stricmp`. A match returns its unchanged mapped record. A missing key constructs a local 0x84 device, a local 0x8c pair, then a 0x9c node. Each copy owns its own nested heads. The temporary pair is destroyed before the temporary device; the inserted node retains its own key and mapped heads. Constructor allocation follows the observed four trees, two vectors, one tree, one vector, two trees order; partial-construction cleanup releases heads in reverse order.

The device record holds checked tree fields at `+00,+0C,+18,+24,+50,+6C,+78` with head sizes `28,28,2C,1C,20,2C,14` hex. Empty vector headers start at `+30,+40,+5C`. These are established by the writes in `0055B520` and allocation helpers `00554BE0`, `00554C30`, `00554C80`, `00554CD0`, `00554D20`, and `004C2700`. Tree header word0 and other unwritten bytes are left untouched. The destructor is the existing `0055B690` reconstruction.

Node links occupy `+00,+04,+08`, key at `+0C`, mapped record at `+14`, color at `+98`, nil at `+99`. Empty head has self links and nil=1. The insertion limit is count `> 0x01D41D3F`, observed in `0055BBE0`; the shared insertion primitive checks the equivalent `>= 0x01D41D40`. It publishes count and updates extrema, rotates and recolors the same checked tree. The implementation does not expose a general populated-device copy routine. `0055B920` (mapped copy), `0055BDE0` (unique fallback), `0055BBE0` (link and RB repair), and `0055BB40` (node allocation) are read-only dependencies; their general populated paths are outside this packet's ownership. Direct calls from the packet's owned functions to those dependencies are in the report.

Original CRT invalid-parameter, `std::length_error`, native pool and SEH exception details remain outside the source ABI. Build/file checks establish only source compilation. The game was not run.
