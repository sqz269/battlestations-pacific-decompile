# Input settings tree indexing and integer-key insertion

Addresses: 0055A9A0, 0069FA40, 006A1E70, 006A6900, 006A1F80.

The complete normal map-index and integer-set unique-insertion paths now operate
on actual native headers in `src/native_input_settings_tree_insertion.cpp`.
`006A7BE0` calls these source APIs directly. This removes the final five methods
and the entire `NativeInputSettingsTableCalls` interface. The separate
14-method `NativeInputKeyboardLibrary` remains required.

| Entry | Storage and behavior | Original ABI |
| --- | --- | --- |
| 0055A9A0 | Case-insensitive string to scalar/checked DWORD vector; node 2Ch, color 28h, nil 29h, mapped 14h | ECX tree, stack key, EAX mapped, RET4 |
| 006A1E70 | Case-insensitive string to five-DWORD preset; node 2Ch, color 28h, nil 29h, mapped 14h | ECX tree, stack key, EAX mapped, RET4 |
| 006A6900 | Case-insensitive string to owned integer/string tree; node 24h, color 20h, nil 21h, mapped 14h | ECX tree, stack key, EAX mapped, RET4 |
| 006A1F80 | Signed integer to pooled string; node 1Ch, color 18h, nil 19h, mapped 10h | ECX tree, stack key, EAX mapped, RET4 |
| 0069FA40 | Signed integer set; node 14h, color 10h, nil 11h | ECX tree, stack output/key, EAX output, RET8 |

All trees use header head+4/count+8 and node left/parent/right at 0/4/8.
String comparison reuses the actual raw-header empty gates and case-insensitive
comparison in `00443D00`. Integer ordering is signed, including both extremes.
Existing keys return the existing mapped storage without default construction.
Missing map keys use lower-bound insertion hints and native red-black linking,
rotation, recoloring, extrema and count publication. The generic mechanics are
the existing parameterized `detail/native_tree_insert_storage.hpp` extraction.
The unique set output writes owner, node and only byte8; padding bytes9..11 survive.

Native link helpers are `00559710`, `0069FBC0`, `006A4F80`, `006A02B0`,
and `0069EF20`. Their first rejected counts are respectively 09249248h,
09249248h, 0CCCCCCBh, 15555554h and 3FFFFFFEh. The legacy SBO length-error
message is the counted 19-byte `map/set<T> too long` string. Source exceptions
use the existing source exception domain; native FH3 identity is not claimed.

## Defaults and ownership

`0055A9A0` leaves its default scalar uninitialized on the native stack; the
source API explicitly receives that DWORD preimage. The DWORD vector starts
empty, while its opaque word stays untouched. The source parser services now
carry `sensitivity_default_stack_preimage`. The scalar is subsequently written
by the actual table parser where the installed input scripts supply it.

`006A1E70` initializes the first four default words to FFFFFFFFh,0,0,0 and
only clears the low byte of word4. Its upper three bytes retain a private-stack
preimage. The source receives that preimage and copies all five words in the
temporary pair and node; it does not silently zero padding. The parser reuses
its existing descriptor flag preimage input.

`006A6900` constructs three empty 1Ch sentinels: the default tree, the temporary
pair's independent copy, and the inserted node's independent copy. The temporary
pair tree/key are destroyed before the default tree, leaving the node's head
owned by the returned mapped header. `006A2930`, `006A1C40`, and `006A1290`
are implemented only along this reached empty-copy path. General populated
tree copy is not supplied. `006A1C40` is a copy body that does not itself clear
an old destination, so its former CopyAssign name is refined to CopyBody.
`0069D430` allocates and initializes a node; its caller publishes the sentinel
and completes the nil/self-link setup.

`006A1F80` constructs empty pooled-string defaults and preserves a found string.
String-key maps construct a temporary key and a separate node key. Guards free
the temporary payload/key, default payload, and failed node allocation in the
corresponding ownership order. Allocation or release callbacks must not mutate
tree topology, keys, or private defaults.

## Evidence and validation

`reports/native_input_settings_tree_insertion.json` records 35 live/disk-equal
native byte envelopes and 188 direct CALL sites at their actual saved owners.
Every instruction in those envelopes was checked for ownership. Call-site flow
repair left five instruction ownership holes in `0055A9A0` and `006A6900`;
the two bodies were recreated through the supported bridge under the write lock,
preserving earlier comments and names. Callee no-return flags were unchanged.
The annotation and repair receipts retain before/after evidence.

The retained isolated Win32 fixture copies 418 original spans and uses the
installed Lua input scripts, source pooled strings, Lua and CRT boundary
services. Its focused insertion scenario compares 168378 words for all five
APIs: missing/existing and case-folded string keys, an empty string key, signed
integer extremes/duplicates, nested populated controller trees, topology,
parent links, extrema, count, colors, mapped values and output padding.
An isolated fixture wrapper seeds native private stack bytes to compare the
otherwise indeterminate sensitivity scalar and upper preset flag bytes.
The source path also checks failure on the second pooled key allocation,
unchanged tree count and release of temporary pooled ownership.

Retained comparisons cover 122545 lifetime words, 309711 partial/full tree-range
words, 1448 vector words and 335 checked-string words. Full settings destruction,
populated constructor failure and raw singleton-manager drain pass with zero
remaining pooled strings. Final combined-source Win32 build, two existing
CTests, fixture hashes and eight disk/live seeds are pinned in the report.

These are reconstructed, build-tested and fixture-tested service interfaces.
They are not drop-in original ABI replacements. Malformed trees, structurally
mutating callbacks, FH3/private-stack aliases and hardware-fault behavior remain
outside the contract. Production keyboard providers, application wiring and
gameplay remain unvalidated. This packet did not start or control the game,
bypass its single-instance guard, or dispatch new agents.
