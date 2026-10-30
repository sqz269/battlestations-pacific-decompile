# Locale record storage and numbered names

This packet covers only the genuinely reached AA0130..AA0256 completed-record
block, before the next stream query. It does not implement or replace AA0020.
The real scanner branch and input/buffer provenance remain caller requirements.

The caller retains a nonmoving frame with its key scratch and nested map attempt
through failure. Key resize/copy occurs before key cleanup is armed. Map and wide
storage use real accepted providers and return a borrowed map-owned value header;
the record caller must not return the node or destroy that value header. Normal
key return uses the early data/length captures, after consuming the key action.
Selected cleanup uses the current key header and must not repeat callee cleanup.

The accepted contract is 0a342da6ced3ced240f99815493e7b9ddfc170541e52e3de12b2960badb2d219.
The exact source revision is df498923926442f74acf85b740f18d1cd7ce25dc. Primary
and independent full-source review accept it; its strict x86 object check passes.
Root registration bb7d2a7a1 adds the source exactly once to bsp_game. The initial
canonical build passes; the final joined Release Win32 build at edfa5a068 includes
peer Main 87c709f68 and passes both existing checks (12.77 seconds). Sixteen provider
pins remain unchanged. No completed-record body or game was executed.

The separate numbered-name provider b8aa50ce is registered once in bsp_core.
Its exact Source passes primary/independent review and a strict x86 object check.
It borrows the actual mapped format and raw cells; current CRT formatting remains
qualified. Selected cleanup consumes its state before destroying the current
temporary, while normal return uses the captured data/length pair. The original
typed formatter remains unchanged. This provider has no loading caller yet.
Full stream composition, raw16 growth, parser/loading and runtime remain open.
