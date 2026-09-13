# Native input vector map indexing

The three source APIs in `native_input_vector_map_index.cpp` index actual Win32 checked tree headers. Native entry ABI is ECX tree, stack native-string key, EAX mapped header, RET4. The source ABI adds `NativeStringStorage&` for concrete pooled-string services. Existing keys return node+14h without constructing a default. Missing keys use case-insensitive raw-header ordering, a lower-bound hint, a temporary key/value pair, a separately copied node key/value, and the shared red-black hint/link/rotation mechanics. No `std::map` participates.

| Root | Mapped header at node+14h | Node color/nil | First rejected tree count |
| --- | --- | --- | --- |
| 006A44B0 | 10h checked DWORD vector; opaque0, begin4/end8/capacityC | 24h/25h | 0AAAAAA9h |
| 006A45C0 | 10h descriptor vector with 14h elements; same header | 24h/25h | 0AAAAAA9h |
| 006A4CA0 | 14h packed-bit vector; count0, opaque4, begin8/endC/capacity10 | 28h/29h | 09249248h |

All three roots construct an empty default. `006A1A20`, `006A19A0`, and `006A20D0` copy the native key, then construct the mapped storage. `00557590`, `005578C0`, and `00557820` leave their destination opaque DWORD unwritten. The bit pair copies the initialized bit count, then constructs its vector; its opaque4 remains unwritten. The source leaves those bytes untouched at each temporary or allocated destination. It does not transfer unrelated private-stack or allocator bytes from an earlier object. Each constructor only copies an empty vector along these roots; its general populated-copy path is outside the reached root contract. A later producer may populate the returned mapped header through the existing vector storage APIs.

The `006A7BE0` table loader calls all three roots consecutively for an input key; it then appends to the first returned vector, resizes the 14h descriptor vector to two elements, and resizes the packed-bit vector to two bits. `006AA640` reads descriptor elements with a 14h stride and reads packed-bit begin/end through mapped offsets 8/C. These consumers confirm the mapped layouts independently of the constructors.

The root's temporary pair owns its copied key and mapped storage. On normal completion and C++ unwind, cleanup releases temporary mapped backing, then the key; the default mapped header is cleared afterward. The inserted node owns separate key and mapped storage. The source mirrors the native checked count limit, head/extrema, link orientation, recoloring and iterator publication, and uses the existing 19-byte `map/set<T> too long` exception message. The source exception ABI and malformed-storage behavior are not claimed as original FH3/CRT compatibility.

The saved Ghidra body repair receipts for the root free-return gaps are in the integrator's `reports/native_input_vector_map_index_ax_flow_repair.json` and `reports/native_input_vector_map_index_ax_body_repair.json`. This worker kept Ghidra read-only. `reports/native_input_vector_map_index.json` lists direct CALL instructions for all twelve owned entries, each with verified saved owner. The standalone Win32 build validates compilation. The primary agent is responsible for combined native differential fixture integration; no gameplay validation is claimed here.
