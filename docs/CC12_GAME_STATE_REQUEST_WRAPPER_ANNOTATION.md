# Game-state request wrapper: saved annotation

Root applied the accepted `004D7920` wrapper evidence to the existing Ghidra function. `BSP_Game_RequestState` and its earlier comments remain preserved. The appended evidence records the borrowed caller-slot pointer, incoming game receiver, direct `RET 4`, fresh xref/caller qualification and deferred enqueue effects.

`tools/ghidra_annotate.py` verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, took the Ghidra write lock, recorded old values and saved the program. Root refreshed the affected export through the typed CLI and checked the saved comment still begins with the prior complete comment. The report retains the original annotation, applied ledger evidence and complete export pins. No prototype, listing, flow or body was repaired; there was no C++ change or runtime validation.

Evidence: `reports/cc12_game_state_request_wrapper_annotation.json`.
