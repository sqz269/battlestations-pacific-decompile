# Allocation-stats process cell primary review

Accepted worker `de826a57f474f89787ee1ad92db0a3d7731660c7` after Root replay,
complete physical-object review and the normal MSVC Win32 build. All three
existing checks passed. No new tests, Native reads or Ghidra changes were needed.

`GameNativeStringProcess` now owns one permanent `void* volatile` cell and returns
its actual reference through `allocation_stats_0109cefc()`. This private Source
composition grows the class from 52 to 56 bytes. Existing offsets 0/4/8/12/24/44/45/48
are preserved; the new four-byte-aligned cell is at 52. It is initially null and
has no reset or publication logic. This API is not a replacement for the
original global address or its binary ABI.

Root independently recovered all 57 complete objects, including physical
regions, primary/AUX records, every section and relocation. 55 saved full physical
indexes were compared; all code sections in all 57 objects were fully decoded. The actual prior
Root process object matches the fresh preimage across complete non-debug
sections, including EH and symbolic SafeSEH entries. The constructor and factory
alone gain `C7 46 34 00 00 00 00`; the factory allocation changes 52 to 56 and two
control displacements adjust. All other code, EH, non-debug data and relocation
targets are preserved. Debug length records are retained separately as metadata.
The new accessor is exactly `8D 41 34 C3`, with no load, store or relocation.

The worker offline replay passed 3706 pin occurrences, 1728 unique pins, 65 Git
preimages and all 1819 frozen bundle payloads. Root compared 65 complete Source/Git
inputs with its current checkout; none required a later Source qualification.
All 997 prior immutable renderer-context pins remain valid. Every one of the 15
genuine Core providers is byte-identical to Root's prior selected baseline.

After integration, Root's 16 complete process/provider objects match the worker's
complete code, EH, non-debug data and relocation edges: 236 function bodies and
no raw RTTI differences. The normal archive retains all 170 previously selected
Core objects byte-for-byte and all 311 positive definitions uniquely. Of 10 prior
selected App objects, six are wholly identical and four differ only in COFF
timestamp bytes 4..7; all their remaining bytes are identical. The process App
object is separately selected as the eleventh, rather than being credited as a
member of the earlier five-object application set.

The new immutable freeze is
`local/cc12_stats_cell_Source_primary/Source993_frozen.json`: 993 selected input
paths and 997 Source/artifact pins. The count remains 993 because both changed
files were already selected. This current cell freeze is distinct from the
earlier renderer-context freeze with the same input count. Complete copies of
all 11 selected App objects are separately pinned by the review receipt.

No stats owner, constructor context, deletion binding, registration, cleanup,
startup or gameplay path is activated. The dependent retained host-binding
packet can now use the accepted permanent cell, with its own layout/provider
review and normal build required.

The [primary receipt](../reports/cc12_allocation_stats_process_cell_source_primary_review.json)
retains the complete comparisons and immutable evidence paths.
