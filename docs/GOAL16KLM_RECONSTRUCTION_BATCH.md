# GOAL16K/L/M reconstruction batch

Thirteen complete Native entries cover **220 bytes / 94 instructions**: nine newly authored functions (34 bytes) and four bindings to existing callable Source kernels (186 bytes). All thirteen entries match their entire compiled bodies. The four reuse bindings add zero Source code.

Validated Source: `b1ccf7864b0d5c4016497176c09e918b8a8eb2ca`. The final normal MSVC Win32 build passes all three existing checks. The primary proof covers nine whole compiled modules (1431 executable bytes), all actual archive members and both linker dictionaries, including 28 unique public bindings. K and L have independent READY reviews; the primary independently accepted M from its full Source, ABI, Native, compiled-object and current-provider evidence. [The batch report](../reports/goal16klm_reconstruction_batch.json) records pins, hashes, exact counts, timing and limits.

K's six integer leaves retain full EAX zero/XOR flags, one-read raw ECX field accesses, and partial AL=1 with upper EAX and flags preserved. These physical artifact properties are tied to the tested Win32 build. L's three getters execute literal FLD m32real/RET in the inherited x87 environment, retaining its exception/stack/control behavior; no runtime FP cases were added.

M maps four new Native addresses to existing complete integer POD8 kernels. Copy/fill_n use two registers plus four stack words and RET10h; fill/backward use three cdecl stack words and plain RET. Ordered word reads/stores, mutable saved/argument cells, null skips where present, wrap, flags, faults and nontermination remain explicit. The two existing modules total 273 bytes; their two old wrappers account for the other 87 bytes and receive no new Native credit.

Thirteen physical prototype projections and evidence comments were saved under the Ghidra write lock. Existing CloudPuff and library names remain; the generic TRIV BC3010 name becomes a provisional LoadF32+2C behavior name. Old values and all author evidence are preserved. Exports, snapshot and index were refreshed.

Readiness, compiler/build, transfer/corrections, file preservation, review, scheduling, metadata and handoff intervals are recorded separately. L moved from a cold preparation workspace to an existing warm workspace. No measured speedup is claimed from overlapping packets. The following 201-byte zlib packet and further callback/adapter discovery remain separate work.

Original formal types, full ABI/SEH, absolute code/fault addresses and L saved FIP, caller adoption and game behavior remain unproved. This batch does not complete the reconstruction goal.
