# Native scene-property record element count (CC12)

The Source implements the complete read-only integer leaf at `008EF7F0` as literal MSVC Win32 naked fastcall bytes. Worker Source credit remains **0**, pending primary registration, fresh complete machine/ABI qualification and integration. This Source-only packet performs no compiler, provider, Native/Ghidra query, fixture or runtime operation.

`bsp::native_scene_property_record_element_count_008ef7f0(const void* actual_record_ecx, uint32_t unused_edx)` places both formals in registers. ECX is the actual readable record root; incoming EDX is unused. There are no stack arguments, external calls or invented globals/providers. Every exit is plain RET.

Whole Native interval `[008EF7F0,008EF81A)` is **42 bytes / 17 instructions / zero CALLs**. SHA256: `e84a191988638f684ec2d81e68f96fae767f4035f8d21a2ae736501afbeb8ce7`. The new Source's 42 `_emit` bytes equal this sealed sequence:

```text
8b410483e809741b83e801741683e801740333c0c3b8abaaaaaaf761248bc2c1e803c38b4124c1e802c3
```

This is Source text equality; emitted COFF, linked bytes, relocation/uniqueness and actual compiler callers have not been inspected. The complete historical Native proof is preserved as data, rather than queried or executed again.

| Tag | Unsigned EAX result | EDX after the body |
|---|---|---|
| 9 or10 | DWORD+24 shifted right2 | Incoming EDX unchanged |
| 11 | High32 of unsigned `AAAAAAAB * DWORD+24`, shifted right3; exact division12 | Retains the MUL high DWORD |
| Any other tag, including8 | 0 | Incoming EDX unchanged |

The body always reads the tag DWORD at +0x04. Tags9/10/11 also read byte size at +0x24. These accessed fields must belong to genuine live readable storage and remain stable during the call. There are no writes, DATA+20 or child reads, heap operations, dispatches or phase/marker/owner/ordinal reads. No complete owning class or vector model is required or admitted by the leaf.

ECX and EBX/ESI/EDI/EBP remain unchanged. RET at8EF804,8EF812 or8EF819 consumes only the return address, leaving any queued caller arguments intact. DF and ES are untouched. The body contains no x87/SIMD operation or CLD.

Final default XOR has CF0/PF1/ZF1/SF0/OF0 (`8C5=44`), with AF undefined. SHR2/3 define only CF/PF/ZF/SF (`C5`): CF is byte-size bit1 for tags9/10, or MUL-high bit2 for tag11; PF is low-result-byte even parity, ZF tests the result, SF=0. **AF and OF are undefined on the shift exits**, so neither `8D5` nor a fixed OF/AF value is promised. Source/header comments describe physical body behavior; generated ordinary/raw callers still need independent inspection.

All unsigned DWORD sizes are interpreted literally. Tags9/10 floor division4, maximum3FFFFFFF; tag11 floors division12, maximum15555555. Zero and nonmultiple sizes are accepted arithmetically. For example, size80000000 yields20000000 for tag9/10; FFFFFFFF yields3FFFFFFF or15555555. These are arithmetic expectations, not runtime observations. No signed-negative, positivity, divisibility, saturation or overflow guard was added.

The existing semantic `scene_property_array_elements(type,byte_size)` interface and independently qualified raw constructors remain separate. Type9 constructor008EF360's positive count/4N readable span/copy-provider/lifetime preconditions do not narrow this getter's arithmetic. Faithful Type9 clone byte-size inversion requires B=4N>0 divisible4 and the independently established storage/copy domain. B0..3 returns count0; nonmultiples truncate. Arbitrary overflowing constructor counts lose their high bits through DWORD scaling. The getter ignores marker+2C, so it cannot establish child/root ownership, release permission or class/clone publication.

Readiness commit `08c78f9e11a6da10936aa9bfd04cfeffe94d1779` supplied the whole Native, physical ABI and four callers/20 exact CALL operands. Its exact64-file family is copied unchanged under `local/cnt42s/frozen_readiness`; all historical paths, hashes and nested receipts remain preserved. Current Header13 CPP/HPP are frozen only as Source style references. Two new Source files and those two style files have strict before/after hashes. No old helper, recipe, compiler or accepted process was invoked.

The primary owns Source registration, full machine/helper/caller gates, production qualification, shared metadata, CMake, build/tests and publication. Whole clone/private allocator/EH/class/game admission does not follow from this literal integer leaf. No existing independent Source qualification is changed here.

`local/cnt42s/receipt.json` and `artifact_manifest.json` use an exact recursive all-files inventory. Only those two exact root filenames are excluded; nested historical receipts/manifests, Source copies, utilities, logs, stops and commit records remain inventoried. Source credit0 persists until primary review and qualification.

## Root production integration

Root independently reviewed the complete Source CPP/HPP and native listing.
The routine is registered in `bsp_core`; the fresh Win32 build and all three
existing checks passed with 4,051 Source/header/CMake inputs unchanged.
The actual I386 production COFF body is 42 bytes; 42 literal
bytes match Native, with only the expected symbolic CALL operands excluded.
Source admission remains **0**. Complete linked helpers, actual callers,
provider/frame captures and the fresh single-process fixture still require
primary qualification. Build/object evidence does not establish game ABI,
class/lifetime behavior, startup or gameplay.

The provisional raw-domain Ghidra name and evidence are saved in the configured
project, with previous names/comments recorded and affected exports refreshed.
This publication leaves Source admission at 0 and fixture/ABI qualification pending.
