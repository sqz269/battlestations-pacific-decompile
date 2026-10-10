# Game-state queue: saved evidence annotations

Root provisionally named `004D20D0` as `BSP_Game_GrowStateRequestMap` and appended the accepted queue growth/enqueue evidence. Existing enqueue comments and its name remain preserved. The growth comment records conditional promotion, current-value reloads, free/capacity/map ordering, the verified missing instruction and `RET 4`; enqueue records late pointer/value reads and the current count update. These descriptive names are hypotheses, not recovered symbols.

Typed metadata establishes the current defect precisely: `004D2209` has `CALL_RETURN`, no separate fallthrough override, and `_free` plus its direct thunk target are marked returning. The stored function body contains 338 bytes in two ranges, excluding the verified three-byte `ADD ESP,4` at `004D220E`. This identifies current metadata; it does not establish how the defect arose. No flow, body, listing or prototype repair occurred.

`ghidra_annotate.py` verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, serialized writes under the Ghidra lock, recorded prior values and saved the project. Root refreshed both exports through `bsp.py` and checked each saved comment retains its complete prior prefix. No C++ change, build, test or runtime proof is included. Evidence: `reports/cc12_game_state_queue_growth_annotation.json`.
