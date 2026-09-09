# Candidate partition: disjoint link-order segments

23937 unnamed non-thunk FUN_ candidates grouped into 85 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 847 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004138d0 | 191 | 0.09 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413d10-0041dd40 | 156 | 0.55 | 1 | 1 | 82 | 0 | 0 | avoidzoneg, avoidzone | 10:11, 0:9 |
| 2 | 0041ddf0-0042a830 | 201 | 0.23 | 1 | 1 | 82 | 0 | 0 | mpakscenes, endgroup, crash, training, terraingridlayer, terraingrid, modes | 1:100, 83:29, 0:20, 80:9 |
| 3 | 0042a920-0044c5d0 | 506 | 0.28 | 1 | 1 | 82 | 0 | 18 | bsp_chk_save, panel, deviceclass, daytime, callback, unlockto, unlockname | 80:207, 1:118, 83:60, 2:44, 79:27, 28:25; +8 more in JSON |
| 4 | 0044c620-00450450 | 64 | 0.67 | 1 | 1 | 82 | 0 | 0 | dialogdefaultpausetime, dialogcharacters, dialogues, dialogglobals, picture, sequence, message | 3:44, 80:27, 1:18, 83:9 |
| 5 | 00450500-0047c2a0 | 612 | 0.35 | 1 | 1 | 82 | 0 | 3 | alpha, action, cameraposition, activatetime, aa_flak, point, periscope | 1:169, 83:103, 3:76, 80:68, 52:44, 2:43; +7 more in JSON |
| 6 | 0047c4d0-004ba080 | 884 | 0.22 | 1 | 1 | 82 | 0 | 22 | soldiertypes, landvehicleclasses, weight, wreckclass, tempid, startpt, smokeefx | 80:127, 1:104, 3:65, 83:61, 5:29, 58:29; +7 more in JSON |
| 7 | 004ba0a0-004c3080 | 171 | 0.3 | 1 | 1 | 82 | 0 | 0 | collect, collectgarbage, traininggrounds, cloudsmall, multi, cloud | 6:26, 28:12 |
| 8 | 004c3180-004d30f0 | 277 | 0.26 | 1 | 1 | 82 | 0 | 1 | white, allbutingame, interface, textures, writestats, userleft, stats | 7:109, 1:69, 83:52, 6:32, 2:25, 5:19; +5 more in JSON |
| 9 | 004d32a0-004e7ef0 | 208 | 0.36 | 1 | 1 | 82 | 1 | 6 | ggame, collect, collectgarbage, ingame, scene, skiptitle, skiplogos | 8:143, 7:80, 2:45, 1:43, 6:39, 83:35; +20 more in JSON |
| 10 | 004e7f90-004f9d30 | 200 | 0.29 | 1 | 1 | 82 | 0 | 5 | ambient, sound, party, noisetexture, landconvoy, stationary, filename | 1:56, 9:36, 2:34, 57:29, 5:22, 80:22; +10 more in JSON |
| 11 | 004f9d80-0051e650 | 336 | 0.37 | 1 | 1 | 82 | 0 | 1 | menuitem_text, vehicleclass, back, globals, dview, navigate, scroll_menu | 1:160, 80:153, 75:106, 83:94, 2:63, 10:44; +12 more in JSON |
| 12 | 0051e6e0-0052dfb0 | 193 | 0.41 | 1 | 1 | 82 | 0 | 1 | attackmove, cycle, target, text_b_text, text_a_text, submarine_group, stearring | 1:52, 75:46, 28:36, 83:25, 38:25, 2:23; +8 more in JSON |
| 13 | 0052e020-005439b0 | 227 | 0.35 | 1 | 1 | 82 | 0 | 0 | mshd, vidm, starty, rowspace, pg_6s_text, pg_6p_text, pg_6_text | 75:55, 1:51, 7:25, 80:24, 83:23, 76:19; +10 more in JSON |
| 14 | 00543a30-00552510 | 172 | 0.35 | 1 | 1 | 82 | 0 | 2 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 28:31, 1:30, 75:20, 2:19, 13:17, 9:16; +8 more in JSON |
| 15 | 00552530-00563440 | 263 | 0.41 | 1 | 1 | 82 | 0 | 1 | fe_pc, preset, presets, opt_normal, opt_inverted, opt_cancel, arrow_right_icon | 75:68, 1:62, 28:42, 83:37, 2:24, 3:21; +4 more in JSON |
| 16 | 00563540-0058d3f0 | 566 | 0.54 | 1 | 1 | 82 | 0 | 3 | globals, back, navigate, select, server_text, players_text, mode_text | 75:128, 1:104, 2:58, 83:53, 76:42, 28:38; +12 more in JSON |
| 17 | 0058d430-005cff40 | 665 | 0.26 | 1 | 1 | 82 | 0 | 2 | globals, mission_mappoint_, bushgroup, visibility, terrain, message, group | 1:129, 16:115, 75:94, 83:63, 80:63, 2:55; +18 more in JSON |
| 18 | 005cff60-005df510 | 112 | 0.74 | 1 | 1 | 82 | 0 | 0 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 1:53, 75:51, 2:33, 7:28, 83:27, 38:24; +10 more in JSON |
| 19 | 005e09a0-005f5af0 | 140 | 0.57 | 1 | 1 | 82 | 0 | 4 | globals, back, select, navigate, arrow_right_icon, arrow_left_icon, change | 75:138, 1:101, 18:74, 2:61, 83:53, 28:46; +11 more in JSON |
| 20 | 005f5b30-00604c80 | 126 | 0.61 | 1 | 1 | 82 | 0 | 1 | xsm_dlcchanged, xsm_saveconfirm, fe_xbox, ingame, felkialtojel_text, aaaaaa, paused | 1:57, 75:49, 19:43, 2:40, 83:29, 8:29; +10 more in JSON |
| 21 | 00604cf0-00612500 | 119 | 0.39 | 1 | 1 | 82 | 0 | 2 | usn_point_text, usn_icon, player_point_text, usn_text, turbo_group, turbo_effect, ship_speed_num3_icon | 1:40, 75:40, 3:38, 42:36, 28:23, 7:22; +6 more in JSON |
| 22 | 00612550-0061cf80 | 110 | 0.38 | 1 | 1 | 82 | 0 | 1 | radar_sweep, pumpermanent, circle_full_, section, number_text, circle_small_02_group, circle_small_01_group | 75:25, 1:22, 3:12, 83:12, 76:10, 26:9 |
| 23 | 0061d010-00639f40 | 176 | 0.48 | 1 | 1 | 82 | 0 | 1 | globals, rank_icon, continue, scoring_unlock_text, playerreview, move_group, debriefing_clipbox | 1:50, 80:28, 75:23, 16:23, 2:23, 83:22; +9 more in JSON |
| 24 | 0063a280-00651370 | 216 | 0.19 | 1 | 1 | 82 | 0 | 5 | ingame, type_icon, commandbuilding_icon, close_group, unit_name_text, vehicleclass, globals | 1:71, 75:49, 23:38, 28:38, 2:36, 83:25; +14 more in JSON |
| 25 | 006515b0-006596e0 | 111 | 0.4 | 1 | 1 | 82 | 0 | 0 |  | 28:14, 38:9, 2:8 |
| 26 | 00659760-006605c0 | 117 | 0.56 | 1 | 1 | 82 | 0 | 0 | ingamegui, normal, icon_l_icon, circle_hl_icon, circle_small_02_group, circle_small_01_group, without_pload_group | 25:34, 1:32, 73:24, 83:16, 75:14, 2:12; +1 more in JSON |
| 27 | 00660b10-00681e60 | 213 | 0.69 | 1 | 1 | 82 | 0 | 2 | circle_hl_icon, sm_cp, globals, ingame, unitclass_spawnpoint, sm_support, icon_l_icon | 26:116, 1:68, 25:60, 75:54, 2:40, 7:35; +15 more in JSON |
| 28 | 00681e90-006ac1f0 | 568 | 0.37 | 1 | 1 | 82 | 0 | 7 | gvmultimenu, ingame, datatables, scripts, sensitivitysettings, pushrequestinterface, inputsettings | 1:103, 15:94, 80:80, 83:60, 3:55, 10:49; +16 more in JSON |
| 29 | 006ac260-006be980 | 196 | 0.41 | 1 | 1 | 82 | 0 | 0 | savedata, scoring, entities, missionid, entidcont, cont1, cont0 | 80:340, 1:85, 2:76, 83:46, 5:35, 3:24; +6 more in JSON |
| 30 | 006beb10-006d64b0 | 303 | 0.5 | 1 | 1 | 82 | 0 | 17 | equipment, state, slots, runwaywidth, runwaylength, runwayfailure, hangarfailure | 28:81, 29:73, 1:73, 38:37, 3:33, 2:27; +9 more in JSON |
| 31 | 006d69e0-006da6b0 | 99 | 0.39 | 2 | 31 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 1:19, 83:11 |
| 32 | 006dadc0-006deca0 | 66 | 0.7 | 3 | 32 | 1 | 0 | 0 | markerclasses, recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 31:49, 80:10, 1:8 |
| 33 | 006dee40-00718350 | 742 | 0.29 | 1 | 1 | 82 | 1 | 32 | orgammo, bulletbase, depthcharge, mvfm, openaftertime, flytime, bulletclass | 80:168, 1:142, 2:95, 3:79, 28:69, 83:67; +22 more in JSON |
| 34 | 00718380-0071d6d0 | 101 | 0.39 | 1 | 1 | 82 | 0 | 3 | sphere, identifier, mzonedesc, mnote, armor, points, category | 33:55, 2:18, 38:15, 84:11, 82:10 |
| 35 | 0071d780-00722fb0 | 86 | 0.53 | 1 | 1 | 82 | 0 | 3 | startmode, followmode, userpath, pathfollowparams, pathcursor, internalclearprimarycommand, emptycommand | 38:27, 2:20, 12:13, 1:12, 34:11, 41:11; +1 more in JSON |
| 36 | 00723030-00728fa0 | 91 | 0.48 | 1 | 1 | 82 | 0 | 9 | mgeommesh | 35:13, 2:8 |
| 37 | 007290d0-00758cf0 | 488 | 0.26 | 1 | 1 | 82 | 0 | 30 | destroyed, speed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed | 1:102, 80:102, 2:84, 38:73, 83:69, 3:42; +20 more in JSON |
| 38 | 00758d30-00782840 | 719 | 0.59 | 1 | 1 | 82 | 0 | 28 | p2p_voice__, reconlevel, multiscore, mmultiplayer, lastbanto, player, recondata | 2:699, 39:72, 1:72, 80:65, 83:48, 33:41; +17 more in JSON |
| 39 | 00782870-0078c9b0 | 138 | 0.36 | 1 | 1 | 82 | 0 | 5 | send_, recv_, time, client, server | 83:29, 2:15, 81:13, 38:10, 1:10 |
| 40 | 0078cf20-007a42c0 | 251 | 0.42 | 1 | 1 | 82 | 0 | 0 | camera, thetalinearblend, theta, rholinearblend, blendtime, postype, initialization | 1:89, 80:57, 3:50, 28:38, 2:37, 83:30; +4 more in JSON |
| 41 | 007a44d0-007b38d0 | 147 | 0.53 | 1 | 1 | 82 | 0 | 11 | pathbaseentity, simple, sustainbefore, rotrefentity, pathpoints, pathintf, pathid | 40:83, 1:64, 3:33, 80:32, 38:21, 79:20; +8 more in JSON |
| 42 | 007b3920-007d1e50 | 330 | 0.59 | 1 | 1 | 82 | 0 | 10 | powerlost, explosion, splash, enginefire, rightspinning, leftspinning, spinning | 1:90, 3:76, 43:70, 38:57, 83:35, 2:33; +11 more in JSON |
| 43 | 007d1f70-007f84e0 | 291 | 0.56 | 1 | 1 | 82 | 1 | 5 | travelspeed, gears, baydoor, wings, state, windsound, timeout | 1:90, 3:58, 42:54, 80:46, 38:38, 2:34; +16 more in JSON |
| 44 | 007f8540-00809820 | 213 | 0.34 | 1 | 1 | 82 | 0 | 2 | enemy, neutral, unlocks, selectedmissionid, selecteddifficulty, seenunlocks, savedlobbyfilters | 80:66, 1:57, 83:35, 3:23, 43:19, 8:16; +5 more in JSON |
| 45 | 00809880-0081aa10 | 213 | 0.51 | 1 | 1 | 82 | 0 | 39 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 38:52, 60:36, 1:31, 3:31, 2:22, 58:21; +9 more in JSON |
| 46 | 0081aa60-008286f0 | 93 | 0.45 | 1 | 1 | 82 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 45:83, 1:31, 2:19, 58:18, 80:18, 48:16; +7 more in JSON |
| 47 | 00828750-00858700 | 495 | 0.3 | 1 | 1 | 82 | 2 | 12 | gameunit, classid, stock, torpedo, torpedoavoidance, object, submarine | 80:92, 1:91, 6:67, 38:47, 3:46, 2:39; +20 more in JSON |
| 48 | 00859240-0086af80 | 241 | 0.46 | 1 | 1 | 82 | 0 | 5 | radius, tvertangle, turninggun, thorzangle, horzrotdir, vertangle, horzangle | 80:42, 3:31, 1:26, 33:12, 49:12, 83:10; +7 more in JSON |
| 49 | 0086afc0-00877e50 | 168 | 0.41 | 1 | 1 | 82 | 0 | 2 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 80:76, 83:60, 1:41, 48:27, 78:11, 38:11; +4 more in JSON |
| 50 | 00877fa0-008828e0 | 117 | 0.44 | 1 | 1 | 82 | 0 | 2 | damage, yellow, weaponsystems, weapondirectorthinktime, warningscrollspeeds, visibletimeout, visibilityrange | 80:41, 49:26, 1:14, 83:12 |
| 51 | 00882ac0-0088b120 | 128 | 0.58 | 1 | 1 | 82 | 0 | 0 | scripts, debugtrap, shallowwater, modelpath, luab, filepath, colormap | 29:51, 80:47, 1:36, 2:25, 83:24, 33:22; +1 more in JSON |
| 52 | 0088b190-008ddf90 | 748 | 0.86 | 1 | 1 | 82 | 528 | 0 | luakod, options, hardwarereported, english, xboxcompatibilitymode, vsync, texturedetail | 80:3876, 1:1168, 2:612, 83:592, 51:436, 38:117; +27 more in JSON |
| 53 | 008ddfe0-008e2b90 | 76 | 0.63 | 1 | 1 | 82 | 0 | 1 | szurkenyil, secobjprefix, missionglobals, flagprocess, quiet, objectivelist, objectiveentities | 52:55, 1:19, 2:16, 38:10, 83:9, 28:9 |
| 54 | 008e2bf0-008e6430 | 69 | 0.35 | 1 | 1 | 82 | 0 | 1 | pinged, defend, attack | 2:33 |
| 55 | 008e64a0-008ec550 | 90 | 0.56 | 1 | 1 | 82 | 0 | 1 | uspumicon, uselimit, unitclassindex, targettype, targetfilter, random, pumicon | 54:39, 1:34, 83:17, 80:17, 3:16, 22:12; +2 more in JSON |
| 56 | 008ec650-008f10a0 | 85 | 0.52 | 1 | 1 | 82 | 0 | 0 | pup_gain, pum1stget, vec3array, pup_lost, powerupclassid, intarray, floatarray | 55:23, 3:15, 54:10, 60:9, 80:8 |
| 57 | 008f10b0-00922b90 | 507 | 0.3 | 1 | 1 | 82 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 1:138, 80:94, 16:89, 3:82, 2:78, 83:67; +18 more in JSON |
| 58 | 00922c80-0092dfe0 | 181 | 0.4 | 1 | 1 | 82 | 0 | 78 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 80:43, 1:23, 2:17, 83:17, 28:12, 29:12; +5 more in JSON |
| 59 | 0092e0b0-00943bb0 | 207 | 0.34 | 1 | 1 | 82 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 1:51, 80:38, 58:31, 2:30, 83:24, 3:17; +3 more in JSON |
| 60 | 00943c00-00973110 | 616 | 0.27 | 1 | 1 | 82 | 0 | 57 | callback, vehicleclass, entity, inferiorfailure, party, oldlevel, newlevel | 80:148, 1:114, 3:80, 2:56, 83:44, 6:40; +19 more in JSON |
| 61 | 00973150-0098c820 | 287 | 0.42 | 1 | 1 | 82 | 0 | 10 | player, exitzone, repair, recon, plane, other, ambient | 60:178, 1:146, 51:93, 83:83, 2:68, 33:37; +13 more in JSON |
| 62 | 0098c870-009965d0 | 153 | 0.51 | 1 | 1 | 82 | 0 | 2 | software, mpkg, cast, bsm_hwd, content, language, eidos | 1:33, 83:15, 0:12, 3:8 |
| 63 | 00996670-009f69c0 | 843 | 0.81 | 1 | 1 | 82 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:375, 42:265, 3:232, 64:167, 0:113, 83:105; +22 more in JSON |
| 64 | 009f6a20-009ffad0 | 117 | 0.64 | 1 | 1 | 82 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:35, 43:15, 3:15, 42:15, 35:14, 80:13; +2 more in JSON |
| 65 | 009ffb40-00a07d40 | 130 | 0.57 | 1 | 1 | 82 | 0 | 0 | vehicle, class, captureweight | 64:34, 1:13 |
| 66 | 00a07e40-00a19480 | 127 | 0.59 | 1 | 1 | 82 | 0 | 5 | commandtype, neutral, aivstable_, enemy, target, vehicleclass, s_to_ | 65:51, 1:49, 69:34, 80:25, 83:18, 3:16; +7 more in JSON |
| 67 | 00a19500-00a1fac0 | 127 | 0.43 | 1 | 1 | 82 | 0 | 0 | coordinator, sell, strategicgain, duel, defend, capture, siege | 1:27, 69:19, 83:11, 2:10 |
| 68 | 00a1fba0-00a284e0 | 108 | 0.64 | 1 | 1 | 82 | 0 | 0 | strategicgain, siege, escort, competitive | 67:51, 66:25, 69:15, 1:13, 65:12 |
| 69 | 00a286a0-00a371a0 | 83 | 0.66 | 1 | 1 | 82 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 66:28, 80:25, 68:23, 1:21, 67:19, 65:16; +5 more in JSON |
| 70 | 00a371c0-00a428d0 | 154 | 0.45 | 1 | 1 | 82 | 0 | 6 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 80:19, 39:15, 1:11, 71:10, 69:9, 83:8 |
| 71 | 00a42cf0-00a625d0 | 179 | 0.46 | 1 | 1 | 82 | 0 | 3 | online, server, player, query, matchmaking, remote, movie | 70:22, 62:21, 1:17, 16:13, 83:10, 28:8; +1 more in JSON |
| 72 | 00a62660-00a7a440 | 335 | 0.63 | 1 | 1 | 82 | 0 | 1 | unexpected, chunk, precompiled, call, complex, expression, many | 1:21, 74:16, 83:13 |
| 73 | 00a7a460-00a82870 | 124 | 0.63 | 1 | 1 | 82 | 0 | 5 | sounjd, memory, sound, volume, play, event, system | 80:35, 1:24, 83:19, 2:9 |
| 74 | 00a82880-00a9a4a0 | 305 | 0.22 | 1 | 1 | 82 | 0 | 6 | memory, sounjd, stream, sound, streaming, stereo, request | 1:59, 83:50, 73:38, 2:30, 78:20, 80:20; +5 more in JSON |
| 75 | 00a9a5a0-00aacd90 | 312 | 0.37 | 1 | 1 | 82 | 0 | 41 | cGuiManager, widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor | 1:38, 80:33, 76:30, 2:22, 83:21, 7:13; +1 more in JSON |
| 76 | 00aace40-00acc0d0 | 381 | 0.37 | 1 | 1 | 82 | 0 | 12 | texture, simplecolor, mvfm, mshd, guidefault, shadername, vertical_scrollbar | 75:129, 1:106, 80:77, 83:58, 2:24, 84:10; +4 more in JSON |
| 77 | 00acc1c0-00ad7b90 | 192 | 0.37 | 1 | 1 | 82 | 0 | 7 | mshd, scalevector, rotationeuler, playbydefault, modeltextureoverride, framesizesy, cGuiSound | 75:38, 1:27, 78:20, 83:16, 76:10, 80:9 |
| 78 | 00ad7fe0-00b20dc0 | 869 | 0.3 | 1 | 1 | 82 | 0 | 18 | param, mvfm, mshd, terrain, emitter, additive, particlefloating | 1:224, 80:143, 83:132, 79:63, 2:55, 84:49; +7 more in JSON |
| 79 | 00b20e70-00b65560 | 736 | 0.26 | 1 | 1 | 82 | 0 | 13 | mshd, cSampleOffsets, mvfm, pf43cc, shadowmap, posteffectsysobj, posteffectsyscam | 1:141, 80:123, 83:82, 78:68, 2:26, 7:12; +2 more in JSON |
| 80 | 00b65660-00b75d80 | 319 | 0.52 | 1 | 1 | 82 | 0 | 31 | dofile, userdata, thread, lightuserdata, dobuffer, cDummy, c3dobject | 1:40, 72:31, 2:19, 79:17, 83:17, 81:9 |
| 81 | 00b75de0-00b86390 | 315 | 0.49 | 1 | 1 | 82 | 0 | 1 | c3dnodeanimator, cLight, cAnimTrack, cAmbientLight, boundingbox, resource, resourcedump_ | 1:51, 83:35, 84:29, 80:24, 33:14, 2:14; +2 more in JSON |
| 82 | 00b86420-00b90280 | 172 | 0.47 | 1 | 1 | 82 | 0 | 3 | cSceneResource, cGroupParamsResource, cGroup, flare, zoomfactor, targetname, node | 80:27, 84:22, 1:21, 81:18, 83:15, 33:8 |
| 83 | 00b90380-00bd4200 | 765 | 0.26 | 1 | 1 | 82 | 0 | 97 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 80:204, 1:195, 84:80, 78:65, 2:49, 81:31; +8 more in JSON |
| 84 | 00bd4270-00c30570 | 609 | 0.34 | 1 | 1 | 82 | 0 | 22 | long, iterator, cFileStore, removefile, removed, profile, nagybetu | 1:173, 83:116, 80:98, 2:45, 0:19, 14:11; +2 more in JSON |
