# Native legacy string integration

Six complete logical string routines now operate on the actual 28-byte legacy
small-buffer storage. Counted and substring assignment, erase, destruction,
allocation and growth are registered in the main Win32 build. Three additional
records describe the first catch, shared commit continuation and second catch
inside the growth operation; they are not three more standalone implementations.

Primary review compared the complete installed assembly with the source,
rechecked 34 code/data spans, and preserved the worker's operation source
unchanged. It removed the header's convenience member initialization, which
was not a recovered constructor and could overwrite the preserved prefix when
this string is embedded in a native exception owner. The type is now trivially
default constructible, and the fixture explicitly establishes its empty state.

The strict MSVC Win32 build and existing CTest checks pass (2/2). The primary
rerun against the integrated library matches 289 observation words from the
original code and original growth FuncInfo, using declared host CRT and EH
services. This covers ordinary assignments and erasure, returning validation
handlers, allocation retry, second-allocation reset/rethrow, and the unusual
copy-handler continuation.

A successful first allocation overwrites the native retry slot with its pointer.
If its copy handler throws, the next allocation uses those pointer bits. After
fallback allocation, the original catch dispatcher returns to a continuation
outside the nested catch: a later copy-handler exception leaves the old string
unchanged despite the stored state still reading 2. The reconstruction follows
that observed behavior and does not add replacement cleanup absent from the game.

Prior Ghidra comments and names are preserved. Returning-free flow was repaired
in ordinary destruction, the growth commit and the second catch. Nine entry
annotations/exports cover six logical functions plus their three supporting
pieces. Growth's entry function still has its original split analysis range;
the ledger explicitly records its wider logical span and supporting pieces.

The C++ exception objects, CRT policy and exception-dispatch service remain
explicit host boundaries. This is not native exception-object ABI compatibility
or game validation. The five 40-byte exception-owner routines, list count/error
construction and range insertion/record assignment remain separate work.
Evidence is in `reports/native_legacy_sbo_integration_audit.json`.
