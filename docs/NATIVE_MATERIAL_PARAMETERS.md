# Actual native material parameter registration

`native_material_parameters` registers directly into material+80/+100 and the
same actual84h parameter records inside the existing88h pool slots. It reads
actual effect/pass/shader metadata and retains borrowed source pointers. There
is no copied MaterialCloneState, private parameter table or shader-name index.

## Evidence and original interfaces

All live batches verify `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Six complete code/table spans match the installed
PE; exact bounds and hashes are in the report. The five standalone functions
already exist and have no call gaps. New descriptive names are hypotheses.

| Entry | Original ABI | End exclusive |
|---|---|---|
| B17E10 | ECX material; stack name/source/count/flag; EAX record; RET10h | B17EEF |
| B44D60 | ECX effect; stack material/existing/name/source/count/flag; EAX record; RET18h | B44F9E |
| B18790 | Incoming ECX ignored; set F8D3E4 and tail B185A0; EAX slot | B1879A |
| B5B820 | ECX20h constant entry; EAX=ECX+14; RET | B5B824 |
| B5B870 | ECX20h constant entry; signed EAX=[ECX]; RET | B5B873 |

The D61A00 effect table's+10 entry is B44D60. The wrapper supports that verified
current profile and requires its actual table view. The direct B44D60 interface
does not dispatch virtually. No complete effect storage/lifetime is inferred.

## Same storage and native ordering

The parameter starts with an actual eight-byte NativeString. Source is+08,
DWORD word count+0C, exact matrix byte+10, and fourteen signed register indices
per stage at+14 and+4C. Padding+11..13 is preserved. The pool alone owns+84.
Inline initialization zeros the name/source, interleaves -1 writes to VS/PS
indices, and clears the matrix byte. It leaves+0C unwritten until registration.

B17E10 returns immediately for null effect7C, before inspecting names, sources
or the table. Otherwise it finds the first equal-length case-insensitive name
in the current material table, reloads the effect and passes that same record
or null to current+10. Equal empty strings do not read their data pointers or
invoke the CRT comparator. The CRT comparison contract/locale is a required
external service, rather than a new ASCII/lowercase implementation.

B44D60 scans fourteen current effect pointers at+C8..FC. Each nonnull pass has
vertex/pixel owners+70/+74. Their ordinary constant list is pointer+78/count+7C,
with20h entries. B5B820 returns each entry's actual name header+14; B5B870 reads
its signed register index. The first matching name wins even when its register
is negative; a later duplicate is not searched for a positive index.

The stage scan preserves its cursor and reloads the current end after a missed
comparison. The pass pointer is reloaded between vertex and pixel searches.
These accesses do not snapshot a second metadata collection.

Only a selector with a nonnegative match in either stage triggers an update.
The first such selector allocates/initializes a missing parameter through the
real pool, publishes its pointer and increments material count before name
resize. It then copies the actual name using the existing0041DD40 service,
skipping a self-header copy, and reloads the headers after that storage call.
Source/count/exact byte are stored once. Every qualifying selector writes both
stage indices, including a negative unmatched index. Skipped selectors remain
unchanged. All-no-match returns the existing record without changing it.

Borrowed source words are never read during registration. Valid source lifetime
is the responsibility of its owner and eventual consumers. Failure after table
publication is not rolled back. Corrupt extents, invalid pointers, unsupported
effect profiles and null backing allocation are outside this C++ interface.

## Validation

MSVC Win32 Release, both existing CTests and one ignored fixture passed. The
fixture executes captured original B17E10/B44D60/B18790/B5B820/B5B870 code.
External pool allocation and string resize use their existing reconstructed
services; comparison/memcpy use the host CRT. No original pool/CRT equivalence
is inferred from those dependency substitutions.

Four full88h comparisons pass for new registration, existing update, self-name
update and all-no-match. They also compare name-comparison call counts, verify
the first negative duplicate behavior, preserve skipped selector indices and
padding, and demonstrate the same live source pointer. Both original and host
wrappers pass the null-effect/null-name bypass.

The fixture uses the actual native string pool with its canonical publication,
shutdown gate and lifetime binding, plus actual material/parameter pools. It
finishes through material final-zero dispatch, real name/slot returns, empty
slab trimming and string-pool shutdown. Effect/pass/shader metadata are controlled
borrowed fixture inputs; their table pointer is relocated for original execution
and restored for the host profile check. The material's temporary metadata
pointer is cleared before retirement, restoring its constructor's null effect.
No fake retained effect owner or successful renderer/shader stub is installed.

## Remaining integration

The raw registration operation now exists, but the GUI callback installer still
takes a semantic MaterialCloneState. Connecting it requires canonical effect
metadata, material publication and actual widget/page/ancestor source lifetime.
This packet does not install that incomplete bridge. Real effect ownership,
shader compilation/draw, source consumers, native binary ABI and gameplay remain
unvalidated. No permanent tests are added.
