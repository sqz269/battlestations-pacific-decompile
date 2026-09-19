# Native session message 88 (R200)

Addresses: 00764C80, 00764CF0, 00764D00, 00764D20, 00764D40, 00764E60; profile 00D03630; partial factory 00768530.

## Scope

Five complete normal game bodies (711 bytes) and one five-slot profile are reconstructed. The existing 32-byte `BSP_CommandMessage_IsKind` / `entity_command_message_is_category_00764d00` predicate is reused from `cruise_command.cpp`; the existing `SceneCommandTarget` is reused from `scene_deferred_refs.hpp`. Those files were left unchanged. Descriptive names remain hypotheses, not recovered symbols. No library code was newly ported.

The factory table at 0076A3F4 maps type 88 to 00769CB0, which allocates 3Ch and calls 00764C80 at 00769CC4. Type 89 maps to the shared default route 0076A277; this does not establish a safe null/rejection contract. The full factory remains unbound.

## Layout and lifetime

`NativeSessionMessage88` is 3Ch: the existing extended message header occupies 00..1F, command byte 20 and flags byte 21 precede retained bytes 22..23, and the existing 18h `SceneCommandTarget` occupies 24..3B.

Constructor 00764C80 (101B) zeros fields 08/0C, owner 14, mutable type 10, sender 18 and flags 1A/1C; sets delivery 1 and profile D03630. It MOVSS-copies the three borrowed floats at F87574/78/7C to 2C/30/34, then zeros target kind/position-valid 24..25, cached object 28, kind 24 again, handle 26 and trailing 38. The observed default vector is zero, but the source binding borrows its storage and retains bitwise copies, including NaNs. Command 20, flags 21 and padding are retained. No Game singleton is read.

Ordinary destructor 00764CF0 (7B) stamps root CE4974 only. Scalar destructor 00764D20 (31B, RET4) stamps that root, frees through the existing concrete allocator iff flag bit 0 is set, and returns captured this. The scalar flow has no listing gaps.

The existing predicate 00764D00 accepts full DWORD values 88, 73, 70 only, independently of receiver and mutable type. Its complete end is 00764D1F: the RET4 at 00764D1D is three bytes. This corrects the older end-address note without editing the peer-owned source. The fifth profile slot reuses 004499C0.

## Wire behavior

Both codecs process mutable type 10/8 bits, sender 18/12, booleans 1A/1C, command 20/8 and boolean 21. Writer 00764D40 (281B, ECX=this, stack raw cursor, RET4) then processes three independent optional fields:

| Presence | Payload | Reader effect when present |
|---|---|---|
| target kind 24 is nonzero | handle 26 /13 bits | kind 24=1; store handle 26; clear cached object 28 |
| position-valid 25 is nonzero | three signed numeric floats /32 bits, max-finite scale | position-valid 25=1; copy decoded vector to 2C/30/34 |
| COMISS trailing 38 >= borrowed 1.0 at D7A24C | one signed numeric float /32 bits, max-finite scale | copy decoded float to 38 |

COMISS/JB makes the trailing field absent for values below 1.0 and unordered values. Signed zero is absent; exactly 1.0 is present. The writer uses the existing numeric array provider 00429790, preserving local MOVSS vector copies; the scalar trailing value and maximum scale pass through x87 load/store in native order.

Reader 00764E60 (291B, ECX=this, stack 18h read stream, cursor at +4, RET4) reads presence into local booleans. **Absent fields preserve previous state**, including target flags, handle, cached object pointer, vector and trailing float. It uses 004294F0 for three floats and 004293F0 for the trailing float, copying outputs with MOVSS semantics. It does not impose the writer's trailing threshold on input. Read-stream ownership bytes and message padding are retained.

## Validation

Strict MSVC Win32 build and three existing CTests pass. The fixture matches **23,627 original/source pairs and 39,418,809 bytes**. New coverage is 2,438 cases: 128 constructor/cleanup cases with borrowed default-vector bit patterns and raw-storage guards, 256 predicate cases, 6 scalar cases, and 2,048 wire cases with two sequential reads into the same populated receiver. Wire coverage includes all eight alignments, four rounding modes, two numeric conversion selectors, 32 float patterns, noncanonical flag bytes, handle truncation, signed zeros, denormals, NaNs/infinities and values adjacent to 1.0. Serialized bytes, retained fields, cursor/ownership state and x87/MXCSR exception flags match. Five inherited source-only cleanup faults remain; no new EH claim is made.

Evidence verifies 33,477 live Ghidra/PE bytes, 26 owned CALL edges and 1,082 fixture relocations. The factory CALL 00769CC4 has verified bytes but no stored Ghidra function membership and is recorded separately. Writer and scalar destructor were defined at their verified ranges. Prior Ghidra names/comments are preserved, annotations are read back and exports refreshed under the normal write-lock workflow. The immutable archive includes 27 relevant COFF objects, including the reused cruise-command object.

## Limits and follow-up

The source profile contains a borrowed context pointer after its five native slots; this is not a full original binary ABI claim. Unmasked FP fault ordering, nondefault DAZ/FTZ/precision controls, arbitrary aliases, concurrent mutation, malformed inputs, allocation failure and original FH3 remain unproved. Build and fixture evidence do not establish gameplay parity.

Next factory entries 90/91 allocate 28h/2Ch and call 0075AF60/0075AF90 with profiles 00CFD9C4/00D02E84. Recover their actual methods before binding the full factory. Recorder, networking, startup and game validation remain open.
