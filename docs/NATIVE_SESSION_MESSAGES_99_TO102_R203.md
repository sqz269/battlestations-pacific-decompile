# Native session messages 99 through 102 (R203)

Addresses: constructors 0075C380, 0075C450, 0075A150, 0075C570; predicates 0075C400, 0075C4D0, 0075A1A0, 0075C5F0; ordinary cleanup 0075C3F0, 0075C4C0, 0075A190, 0075C5E0; scalar cleanup 0075C430, 0075C500, 0075A1D0, 0075C620; shared codec 006D2F20/006D1550; message 101 wrappers 0075C520/0075C540; partial factory 00768530.

## Scope and profiles

Twenty complete normal game bodies (962 bytes) and four five-slot profiles are reconstructed using existing base/header, bit, allocation and CRT providers. No library code was newly ported. Names remain descriptive hypotheses.

| Type | Allocation | Factory constructor | Profile | Payload |
|---|---|---|---|---|
| 99 | 24h | 0075C380 | 00D02F6C | float bits20, default -1 |
| 100 | 24h | 0075C450 | 00D02F80 | float bits20, default 256 |
| 101 | 28h | 0075A150 | 00D02CF4 | float bits20, default 256; retained DWORD24 |
| 102 | 24h | 0075C570 | 00D02F94 | float bits20, default 256 |

Predicates accept their fixed type plus 98, 73 and 70. They compare the full DWORD and ignore receiver/mutable type. All ordinary cleanup bodies stamp root CE4974; scalar cleanup stamps the root, frees through existing BF65AC if flag bit 0 is set, and returns captured this. Slot 4 reuses 004499C0. Profiles 99/100/102 share the exact same writer and reader adapters; profile 101 wraps the shared codec. Five native slots precede a borrowed source-only context; full binary ABI is not claimed.

Category 98 storage/codec is a source naming hypothesis. It does not bind factory type 98, whose previously inspected switch entry reaches shared default 0076A277.

## Constructor ordering

Constructors 99/100/102 are 102 bytes each. They inline the base initialization and capture Game E188A8: selected index18EC in 0..7 reads owner18CC+4*index, otherwise owner NULL. Mutable type is the fixed class type. Then MOVSS captures the borrowed default before clearing sender18/flags1A/1C and setting delivery1. Type 99 writes its own profile before value20; types 100/102 write value20 before their profile. Source reuses concrete 75B430 for the equivalent inline base sequence; no additional native CALL is asserted.

Constructor 101 is 53 bytes. Its first operation, MOVSS, captures CF8B3C, then it clears fields08/0C, owner14, mutable type 10, sender18 and flags1A/1C, sets delivery1, stores value20 and finally its profile. It never reads Game. DWORD24 and padding retain their old bits. Raw signaling-NaN defaults remain unchanged by MOVSS.

## Shared scalar writer 006D2F20

The 124-byte writer receives this in ECX and a raw cursor on the stack (RET4). It writes the common 75B480 header, then boolean1C. Presence follows this exact x87 sequence: load value20, load double threshold D7A3A0, FCOMIP threshold,value, pop remaining value, branch on carry. Presence therefore means value greater than threshold **or unordered**, including NaNs.

The threshold is binary64 `0.10000000149011612` (bytes `000000a09999b93f`), the widened single-precision value, rather than binary64 literal 0.1. An absent value emits only presence0. A present value emits presence1, reloads value20, multiplies in x87 by double D7A328=4, and calls the existing CRT BF7420 provider with ST0. Only then is low EAX interpreted as signed32 and clamped to 0..255 for the unsigned8 write.

The order matters for large finite values and NaNs. The actual 0109EEA4 selector chooses the existing SSE conversion or the 64-bit fallback's low EAX. For example, a value just above 2^30 multiplied by 4 can produce a positive low DWORD in the fallback while the SSE conversion returns an invalid-conversion sentinel. Both modes remain observable; no input-value clamp was substituted.

## Shared scalar reader 006D1550 and message 101

The 114-byte reader takes this in ECX and an 18h stream on the stack (RET4), using its cursor at +4. After common 75B4C0 and boolean1C, it reads a local presence bit. Presence reads unsigned8, loads it with FILD signed32, retains the native negative-value correction using float CE3978=2^32, multiplies by double D7A348=.25 and stores value20. The correction branch is unreachable with a valid eight-bit result.

Absence overwrites value20 using MOVSS from borrowed D7A260 (normally -1). Repeated reads therefore clear the old scalar. Other fields and padding retain their values. Message 101's 32-byte writer and 33-byte reader call this shared codec and then write/read unsigned DWORD24 at width 2.

## Validation and factory evidence

Strict MSVC Win32 compilation and all three existing CTests pass. The cumulative ignored native differential fixture compares **43,775 original/source pairs and 45,183,709 matching bytes**. New coverage is 8,328 cases: 368 constructor/cleanup, 1,024 predicate, 24 scalar, 4,096 shared scalar wire, 512 message 101 wrapper wire, 128 profiles 100/102 wire and 2,176 direct reader cases. The five source-only cleanup faults are inherited coverage; no new exception claim is made here.

Scalar coverage includes 64 input variants, all eight bit alignments, both CRT selectors, four x87 rounding modes, and comparison of x87/MXCSR exception flags. Direct reader coverage exercises every byte 0..255 and 16 borrowed absent-value bit patterns. Sequential reads verify absent-value overwriting; constructor cases cover seven owner-index cases, borrowed NaN defaults and message 101 with a null Game cell. Source profile slot identity is checked for 99/100/102. There are no new committed test cases.

The collector compares 37,772 live/PE bytes, 19 owned CALL rows and 1,277 relocation records. Ten missing functions were defined; all four scalar flows have zero gaps. Six borrowed constant spans were independently checked (36 bytes). Constants already provided by existing fixture cells are not duplicated as cloned globals.

Table 0076A420 maps 99..102 to 00768E72/00768E92/00768EB2/00768ED2. Allocation sizes and constructors match the table above. Four raw CALLs 00768E86/00768EA6/00768EC6/00768EE6 have no stored Ghidra function membership, so they remain separate from the 19 owned rows. Types 103..105 route to 00768EF2/00768F12/00768F32 and remain follow-up.

The report records source/artifact hashes, native bytes, contracts, call checks, prior annotation values and immutable local evidence archives. Final integration checks 30 relevant COFF objects while ignoring only timestamp bytes 4..7 and reruns the same fixture against the final merged library.

## Limits and follow-up

Full factory 00768530, recorder, networking, startup and gameplay remain open. Masked FP fixtures do not establish unmasked exception ordering, nondefault DAZ/FTZ/precision, arbitrary aliases or concurrent mutation. Allocation failure, malformed inputs and original CRT/FH3 behavior remain unproved. Contexts and pointed-to cells must outlive their profiles and calls. The next bounded factory entries are 103 onward.
