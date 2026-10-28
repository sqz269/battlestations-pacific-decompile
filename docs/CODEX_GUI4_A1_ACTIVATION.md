# A1 startup scene-domain activation candidate

Candidate base: accepted OFF source `5ca4639f0b` (code `381792703`). The only
source change turns `kNativeRendererStartupCc15Bound` ON in
`include/bsp/game_native_renderer_application.hpp`. The domain-only fallback
and accepted shutdown correction remain the implementation being exercised.

The accepted OFF executable is preserved from
`cc16-progress/local/codex_cc16_candidate/bsp_game.exe`, SHA-256
`1A30FA1D6C67B51BEF537C69AF3179DC8454A0C4DDF608229F97DEB1D0C80BDB`.
Its 13-row receipts are preserved before building the ON candidate. New receipts
live under `cc9/local/codex_a1_activation/`; prior coordinator receipts are not
overwritten.

Acceptance is pending. Required gates: Win32 Release build and existing suites;
PC24 lint; short reference USN04 and milestone-D USN04 through 1600 mission
frames with exit 0; the accepted 13-row plans/frame counts/symmetric bounds aids;
gameplay pair diff, strict ordered dialog/message/Lua comparison, startup frame
36 and first RNG draw, and teardown with only expected A1 records separated.
The remaining gate is a Normal-priority serial OFF/ON/OFF/ON/OFF/ON/OFF/ON USN21
series in a coordinator-approved light-load window. All four OFF results must
agree and ON must match. A no-bounds-aid OFF/ON check is also required.

No publication or accepted ON claim is made by this candidate. GUI Text type
ids remain provisional; activation does not establish GUI page rendering,
startup shader preload, reload or late-plane drawing.
