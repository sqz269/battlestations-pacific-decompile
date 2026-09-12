# Native startup VFS callback installation

Addresses: `00530620`, `00735B30`; application fragment
`0073D63C..0073D65B` within `0073D410`.

Both callback bodies are exactly one `RET` byte followed by INT3 padding. Startup
loads the current manager publication, writes 530620 to +90h, reloads the
publication and writes 735B30 to +8Ch. The source installation helper preserves
those two reads and stores. It covers only that 32-byte fragment; the enclosing
first-time gate, allocation and manager construction remain the caller's work.

`NativeVfsStartupCallbacks` supplies the verified concrete 530620 target for
BE18B8's captured-entry failure dispatch. The native callback ignores the live
registers and returns. Other targets produce an explicit source-domain error;
they do not acquire a default callback. This binding completes the known
application callback contract used by the previous mount-registration packet.

The missing 735B30 function was defined from its exact byte under the Ghidra
write lock. No new semantic behavior was inferred from its descriptive name.
See `reports/native_vfs_startup_callbacks.json` and the definition history in
`reports/native_ax_function_definitions.json`. Combined build/fixture validation
is pending; this fragment does not establish whole application startup.
