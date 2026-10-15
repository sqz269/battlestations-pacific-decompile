# POD8 checked insertion and append

The complete 137-byte `00421A50..00421AD8` checked entry and 131-byte `004223B0..00422432` append entry now use the genuine POD8 insertion, fill and current SDK validation providers. Both Source bodies retain the raw x86 instruction schedule. Their names describe recovered behavior; Original template symbols are unproven.

Checked insertion takes ECX as the raw 16-byte vector and four stack words: output iterator, input iterator owner, position and value. It returns the actual output pointer in EAX with RET10h. Append takes ECX plus one stacked value pointer and RET4; a semantic return is unclaimed. Source declarations reserve an unused EDX binding placeholder, which is not an Original input.

The checked entry captures owner and first early, preserves the empty-vector bypass and raw SAR3 index, and continues through all four returning invalid-policy branches. It reloads physical argument cells around insertion, overwrites its actual incoming position slot, and stores result position before result owner. Append preserves the captured first/last, unsigned capacity decision, actual C7E0 stack metadata, byte-only initialization of its local tag, and captured-last publication. Growth reloads the physical value argument after validation and supplies a real stack iterator result to the checked entry.

The actual current SDK validation service can return, throw or terminate under its real policy. No callback, invented success, extra iterator admission rule or std::vector replaces it. The complete F80 implementation supplies all growth and in-place branches. Every reached raw address and stack/control cell needs valid backing.

| Physical CALL site | Callee |
| --- | --- |
| `00421a76` | `00bf6713` |
| `00421a83` | `00bf6713` |
| `00421aa0` | `00420f80` |
| `00421aad` | `00bf6713` |
| `00421ac3` | `00bf6713` |
| `004223f9` | `0041c7e0` |
| `00422413` | `00bf6713` |
| `00422426` | `00421a50` |

Strict optimized MSVC Win32 /MD /W4 /WX compilation passed. COFF, target-verified live bytes and the original disk image agree on all 236 fixed bytes. Each of the eight excluded four-byte CALL operands is separately checked for I386_REL32 relocation kind, actual Source callee symbol and Original target. There are no other exclusions. An independent read-only schedule review found no discrepancy. Registered build/archive composition is recorded in the batch report after integration.

Current CRT, C++ frame, Native EH/virtual ABI and gameplay equivalence remain unclaimed. The local tag retains its actual Source preimage; its upper bytes are not equated to an Original frame. The byte proof covers these two raw bodies and their bindings, not the implementation identity of current provider services.
