# Candidate partition: disjoint link-order segments

23700 unnamed non-thunk FUN_ candidates grouped into 98 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 863 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004138d0 | 190 | 0.08 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413d10-0042a830 | 352 | 0.35 | 2 | 1 | 83 | 0 | 0 | mpakscenes, endgroup, crash, avoidzoneg, avoidzone, training, terraingridlayer | 0:27, 11:13, 87:9, 94:8 |
| 2 | 0042a920-0043e840 | 265 | 0.42 | 2 | 1 | 83 | 0 | 14 | bsp_chk_save, unlockto, unlockname, unlockfrom, playtime, difficulty, united | 87:147, 1:59, 86:27, 28:25, 53:19, 95:17; +3 more in JSON |
| 3 | 0043e8f0-004486a0 | 162 | 0.35 | 2 | 1 | 83 | 0 | 3 | deviceclass, daytime, device, platform, weatherreconmodifiers, weatherampmultipliers, viewmodeampmultipliers | 87:52, 55:9, 1:8 |
| 4 | 004486c0-00453c30 | 179 | 0.59 | 2 | 1 | 83 | 0 | 1 | panel, sequence, message, callback, suppressinterruptmsg, setpanel, requesttime | 87:38, 1:16, 2:8 |
| 5 | 00453cc0-004654d0 | 148 | 0.55 | 2 | 1 | 83 | 0 | 1 | action, cameraposition, activatetime, aa_flak, alpha, point, periscope | 6:41, 1:25, 56:15, 87:12 |
| 6 | 00465610-0047c680 | 428 | 0.23 | 2 | 1 | 83 | 0 | 2 | groupname, scenebrowsergroups, s_ls_, cloudclass, browsergroup, properties, groups | 87:37, 7:34, 56:29, 59:28, 1:27, 2:25; +2 more in JSON |
| 7 | 0047c6a0-00491070 | 351 | 0.34 | 2 | 1 | 83 | 0 | 15 | weight, soldiertypes, landvehicleclasses, entity, outingdelay, mininrow, maxinrow | 1:17, 60:16, 87:14, 8:13, 59:12, 53:11; +2 more in JSON |
| 8 | 00491170-004ba200 | 536 | 0.14 | 2 | 1 | 83 | 0 | 7 | soldiertypes, landvehicleclasses, wreckclass, tempid, smokeefx, rotationdecline, gravitymul | 7:227, 87:97, 1:19, 3:18, 46:18, 2:17; +7 more in JSON |
| 9 | 004ba290-004d0860 | 395 | 0.25 | 2 | 1 | 83 | 0 | 1 | white, allbutingame, collect, collectgarbage, interface, textures, writestats | 8:41, 1:26, 28:25, 74:16, 88:9, 11:9; +1 more in JSON |
| 10 | 004d0920-004e7db0 | 250 | 0.38 | 2 | 1 | 83 | 1 | 5 | ggame, collect, collectgarbage, ingame, universe, scene, skiptitle | 9:233, 1:54, 8:46, 87:28, 74:22, 28:21; +18 more in JSON |
| 11 | 004e7dd0-004f9d30 | 205 | 0.33 | 2 | 1 | 83 | 0 | 6 | ambient, sound, party, noisetexture, landconvoy, stationary, filename | 59:42, 1:41, 10:36, 9:23, 87:22, 60:21; +4 more in JSON |
| 12 | 004f9d80-00519fe0 | 267 | 0.39 | 2 | 1 | 83 | 0 | 1 | menuitem_text, vehicleclass, back, dview, navigate, globals, scroll_menu | 87:141, 1:62, 77:50, 11:41, 75:30, 28:29; +10 more in JSON |
| 13 | 0051a0e0-0051e730 | 60 | 0.5 | 3 | 13 | 1 | 0 | 0 | siege_group, island_capture_group, ijn05, fe_briefing_listbox, escort_group, duel_group, competitive_group | 9:10 |
| 14 | 0051e7e0-00527c80 | 108 | 0.72 | 2 | 1 | 83 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 28:29, 9:20, 1:14, 74:10, 2:10, 43:10; +1 more in JSON |
| 15 | 00527cb0-00544b60 | 334 | 0.45 | 2 | 1 | 83 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 77:56, 9:45, 1:37, 78:29, 11:22, 28:20; +11 more in JSON |
| 16 | 00544e60-00554600 | 198 | 0.28 | 2 | 1 | 83 | 0 | 2 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 28:32, 9:23, 1:22, 77:18, 10:16, 78:12; +4 more in JSON |
| 17 | 00554680-00568930 | 275 | 0.4 | 2 | 1 | 83 | 0 | 1 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 75:44, 16:40, 77:32, 1:32, 28:31, 87:29; +7 more in JSON |
| 18 | 00568cb0-0056d660 | 91 | 0.7 | 2 | 1 | 83 | 0 | 0 | tilt_icon, slider_pos_group, setting_template_next_line_group, servers_framebox, servers_fix_group, servers_clipbox, realation_server_icon | 17:28 |
| 19 | 0056d9a0-00580530 | 236 | 0.44 | 2 | 1 | 83 | 0 | 1 | globals, server_text, players_text, mode_text, scroll_right_icon, wave_icon, setting_2_text | 18:60, 1:33, 17:27, 77:18, 28:17, 75:17; +3 more in JSON |
| 20 | 005805a0-005c5600 | 666 | 0.51 | 2 | 1 | 83 | 0 | 3 | globals, navigate, back, mission_mappoint_, select, main_newprofile, bushgroup | 1:98, 77:90, 78:60, 19:58, 15:35, 28:34; +16 more in JSON |
| 21 | 005c57d0-005cce70 | 115 | 0.41 | 2 | 1 | 83 | 0 | 0 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, uniquemultisettings | 20:17, 87:16, 11:12, 9:8 |
| 22 | 005cd070-005df510 | 171 | 0.77 | 2 | 1 | 83 | 0 | 1 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 9:59, 1:37, 43:28, 77:23, 10:21, 75:21; +10 more in JSON |
| 23 | 005e09a0-00604a50 | 263 | 0.77 | 2 | 1 | 83 | 0 | 5 | globals, back, xsm_saveconfirm, fe_xbox, mainlistbox_text, select, main_listbox | 1:109, 9:82, 75:82, 22:76, 77:76, 28:71; +15 more in JSON |
| 24 | 00604bc0-0060cb40 | 62 | 0.55 | 2 | 1 | 83 | 0 | 1 | turbo_group, turbo_effect, ship_speed_num3_icon, ship_payload_2_icon, ship_payload_1_icon, repairzone_text, planewindsmoke | 47:36, 2:33, 1:23, 28:19, 9:14, 11:11; +3 more in JSON |
| 25 | 0060cb60-0061d330 | 176 | 0.4 | 2 | 1 | 83 | 0 | 2 | usn_point_text, usn_icon, radar_sweep, pumpermanent, player_point_text, circle_full_, section | 77:34, 9:20, 11:16, 1:15, 78:13, 27:13; +3 more in JSON |
| 26 | 0061d5c0-00654a70 | 416 | 0.21 | 2 | 1 | 83 | 0 | 6 | globals, ingame, rank_icon, continue, vehicleclass, type_icon, scoring_unlock_text | 1:83, 28:56, 77:50, 87:44, 9:43, 20:28; +20 more in JSON |
| 27 | 00654a90-00680d30 | 408 | 0.41 | 2 | 1 | 83 | 0 | 2 | circle_hl_icon, ingamegui, normal, icon_l_icon, globals, sm_cp, ingame | 1:70, 77:51, 9:46, 20:44, 28:43, 74:36; +12 more in JSON |
| 28 | 00680db0-00696450 | 266 | 0.54 | 2 | 1 | 83 | 0 | 7 | gvmultimenu, ingame, pushrequestinterface, interface, textures, warning_2_text, missionunique | 87:53, 9:51, 10:42, 11:41, 1:37, 23:27; +9 more in JSON |
| 29 | 00696470-006beb10 | 502 | 0.41 | 2 | 1 | 83 | 0 | 0 | savedata, scoring, sensitivitysettings, inputsettings, devicetype, deviceidx, slider | 87:390, 1:104, 17:69, 3:28, 16:25, 42:19; +13 more in JSON |
| 30 | 006beb70-006d4340 | 282 | 0.54 | 2 | 1 | 83 | 0 | 18 | equipment, state, slots, runwayfailure, hangarfailure, planes, classid | 28:81, 29:72, 1:62, 2:29, 43:23, 60:16; +8 more in JSON |
| 31 | 006d4370-006d8ac0 | 90 | 0.48 | 3 | 31 | 1 | 0 | 0 | section_, hangars, exitpathid, exitpath, entrypathid, entrypath, entityid | 30:16, 1:14 |
| 32 | 006d8b00-006dec70 | 94 | 0.69 | 4 | 32 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers, markerclasses | 31:58, 87:10 |
| 33 | 006deca0-006fa320 | 328 | 0.43 | 2 | 1 | 83 | 1 | 28 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 1:103, 87:72, 28:43, 60:34, 14:33, 42:33; +17 more in JSON |
| 34 | 006fa540-00701ad0 | 107 | 0.51 | 2 | 1 | 83 | 0 | 11 | openaftertime, velocity, openstate, dragvert, divedepth, curfloor, cargorelpos | 87:39, 33:37, 60:21, 28:20, 7:16, 62:14; +3 more in JSON |
| 35 | 00701b70-0070bba0 | 126 | 0.31 | 2 | 1 | 83 | 0 | 2 | messages, whosaysthat, datatables, scripts, mvfm, mshd, unloaded | 1:17, 2:9 |
| 36 | 0070bc30-00714030 | 113 | 0.47 | 2 | 1 | 83 | 0 | 2 | unitlist, unitid, shouldblast, shipnumber, shape, num_0, mindist2 | 1:13, 88:11 |
| 37 | 00714060-0071b710 | 104 | 0.51 | 0 | 37 | 1 | 0 | 3 | sphere, mzonedesc, mnote, identifier, armor, points, category |  |
| 38 | 0071ba20-00722fb0 | 122 | 0.68 | 2 | 1 | 83 | 0 | 5 | startmode, followmode, userpath, pathfollowparams, pathcursor, internalclearprimarycommand, emptycommand | 1:46, 43:21, 42:21, 14:14, 46:12, 28:9 |
| 39 | 00723030-00728960 | 87 | 0.54 | 2 | 1 | 83 | 0 | 10 | mgeommesh | 38:13 |
| 40 | 00728a90-00735ec0 | 167 | 0.47 | 2 | 1 | 83 | 0 | 22 | destroyed, barreldelaytime, torpedo, titlelist, throwb, throwa, nextfirebarrel | 1:46, 87:19, 42:18, 3:16, 39:12, 53:10; +3 more in JSON |
| 41 | 00735f30-00758eb0 | 327 | 0.27 | 2 | 1 | 83 | 0 | 16 | landvehicle, landingship, landfort, sumleaks, sumforces, rampaelfordulas, partraszalltunk | 1:73, 87:70, 42:30, 62:28, 60:25, 43:22; +15 more in JSON |
| 42 | 00758f90-0076a970 | 348 | 0.87 | 2 | 1 | 83 | 0 | 5 | runwaycenter, maxlandingplanesonboard, liftexitpoint, deckcamera, carrierescort, elevator_2, vertangle | 1:649, 30:9, 87:8 |
| 43 | 0076aa00-00787040 | 445 | 0.37 | 2 | 1 | 83 | 0 | 28 | p2p_voice__, reconlevel, multiscore, mmultiplayer, lastbanto, player, recondata | 42:103, 1:64, 87:56, 9:34, 60:27, 22:19; +10 more in JSON |
| 44 | 007870d0-0078d850 | 72 | 0.46 | 2 | 1 | 83 | 0 | 0 | send_, recv_, time, client, server | 89:27, 92:25 |
| 45 | 0078d880-007a4860 | 242 | 0.42 | 2 | 1 | 83 | 0 | 0 | postype, camera, thetalinearblend, theta, rholinearblend, blendtime, initialization | 1:82, 87:61, 2:47, 28:38, 21:11, 53:10; +3 more in JSON |
| 46 | 007a49a0-007b2d80 | 131 | 0.63 | 2 | 1 | 83 | 0 | 8 | simple, pathid, pathbaseentity, paratrooper, soldieranim, slowfactoropened, slowfactorclosed | 45:79, 1:58, 2:28, 87:26, 86:20, 28:18; +5 more in JSON |
| 47 | 007b2dd0-007d1dc0 | 342 | 0.68 | 2 | 1 | 83 | 0 | 13 | powerlost, explosion, splash, enginefire, rightspinning, pathpoints, leftspinning | 1:97, 2:70, 48:70, 42:33, 60:30, 43:24; +12 more in JSON |
| 48 | 007d1e50-007f8390 | 288 | 0.63 | 2 | 1 | 83 | 1 | 14 | travelspeed, gears, baydoor, wings, state, windsound, timeout | 1:100, 47:54, 2:51, 87:39, 60:32, 53:28; +14 more in JSON |
| 49 | 007f8400-0080d9b0 | 267 | 0.21 | 2 | 1 | 83 | 0 | 2 | enemy, neutral, unlocks, selectedmissionid, selecteddifficulty, seenunlocks, savedlobbyfilters | 87:60, 2:31, 9:23, 1:22, 28:18, 48:17; +4 more in JSON |
| 50 | 0080da00-0081aa10 | 163 | 0.64 | 2 | 1 | 83 | 0 | 38 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 1:42, 62:35, 42:27, 43:25, 60:18, 38:17; +4 more in JSON |
| 51 | 0081aa60-00828870 | 97 | 0.43 | 2 | 1 | 83 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 50:77, 1:32, 53:18, 60:18, 87:16, 41:12; +5 more in JSON |
| 52 | 008288d0-00851cb0 | 430 | 0.23 | 2 | 1 | 83 | 2 | 7 | gameunit, classid, stock, torpedoavoidance, object, state, torpedoenabled | 87:60, 1:46, 38:36, 8:36, 43:24, 6:22; +18 more in JSON |
| 53 | 00851e10-008740e0 | 413 | 0.36 | 2 | 1 | 83 | 0 | 16 | radius, effects, submarine, torpedo, unlimitedair, tvertangle, turninggun | 87:115, 1:59, 2:39, 88:32, 33:22, 92:21; +17 more in JSON |
| 54 | 008742a0-00878350 | 66 | 0.56 | 2 | 1 | 83 | 0 | 2 | fragile, damage | 60:10, 43:9 |
| 55 | 00878530-0088b3d0 | 239 | 0.26 | 2 | 1 | 83 | 0 | 2 | damage, scripts, debugtrap, datatables, yellow, weaponsystems, weapondirectorthinktime | 87:79, 29:51, 1:34, 54:30, 33:21, 2:11; +3 more in JSON |
| 56 | 0088b410-008dcd40 | 716 | 0.77 | 2 | 1 | 83 | 528 | 0 | luakod, options, hardwarereported, english, xboxcompatibilitymode, vsync, texturedetail | 87:3870, 1:611, 55:437, 43:74, 29:62, 2:48; +31 more in JSON |
| 57 | 008dcda0-008e3d50 | 116 | 0.62 | 2 | 1 | 83 | 0 | 1 | szurkenyil, objectivelist, objectiveentities, secobjprefix, pinged, missionglobals, flagprocess | 56:48, 1:40, 42:13, 43:9, 28:9 |
| 58 | 008e3de0-008ee5f0 | 159 | 0.36 | 3 | 58 | 1 | 0 | 2 | pup_gain, pum1stget, uspumicon, uselimit, unitclassindex, targettype, targetfilter | 1:48, 87:25, 25:17, 42:12, 28:10, 20:9 |
| 59 | 008ee670-00922240 | 571 | 0.22 | 2 | 1 | 83 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 1:110, 2:87, 87:80, 20:65, 12:43, 19:39; +11 more in JSON |
| 60 | 009222e0-0092dfe0 | 190 | 0.42 | 2 | 1 | 83 | 0 | 79 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 87:54, 1:20, 28:12, 29:12, 59:10, 6:10; +3 more in JSON |
| 61 | 0092e0b0-00951f20 | 360 | 0.21 | 2 | 1 | 83 | 0 | 2 | ownerplayer, resourceusage, cSmoothMapZoomLevel, supportmanager, periszkop, hajobelso, fizika_ | 87:73, 1:71, 59:43, 60:36, 88:32, 53:22; +6 more in JSON |
| 62 | 00951f40-00966890 | 210 | 0.36 | 2 | 1 | 83 | 0 | 37 | vehicleclass, inferiorfailure, reconplane, torpedobomber, torpedoboat, divebomber, cargo | 87:74, 1:50, 2:43, 3:20, 41:18, 61:17; +11 more in JSON |
| 63 | 009668c0-00968e00 | 65 | 0.38 | 0 | 63 | 1 | 0 | 0 |  |  |
| 64 | 00968e80-0097aaf0 | 311 | 0.6 | 3 | 64 | 1 | 0 | 26 | callback, player, entity, message, oldlevel, newlevel, ambient | 63:54, 1:47, 28:14, 87:13, 20:10, 43:10; +1 more in JSON |
| 65 | 0097ab70-0098c510 | 162 | 0.6 | 4 | 65 | 1 | 0 | 3 | repair, player, shiplanded, musicover, hpevent, generate, entitykilled | 64:158, 55:93, 1:39, 33:32, 29:24, 63:20; +9 more in JSON |
| 66 | 0098c630-00996060 | 150 | 0.52 | 1 | 66 | 1 | 0 | 2 | software, bsm_hwd, eidos, mpkg, cast, content, language | 0:11 |
| 67 | 00996120-009f6060 | 840 | 0.81 | 2 | 1 | 83 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:400, 47:266, 2:234, 68:167, 95:123, 0:111; +19 more in JSON |
| 68 | 009f6090-009fe120 | 96 | 0.71 | 2 | 1 | 83 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:36, 38:19, 48:15, 2:13, 87:13, 28:13; +2 more in JSON |
| 69 | 009fe130-00a16030 | 240 | 0.6 | 2 | 1 | 83 | 0 | 5 | commandtype, neutral, aivstable_, enemy, vehicle, class, target | 1:53, 70:39, 87:27, 68:15, 43:14, 3:11; +4 more in JSON |
| 70 | 00a16050-00a371a0 | 364 | 0.74 | 2 | 1 | 83 | 1 | 0 | coordinator, capture, strategicgain, reconratio, objectivemembers, autogrouping, unittypes | 1:57, 69:57, 87:29, 9:24, 28:18, 61:10; +1 more in JSON |
| 71 | 00a371c0-00a46930 | 224 | 0.33 | 2 | 1 | 83 | 0 | 8 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 72:43, 87:19, 9:15, 43:15, 66:12, 1:11; +1 more in JSON |
| 72 | 00a46b80-00a625d0 | 109 | 0.41 | 2 | 1 | 83 | 0 | 1 | online, server, player, query, matchmaking, remote, movie | 71:23, 19:9, 66:9 |
| 73 | 00a62660-00a7a3f0 | 334 | 0.63 | 2 | 1 | 83 | 0 | 1 | unexpected, chunk, precompiled, call, complex, expression, many | 74:16 |
| 74 | 00a7a440-00a9ac40 | 437 | 0.28 | 2 | 1 | 83 | 0 | 15 | memory, sounjd, sound, stream, streaming, stereo, request | 1:47, 87:35, 94:31, 88:20, 73:19, 83:19; +5 more in JSON |
| 75 | 00a9ac70-00aa0f50 | 88 | 0.55 | 2 | 1 | 83 | 0 | 1 | heightplus, dontmovetheitems, centervertical, autocontrol, lockit, linedistance, items | 77:19, 1:11, 9:9 |
| 76 | 00aa0f70-00aa6720 | 103 | 0.68 | 2 | 1 | 83 | 0 | 0 | cGuiManager, camerastore, scrollbar, safezone_43_framebox, safezone_169_framebox, progbar, mouseptrgui_icon | 78:24, 77:13, 79:12 |
| 77 | 00aa6750-00aace40 | 113 | 0.47 | 2 | 1 | 83 | 0 | 38 | widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor, label | 88:26 |
| 78 | 00aacf10-00ac9640 | 335 | 0.19 | 2 | 1 | 83 | 0 | 12 | texture, simplecolor, mvfm, mshd, guidefault, shadername, verticalalign | 77:91, 88:31, 87:25, 1:24, 9:12, 95:9 |
| 79 | 00ac97f0-00ad5360 | 188 | 0.33 | 2 | 1 | 83 | 0 | 7 | vertical_scrollbar, horizontal_scrollbar, visual_group, scalevector, rotationeuler, playbydefault, modeltextureoverride | 77:41, 78:15, 76:9 |
| 80 | 00ad5590-00ada240 | 79 | 0.65 | 2 | 1 | 83 | 0 | 0 | trees_pc, bushes_pc, bush, foliage, effects, terrain, normal | 81:22 |
| 81 | 00ada420-00af3750 | 304 | 0.38 | 2 | 1 | 83 | 0 | 7 | mshd, terrain, mvfm, visibility, group, atlas, particlefloating | 88:43, 97:26, 1:22, 87:12, 80:11, 89:8 |
| 82 | 00af37d0-00b1bd20 | 428 | 0.39 | 2 | 1 | 83 | 0 | 17 | param, emitter, additive, dynamic_light_, rotationspeed, particle, mvfm | 86:47, 88:35, 1:33, 87:30, 81:27, 83:22; +2 more in JSON |
| 83 | 00b1bf40-00b25d90 | 175 | 0.47 | 0 | 83 | 1 | 0 | 1 |  |  |
| 84 | 00b25dc0-00b402b0 | 239 | 0.23 | 2 | 1 | 83 | 0 | 7 | pf43cc, debugshader, mvfm, mshd, shaders, handmade, dsprites | 83:85, 88:19, 9:13, 86:9 |
| 85 | 00b402e0-00b4b0a0 | 122 | 0.29 | 0 | 85 | 1 | 0 | 3 | uf44uf44uf44, shadow_passtrough, shfx, default, mvfm |  |
| 86 | 00b4b490-00b65ac0 | 256 | 0.3 | 2 | 1 | 83 | 0 | 5 | mshd, cSampleOffsets, posteffectsysobj, posteffectsyscam, oldfilm_dust, cSampleWeights, mvfm | 87:67, 83:23, 88:21, 85:19, 84:15, 1:12 |
| 87 | 00b65ba0-00b6d1b0 | 133 | 0.8 | 2 | 1 | 83 | 0 | 0 | dofile, userdata, thread, lightuserdata, dobuffer, fundamentals, scripts | 73:31, 1:14 |
| 88 | 00b6d3c0-00b80ed0 | 357 | 0.55 | 2 | 1 | 83 | 0 | 32 | c3dnodeanimator, cLight, cAnimTrack, cAmbientLight, c3dnode, cPointLight, cOptimized3dNodeAnimator | 86:21, 34:15, 91:13, 89:10, 97:9 |
| 89 | 00b80fd0-00b86e10 | 99 | 0.67 | 2 | 1 | 83 | 0 | 1 | cSceneResource, resourcedump_, refcounter, cNullSceneResource | 88:21, 1:8, 83:8 |
| 90 | 00b86e40-00b8a200 | 60 | 0.65 | 3 | 90 | 1 | 0 | 2 | node | 88:8 |
| 91 | 00b8a2b0-00b999a0 | 207 | 0.54 | 2 | 1 | 83 | 0 | 10 | cRenderMeshResource, cGroupParamsResource, cGroup, flare, zoomfactor, targetname, simplecolor2 | 88:45, 34:10, 89:9 |
| 92 | 00b999f0-00bb3ce0 | 215 | 0.75 | 2 | 1 | 83 | 0 | 3 | mvfm, mshd, coast, cCorner3, cCorner2, cCorner1, cCorner0 | 88:83, 83:24, 1:23, 89:20, 82:16, 86:9; +1 more in JSON |
| 93 | 00bb3db0-00bbddd0 | 167 | 0.54 | 3 | 93 | 1 | 0 | 4 | mpak, deviceid, shorewavetexturesource2, shorewavetexturesource1, shorewavetexturesource0, open, oceanheightmap | 1:15, 97:8 |
| 94 | 00bbdec0-00bd2e60 | 210 | 0.33 | 2 | 1 | 83 | 0 | 3 | mvfm, mshd, cloud, waterdrops, thunder, streamed, lightning_002 | 88:40, 1:19, 95:8 |
| 95 | 00bd2f10-00bd9210 | 82 | 0.55 | 2 | 1 | 83 | 0 | 2 |  | 87:86, 1:20 |
| 96 | 00bd9220-00be4410 | 178 | 0.49 | 0 | 96 | 1 | 0 | 4 | nagybetu, long, startup, srch, rejected, perftime, duplicate |  |
| 97 | 00be4460-00c30570 | 343 | 0.41 | 2 | 1 | 83 | 0 | 19 | cFileStore, iterator, long, removefile, removed, profile, cShaderTextureSource | 87:15, 1:10, 96:9, 95:9 |
