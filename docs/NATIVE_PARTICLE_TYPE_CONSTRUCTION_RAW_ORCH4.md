# Raw particle type construction

`native_particle_type_construction.hpp` adds seven overloads that use the actual
raw string pool, concrete record storage, and actual loaded `d3dx9_40` export.
The previous host-binding overloads remain available. No emitter-definition
binding, allocator callback, replacement profile, parser, or parameter pool is
needed by these constructors.

| Complete original body | Native ABI | Profile on return |
| --- | --- | --- |
| B01150..B012E8 | ECX owner; stack(name8h, word14, parent18); RET0C / EAX owner | D5DDC0 |
| B08830..B0886B | same | D5DFF4 |
| B058E0..B05934 | same | D5DF30 |
| B076F0..B0772F | same | D5DFB0 |
| B00770..B00794 | same | D5DCEC |
| AF89E0..AF8A2B | same | D5DB00 |
| B0A0B0..B0A0EF | same | D5E048 |

Names remain descriptive hypotheses. The Sprite and Axial constructors return
the intermediate profiles shown above; the later factory installs D5DD18 and
D5DCC0. Floating B00770 first calls B076F0, then installs D5DCEC. The source
does not silently apply final factory profiles inside intermediate constructors.

## Base schedule and sparse bytes

B01150 stores CEB130, reference count 1, then D5DDC0. It zeros the actual +08
name and +68 record descriptor before comparing the incoming name with +08.
Consequently a self-name becomes empty. The raw 41DD40 resize runs against the
actual name header and publication cells; subsequent source length, destination
length, source data and destination data are current reads. The BF7680 copy
admits overlap. A zero-byte source copy is omitted in C++ to avoid imposing
pointer validity for a copy that transfers no bytes.

The native constant schedule is: capture D7A24C, capture CE3804, store +24,
store incoming +14, clear +58, then capture CE6650. The four record floats are
raw words `{0,0,current_CE6650,current_CE6650}`. B01274 calls the actual
`D3DXFloat32To16Array` import, whose original C2DFCE thunk jumps through CE2410.
MOVSS and XORPS perform all floating-bit loads/stores here. None of these seven
bodies has x87 arithmetic; no additional float rounding step is introduced.

The source preserves the native ordered sparse stores:

- +18 receives the incoming parent, +78 receives byte 1.
- +1C is zero; bytes +29 then +28 are zero.
- DWORDs +2C,+30,+48,+34,+38,+3C,+40,+44,+74 are zero.
- Byte +4C is zero, followed by DWORDs +50,+54,+5C.
- Bytes +60,+61 are zero, +62 is 1, +63,+65 are zero; +20 receives captured CE3804.

Unwritten fields, including the common base's +10, +2A..2B, +4D..4F, +64,
+66..67 and +79..7F, retain incoming bytes. Every derived body retains its
own sparse layout: for example, Axial clears only byte +80 and leaves +81..83;
Object leaves +88; Tracer only adds +98,+9C,+A0 and +10 beyond its identity.

The native seven-DWORD record copy includes a DWORD at record+18 that the
constructor never initializes. `initial_record_stack_word18` is a required
explicit borrowed input, with no invented default and no undefined C++ read.
Record growth uses signed `max(wrapping_capacity*2,1)`, the genuine B00C20
reserve, current descriptor fields, and forward REP MOVSD. Count increments
even when the computed destination is null, as in the original branches.

## Cleanup evidence

FH3 handler CBB46E points to info DF34BC and unwind map DF34A4:

| State | Next state | Funclet | Concrete action |
| --- | --- | --- | --- |
| 2 | 1 | CBB463..CBB46D | B00F70 on actual owner+68 |
| 1 | 0 | CBB458..CBB462 | raw 41DD20 on actual owner+08 |
| 0 | -1 | CBB450..CBB457 | BD30F0 on actual owner |

The native constructor publishes state 0 after base identity and state 2 after
the name and descriptor are initialized; no throwing call occurs between those
stores. The source guard performs these genuine leaves during C++ unwind.
Secondary unwind exceptions terminate. Success disarms cleanup. The six derived
bodies introduce no further allocation, call after base construction, or local
FH3 map, so they add no separate cleanup owner.

## Evidence and verification

The report records complete original byte spans, hashes, direct call-site rows,
old Ghidra annotations, prior ledger records, and FH3/import/constant bytes.
Every span was compared with the installed PE and the live program
`/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`. Ghidra was read only;
the existing complete listings needed no repair.

The ignored probe under `local/particle_type_construction/` executes all seven
complete original bodies plus original B00C20, relocating only decoded calls
and absolute operands. Original child constructors call copied original parents.
ABI adapters connect the original library boundaries to concrete raw string
resize, fixed CRT allocation/free, and the actual Windows SysWOW64 D3DX export.
Physical profile words remain original numeric identities. The input wrapper
places the explicit stack residue at the correct depth for base, derived and
two-level Floating construction and verifies native RET0C stack balance.

The probe passed 28 original/source comparisons: seven constructors across
ordinary names, self-name, long names with changed raw NaN/-0 constant bits,
and constant cells aliased to current owner storage. It compares all E8 owner
bytes (normalizing only separately allocated string/record pointers), all 1Ch
record bytes, and name bytes. Actual string pools and concrete record cleanup
are used on both sides. Strict MSVC Win32 `/O2 /EHsc /MD /fp:strict /W4 /WX`
compilation passed; the probe embeds its manifest. Repository build/CTest
results and exact source/probe hashes are recorded in the report.

This establishes reconstructed, build-tested and bounded native fixture-tested
normal-path behavior. Original FH3/SEH exception transport, arbitrary invalid
pointer faults, binary replacement compatibility and gameplay remain unvalidated.
B00CE0 factory/parser and property/resource domains are outside this packet.
