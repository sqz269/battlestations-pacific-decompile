# Native session messages 12 through 15 — R182

Addresses: 00429030; 004B5FB0,004B5FE0,004B5FF0,004B6010,004B6030; 0075D4B0,0075D530,0075D550,0075D580,0075D5B0; 0075D2E0,0075D350,0075D370,0075D3A0,0075D3D0; 0075CAA0,0075CB00,0075CB10,0075CB60,0075CE40. Parent fragment:00768530.

## Result

Reconstruct 21 complete normal bodies,894 native bytes: four actual five-slot message profiles and the signed-word stack writer. The full factory remains a dependency fragment. Names are descriptive hypotheses, not recovered symbols. Implementation uses actual existing bit readers/writers, current-game selection and scalar free providers.

| Tag | Size/profile | Constructor | Predicate | Writer | Reader | Scalar |
| --- | --- | --- | --- | --- | --- | --- |
| 12 | 18h/CE74C8 | 4B5FB0,27B | 4B5FE0,13B | 4B5FF0,19B | 4B6010,21B | 4B6030,31B |
| 13 | 1Ch/D03034 | 75D4B0,99B | 75D530,30B | 75D550,39B | 75D580,40B | 75D5B0,31B |
| 14 | 1Ch/D0300C | 75D2E0,96B | 75D350,30B | 75D370,39B | 75D3A0,40B | 75D3D0,31B |
| 15 | 1Ch/D02FBC | 75CAA0,92B | 75CB00,13B | 75CB10,76B | 75CB60,78B | 75CE40,31B |

Each native profile orders scalar delete,write,read,type predicate,always-true4499C0. Native ECX carries the object. Constructors return the object in EAX with no stack arguments except type14's DWORD flag and RET4. Other class methods take one DWORD/pointer argument and RET4; predicate12/15 ignore ECX. Source profiles expose the same five slots through Win32 fastcall adapters; explicit C++ constructor/context interfaces are not whole binary replacements.

## Constructors and payloads

All constructors initialize the18h base: mode3,zero08/0C,baseprofileD02C68,typebyte10,one E188A8 game capture and signed18EC index0..7 selecting the18CC pointer table,else null. Bytes11..13 remain untouched.

Types12/13 set mode1 then final profile. Type13 leaves its raw byte18 and padding19..1B untouched. Type14 uses only the low byte of its DWORD argument; after base selection it sets mode1,final profile,then byte18. It does not normalize the constructor flag. Type12 carries no payload.

Type15 initializes mode0,countWORD18=0 and final profile,retaining1A/1B. On the valid selection branch it reads the selected owner,then stores mode0/count0 BEFORE owner14; on invalid selection it stores null owner first. Source retains that branch-dependent store order instead of calling the generic base constructor.

Types13/14 serialize type8 then raw flag18!=0 as one bit; readers replace type10 and normalize byte18 to0/1. Their predicates compare the full DWORD query first with fixed13/14,then with current typebyte10. Types12/15 predicates compare only their fixed type,independent of the mutable wire byte. Payload writers/readers always use the mutable typebyte.

## Type15 count and padding payload

Writer75CB10 writes type8,WORD18 as16bits through429030,then writes count copies of byte0x21. The first loop test reloads the count after the header write; every iteration reloads unsignedWORD18 after the byte write, increments its signed32-bit index and compares. Values32768..65535 are valid positive loop bounds.

Reader75CB60 reads type8 and unsignedWORD18,then calls the actual byte reader once per count,discarding each byte. It does not check for0x21 or replace the loop with cursor arithmetic. It reloads count after each read. Native discarded storage is a reused stack slot; source uses a local byte. Both paths preserve helper carry/advance semantics. No capacity or count clamp is introduced.

429030 is a complete18-byte ECX-cursor wrapper with DWORD value/bits stack arguments and RET8. It passes the address of its value to428F50; signed interpretation is not performed by this writer. The explicit source wrapper retains its full DWORD storage.

All four scalar methods stamp actual root profileCE4974,free only if flags bit0 is set,and return identity. They compose the actual existing scalar root implementation. No payload ownership is introduced.

## Evidence and Ghidra

9,184 live Ghidra/PE bytes match:894 new bodies,650 support,7,528 full factory bytes and112 profile bytes. Thirteen absent functions were defined under the write lock. All four new scalar listings have zero gaps after returning free. Prior names/comments and annotation receipts are preserved; affected exports are refreshed after naming and saving.

32 owned CALL/tail-JMP rows pass exact stored ownership verification. Factory allocation/call pairs are1Ch->75D4B0 at76889C,18h->4B5FB0 at7688BC,1Ch plus flag0->75D2E0 at7688DE,and1Ch->75CAA0 at7688FE. These four live instructions and PE bytes are verified but lack stored Ghidra function ownership; the report records them separately. Earlier factory metadata gaps persist; no full factory repair claim.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. One ignored local differential probe copies verified native code into private executable buffers,relocates reviewed calls/data,executes original and source raw profiles,and compares normalized object,backing,cursor and game-selection observations.

3,148 pairs match4,999,100 bytes:196 constructors,2,128 five-slot class cases,816 signed-word cases,and8 freeing scalar returns. Coverage includes all cursor alignments,signed selection bounds,raw flags0/1/2/80/FF/100/101,mutated tags0/12/13/14/15/FF,predicate high bits,all word widths0..16,and counts0/1/31/255/256/32768/65535. Writers have both zero and nonzero backing and declared length0 with full physical backing; readers consume either0x21 or arbitrary patterned payload. Retained root stamps are observed; original pre-free stamps are recorded before actual singleton free; released storage is never inspected.

The report pins object/source hashes and immutable tested/integrated artifacts. No permanent tests were added. These are normal-path comparisons,not original FH3/SEH,arbitrary alias/fault/concurrency,whole binary ABI,network exchange or gameplay proof.

## Follow-up packets

Continue the factory's remaining message types with independent constructor,profile,wire-layout and lifetime evidence. Then compose the full768530 factory,787850 packet recorder and network workers using real providers. Ordinary startup and gameplay remain open.
