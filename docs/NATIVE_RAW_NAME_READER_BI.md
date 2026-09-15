# Qualified raw counted-name reader source BI

Addresses: BE4300, BE4620, BF0510, BE9FE0, BEA010. Base:
a808f88d60c77cc8c724393bb0036262ef9f9441, which includes scalar source e37775a4.

The five context-bearing C++ interfaces reconstruct the actual8h counted-name
path for current D642C0 memory and D691B0 physical streams. They borrow the
existing raw scalar and string-pool contexts, call the existing real byte-read
providers and actual pool operations, and add no allocator or virtual read stub.
One deferred registration was added; scalar files and CMakeLists.txt are unchanged.

BE4300 reuses the existing naked BE42E0 source: their full native26-byte bodies
are identical. Partial length-header reads retain the actual-count-pointer word's
untouched bytes. The counted producer checks slot38; the primitive itself checks
only its consumed byte provider through the existing scalar body. No zero-seeded
length or short-read rejection is introduced.

BE4620 initializes a temporary actual8h header, resizes it through the raw pool,
captures its length/data and fills the buffer with spaces20h. It reloads the
current byte provider and requests the original declared length. Actual header
plus payload bytes are published before output construction. The output header
is freshly zeroed and resized; previous output data is not released. Copy retains
the captured temporary pointer/length while reloading current output length/data
after resize. This preserves short payloads as space-tailed counted strings.

After copying output, the result-constructed bit is set and cleanup state is
lowered from1 to0 BEFORE the normal temporary return. That return uses captured
data and length+1 through the actual pool getter and return helpers. Exceptional
state1 instead destroys the current temporary header, then guarded state0 clears
the result bit and destroys output. A failure during normal temporary return
therefore cleans output without retrying the temporary. Source __try/__finally
expresses the recovered actions; it does not reproduce native FH3 metadata.

BF0510 qualifies current slot48, calls the producer, then debits actual bytes
modulo32 only after normal return. Its source count retains the native initial
output-pointer word in a new frame. BE9FE0 and BEA010 perform the raw node/reader
loads and return the original output pointer, without ownership or readiness
guards. The real string-pool helpers preserve their existing source boundary,
including omission of zero-byte standard-library copy calls.

Physical numeric profile tables and the literal null-buffer fallback0109DB64
must be accessible when consumed. Physical failure dispatch requires the existing
actual manager field90 original-ABI service. Unexpected profiles/slots throw a
source-domain exception, not a native read error. These source interfaces add
contexts and frames; exact original caller-stack pointer seeds, ABI thunks,
arbitrary profile support, native double-exception/asynchronous-fault identity
and gameplay validation are not claimed.

Current read-only Ghidra queries matched all22 complete native code/table/EH
spans against the PE and retained five current listings. The report carries
37 native call rows:27 direct checks and10 separately qualified indirect calls.
No Ghidra annotation, definition, prototype, save or ledger mutation occurred.

The strict full Win32 build, both existing CTests and all27 direct call checks
passed. One
focused local case uses the existing real memory read leaf and a real constructed
8AD4A0 string pool: declared length5 with only payload AB returns five bytes
`AB   ` plus NUL, advances six bytes and debits budget100 to94 through all five
interfaces. It uses a raw14h fixture read view and verified native profile words;
it does not test backing ownership, physical I/O, malformed headers or exceptional
cleanup execution. No new framework or persistent test suite was added.

Current compiler command/read/write blocks bind the new source and header to
the object; a unique archive member must equal that object byte-for-byte. The
report records the hashes, full build/test and probe output, native evidence,
scripts and failure history. Its whole-local inventory is outside local/ and
is rechecked twice with SHA256 and SHA512 before committing.
