# Native session messages 16 through 20 — R183

Addresses: 007862C0; 0075CBC0, 0075CD30, 0075CD40, 0075CDB0, 0075CE60; 0075D3F0, 0075D470, 0075D490; 0075E740, 0075E7C0, 0075E7D0, 0075E7F0, 0075E810; 004B6050, 004B6080, 004B6090, 004B60C0, 004B60F0; 0075E830, 0075E8B0, 0075E8C0, 0075E8F0, 0075E920. Parent fragment: 00768530.

## Result

Reconstruct 24 complete normal bodies (1,372 native bytes): five actual message profiles and the full 194-byte smoothed-remainder callee. The full factory remains a dependency fragment. Names are descriptive hypotheses, not recovered symbols. No unresolved callee is replaced with a stub.

| Tag | Size/profile | Constructor | Predicate | Writer | Reader | Scalar |
| --- | --- | --- | --- | --- | --- | --- |
| 16 | 78h/D02FD0 | 75CBC0,206B | 75CD30,13B | 75CD40,97B | 75CDB0,113B | 75CE60,31B |
| 17 | 18h/D03020 | 75D3F0,99B | 75D470,30B | shared449940 | shared449960 | 75D490,31B |
| 18 | 18h/D03160 | 75E740,99B | 75E7C0,13B | 75E7D0,19B | 75E7F0,21B | 75E810,31B |
| 19 | 1Ch/CE74DC | 4B6050,31B | 4B6080,13B | 4B6090,39B | 4B60C0,40B | 4B60F0,31B |
| 20 | 1Ch/D03174 | 75E830,98B | 75E8B0,13B | 75E8C0,39B | 75E8F0,40B | 75E920,31B |

Profiles order scalar delete, writer, reader, predicate and always-true4499C0. Native ECX carries the object. All constructors return identity in EAX with no stack arguments and RET; other listed class methods take one DWORD/pointer stack argument and RET4. Source raw virtual adapters retain the ECX/stack placement. Type16's source profile adds borrowed numeric/smoothing context pointers after its five visible slots; this metadata is SOURCE-ONLY, and its backing must outlive use.

## Constructors and simple messages

The base stores mode3, zero08/0C, profileD02C68 and the fixed typebyte10, then captures E188A8 once. Signed index18EC in0..7 selects owner14 from table18CC, otherwise null. Padding11..13 is retained.

Types17/18 set mode1 then their final profile and have no payload. Type17's predicate accepts either full DWORD17 or the current typebyte10; type18 accepts only18. Type17 directly shares existing native writer449940 and reader449960, so only three new native bodies are counted for it.

Types19/20 carry raw byte18 plus retained padding19..1B. Writers send type8 and byte18!=0 as one bit; readers replace type10 and write canonical0/1 to byte18. Predicates accept only fixed19/20. Type19 initializes mode1, final profile, then flag0. Type20 reads the selected owner, clears flag0 BEFORE publishing that owner in its valid-index branch; its invalid branch stores null owner before clearing the flag. Mode1/profile follow both branches. Source preserves this distinction.

All five scalar methods stamp real root profileCE4974, free only for flags bit0 and return identity through the existing root scalar implementation. No new payload ownership exists.

## Type16 records and wire format

The object contains eight 12-byte records at18h, each represented as three raw float words. Its constructor captures D7A24C with MOVSS once, sets profileD02FD0, initializes each record to0,0,captured-one and sets mode0 last. The capture preserves raw bits, including MOVSS behavior, rather than converting through an x87 float return.

The wire is168bits: type8 followed by eight pairs of10-bit numeric values. Each numeric call uses flags0/0 and literal scale1. Writer75CD40 performs FLD/FSTP on each source value before calling the existing numeric codec; source retains those signaling-NaN and precision boundaries. The third record value is not serialized.

Reader75CDB0 decodes both10-bit fields into each record, captures second then first through FLD/FSTP, then calls7862C0 on the record. The third word therefore depends on its previous value. No substitute recomputation, callback stub or zero initialization is inserted on read.

## Smoothed-remainder callee7862C0

Native ABI: ECX points to the12-byte record, two float stack values, RET8; no defined return value. Source exposes raw argument bits plus borrowed references to the actual constants. Semantic names remain provisional.

1. Clamp each input with COMISS/JBE against positive zero and one capture of D7A24C. Unordered comparisons retain NaN input bits. Store both clamped values with MOVSS.
2. Load second with x87, add first, use FLD1/FSUBRP, then spill the remainder to float. Reload it and compare zero using FCOMIP; only an ordered negative remainder becomes positive zero.
3. Load and spill the old third value through float, reload both values, compare remainder to old with FCOMI, then FXCH. The carry branch is taken for a smaller remainder OR unordered values.
4. Multiply old and new values by the original double constants, in native operation order; add and spill the new third value to float. x87 precision/rounding remains caller-controlled.

| Branch | New coefficient address/raw64 | Old coefficient address/raw64 |
| --- | --- | --- |
| smaller or unordered | CE3E20 /3FB47AE140000000 | D04308 /3FED70A3E0000000 |
| other | CE4D68 /3FA47AE140000000 | D04310 /3FEEB851E0000000 |

These are the actual double bit patterns; replacing them with rounded decimal0.08/0.92/0.04/0.96 changes arithmetic. The source uses a bounded MSVC Win32 assembly block to preserve comparisons, double FMUL operands, spill widths and NaN behavior.

## Evidence and Ghidra

10,673 live Ghidra/PE bytes match:1,372 new bodies,1,402 support bodies,7,528 full factory bytes,200 profile/constant bytes and171 bytes of the native CRT conversion block used by the copied codec. Fourteen absent functions were defined under the write lock. All five new scalar listings have zero returning-free gaps. Prior names/comments are retained and affected exports refreshed after annotation/save.

45 owned CALL/tail-JMP rows pass exact stored ownership checks. The five factory calls are76891E->75CBC0(size78h),76893E->75D3F0(18h),76895E->75E740(18h),76897E->4B6050(1Ch),76899E->75E830(1Ch). These live instructions and PE bytes are verified but lack stored Ghidra function ownership; they are recorded separately. Earlier factory metadata gaps persist.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. One ignored local differential probe executes copied, relocated, verified native bodies and the reconstructed implementations with the same private backing. Constants are initialized from the captured native bytes. Source uses actual existing numeric and singleton allocation/free providers.

117,526 pairs match9,782,792 bytes:140 constructor cases,3,136 simple class cases,110,976 direct smoothing cases,3,264 type16 numeric cases and10 freeing scalar returns. Coverage includes all cursor alignments; signed selection bounds; raw boolean bytes; mutated wire tags; full DWORD predicates; retained padding; both CRT selector modes; x87 precision fields0/2/3 and all four rounding modes; positive/negative zero, subnormals, finite bounds, infinities and signed quiet/signaling NaNs. Smoothing additionally crosses all DAZ/FTZ combinations. The fixture compares x87 exception flags, stack balance, control word and MXCSR status/control. Type16 reads execute twice against retained third values.

The report pins source/object hashes and immutable tested/integrated evidence. Native pre-free stamps are observed before actual free; released memory is not inspected. No permanent tests were added. This is normal-path arithmetic/stream/profile evidence, not an unmasked FP trap, arbitrary alias, concurrency, original FH3/SEH, whole binary ABI, network exchange or gameplay claim.

## Follow-up packets

Continue remaining factory message classes starting with the190h and138h allocations using constructors764340/7643E0. Recover their actual record/lifetime/codec dependencies, then compose the full768530 factory,787850 packet recorder and network workers. Ordinary startup and gameplay remain open.
