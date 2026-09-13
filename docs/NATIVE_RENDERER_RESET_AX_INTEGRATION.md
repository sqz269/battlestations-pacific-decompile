# AX renderer queue, presentation mode and Reset integration

Addresses: 00b1f280, 00b29e60, 00b2abd0, 00bec230

Four complete normal bodies across three modules use the existing actual
storage and service domains. Reviewed source `91a4d59c82f680586c3cf6ab0fd76881d6c2432f`
was merged with current main; exact combined source `16dcc9e774a8d33eab4cb9565155d4c151bbd242` passed
the strict Win32 build, both existing tests and three current-library-only
original-byte probes. Queue, mode and Reset have 5,9 and14 comparisons.

Four reviewed names/original signatures and complete stored bodies were saved
and exports refreshed. CBCC0E, CBD348 and CBD408 are defined analysis-only EH
handlers. All 39 numeric report rows pass; current indirect targets
require separate assembly, producer and fixture evidence.

Startup sets the pending flag to1 at B2B1CC before calling Reset at B2B1D2;
BeginFrame calls it at B2B229. Reset success requires exactly HRESULT0.
The focus fixture calls the actual API but supplies controlled results; it
preserves user foreground and does not prove actual foreground integration.

See `reports/native_renderer_reset_ax_validation.json` for source/library
hashes, preserved fixture captures, case coverage and limitations. Whole
startup/render submission, queue lifecycle, gameplay, arbitrary worker
concurrency and binary ABI compatibility remain open.
