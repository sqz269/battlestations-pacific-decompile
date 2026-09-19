# Native numeric bit codecs and session messages 03/06/07 (R178)

Eighteen complete normal bodies (1,482 bytes) supply the shared numeric codec and three factory dependencies. The factory and packet recorder remain incomplete.

## Reconstructed behavior

| Native address | Behavior |
| --- | --- |
| 004293F0 / 004295C0 | Read/write numeric float; zero marker, signed/unsigned quantization, optional scale and raw32 |
| 00429090 | DWORD-value bit writer |
| 0075CE80 / CEF0 / CF70 / CFE0 / CFF0 | Type03 construction, write, read, fixed type test, scalar deletion |
| 0075D990 / DA20 / DA50 / DA10 / DA80 | Type06 corresponding methods |
| 0075E940 / E9D0 / E9F0 / E9C0 / EA10 | Type07 corresponding methods |

The codec uses only the low bytes of its zero and signed flags. Ordered positive/negative zero emits the short marker before any scale access. A float scale equal to the exact double representation of FLT_MAX skips scaling; raw32 then retains payload bits, including signaling NaNs. Other scales follow the original x87 divide, float spill, ordered clamps and unordered branches. Quantized writes multiply by the signed integer denominator and add/subtract one half, with unordered taking subtraction. Signed conversion uses the existing actual BF7420 implementation and borrowed mutable conversion-mode cell; unsigned conversion uses FISTP int64 under temporary truncation and restores the control word. Reads retain sign extension, unsigned bias, integer division, the float spill and subsequent scale multiply. Native zero-width/one-bit denominator cases are retained.

Type03 owns three float fields at +18/+1C/+20 in a 24h object. Its writer FLD/FSTP loads each payload before the raw32 codec, so a signaling NaN is quieted at the class writer even though the unscaled generic codec retains raw bits. Type06 stores a signed DWORD at +18 in a 1Ch object and serializes its low six bits; reading sign-extends six bits. Type07 is an 18h type-only object. Every class writes the current type byte, while its IsType compares the full query DWORD against a fixed class tag, independent of that mutable byte.

Constructors capture the actual game publication, select the owner using signed index +18EC in 0..7 and the +18CC table, preserve padding and uninitialized payloads, and publish the proper five-slot profile. Type03's valid branch sets mode before selected-owner storage; its invalid branch reverses these two stores. Modes are literal 0/1/1, with broader meaning provisional. Scalar deletion stamps the actual translated root, frees only for flags bit0 and returns the captured identity. Existing always-true, raw bit and allocation/free implementations are reused.

## Bindings and ABI

The original numeric routines take ECX cursor and five stack arguments, RET14h. Original class methods take ECX object and one stack argument, RET4; constructors have no stack arguments. Source interfaces explicitly borrow process constants and the conversion selector. Type03's translated profile contains five ABI-visible adapters followed by **source-only** binding metadata; the original D02FE4 table has just five slots. Its caller must maintain the profile and binding lifetimes. Type06/07 use static translated tables. This is component reconstruction, not a claim of whole-module drop-in ABI parity.

Writes receive the raw 10h bit cursor. Reads receive an 18h stream wrapper whose cursor starts at +4. Neither path fabricates a process owner, allocator, clock or numeric default state.

## Evidence and validation

- Verified the existing bsp.gpr / battlestationspacific.exe and 9,733 live/PE bytes: 1,482 new body bytes, 420 supporting body bytes, 171-byte original CRT block, 7,528-byte factory and 132 data bytes.
- Ten missing functions defined under the Ghidra write lock, with prior values/definition receipts preserved. Eighteen bodies and one factory fragment named/commented/saved; exports refreshed. All 33 reported owned direct calls checked against exact instruction ownership. Three more raw factory calls (00768776/007687B9/007687FC) have live instruction/PE evidence but absent stored function ownership; these are recorded separately.
- Strict MSVC Win32 build and all three existing CTests passed. No permanent tests added.
- One local original-code fixture: **173,211 pairs; 16,110,776 identical observation bytes**. Includes 172,032 numeric cases, 1,176 constructor/virtual/scalar cases and three freeing scalars. The original side executes copied CRT code, not the source converter.
- Numeric coverage includes PC0/2/3, all four RC settings, both CRT conversion modes, low-byte-only flags, widths0/1/2/6/16/24/31/32, signed zero, subnormals, finite bounds, infinities and quiet/signaling NaNs; seven scales include zero, signed zero and signaling NaN. Observations include cursor/buffer/output, x87 exception flags, control word and stack depth, and MXCSR flags/rounding. All eight cursor alignments are covered by class calls.
- Class observations preserve return/profile/cursor identities before normalization, constructor padding/payloads, invalid/valid selections, fixed type queries after changing the serialized type byte, stream wrapper state and retained scalar root stamps. Original freeing calls record the pre-free stamp/count; no released storage is inspected.

## Limits and follow-up packets

The full factory, remaining native message classes, recorder, socket/worker composition and ordinary startup/gameplay remain open. Continue from the factory's remaining class constructor/deserializer dependencies; do not substitute a generic parser. R177's factory membership gaps at 00768592/00768756 persist; three additional raw constructor sites at 00768776/007687B9/007687FC have the same missing ownership. All three instructions and targets are separately verified; no body-membership repair is claimed. R177 established that bridge scripting is disabled and GUI access unavailable; the shared service was not restarted or reconfigured.

The fixture does not establish widths above32, arbitrary object/cursor/context aliasing, concurrent constant changes, unmasked floating-point traps, all x87 condition-code bits, exceptional faults, allocator failure, original private EH or whole binary ABI. Store order is preserved by volatile source operations and checked against the listing; snapshots do not prove concurrent observers. Artifact and integrated COFF seals are recorded in reports/native_session_message_numeric_r178.json; file/build/differential evidence does not establish game validation.
