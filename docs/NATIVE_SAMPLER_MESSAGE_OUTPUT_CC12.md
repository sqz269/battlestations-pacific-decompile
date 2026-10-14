# Sampler message output storage

The original `00BECCD0..00BECD36` allocates an unwritten 1Ch local `MSG`
and passes its address directly to `PeekMessageA`. It consumes that output
through XLive pretranslation, translation and dispatch only after a successful
retrieval. A zero result exits to cursor/focus processing without reading the
message. No caller MSG is copied or required by that body.

The Source sampler-entry adapter previously added an unconditional copy from
a caller message before entering this pump. That made a prior message a
production prerequisite, although the original requires only output storage.
`NativePlatformLoadMessagePreimage` now makes the capture optional. Its empty
metadata pointer leaves the operation's MSG unwritten; it supplies no zero,
message value, or replacement transport. Real `PeekMessageA` remains the first
producer and the successful-result guard remains unchanged.

An explicit caller-qualified MSG can still be copied for observations that
need its byte preimage. The converting constructor preserves existing Source
aggregate callers that provide a `const MSG&`. This is a Source interface,
not the original private stack or ABI.

The platform, XLive, input and online domains, persistent failure ownership,
current cursor/focus behavior and sampler root/mode stack-word guard remain
unchanged. This correction establishes no effect acquisition, native Text,
world rendering or gameplay result. Byte evidence and build validation are
recorded in `reports/native_sampler_message_output_cc12.json`.
