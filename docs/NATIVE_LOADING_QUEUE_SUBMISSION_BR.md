# Native loading work submission (BR)

Addresses: `004FDC00`, `004FE700`, `00501720`, `005048F0`, `00504D20`.

Five complete ordinary bodies (591 bytes) are reconstructed in
`src/native_loading_queue_submission.cpp`. The actual 10h work record owns its
name at +0/+4 and borrows context/result at +8/+C. The source takes explicit raw
string-pool publication bindings; it does not create another pool or invoke
original numeric code. Descriptive names are hypotheses.

| Address | Original ABI | Recovered behavior |
| --- | --- | --- |
| 004FDC00, 29 bytes | ECX record; RET | Capture name data, then length+1; current raw getter and return; leave all words unchanged. |
| 004FE700, 158 bytes | ECX output; three stack words comprising a **by-value** name length/data and context; EAX output; RET0C | Capture parameter data and length before clearing output. Copy from captured data, read current argument context, set result zero, then return captured parameter allocation. |
| 00501720, 78 bytes | ECX output; stack source; EAX output; RET4 | Clear name even on identity, copy current name fields, copy context before reading/copying result. |
| 005048F0, 111 bytes | ECX vector; stack source; RET4 | On count==capacity reserve signed max(wrapping double,1); compute current destination, copy if nonzero, increment current count. |
| 00504D20, 215 bytes | ECX loader; stack name/context; RET8 | Check only initial last job's stop byte; build consumed argument and temporary, reload current array/count/last job, append without another stop check, destroy temporary name. |

The 004FE700 source exposes writable 0Ch argument storage representing the three
native stack words. It is not a native pointer-to-name parameter. Native copies
call the overlap-capable BF7680 body; source uses `std::memmove` and preserves
the observed capture/read order. No empty queue, alias, overflow, allocation
success or result-ownership policy is added.

The three FH3 maps were read with the complete body and unwind listings:

* C688E8 -> D91AE4, map D91ADC: state0 -> -1 via C688E0, which destroys the
  **current parameter header at synthetic parent EBP+4** through 41DD20. Copy
  failure does not destroy the incomplete output. Normal final return disarms
  parameter cleanup before the possibly throwing raw getter.
* C6900C -> D92328, map D92320: state0 -> -1 via C68FF0. The action rereads the
  current vector count/data to compute the placement address before calling
  RET-only 401130 with the original destination and current address. The source
  standard placement delete preserves these reads and does not free storage.
* C69088 -> D92440, map D92438: state0 -> -1 via C69080 destroys the completed
  temporary through 4FDC00. The caller arms this only after selecting the
  current last job; no outer cleanup covers initial inline-name construction.

Strict MSVC Win32 compilation and a focused actual-service fixture passed. It
exercises copied name/context/result, stopped-job skip, vector growth and
identity copy. The same fixture retires the jobs through actual pool, FileStore
and memory-stream source services. Full byte/ownership/call receipts are in
`reports/native_loading_queue_submission_br.json`; combined build evidence is
in `reports/native_loading_queue_integration_br.json`.

These are new C++ interfaces. Source C++ unwind ordering is represented where
stated; original FH3/SEH/CRT identity, native stack/EH-spill aliases, hardware
faults and callee-internal exception boundaries remain unverified. The production
queue update, worker resource-loading path and shutdown are not supplied here.
