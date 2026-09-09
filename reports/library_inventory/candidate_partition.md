# Candidate partition: disjoint link-order segments

23985 unnamed non-thunk FUN_ candidates grouped into 90 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 847 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004133f0 | 188 | 0.13 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413470-0042a830 | 360 | 0.37 | 1 | 1 | 81 | 0 | 0 | mpakscenes, endgroup, crash, avoidzoneg, avoidzone, training, terraingridlayer | 87:35, 0:29, 10:14, 82:9 |
| 2 | 0042a920-0043f480 | 281 | 0.39 | 1 | 1 | 81 | 0 | 14 | bsp_chk_save, unlockto, unlockname, unlockfrom, playtime, difficulty, united | 82:152, 1:108, 87:37, 81:27, 29:25, 53:19; +4 more in JSON |
| 3 | 0043f4d0-0044c390 | 223 | 0.29 | 1 | 1 | 81 | 0 | 3 | panel, deviceclass, daytime, callback, setpanel, hidepanel, character | 82:55, 1:53, 87:23, 56:8 |
| 4 | 0044c5a0-00453de0 | 106 | 0.5 | 1 | 1 | 81 | 0 | 0 | dialogues, sequence, dialogdefaultpausetime, dialogcharacters, suppressinterruptmsg, requesttime, panelstates | 3:66, 1:40, 82:37, 87:17, 2:8 |
| 5 | 00453ed0-00491070 | 925 | 0.26 | 1 | 1 | 81 | 0 | 18 | entity, alpha, action, cameraposition, activatetime, aa_flak, point | 1:232, 87:112, 82:77, 58:44, 61:40, 2:39; +7 more in JSON |
| 6 | 00491170-004ab3f0 | 235 | 0.4 | 1 | 1 | 81 | 0 | 3 | soldiertypes, landvehicleclasses, tempid, trafficglobals, topbox, timetolive, startvelocitydiradd | 5:223, 82:54, 1:45, 87:23, 3:21, 47:18; +3 more in JSON |
| 7 | 004ab430-004c8140 | 569 | 0.22 | 1 | 1 | 81 | 0 | 5 | wreckclass, white, timeleft, smoke, wreck, collect, collectgarbage | 1:68, 82:49, 87:38, 5:32, 3:18, 29:17; +4 more in JSON |
| 8 | 004c8160-004d9e40 | 269 | 0.33 | 1 | 1 | 81 | 0 | 0 | ingame, ggame, allbutingame, interface, textures, writestats, userleft | 7:139, 1:103, 87:51, 5:18, 82:18, 29:17; +3 more in JSON |
| 9 | 004d9e60-004e7bb0 | 106 | 0.42 | 1 | 1 | 81 | 1 | 5 | ggame, collect, collectgarbage, skiptitle, skiplogos, skipbriefings, noskiplogos | 8:92, 7:74, 1:54, 87:26, 5:21, 82:18; +16 more in JSON |
| 10 | 004e7c00-00501620 | 318 | 0.32 | 1 | 1 | 81 | 0 | 6 | menuitem_text, vehicleclass, ambient, back, sound, party, globals | 1:159, 82:60, 76:51, 5:39, 87:39, 9:35; +7 more in JSON |
| 11 | 00501670-0051e4a0 | 224 | 0.4 | 1 | 1 | 81 | 0 | 1 | back, vehicleclass, dview, scroll_menu, globals, navigate, datatables | 1:154, 10:137, 82:115, 87:70, 76:63, 7:35; +9 more in JSON |
| 12 | 0051e4d0-00529490 | 132 | 0.63 | 1 | 1 | 81 | 0 | 1 | attackmove, cycle, target, submarine_group, stearring, showocean, showfoliage | 1:33, 29:31, 7:25, 76:19, 10:13, 2:12; +5 more in JSON |
| 13 | 005294f0-005439b0 | 290 | 0.28 | 1 | 1 | 81 | 0 | 0 | mshd, vidm, text_b_text, text_a_text, starty, rowspace, pg_6s_text | 1:106, 76:82, 87:40, 7:37, 78:27, 82:21; +8 more in JSON |
| 14 | 00543a30-00558640 | 309 | 0.29 | 1 | 1 | 81 | 0 | 3 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 1:74, 29:35, 87:30, 76:29, 7:24, 8:17; +6 more in JSON |
| 15 | 00558680-00563440 | 126 | 0.41 | 1 | 1 | 81 | 0 | 0 | fe_pc, preset, presets, opt_normal, opt_inverted, opt_cancel, globals | 14:75, 1:61, 76:59, 87:20, 29:18, 82:16; +4 more in JSON |
| 16 | 00563540-0058b7f0 | 550 | 0.49 | 1 | 1 | 81 | 0 | 3 | globals, navigate, back, select, server_text, players_text, mode_text | 1:150, 76:123, 87:51, 29:36, 78:36, 82:28; +9 more in JSON |
| 17 | 0058b830-005b68d0 | 348 | 0.23 | 1 | 1 | 81 | 0 | 0 | globals, mission_mappoint_, bushgroup, visibility, terrain, group, back | 1:116, 16:113, 76:65, 87:34, 78:30, 13:27; +11 more in JSON |
| 18 | 005b6960-005bcce0 | 79 | 0.51 | 1 | 1 | 81 | 0 | 0 | message, played, sound, warnings_group, vanmeg, textbox_obj_text, textbox_obj_framebox | 1:29, 37:18, 76:16, 17:11, 78:11, 29:10; +2 more in JSON |
| 19 | 005bcd20-005d0050 | 257 | 0.42 | 1 | 1 | 81 | 0 | 2 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, unit_marker_group | 1:56, 7:27, 87:23, 10:20, 76:19, 82:19; +3 more in JSON |
| 20 | 005d0770-005df510 | 109 | 0.64 | 1 | 1 | 81 | 0 | 0 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 1:81, 76:50, 7:34, 87:26, 44:23, 29:19; +7 more in JSON |
| 21 | 005e09a0-005ef4d0 | 81 | 0.68 | 2 | 21 | 1 | 0 | 3 | globals, navigate, back, achievement_lock_icon, players_listbox, rank_icon, change | 76:94, 1:81, 20:74, 29:45, 7:32, 16:31; +8 more in JSON |
| 22 | 005ef500-00604c20 | 184 | 0.57 | 1 | 1 | 81 | 0 | 2 | xsm_saveconfirm, fe_xbox, xsm_dlcchanged, arrow_left_icon, ingame, arrow_right_icon, mainlistbox_text | 1:178, 76:93, 87:60, 8:46, 7:38, 58:32; +9 more in JSON |
| 23 | 00604c80-0061f630 | 270 | 0.21 | 1 | 1 | 81 | 0 | 3 | usn_point_text, usn_icon, radar_sweep, pumpermanent, player_point_text, circle_full_, section | 1:88, 76:66, 7:39, 2:38, 48:36, 29:28; +8 more in JSON |
| 24 | 0061f6d0-00639f40 | 136 | 0.38 | 1 | 1 | 81 | 0 | 1 | globals, rank_icon, continue, scoring_unlock_text, playerreview, move_group, debriefing_clipbox | 1:67, 23:62, 82:28, 76:22, 16:21, 87:18; +6 more in JSON |
| 25 | 0063a280-00644220 | 95 | 0.43 | 2 | 25 | 1 | 0 | 0 | type_icon, commandbuilding_icon, close_group, globals, yellow_icon, white_icon, spectator_text | 1:58, 24:38, 76:24, 87:13 |
| 26 | 00644240-00654650 | 150 | 0.43 | 1 | 1 | 81 | 0 | 5 | ingame, vehicleclass, sub_depth_arrow_icon, sub_depht_arrow_dest_icon, sub_air_warning_icon, sub_air_arrow_icon, shipcaptain | 1:53, 29:43, 76:25, 7:24, 44:17, 14:17; +10 more in JSON |
| 27 | 00654a70-00664030 | 208 | 0.44 | 1 | 1 | 81 | 0 | 0 | circle_hl_icon, ingamegui, normal, icon_l_icon, circle_small_02_group, circle_small_01_group, number_text | 1:60, 74:24, 76:23, 29:21, 87:21 |
| 28 | 00664070-00681ef0 | 207 | 0.43 | 1 | 1 | 81 | 0 | 2 | ingame, sm_cp, globals, unitclass_spawnpoint, sm_support, inferiorfailure, unitclass_kamikazeboat | 27:172, 1:98, 76:47, 7:42, 17:34, 82:29; +16 more in JSON |
| 29 | 00681f40-006964b0 | 262 | 0.37 | 1 | 1 | 81 | 0 | 7 | gvmultimenu, ingame, pushrequestinterface, interface, textures, warning_2_text, missionunique | 1:90, 8:57, 10:43, 87:36, 7:35, 82:30; +12 more in JSON |
| 30 | 006964f0-0069f940 | 160 | 0.58 | 1 | 1 | 81 | 0 | 0 | swapstickpairs, swapstickmap, swapstickgeneral, press, invio, inputmodifiers, indietro | 1:25, 82:21, 14:12, 87:8 |
| 31 | 0069fa40-006a9850 | 125 | 0.62 | 1 | 1 | 81 | 0 | 0 | devinputs, sensitivitysettings, inputsettings, devicetype, deviceidx, slider, down | 30:54, 14:27, 15:27, 1:25, 3:20, 82:20; +2 more in JSON |
| 32 | 006a9a40-006be980 | 214 | 0.41 | 1 | 1 | 81 | 0 | 0 | savedata, scoring, entities, missionid, entidcont, cont1, cont0 | 82:349, 1:168, 87:49, 5:29, 3:24, 31:19; +10 more in JSON |
| 33 | 006beb10-006d6450 | 301 | 0.55 | 1 | 1 | 81 | 0 | 17 | equipment, state, slots, runwaywidth, runwaylength, runwayfailure, hangarfailure | 1:100, 29:81, 32:73, 2:30, 44:24, 5:19; +9 more in JSON |
| 34 | 006d6470-006da6b0 | 101 | 0.42 | 2 | 34 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 1:21, 87:11 |
| 35 | 006dadc0-006deca0 | 66 | 0.73 | 3 | 35 | 1 | 0 | 0 | markerclasses, recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 34:49, 82:10, 1:8 |
| 36 | 006dee40-006faf00 | 333 | 0.41 | 1 | 1 | 81 | 1 | 22 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 1:137, 82:81, 29:43, 41:41, 61:34, 12:33; +16 more in JSON |
| 37 | 006fb030-0070c1f0 | 237 | 0.27 | 1 | 1 | 81 | 0 | 10 | openaftertime, messages, whosaysthat, velocity, openstate, dragvert, divedepth | 1:80, 82:51, 36:35, 87:32, 83:22, 61:21; +8 more in JSON |
| 38 | 0070c210-00717980 | 154 | 0.32 | 1 | 1 | 81 | 0 | 0 | unitlist, unitid, shipnumber, shape, num_0, relativeposition, leader | 1:20, 2:8 |
| 39 | 00717c70-0071b940 | 82 | 0.63 | 2 | 39 | 1 | 0 | 1 | mzonedesc, sphere, identifier, mnote, armor, points, category | 38:57, 86:13, 89:11 |
| 40 | 0071b9b0-007290d0 | 215 | 0.47 | 1 | 1 | 81 | 0 | 14 | startmode, followmode, userpath, pathfollowparams, pathcursor, mgeommesh, internalclearprimarycommand | 1:68, 43:29, 44:21, 12:14, 47:12, 29:9 |
| 41 | 007292c0-0074e540 | 431 | 0.27 | 1 | 1 | 81 | 0 | 25 | destroyed, speed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed | 1:158, 82:100, 87:66, 63:33, 43:32, 44:29; +16 more in JSON |
| 42 | 0074e5d0-00758eb0 | 60 | 0.57 | 1 | 1 | 81 | 0 | 5 | sumleaks, sumforces, iswater, water, launchairstrike, elevator_2, runwayfailure | 1:23, 50:11, 5:8, 33:8, 43:8 |
| 43 | 00758f90-0076e990 | 433 | 0.74 | 1 | 1 | 81 | 0 | 5 | p2p_voice__, woice, serversendscenescoring, runwaycenter, receive, nonce, myplayer | 1:679, 44:38, 87:12, 33:11, 82:9, 19:9 |
| 44 | 0076ea10-007866b0 | 353 | 0.21 | 1 | 1 | 81 | 0 | 27 | reconlevel, multiscore, lastbanto, p2p_voice__, recondata, peer, netentity | 43:133, 1:101, 82:56, 87:36, 7:28, 61:27; +10 more in JSON |
| 45 | 007868c0-0078cf20 | 68 | 0.44 | 2 | 45 | 1 | 0 | 0 | send_, recv_, time, client, server | 87:30, 1:16, 85:13 |
| 46 | 0078cff0-007a42c0 | 250 | 0.47 | 1 | 1 | 81 | 0 | 0 | camera, thetalinearblend, theta, rholinearblend, blendtime, postype, initialization | 1:126, 82:57, 2:48, 29:38, 87:29, 19:19; +3 more in JSON |
| 47 | 007a44d0-007b2e40 | 136 | 0.57 | 1 | 1 | 81 | 0 | 8 | pathid, pathbaseentity, simple, paratrooper, soldieranim, slowfactoropened, slowfactorclosed | 46:83, 1:78, 82:32, 2:28, 81:20, 29:18; +6 more in JSON |
| 48 | 007b2ec0-007d1d30 | 339 | 0.68 | 1 | 1 | 81 | 0 | 13 | powerlost, explosion, splash, enginefire, rightspinning, pathpoints, leftspinning | 1:127, 49:77, 2:72, 87:36, 43:33, 61:30; +12 more in JSON |
| 49 | 007d1dc0-0080d9b0 | 556 | 0.44 | 1 | 1 | 81 | 1 | 7 | travelspeed, gears, baydoor, wings, enemy, neutral, state | 1:201, 82:107, 2:82, 87:55, 48:54, 29:41; +21 more in JSON |
| 50 | 0080da00-0081aa10 | 163 | 0.65 | 1 | 1 | 81 | 0 | 38 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 1:46, 63:36, 43:27, 44:25, 40:18, 61:17; +5 more in JSON |
| 51 | 0081aa60-00828810 | 96 | 0.43 | 1 | 1 | 81 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 50:77, 1:50, 53:18, 61:18, 82:18, 87:14; +5 more in JSON |
| 52 | 00828870-00851e40 | 433 | 0.27 | 1 | 1 | 81 | 2 | 7 | gameunit, classid, stock, torpedoavoidance, object, state, torpedoenabled | 1:92, 82:64, 5:45, 40:39, 6:34, 44:24; +16 more in JSON |
| 53 | 00851e90-0086ad50 | 295 | 0.53 | 1 | 1 | 81 | 0 | 10 | radius, submarine, torpedo, unlimitedair, tvertangle, turninggun, thorzangle | 1:73, 82:67, 2:38, 36:21, 87:17, 5:16; +12 more in JSON |
| 54 | 0086ae00-00874540 | 120 | 0.59 | 1 | 1 | 81 | 0 | 2 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 82:76, 87:60, 1:47, 53:35, 7:9, 8:9 |
| 55 | 00874640-00878350 | 62 | 0.48 | 1 | 1 | 81 | 0 | 2 | fragile, damage | 61:10, 44:9 |
| 56 | 00878530-008828e0 | 108 | 0.44 | 1 | 1 | 81 | 0 | 2 | damage, yellow, weaponsystems, weapondirectorthinktime, warningscrollspeeds, visibletimeout, visibilityrange | 82:36, 55:30, 1:21, 87:12 |
| 57 | 00882ac0-0088ac20 | 127 | 0.61 | 1 | 1 | 81 | 0 | 0 | scripts, debugtrap, shallowwater, modelpath, luab, filepath, colormap | 1:61, 32:51, 82:47, 87:24, 36:20 |
| 58 | 0088b120-008e5c10 | 877 | 0.78 | 1 | 1 | 81 | 528 | 2 | luakod, szurkenyil, options, hardwarereported, english, xboxcompatibilitymode, vsync | 82:3873, 1:1850, 87:602, 57:434, 44:75, 32:57; +29 more in JSON |
| 59 | 008e5c20-008f10b0 | 193 | 0.59 | 1 | 1 | 81 | 0 | 1 | pup_gain, pum1stget, vec3array, uspumicon, uselimit, unitclassindex, targettype | 58:59, 1:59, 82:25, 87:23, 23:16, 3:16; +4 more in JSON |
| 60 | 008f10c0-00922c80 | 507 | 0.34 | 1 | 1 | 81 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 1:218, 82:94, 16:88, 2:77, 87:67, 59:62; +15 more in JSON |
| 61 | 00922de0-0092e0b0 | 181 | 0.46 | 1 | 1 | 81 | 0 | 78 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 82:43, 1:38, 5:19, 87:17, 29:12, 32:12; +1 more in JSON |
| 62 | 0092e110-00943bb0 | 206 | 0.34 | 1 | 1 | 81 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 1:81, 61:32, 82:26, 87:24, 53:14, 83:14; +5 more in JSON |
| 63 | 00943c00-0097c8a0 | 769 | 0.3 | 1 | 1 | 81 | 0 | 64 | callback, vehicleclass, player, entity, message, inferiorfailure, ambient | 1:269, 82:161, 87:82, 2:60, 60:39, 5:39; +20 more in JSON |
| 64 | 0097c9b0-0098c510 | 132 | 0.41 | 2 | 64 | 1 | 0 | 3 | repair, player, shiplanded, musicover, hpevent, generate, entitykilled | 63:175, 1:115, 57:93, 87:45, 36:32, 32:23; +8 more in JSON |
| 65 | 0098c630-009965d0 | 155 | 0.5 | 1 | 1 | 81 | 0 | 2 | software, bsm_hwd, eidos, mpkg, cast, content, language | 1:38, 87:15, 0:12 |
| 66 | 00996670-009f69c0 | 843 | 0.83 | 1 | 1 | 81 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:417, 48:265, 2:232, 67:167, 0:113, 87:105; +22 more in JSON |
| 67 | 009f6a20-009fe170 | 90 | 0.73 | 1 | 1 | 81 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:41, 49:19, 40:19, 2:15, 82:13, 29:13; +2 more in JSON |
| 68 | 009fe1b0-00a16030 | 238 | 0.49 | 1 | 1 | 81 | 0 | 5 | commandtype, neutral, aivstable_, enemy, vehicle, class, target | 1:73, 70:31, 82:27, 87:18, 63:18, 67:15; +6 more in JSON |
| 69 | 00a16050-00a1fa60 | 171 | 0.6 | 1 | 1 | 81 | 0 | 0 | coordinator, capture, sell, strategicgain, duel, defend, siege | 1:45, 70:30, 87:15, 68:12 |
| 70 | 00a1fa90-00a371a0 | 193 | 0.72 | 1 | 1 | 81 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 69:110, 1:48, 68:45, 82:27, 7:22, 87:16; +2 more in JSON |
| 71 | 00a371c0-00a41660 | 143 | 0.36 | 1 | 1 | 81 | 0 | 6 | client, online, xenonsystemmanager, online__, network, player, changestate | 82:19, 1:16, 70:9, 44:9, 87:8 |
| 72 | 00a41880-00a625d0 | 190 | 0.45 | 2 | 72 | 1 | 0 | 3 | online, server, player, query, matchmaking, mnetworkclientxlive, remote | 71:32, 1:23, 65:21, 7:15, 16:13, 87:10; +1 more in JSON |
| 73 | 00a62660-00a799b0 | 331 | 0.63 | 1 | 1 | 81 | 0 | 1 | unexpected, chunk, precompiled, complex, expression, call, many | 1:24, 75:16, 87:13 |
| 74 | 00a79a40-00a82b70 | 132 | 0.68 | 1 | 1 | 81 | 0 | 5 | memory, sounjd, sound, play, event, system, init | 82:35, 1:33, 87:19 |
| 75 | 00a82c60-00a9a4a0 | 301 | 0.26 | 1 | 1 | 81 | 0 | 6 | memory, sounjd, stream, sound, streaming, stereo, request | 1:89, 87:50, 74:44, 80:20, 89:17, 82:13; +2 more in JSON |
| 76 | 00a9a5a0-00aacd90 | 312 | 0.35 | 1 | 1 | 81 | 0 | 41 | cGuiManager, widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor | 1:60, 78:38, 87:21, 82:19, 7:16, 83:14 |
| 77 | 00aace40-00ab6070 | 123 | 0.59 | 1 | 1 | 81 | 0 | 2 | partialdisplaytype, partialdisplayratio, dynamicvb, delayedtextureload, autorotate, moviename, hastexture | 1:57, 76:27, 87:27, 82:11, 83:9 |
| 78 | 00ab6100-00af44c0 | 808 | 0.24 | 1 | 1 | 81 | 0 | 24 | mshd, mvfm, simplecolor, texture, guidefault, terrain, normal | 1:234, 76:144, 83:111, 87:104, 82:71, 89:45; +9 more in JSON |
| 79 | 00af45d0-00aff690 | 117 | 0.56 | 1 | 1 | 81 | 0 | 1 | param, persec, permeter, sphereemitter, softparticle, smartareaemitter, renderpriority | 80:45, 1:43, 87:26, 78:24, 83:14 |
| 80 | 00aff700-00b20bf0 | 398 | 0.23 | 1 | 1 | 81 | 0 | 10 | param, additive, emitter, particle, mvfm, initialrotation, emittedspeed | 1:116, 79:64, 81:56, 87:55, 82:46, 78:29; +1 more in JSON |
| 81 | 00b20c50-00b659d0 | 746 | 0.29 | 1 | 1 | 81 | 0 | 13 | mshd, cSampleOffsets, mvfm, pf43cc, shadowmap, posteffectsysobj, posteffectsyscam | 1:167, 82:93, 87:82, 80:60, 83:40, 7:12; +2 more in JSON |
| 82 | 00b65ac0-00b716d0 | 223 | 0.57 | 1 | 1 | 81 | 0 | 31 | dofile, userdata, thread, lightuserdata, dobuffer, c3dnode, fundamentals | 1:51, 73:31, 81:20, 87:14 |
| 83 | 00b71770-00b75d80 | 92 | 0.47 | 1 | 1 | 81 | 0 | 0 | cDummy, c3dobject, cMesh, cCamera | 82:25, 1:8 |
| 84 | 00b75de0-00b7abd0 | 85 | 0.66 | 1 | 1 | 81 | 0 | 0 | c3dnodeanimator, cAnimTrack, cOptimized3dNodeAnimator, cCameraAnimator | 1:15, 87:9, 86:8 |
| 85 | 00b7ac90-00b86780 | 239 | 0.4 | 1 | 1 | 81 | 0 | 1 | cLight, cAmbientLight, boundingbox, resource, resourcedump_, refcounter, matrix | 1:50, 89:25, 87:24, 83:11, 82:9, 80:9 |
| 86 | 00b86820-00b91000 | 184 | 0.47 | 1 | 1 | 81 | 0 | 3 | cSceneResource, cGroupParamsResource, cGroup, flare, zoomfactor, targetname, node | 1:28, 89:22, 83:18, 84:13, 87:13, 82:13 |
| 87 | 00b911d0-00bd4200 | 758 | 0.28 | 1 | 1 | 81 | 0 | 97 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 1:251, 83:164, 82:81, 89:80, 80:65, 85:47; +6 more in JSON |
| 88 | 00bd4270-00bdb1e0 | 98 | 0.4 | 1 | 1 | 81 | 0 | 2 |  | 82:83, 1:40, 87:21 |
| 89 | 00bdb2e0-00c30570 | 535 | 0.36 | 1 | 1 | 81 | 0 | 20 | cFileStore, long, iterator, cPhysicalDirectoryX86, openfileoverlapped, removefile, removed | 1:197, 87:105, 88:39, 0:19, 82:15, 14:12; +2 more in JSON |
