# Twenty-second reconstruction batch

Eight complete ordinary entries, one lexical catch and two logical unwind actions
cover 596 distinct owned Native bytes and 232 instruction starts in this batch.
Supporting callers, handlers, raw EH maps and compiled C++ have separate counts.
This is not a global coverage increment over older typed or AV projections.

The ownership-transfer core constructs each output from a private empty header,
then swaps the three ownership fields in native order. Output advances before
the final source-capacity store. Completed-prefix cleanup remains active through
normal private-header free, with private cleanup armed until that free returns.
The dispatcher returns the actual core result. No deep-copy substitute is used.

Range destruction frees each captured inner backing before ordered clears.
Conflict tidy reloads the current outer backing after the range, then frees it
and clears the header. Native's saved private slot contains known header bits;
the inherited uninitialized-word description is corrected with provenance kept.

Preset-key subtree destruction captures the key pointer before the left child,
then conditionally reads current length and resolves the actual raw pool. Node
free precedes the captured-left nil check. Its clear caller retains three
separate head reloads around the root, count, left and right reset stores.

Primary review passed 49 checks. All 27 report and 27 documentation transfer
claims passed, with no failed or unchecked claims. Exact Source at 3184eefb210fd8f1b542a933e551b3d14a4e0c26
passed the default MSVC Win32 build and all three existing suites in 12.89s.
The temporary build overlay was removed. Actual compiler objects were inspected
separately from preliminary probes, including their differing private frames.
Eleven annotations were saved with preimages, then snapshot and exports refreshed.

New Source interfaces, raw contexts, current C++/CRT and qualified dead metadata
do not prove original ABI, FH3/SEH/RTTI, fault, heap or gameplay compatibility.
Existing differential tests exercise older math seeds, not these new routines.
No new tests or runtime/game claims were added. Prior typed/AV interfaces remain.
