# Raw wide append and copy-constructor bindings

These Core additions bind the existing native algorithms to actual byte-backed
raw8 headers and the same AA8/AA4/AA0 pool cells. Existing typed algorithms,
providers and host paths remain unchanged. No semantic string or storage adapter
stands in for the raw operations.

Append005CF090 captures source length first; zero skips destination and pool
reads. Otherwise it captures old destination length, performs real raw resize,
then reads CURRENT source data before destination data. Captured lengths control
the DWORD-wrapped byte count and destination offset. It copies no terminator,
adds no null guard and never treats exact self-alias as a no-op. Operand reads
remain even when the byte count wraps to zero before CRT-call omission.

The raw4C8DD0 overload compares identity before clearing length then data,
including self identity. Otherwise it calls real raw resize using captured source
length, gates on CURRENT source length, reads source data before destination data
and copies forward WORDs through NUL. Fenced byte accesses avoid typed raw-storage
lvalues. There is no counted-copy replacement or new constructor cleanup.

Both borrow actual headers/backing/providers and propagate partial effects on
escape. Caller owns cleanup, alias/copy validity, no-writer and provider lifetime
obligations. No rollback, retry, Frame, new pool, default cell or native cleanup
from a C++ destructor is introduced. These normalized interfaces grant no
Original ABI/FH3/SEH/hardware/currentCRT identity, loaded resolver, Lua, concat,
startup activation or gameplay permission. Existing body credit is preserved.

Exact source review, strict object and common build receipts are recorded in the
matching report. Each packet passed primary and two independent complete
Source reviews and one strict x86 object compile(exit0/014C). The joined
retained-link build b315afeed passed2/2 existing checks12.27s and retained
append10300be0/rawcopy10300c30. After overlay removal, canonical Release Win32
passed2/2 checks14.12s; forced cache/project entries and both dormant raw symbols
are absent. Each Core registration occurs once and25 baseline paths are exact.
All9 raw Text flags remain off; no new entry or game was invoked.
