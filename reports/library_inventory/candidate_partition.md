# Candidate partition: disjoint link-order segments

23756 unnamed non-thunk FUN_ candidates grouped into 94 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 864 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004138d0 | 190 | 0.08 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413d10-0041db10 | 153 | 0.56 | 2 | 1 | 86 | 0 | 0 | avoidzoneg, avoidzone | 11:11, 0:9 |
| 2 | 0041dd20-0042a830 | 201 | 0.23 | 2 | 1 | 86 | 0 | 0 | mpakscenes, endgroup, crash, training, terraingridlayer, terraingrid, modes | 1:61, 0:18, 83:9 |
| 3 | 0042a920-0043e700 | 263 | 0.43 | 2 | 1 | 86 | 0 | 14 | bsp_chk_save, unlockto, unlockname, unlockfrom, playtime, difficulty, united | 83:147, 2:33, 1:28, 82:27, 29:25, 91:18; +5 more in JSON |
| 4 | 0043e780-004484f0 | 163 | 0.41 | 2 | 1 | 86 | 0 | 3 | deviceclass, daytime, device, platform, weatherreconmodifiers, weatherampmultipliers, viewmodeampmultipliers | 83:52, 53:9 |
| 5 | 004486a0-00453260 | 178 | 0.68 | 2 | 1 | 86 | 0 | 1 | panel, sequence, message, callback, suppressinterruptmsg, setpanel, requesttime | 83:38, 2:15, 3:8 |
| 6 | 00453710-004654d0 | 150 | 0.55 | 2 | 1 | 86 | 0 | 1 | action, cameraposition, activatetime, aa_flak, alpha, point, periscope | 7:41, 2:20, 54:15, 83:12 |
| 7 | 00465610-00490990 | 779 | 0.17 | 2 | 1 | 86 | 0 | 17 | entity, groupname, scenebrowsergroups, s_ls_, cloudclass, browsergroup, weight | 83:51, 56:40, 57:38, 3:32, 2:31, 54:29; +8 more in JSON |
| 8 | 00491070-004ba330 | 539 | 0.21 | 2 | 1 | 86 | 0 | 7 | soldiertypes, landvehicleclasses, wreckclass, tempid, smokeefx, rotationdecline, gravitymul | 7:242, 83:97, 4:18, 44:18, 3:17, 51:16; +6 more in JSON |
| 9 | 004ba3d0-004d6790 | 481 | 0.25 | 2 | 1 | 86 | 0 | 1 | collect, collectgarbage, white, allbutingame, interface, textures, player | 8:66, 2:33, 29:25, 72:21, 7:19, 83:17; +5 more in JSON |
| 10 | 004d6800-004e7db0 | 162 | 0.4 | 2 | 1 | 86 | 1 | 5 | ggame, collect, collectgarbage, ingame, skiptitle, skiplogos, skipbriefings | 9:196, 2:43, 8:23, 72:23, 29:21, 7:17; +16 more in JSON |
| 11 | 004e7dd0-004fa300 | 208 | 0.43 | 2 | 1 | 86 | 0 | 6 | ambient, sound, party, noisetexture, landconvoy, stationary, filename | 56:42, 2:40, 7:38, 10:34, 9:25, 83:22; +4 more in JSON |
| 12 | 004fa3d0-0051e6e0 | 323 | 0.38 | 2 | 1 | 86 | 0 | 1 | menuitem_text, vehicleclass, back, globals, dview, navigate, scroll_menu | 83:141, 73:66, 2:63, 72:50, 11:44, 9:43; +8 more in JSON |
| 13 | 0051e730-00527c30 | 108 | 0.75 | 2 | 1 | 86 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 29:31, 9:21, 72:11, 3:10, 41:10, 11:9; +2 more in JSON |
| 14 | 00527c80-00543a30 | 310 | 0.56 | 2 | 1 | 86 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 73:75, 9:50, 72:31, 2:30, 75:29, 11:22; +8 more in JSON |
| 15 | 00543a60-00552590 | 173 | 0.55 | 2 | 1 | 86 | 0 | 2 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 29:31, 9:27, 73:20, 2:18, 14:17, 10:13; +5 more in JSON |
| 16 | 005525b0-005696e0 | 333 | 0.45 | 2 | 1 | 86 | 0 | 1 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 72:56, 73:40, 9:35, 29:33, 2:31, 83:29; +4 more in JSON |
| 17 | 00569740-0056ba50 | 75 | 0.71 | 2 | 1 | 86 | 0 | 0 |  | 16:27 |
| 18 | 0056bb50-0057ff00 | 235 | 0.4 | 2 | 1 | 86 | 0 | 1 | globals, server_text, players_text, mode_text, scroll_right_icon, wave_icon, setting_2_text | 17:56, 2:34, 16:33, 73:27, 72:23, 29:19; +3 more in JSON |
| 19 | 0057ff80-005c5cd0 | 680 | 0.53 | 2 | 1 | 86 | 0 | 3 | globals, navigate, back, mission_mappoint_, select, main_newprofile, bushgroup | 73:99, 2:85, 72:56, 75:54, 18:47, 29:34; +17 more in JSON |
| 20 | 005c5da0-005cd1a0 | 114 | 0.42 | 2 | 1 | 86 | 0 | 0 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, uniquemultisettings | 83:16, 19:15, 11:12, 9:11, 73:8 |
| 21 | 005cd240-005e0ad0 | 172 | 0.67 | 2 | 1 | 86 | 0 | 1 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 9:67, 2:37, 72:31, 73:31, 41:27, 29:20; +9 more in JSON |
| 22 | 005e0db0-005fb080 | 178 | 0.72 | 2 | 1 | 86 | 0 | 5 | xsm_saveconfirm, fe_xbox, globals, back, xsm_dlcchanged, arrow_left_icon, mainlistbox_text | 2:91, 72:80, 73:76, 21:68, 9:59, 29:58; +11 more in JSON |
| 23 | 005fc620-00604bc0 | 81 | 0.68 | 2 | 1 | 86 | 0 | 0 | ingame, felkialtojel_text, aaaaaa, paused, title_group, silverline_framebox, secondary_objectives_text | 9:44, 72:26, 73:17, 2:15, 29:13, 10:10; +1 more in JSON |
| 24 | 00604c20-0061f4a0 | 269 | 0.39 | 2 | 1 | 86 | 0 | 3 | usn_point_text, usn_icon, radar_sweep, pumpermanent, player_point_text, circle_full_, section | 73:57, 9:44, 3:38, 45:36, 29:30, 11:27; +8 more in JSON |
| 25 | 0061f540-00651760 | 356 | 0.22 | 2 | 1 | 86 | 0 | 6 | globals, ingame, rank_icon, continue, vehicleclass, type_icon, scoring_unlock_text | 73:66, 2:65, 29:51, 9:48, 24:46, 83:44; +20 more in JSON |
| 26 | 006529b0-006596e0 | 109 | 0.39 | 2 | 1 | 86 | 0 | 0 |  | 29:12 |
| 27 | 00659760-0065cea0 | 90 | 0.6 | 2 | 1 | 86 | 0 | 0 |  | 26:33, 29:10 |
| 28 | 0065ced0-00680d30 | 237 | 0.79 | 2 | 1 | 86 | 0 | 2 | circle_hl_icon, ingamegui, normal, icon_l_icon, globals, sm_cp, ingame | 27:99, 26:61, 73:56, 2:55, 9:51, 19:43; +12 more in JSON |
| 29 | 00680db0-00697ec0 | 295 | 0.42 | 2 | 1 | 86 | 0 | 7 | gvmultimenu, ingame, pushrequestinterface, interface, textures, warning_2_text, missionunique | 9:59, 83:57, 11:42, 10:39, 2:34, 25:22; +11 more in JSON |
| 30 | 00697f40-006be980 | 472 | 0.4 | 2 | 1 | 86 | 0 | 0 | savedata, scoring, sensitivitysettings, inputsettings, devicetype, deviceidx, slider | 83:386, 2:94, 16:88, 4:28, 7:21, 40:19; +14 more in JSON |
| 31 | 006beb10-006d4370 | 284 | 0.63 | 2 | 1 | 86 | 0 | 18 | equipment, state, slots, runwayfailure, hangarfailure, planes, classid | 29:81, 30:71, 1:63, 3:29, 41:23, 2:16; +9 more in JSON |
| 32 | 006d4420-006d8ec0 | 100 | 0.46 | 3 | 32 | 1 | 0 | 0 | section_, hangars, exitpathid, exitpath, entrypathid, entrypath, entityid | 31:16, 2:11 |
| 33 | 006d8ef0-006deff0 | 87 | 0.72 | 4 | 33 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers, markerclasses | 32:52, 83:10 |
| 34 | 006df170-006fa6d0 | 326 | 0.41 | 2 | 1 | 86 | 1 | 28 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 83:72, 2:70, 29:43, 1:37, 13:33, 57:33; +18 more in JSON |
| 35 | 006fac20-00714030 | 345 | 0.3 | 2 | 1 | 86 | 0 | 15 | openaftertime, messages, mvfm, whosaysthat, velocity, unitlist, unitid | 83:48, 34:42, 29:26, 2:25, 3:25, 57:24; +10 more in JSON |
| 36 | 00714060-0071b710 | 104 | 0.52 | 0 | 36 | 1 | 0 | 3 | sphere, mzonedesc, mnote, identifier, armor, points, category |  |
| 37 | 0071ba20-00728f20 | 212 | 0.48 | 2 | 1 | 86 | 0 | 15 | startmode, followmode, userpath, pathfollowparams, pathcursor, mgeommesh, internalclearprimarycommand | 2:44, 40:26, 41:21, 1:17, 13:14, 44:12; +2 more in JSON |
| 38 | 00728fa0-00735ec0 | 164 | 0.45 | 2 | 1 | 86 | 0 | 22 | destroyed, barreldelaytime, torpedo, titlelist, throwb, throwa, nextfirebarrel | 2:29, 83:19, 4:16, 40:13, 37:13, 1:12; +4 more in JSON |
| 39 | 00735f30-00758eb0 | 327 | 0.29 | 2 | 1 | 86 | 0 | 16 | landvehicle, landingship, landfort, sumleaks, sumforces, rampaelfordulas, partraszalltunk | 83:70, 2:58, 40:30, 91:27, 57:25, 7:24; +15 more in JSON |
| 40 | 00758f90-0076ad60 | 350 | 0.88 | 2 | 1 | 86 | 0 | 5 | runwaycenter, maxlandingplanesonboard, liftexitpoint, deckcamera, carrierescort, elevator_2, vertangle | 2:649, 31:9, 83:8 |
| 41 | 0076ada0-00787000 | 442 | 0.34 | 2 | 1 | 86 | 0 | 28 | p2p_voice__, reconlevel, multiscore, mmultiplayer, lastbanto, player, recondata | 40:104, 2:59, 83:56, 9:37, 57:27, 35:18; +13 more in JSON |
| 42 | 00787040-0078d850 | 73 | 0.47 | 2 | 1 | 86 | 0 | 0 | send_, recv_, time, client, server | 86:27, 89:25 |
| 43 | 0078d880-007a49a0 | 243 | 0.41 | 2 | 1 | 86 | 0 | 0 | postype, camera, thetalinearblend, theta, rholinearblend, blendtime, initialization | 83:61, 1:61, 3:47, 29:38, 2:37, 20:11; +4 more in JSON |
| 44 | 007a4b60-007b2d80 | 130 | 0.63 | 2 | 1 | 86 | 0 | 8 | simple, pathid, pathbaseentity, paratrooper, soldieranim, slowfactoropened, slowfactorclosed | 43:79, 1:47, 3:28, 83:26, 82:20, 29:18; +6 more in JSON |
| 45 | 007b2dd0-007d1c90 | 340 | 0.71 | 2 | 1 | 86 | 0 | 13 | powerlost, explosion, splash, enginefire, rightspinning, pathpoints, leftspinning | 1:70, 3:70, 46:70, 2:39, 40:33, 57:30; +15 more in JSON |
| 46 | 007d1d30-007f7cf0 | 278 | 0.66 | 2 | 1 | 86 | 1 | 14 | travelspeed, gears, baydoor, wings, state, windsound, timeout | 1:73, 45:54, 3:51, 83:39, 2:32, 57:32; +12 more in JSON |
| 47 | 007f7d40-0080d9b0 | 279 | 0.24 | 2 | 1 | 86 | 0 | 2 | enemy, neutral, unlocks, selectedmissionid, selecteddifficulty, seenunlocks, savedlobbyfilters | 83:60, 3:31, 9:24, 29:18, 2:15, 46:11; +6 more in JSON |
| 48 | 0080da00-0081aa10 | 163 | 0.65 | 2 | 1 | 86 | 0 | 38 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 59:32, 40:27, 41:25, 1:24, 2:22, 37:18; +5 more in JSON |
| 49 | 0081aa60-00828870 | 97 | 0.42 | 2 | 1 | 86 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 48:77, 2:20, 1:18, 57:18, 51:16, 83:16; +5 more in JSON |
| 50 | 008288d0-00859240 | 492 | 0.27 | 2 | 1 | 86 | 2 | 15 | gameunit, classid, stock, torpedo, torpedoavoidance, object, submarine | 83:75, 7:52, 2:41, 37:40, 1:39, 51:38; +22 more in JSON |
| 51 | 00859410-008744a0 | 354 | 0.36 | 2 | 1 | 86 | 0 | 8 | radius, effects, tvertangle, turninggun, thorzangle, minlifetime, maxlifetime | 83:100, 3:30, 91:27, 1:25, 9:24, 89:21; +10 more in JSON |
| 52 | 00874540-00878350 | 63 | 0.52 | 2 | 1 | 86 | 0 | 2 | fragile, damage | 57:10, 41:9 |
| 53 | 00878530-0088b4b0 | 242 | 0.26 | 2 | 1 | 86 | 0 | 2 | damage, scripts, debugtrap, datatables, yellow, weaponsystems, weapondirectorthinktime | 83:79, 30:51, 2:31, 52:30, 34:21, 3:11; +3 more in JSON |
| 54 | 0088b520-008dcd40 | 713 | 0.79 | 2 | 1 | 86 | 528 | 0 | luakod, options, hardwarereported, english, xboxcompatibilitymode, vsync, texturedetail | 83:3870, 2:590, 53:439, 41:74, 30:62, 3:48; +31 more in JSON |
| 55 | 008dcda0-008ee5f0 | 275 | 0.27 | 2 | 1 | 86 | 0 | 3 | szurkenyil, pup_gain, pum1stget, objectivelist, objectiveentities, uspumicon, uselimit | 2:88, 54:48, 40:25, 83:25, 29:19, 24:17; +2 more in JSON |
| 56 | 008ee670-00922990 | 574 | 0.22 | 2 | 1 | 86 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 83:94, 3:87, 2:83, 19:65, 12:43, 7:41; +13 more in JSON |
| 57 | 009229f0-0092dfe0 | 187 | 0.48 | 2 | 1 | 86 | 0 | 80 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 83:40, 7:19, 2:17, 29:12, 30:12, 56:10; +2 more in JSON |
| 58 | 0092e0b0-00951f20 | 360 | 0.22 | 2 | 1 | 86 | 0 | 2 | ownerplayer, resourceusage, cSmoothMapZoomLevel, supportmanager, periszkop, hajobelso, fizika_ | 83:73, 2:51, 56:43, 57:36, 84:28, 1:27; +7 more in JSON |
| 59 | 00951f40-00958670 | 93 | 0.57 | 2 | 1 | 86 | 0 | 27 | inferiorfailure, usaflag, msmallreconplane, mlargereconplane, japanflag, englishflag, dutchflag | 3:19, 7:16, 1:14, 57:8 |
| 60 | 009586b0-00968e00 | 182 | 0.23 | 2 | 1 | 86 | 0 | 9 | vehicleclass, reconplane, torpedobomber, torpedoboat, divebomber, cargo, fighter | 83:74, 59:32, 2:28, 3:24, 4:16, 58:14; +4 more in JSON |
| 61 | 00968e80-0096bbd0 | 79 | 0.29 | 2 | 1 | 86 | 0 | 1 | callback, matched, lualistener, kill, called | 60:34 |
| 62 | 0096bc60-0098c510 | 394 | 0.43 | 2 | 1 | 86 | 0 | 28 | player, entity, message, callback, oldlevel, newlevel, ambient | 61:96, 53:94, 2:79, 60:47, 34:34, 83:31; +13 more in JSON |
| 63 | 0098c630-00996060 | 150 | 0.52 | 1 | 63 | 1 | 0 | 2 | software, bsm_hwd, eidos, mpkg, cast, content, language | 0:11 |
| 64 | 00996120-009f69c0 | 848 | 0.87 | 2 | 1 | 86 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:373, 45:266, 3:234, 65:167, 91:125, 0:111; +20 more in JSON |
| 65 | 009f6a20-009ffd80 | 123 | 0.63 | 2 | 1 | 86 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:33, 37:19, 46:15, 45:15, 3:13, 83:13; +2 more in JSON |
| 66 | 009ffe00-00a079b0 | 119 | 0.59 | 2 | 1 | 86 | 0 | 0 | vehicle, class, captureweight | 65:35 |
| 67 | 00a07a60-00a160b0 | 90 | 0.69 | 2 | 1 | 86 | 0 | 5 | commandtype, neutral, aivstable_, enemy, target, vehicleclass, s_to_ | 66:45, 1:28, 83:25, 65:19, 69:19, 68:16; +5 more in JSON |
| 68 | 00a160d0-00a335d0 | 359 | 0.49 | 2 | 1 | 86 | 1 | 0 | coordinator, capture, strategicgain, reconratio, objectivemembers, autogrouping, unittypes | 1:30, 83:29, 2:27, 9:23, 67:23, 66:22; +6 more in JSON |
| 69 | 00a371a0-00a45510 | 200 | 0.32 | 2 | 1 | 86 | 0 | 8 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 70:35, 83:19, 41:14, 9:10, 68:9 |
| 70 | 00a45570-00a625d0 | 134 | 0.5 | 2 | 1 | 86 | 0 | 1 | online, server, player, query, matchmaking, remote, movie | 69:34, 63:14, 9:12, 18:9 |
| 71 | 00a62660-00a79a40 | 332 | 0.63 | 2 | 1 | 86 | 0 | 1 | unexpected, chunk, precompiled, call, complex, expression, many | 72:16 |
| 72 | 00a7a2e0-00aa0f50 | 527 | 0.26 | 2 | 1 | 86 | 0 | 16 | memory, sounjd, sound, stream, streaming, stereo, request | 2:53, 83:38, 91:32, 9:26, 79:20, 84:20; +8 more in JSON |
| 73 | 00aa0f70-00aacd90 | 215 | 0.46 | 2 | 1 | 86 | 0 | 38 | cGuiManager, widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor | 75:23, 84:16, 85:15, 76:14, 9:10, 72:9; +1 more in JSON |
| 74 | 00aace40-00ab17b0 | 72 | 0.57 | 2 | 1 | 86 | 0 | 1 | moviename, subtitleswidescreen_text, subtitlesnormal_text, extratitles, subtitles, guidefault, english | 73:17, 83:11 |
| 75 | 00ab1ef0-00ac9640 | 264 | 0.39 | 2 | 1 | 86 | 0 | 11 | texture, simplecolor, mvfm, mshd, shadername, guidefault, verticalalign | 73:79, 85:19, 2:19, 84:19, 83:14, 9:11; +2 more in JSON |
| 76 | 00ac97f0-00ad5360 | 188 | 0.33 | 2 | 1 | 86 | 0 | 7 | vertical_scrollbar, horizontal_scrollbar, visual_group, scalevector, rotationeuler, playbydefault, modeltextureoverride | 73:50, 75:13 |
| 77 | 00ad5590-00ada240 | 79 | 0.58 | 2 | 1 | 86 | 0 | 0 | trees_pc, bushes_pc, bush, foliage, effects, terrain, normal | 78:22 |
| 78 | 00ada420-00b1bc70 | 733 | 0.32 | 2 | 1 | 86 | 0 | 24 | param, mvfm, mshd, emitter, terrain, particlefloating, additive | 84:49, 82:49, 2:48, 85:37, 83:33, 93:29; +8 more in JSON |
| 79 | 00b1bd20-00b20c50 | 97 | 0.73 | 2 | 1 | 86 | 0 | 0 | instanced | 83:9, 82:8 |
| 80 | 00b20d30-00b402b0 | 340 | 0.33 | 2 | 1 | 86 | 0 | 9 | pf43cc, debugshader, mvfm, mshd, shadowtexture, shadowmap, shaders | 9:18, 84:17, 82:16, 79:11 |
| 81 | 00b402e0-00b4b0a0 | 130 | 0.32 | 0 | 81 | 1 | 0 | 3 | mvfm, uf44uf44uf44uf44uf44uf44uf44uf44uf44, uf44uf44uf44, shadow_passtrough, generic, building, shfx |  |
| 82 | 00b4b490-00b65990 | 261 | 0.26 | 2 | 1 | 86 | 0 | 6 | mshd, cSampleOffsets, posteffectsysobj, posteffectsyscam, oldfilm_dust, cSampleWeights, mvfm | 83:67, 79:23, 84:20, 81:19, 80:15, 2:14; +1 more in JSON |
| 83 | 00b659d0-00b6d1b0 | 135 | 0.81 | 2 | 1 | 86 | 0 | 0 | dofile, userdata, thread, lightuserdata, dobuffer, fundamentals, scripts | 71:31, 2:14 |
| 84 | 00b6d3c0-00b72f80 | 123 | 0.55 | 2 | 1 | 86 | 0 | 31 | c3dnode, cCamera | 82:18, 88:8 |
| 85 | 00b732b0-00b811a0 | 249 | 0.49 | 2 | 1 | 86 | 0 | 1 | c3dnodeanimator, cLight, cDummy, cAnimTrack, cAmbientLight, c3dobject, cPointLight | 84:25, 35:13 |
| 86 | 00b811c0-00b86e10 | 97 | 0.69 | 2 | 1 | 86 | 0 | 1 | cSceneResource, resourcedump_, refcounter, cNullSceneResource | 85:15, 2:8, 79:8 |
| 87 | 00b86e40-00b8e5a0 | 106 | 0.37 | 3 | 87 | 1 | 0 | 7 | flare, zoomfactor, targetname, simplecolor2, node, flare2, dissolve | 85:16 |
| 88 | 00b8e6c0-00b999f0 | 162 | 0.57 | 2 | 1 | 86 | 0 | 5 | cRenderMeshResource, cGroupParamsResource, cGroup, quadtreenode, cSkinedMeshResource, cSkined3dObject, cQuadTreeNode | 84:23, 85:17, 35:9 |
| 89 | 00b99bf0-00bb3ce0 | 214 | 0.77 | 2 | 1 | 86 | 0 | 3 | mvfm, mshd, coast, cCorner3, cCorner2, cCorner1, cCorner0 | 84:57, 85:40, 79:24, 86:20, 1:17, 78:17; +3 more in JSON |
| 90 | 00bb3db0-00bbcd70 | 136 | 0.59 | 3 | 90 | 1 | 0 | 4 | mpak, deviceid, shorewavetexturesource2, shorewavetexturesource1, shorewavetexturesource0, open, oceanheightmap | 2:13, 93:8 |
| 91 | 00bbcd90-00bd93e0 | 328 | 0.34 | 2 | 1 | 86 | 0 | 5 | mvfm, mshd, cloud, waterdrops, thunder, streamed, lightning_002 | 83:86, 2:34, 85:29, 84:23, 1:14 |
| 92 | 00bd94d0-00be4410 | 173 | 0.45 | 2 | 1 | 86 | 0 | 4 | nagybetu, long, startup, srch, rejected, perftime, duplicate | 9:10 |
| 93 | 00be4460-00c30570 | 343 | 0.41 | 2 | 1 | 86 | 0 | 19 | cFileStore, iterator, long, removefile, removed, profile, cShaderTextureSource | 83:15, 91:12, 92:9, 2:9 |
