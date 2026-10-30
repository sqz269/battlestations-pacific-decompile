# Raw Movie record backing allocation

This source prerequisite adds the complete AACD30 and AACD90 allocation
contracts at the existing Source CRT service boundary. It prepares genuine
14h/24h backing domains for future raw Movie vector composition. It does not
activate a Movie, allocate a widget pool slot, construct records or tidy a
vector. Existing semantic `allocate_elements` and its callers are unchanged.

Frozen published source base: `60c8453a1b661dfa62625945986e38d71bee0216`.
Names are descriptive hypotheses. No additional native-body, vendor-library,
original-ABI or gameplay credit is claimed.

The configured existing `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` were verified before the live analysis batch.
Each original body is 85 bytes and 27 instruction starts: AACD30..AACD84 and
AACD90..AACDE4 inclusive. ECX carries an unsigned count; incoming EDX is dead.
The returning path leaves the allocator result in EAX, consumes no stacked
inputs and uses plain RET. The new fastcall declarations explicitly reserve
the unused EDX slot. Native stack temporaries and exception identities are
not recreated or asserted ABI-compatible.

Zero reaches the allocation call with zero bytes. Positive counts divide
FFFFFFFF by count and compare with stride14h/24h. The accepted maxima are
0CCCCCCC and 071C71C7, with products FFFFFFF0 and FFFFFFFC respectively.
The first rejected positive values overflow that extent. Source comparison
against those exact maxima replaces the quotient branch without changing
the unsigned input partition. Multiplication is in the 32-bit domain.

Each original body has three direct calls: BF681B allocation, BF6340 exception
construction and BF6885 throw. The overflow arm constructs native bad_alloc
identity D6923C with throw metadata E03CC0 and does not return. This source
uses the established `std::bad_alloc` transport, as the admitted raw FrameBox
allocation contract does; it does not port vendor exception/handler bodies.

The allocation arm calls the real `singleton_lifetime_allocate` with object
kind and equal native/host byte extents. That service implements the admitted
malloc / actual CRT `_callnewh(bytes)` / retry / bad_alloc loop. It is called
even for count zero. No fallback, provider callback, cached result, C++ record
container or inferred success is introduced. Its matching backing return is
the actual `singleton_lifetime_free` / free domain. This does not authenticate
foreign heaps, arbitrary operator-new arrays, string-pool buffers or widget
pool slots, nor reinterpret any existing semantic allocation as this domain.

The caller owns backing, any constructed record prefix and all borrowed
providers through normal or failed record operations. These helpers perform
no stores to the allocation, no cleanup, no publication, no local rollback and
no destruction. Future range/tidy/insertion and the whole populated Movie
must establish their own actual ownership and failure schedules.

Strict MSVC Win32 compilation and the static byte/call/object receipts are
recorded in `reports/cc16_native_gui_movie_record_allocation.json`. No new
test, executable, game, DLL load or allocation execution is used for this
source packet. Independent review, common build/link and primary admission
are separate later stages; no consumer or guard changes here.

The strict public records are each 94 bytes including one terminal trap byte.
At +0E/+14, unsigned comparison/JA selects the exact accepted count partition;
+16/+20 form the byte product, +23/+27 store equal request extents, and +2F
calls the actual allocation service once. The normal path returns at +45;
the overflow arm calls Source bad_alloc construction at +49 and the Source
throw service at +58. Security-cookie/RTTI/CRT support is ordinary host
emission, not reconstructed native machinery. No incoming EDX value or
returned backing is accessed. All six original direct-call rows pass the
existing static checker; both 85-byte live spans match the installed PE.
