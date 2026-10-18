# cc13-seg61L: large segment 61 bodies (triage only)

Packet cc13-seg61L (2026-10-18). Rule 1 found every listed body already reconstructed by cc13-large61
(`src/cc13_large61.cpp`, docs/CC13_LARGE61.md), each with a `reconstructed_build_tested` ledger record. No
second copy was written, and none of them lies in cc9's 0092E5B0..0093BD80 set.

| Address | Existing reconstruction |
|---|---|
| 0093D2D0 | `bsp::cc13_large61::map_scene_setup_ocean_materials_0093d2d0` |
| 0093FF10 | `bsp::cc13_large61::sound_overlay_draw_0093ff10` |
| 00945450 | `bsp::cc13_large61::lua_spawn_light_00945450` |
| 00947BC0 | `bsp::cc13_large61::spawn_message_construct_00947bc0` |
| 0094B600 | `bsp::cc13_large61::spawn_manager_handle_message_0094b600` |
| 0094BFF0 | `bsp::cc13_large61::powerup_spawn_request_0094bff0` |

Record fix: the 004C74C0 record (cc13-sweepA1) said a NaN distance could pick an edge. The listing has JBE after every
COMISS / FCOMI edge test (004C74ED/004C7526, 004C752A/004C752C, 004C7548/004C754B, 004C756A/004C756D), and JBE is
taken on unordered input, so a NaN picks no edge. The record now says there is no difference from the cc9 host.
docs/CC13_SWEEPA1.md did not repeat the claim.
