# Raw ABAED0 unchanged-source storage

Library-only source over `d8cb0f1621c767c8cc76694f26a38aad128b07be`, following
the accepted source contract `a92c6b105b8069177c38b288f49b8f04e792ba38d7688e295bf2e50c760386a3`
and pins `79428ebfd9b4af997540142ac52ce21023020fabfb7b1f2539b1b3a87a984b70`.
New files only: source-storage HPP/CPP and this document. No owner/host, CMake,
ledger, shared API, raw admission, flag or Ghidra change is supplied here.
The new C++ types/names are descriptive hypotheses, not recovered ABI types.

The API is `submit_native_gui_text_source_unchanged_storage_00abaed0` over
`NativeGuiTextRawModelReference`, its exact widget/factory graph, one immutable
Source Frame/Acquired and one actual caller input8B/progress record. Its sole
normal result is `unchanged`. It has no localise argument, native register or
RET8/FH3 interface and cannot replace the whole native ABAED0 function.

Saved native evidence (existing shared exports, read-only):

- `00abaed0/assembly.txt`: ABAEF1 forms Text+F4, ABAEFA calls00435C40,
  ABAEFF tests AL and ABAF01 JNZ skips to AFE5. Resize/copy starts at AF07–27;
  the localise byte is first read at AF2F; the changed arm's current50 call is
  AFD7–E3. None is entered by this packet's unchanged result.
- `00435c40/assembly.txt`: left length then right length; differing lengths
  return false; two zero lengths return true without either data-pointer read.
  Equal nonzero lengths load right+4 then left+4 and call stricmp.
- Existing `native_physical_file_date.hpp/.cpp` supplies that actual8B adapter.
  This packet calls it ONCE and adds no comparator/library-body credit.
- Raw93 seeds F4/F8 zero (`native_gui_text_default_storage.cpp:306–307`).
  Existing empty raw retirement still requires both zero. No real raw F4
  publisher/ownership receipt is installed or inferred by this operation.

Initial admission is current constructor-owned F4/F8==0, a completed canonical
factory93/85/raw122 and live bound Text profile. It does not require Font, Main,
locale, UTF16, glyph, shader, current50, an allocation or a Native type getter.
The actual comparison therefore returns true only for zero input length; a
nonzero input length is a real false result before any input data/CRT access.
Even zero-length F4 with a foreign nonnull F8 is excluded by the ownership
admission. Those extra F4/F8 guard reads are separate from comparator epochs;
the implementation never follows F8 or treats that address as owned backing.

Frame construction stores addresses only. Context binds the exact widget Record,
WidgetLifetime and real raw string-pool view. Frame additionally fixes the
actual input header, its independent progress, expected private122 Frame and
Acquired. No constructor reads factory/publication, input/header or native cells.
The input record's immutable header/pool binding and six public markers are
caller custody evidence, not unforgeable allocation or Text ownership receipts.
The caller must actually perform existing41E870 and41DD20 over the same pool,
disabled and manager CELL addresses. No handmade std::string header projection.

Each call arms private entry first. Concrete Frame/Acquired/input/control,
factory Text/companion and disposal extents are checked before diagnostic writes
or header/native reads. Independent bind/scalar/disposal/ended history fences
all saved companion access. Exact bound phase, reference-context/Frame identity,
registry and count CELL identity precede the existing widget raw-transport
authentication. Bound-only history and actual current Text profile are checked
again afterward; constructor/retiring/base prefixes are not admitted. No retain,
positive-count test, count copy or second registration is introduced.

The exact completed93/resources/internal188 record, actual pool and widget
bindings are authenticated. Raw-pool comparison uses addresses of the three
canonical cells, permitting distinct live C++ context views over the same cells;
it neither samples/replaces those publications nor calls a getter. Input must
have real construct-entered/returned, no failure and no return-entered history.
Input/control/native Text/companion aliases are rejected, including self-header.
The current Text188 pointer is checked against its live producer record as an
extra admission fence; no Model/Font/native backing is followed by that read.

Comparison-entered and comparison-returned are independent. A false result is
recorded, then private failure is latched and `unsupported_changed` throws BEFORE
AF07. It never reports pending/changed completion or writes the source cache.
True alone seals private normal unchanged return. Private entry/comparison/
normal-return history survives a later rejected replay or tuple. Public diagnostic
changes cannot reset Frame phase, manufacture the seal or authorize F4 ownership.

Failure poisons Frame and the attempted/prepared Acquired records once their
concrete writable regions have been admitted. An unsafe metadata alias is not
written merely to record an error: Frame failure still retains that selection.
Acquired's saved Frame identity is never followed. Valid live independent C++
backing is a caller precondition, including on rejected tuples. No cleanup,
input return, Text destructor, slot return, Native EH-origin inference or retry
is attempted on escape. The caller must keep the entire reached graph retained.

Frame status/matches/destructor use own metadata/address identities only, even
after native retirement; no input/header/companion/provider reauthentication.
Active/failed Frame destruction terminates instead of dropping reached custody.
Normal-return history is historical, not current permission: the owner checks
Frame AND Acquired retention/failure and caller-input pending/failure separately.
Real input return must finish before scalar. No returned header/data is reread.

This synchronous leaf publishes no input/Frame pointer, header ownership or debt
into raw122/retirement, so it needs no new optional retirement provider/API.
Concrete checks do not discover opaque allocations behind borrowed services,
all dynamic provider backings or the owner's later flags cell. Caller proves
those live/disjoint, keeps every pinned C++ record alive during calls, forbids
concurrent scalar/native writers and keeps actual8B backing readable. The local
0x1F8 Text protection span matches existing raw122 checks; it is not allocator,
native-size, binary-ABI or pool-return evidence.

Root owns separate CMake/ledger/owner integration and any future qualification.
An eventual owner can reserve records before pools and call real41E870("") →
this fragment → real41DD20 over its ordinary DEFAULT graph, with no Font link.
Existing projected runtime_submit constructs synthetic headers from std::string
and localises/converts into std::u16string; it is not reused or credited here.
Changed/localised/nonempty F4 publication needs genuine retained writer/return
receipts and shared admission before it can be supported. Whole ABAED0/ABA8D0,
properties, content/glyph/layout, FE_initial/Page/P4 and visual/ABI/runtime proof
remain outside this library packet.

Validation at source freeze: isolated MSVC Win32 `/std:c++17 /W4 /WX /MD` object
compile passed. Its first attempt rejected dropping volatile from the genuine
pool-cell address view; the local Region now preserves const/volatile and the
failed log/receipt remain alongside the passed receipt. Artifacts are ignored
under `local/cc16_raw_source_equal_library_20261029`; no shared build outputs.
This proves object compilation only, not linking, differential behavior or
runtime/ABI acceptance. Root still owns integration and existing configured
checks. No tests, full build, games or activation were performed by this worker.
