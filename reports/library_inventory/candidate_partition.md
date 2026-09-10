# Candidate partition: disjoint link-order segments

23728 unnamed non-thunk FUN_ candidates grouped into 99 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 863 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004138d0 | 190 | 0.11 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413d10-0041dac0 | 152 | 0.55 | 2 | 1 | 83 | 0 | 0 | avoidzoneg, avoidzone | 12:11, 0:9 |
| 2 | 0041db10-0042a920 | 203 | 0.23 | 2 | 1 | 83 | 0 | 0 | mpakscenes, endgroup, crash, training, terraingridlayer, terraingrid, modes | 1:67, 0:18, 86:9 |
| 3 | 0042a930-0043e840 | 264 | 0.42 | 2 | 1 | 83 | 0 | 14 | bsp_chk_save, unlockto, unlockname, unlockfrom, playtime, difficulty, united | 86:147, 2:33, 1:28, 85:27, 30:25, 95:17; +5 more in JSON |
| 4 | 0043e8f0-004484f0 | 161 | 0.35 | 2 | 1 | 83 | 0 | 3 | deviceclass, daytime, device, platform, weatherreconmodifiers, weatherampmultipliers, viewmodeampmultipliers | 86:52, 52:10, 39:8 |
| 5 | 004486a0-00453260 | 178 | 0.69 | 2 | 1 | 83 | 0 | 1 | panel, sequence, message, callback, suppressinterruptmsg, setpanel, requesttime | 86:38, 2:15, 3:8 |
| 6 | 00453710-00465350 | 148 | 0.53 | 2 | 1 | 83 | 0 | 1 | action, cameraposition, activatetime, aa_flak, alpha, point, comment | 7:41, 2:18, 53:15, 86:12 |
| 7 | 00465410-00485520 | 564 | 0.2 | 2 | 1 | 83 | 0 | 16 | groupname, scenebrowsergroups, s_ls_, cloudclass, browsergroup, properties, groups | 86:37, 8:33, 57:30, 53:29, 58:27, 3:25; +5 more in JSON |
| 8 | 00485690-00491070 | 217 | 0.34 | 2 | 1 | 83 | 0 | 1 | weight, soldiertypes, landvehicleclasses, entity, outingdelay, mininrow, maxinrow | 7:78, 86:14, 58:11, 57:10, 2:9, 51:9; +1 more in JSON |
| 9 | 00491170-004990b0 | 81 | 0.53 | 2 | 1 | 83 | 0 | 0 |  | 8:68, 7:51 |
| 10 | 00499140-004ba170 | 454 | 0.16 | 2 | 1 | 83 | 0 | 7 | soldiertypes, landvehicleclasses, wreckclass, tempid, smokeefx, rotationdecline, gravitymul | 86:97, 8:82, 9:78, 7:40, 3:14, 51:14; +7 more in JSON |
| 11 | 004ba200-004d0860 | 396 | 0.27 | 2 | 1 | 83 | 0 | 1 | white, allbutingame, collect, collectgarbage, interface, textures, writestats | 10:40, 30:25, 2:25, 74:16, 12:9, 94:9 |
| 12 | 004d0920-004fa300 | 458 | 0.34 | 2 | 1 | 83 | 1 | 11 | ggame, collect, collectgarbage, party, ingame, universe, scene | 11:256, 2:92, 7:57, 57:54, 10:50, 86:50; +19 more in JSON |
| 13 | 004fa3d0-00519fe0 | 264 | 0.42 | 2 | 1 | 83 | 0 | 1 | menuitem_text, vehicleclass, back, dview, navigate, globals, scroll_menu | 86:141, 76:58, 2:56, 12:47, 30:29, 75:29; +10 more in JSON |
| 14 | 0051a0e0-0051f7e0 | 76 | 0.5 | 2 | 1 | 83 | 0 | 0 | siege_group, island_capture_group, ijn05, fe_briefing_listbox, escort_group, duel_group, competitive_group | 11:21, 76:11, 12:10, 3:9, 74:8 |
| 15 | 0051f830-00527c30 | 91 | 0.75 | 2 | 1 | 83 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 30:28, 40:14, 11:9, 14:9, 1:9, 17:8 |
| 16 | 00527c80-005439b0 | 309 | 0.5 | 2 | 1 | 83 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 76:75, 11:44, 12:37, 2:30, 78:29, 30:20; +8 more in JSON |
| 17 | 00543a30-00554600 | 224 | 0.33 | 2 | 1 | 83 | 0 | 2 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 30:32, 12:26, 11:24, 76:22, 2:18, 16:17; +4 more in JSON |
| 18 | 00554680-0056a590 | 314 | 0.35 | 2 | 1 | 83 | 0 | 1 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 75:45, 17:40, 76:38, 30:32, 2:31, 86:29; +6 more in JSON |
| 19 | 0056a5a0-0057ff00 | 279 | 0.57 | 2 | 1 | 83 | 0 | 1 | globals, server_text, players_text, mode_text, scroll_right_icon, wave_icon, setting_2_text | 18:75, 2:34, 76:28, 75:21, 30:21, 78:18; +2 more in JSON |
| 20 | 0057ff80-005c51f0 | 673 | 0.51 | 2 | 1 | 83 | 0 | 3 | globals, navigate, back, mission_mappoint_, select, main_newprofile, bushgroup | 76:99, 2:85, 78:57, 19:46, 30:34, 3:31; +16 more in JSON |
| 21 | 005c5470-005cce70 | 117 | 0.41 | 2 | 1 | 83 | 0 | 0 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, uniquemultisettings | 20:16, 86:16, 12:15, 11:10, 19:8, 76:8 |
| 22 | 005cd070-005df510 | 171 | 0.76 | 2 | 1 | 83 | 0 | 1 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 11:59, 2:37, 12:31, 76:31, 40:26, 30:20; +8 more in JSON |
| 23 | 005e09a0-0060cb40 | 325 | 0.69 | 2 | 1 | 83 | 0 | 6 | globals, back, xsm_saveconfirm, fe_xbox, mainlistbox_text, select, main_listbox | 2:115, 76:104, 11:96, 12:95, 30:90, 75:80; +18 more in JSON |
| 24 | 0060cb60-0061f630 | 210 | 0.43 | 2 | 1 | 83 | 0 | 2 | usn_point_text, usn_icon, radar_sweep, pumpermanent, player_point_text, circle_full_, section | 76:46, 12:28, 11:21, 78:13, 29:13, 30:11; +1 more in JSON |
| 25 | 0061f6d0-00644220 | 231 | 0.23 | 2 | 1 | 83 | 0 | 1 | globals, rank_icon, continue, type_icon, scoring_unlock_text, score_text, playerreview | 24:62, 2:46, 76:41, 86:28, 20:25, 12:19; +9 more in JSON |
| 26 | 00644240-00650b00 | 117 | 0.54 | 2 | 1 | 83 | 0 | 4 | ingame, vehicleclass, shipcaptain, ship_torpedo_num2_icon, ship_torpedo_num1_icon, ship_torpedo_icon, ship_stick_icon | 30:36, 11:22, 76:21, 40:20, 12:16, 2:16; +6 more in JSON |
| 27 | 00650b90-00659760 | 116 | 0.43 | 2 | 1 | 83 | 0 | 1 | sub_depth_arrow_icon, sub_depht_arrow_dest_icon, sub_air_warning_icon, sub_air_arrow_icon, periscopeemerge, periscopedrops, periscope_model | 30:15, 2:9, 40:9 |
| 28 | 00659780-0065cea0 | 89 | 0.62 | 2 | 1 | 83 | 0 | 0 |  | 27:34, 30:10 |
| 29 | 0065ced0-00680d30 | 237 | 0.81 | 2 | 1 | 83 | 0 | 2 | circle_hl_icon, ingamegui, normal, icon_l_icon, globals, sm_cp, ingame | 28:98, 27:62, 76:56, 2:55, 11:45, 20:43; +14 more in JSON |
| 30 | 00680db0-00696450 | 266 | 0.55 | 2 | 1 | 83 | 0 | 7 | gvmultimenu, ingame, pushrequestinterface, interface, textures, warning_2_text, missionunique | 12:84, 86:53, 11:51, 2:34, 23:33, 76:19; +8 more in JSON |
| 31 | 00696470-006be980 | 501 | 0.39 | 2 | 1 | 83 | 0 | 0 | savedata, scoring, sensitivitysettings, inputsettings, devicetype, deviceidx, slider | 86:390, 2:94, 18:69, 4:28, 17:25, 40:21; +13 more in JSON |
| 32 | 006beb10-006d6450 | 301 | 0.59 | 2 | 1 | 83 | 0 | 18 | equipment, state, slots, runwaywidth, runwaylength, runwayfailure, hangarfailure | 30:81, 31:73, 1:66, 40:37, 3:30, 2:27; +9 more in JSON |
| 33 | 006d6470-006d8ac0 | 72 | 0.53 | 0 | 33 | 1 | 0 | 0 |  |  |
| 34 | 006d8b00-006deff0 | 98 | 0.67 | 3 | 34 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers, markerclasses | 33:58, 86:10 |
| 35 | 006df170-006fa540 | 325 | 0.37 | 2 | 1 | 83 | 1 | 28 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 86:72, 2:70, 40:45, 30:43, 39:40, 1:37; +15 more in JSON |
| 36 | 006fa6d0-00714030 | 345 | 0.29 | 2 | 1 | 83 | 0 | 15 | openaftertime, messages, mvfm, whosaysthat, velocity, unitlist, unitid | 86:48, 35:43, 30:26, 2:25, 3:25, 58:24; +10 more in JSON |
| 37 | 00714060-0071b710 | 104 | 0.52 | 0 | 37 | 1 | 0 | 3 | sphere, mzonedesc, mnote, identifier, armor, points, category |  |
| 38 | 0071ba20-00728f20 | 212 | 0.43 | 2 | 1 | 83 | 0 | 15 | startmode, followmode, userpath, pathfollowparams, pathcursor, mgeommesh, internalclearprimarycommand | 40:47, 2:44, 1:17, 15:14, 44:12, 51:9; +2 more in JSON |
| 39 | 00728fa0-00758eb0 | 491 | 0.32 | 2 | 1 | 83 | 0 | 40 | destroyed, speed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed | 86:89, 2:87, 40:74, 51:34, 61:32, 1:31; +22 more in JSON |
| 40 | 00758f90-00782840 | 715 | 0.61 | 2 | 1 | 83 | 0 | 28 | p2p_voice__, reconlevel, multiscore, mmultiplayer, lastbanto, player, recondata | 2:700, 41:71, 86:64, 58:38, 11:34, 22:21; +10 more in JSON |
| 41 | 00782870-00786a80 | 73 | 0.47 | 2 | 1 | 83 | 0 | 5 |  | 40:9, 2:8 |
| 42 | 00786be0-0078d850 | 77 | 0.44 | 2 | 1 | 83 | 0 | 0 | send_, recv_, time, client, server | 89:27, 92:25 |
| 43 | 0078d880-007a49a0 | 243 | 0.41 | 2 | 1 | 83 | 0 | 0 | postype, camera, thetalinearblend, theta, rholinearblend, blendtime, initialization | 86:61, 1:61, 3:46, 30:38, 2:38, 21:11; +4 more in JSON |
| 44 | 007a4b60-007b2d80 | 130 | 0.6 | 2 | 1 | 83 | 0 | 8 | simple, pathid, pathbaseentity, paratrooper, soldieranim, slowfactoropened, slowfactorclosed | 43:79, 1:47, 3:28, 86:26, 40:21, 85:20; +5 more in JSON |
| 45 | 007b2dd0-007d1d30 | 341 | 0.67 | 2 | 1 | 83 | 0 | 13 | powerlost, explosion, splash, enginefire, rightspinning, pathpoints, leftspinning | 1:70, 3:70, 46:70, 40:57, 2:39, 58:34; +12 more in JSON |
| 46 | 007d1dc0-007f8100 | 282 | 0.66 | 2 | 1 | 83 | 1 | 14 | travelspeed, gears, baydoor, wings, state, windsound, timeout | 1:73, 45:54, 3:51, 86:39, 40:38, 51:36; +11 more in JSON |
| 47 | 007f8180-0080d9b0 | 274 | 0.22 | 2 | 1 | 83 | 0 | 2 | enemy, neutral, unlocks, selectedmissionid, selecteddifficulty, seenunlocks, savedlobbyfilters | 86:60, 3:31, 11:23, 30:18, 2:13, 58:12; +6 more in JSON |
| 48 | 0080da00-0081aa10 | 163 | 0.61 | 2 | 1 | 83 | 0 | 38 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 40:52, 61:35, 1:24, 2:22, 58:19, 38:18; +5 more in JSON |
| 49 | 0081aa60-00828810 | 96 | 0.44 | 2 | 1 | 83 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 48:77, 2:20, 1:18, 58:18, 51:17, 86:16; +6 more in JSON |
| 50 | 00828870-00859410 | 494 | 0.33 | 2 | 1 | 83 | 2 | 15 | gameunit, classid, stock, torpedo, torpedoavoidance, object, submarine | 86:75, 40:48, 2:41, 7:40, 38:40, 1:39; +22 more in JSON |
| 51 | 00859550-00877e50 | 407 | 0.35 | 2 | 1 | 83 | 0 | 8 | radius, effects, tvertangle, turninggun, thorzangle, minlifetime, maxlifetime | 86:100, 3:31, 1:27, 92:21, 40:20, 87:18; +13 more in JSON |
| 52 | 00877fa0-0088b550 | 253 | 0.21 | 2 | 1 | 83 | 0 | 2 | damage, scripts, debugtrap, datatables, yellow, weaponsystems, weapondirectorthinktime | 86:79, 31:51, 2:31, 51:27, 35:21, 40:12; +2 more in JSON |
| 53 | 0088b5a0-008dd820 | 731 | 0.76 | 2 | 1 | 83 | 528 | 0 | luakod, options, hardwarereported, english, xboxcompatibilitymode, vsync, texturedetail | 86:3870, 2:609, 52:438, 40:112, 31:62, 58:51; +29 more in JSON |
| 54 | 008dd8c0-008e3d50 | 96 | 0.52 | 2 | 1 | 83 | 0 | 1 | szurkenyil, objectivelist, objectiveentities, secobjprefix, pinged, missionglobals, flagprocess | 53:55, 2:21, 40:17, 30:9 |
| 55 | 008e3de0-008e6d30 | 72 | 0.43 | 3 | 55 | 1 | 0 | 1 | unitclassindex, mapiconpicture, equipmentindex, description | 2:30 |
| 56 | 008e6e00-008ee020 | 86 | 0.55 | 4 | 56 | 1 | 0 | 1 | pup_gain, pum1stget, uspumicon, uselimit, targettype, targetfilter, random | 55:48, 86:25, 2:18, 24:15, 40:12, 30:10; +1 more in JSON |
| 57 | 008ee5f0-00905340 | 359 | 0.38 | 2 | 1 | 83 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 3:77, 86:44, 1:31, 53:29, 7:29, 2:29; +9 more in JSON |
| 58 | 00905350-0092dfe0 | 403 | 0.41 | 2 | 1 | 83 | 0 | 79 | party, scoring, entity, winnermode, usedslot, unit_usage, unit_suicide | 86:90, 2:71, 20:65, 13:43, 19:39, 25:24; +12 more in JSON |
| 59 | 0092e0b0-00941c20 | 197 | 0.34 | 2 | 1 | 83 | 0 | 0 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 58:35, 87:29, 2:26, 1:15, 51:15, 3:13 |
| 60 | 00941d30-00951f20 | 163 | 0.44 | 2 | 1 | 83 | 0 | 2 | ownerplayer, resourceusage, supportmanager, velocitysi, effect, camocolor, autoattacktarget | 86:69, 57:36, 2:25, 59:13, 1:12, 11:12; +4 more in JSON |
| 61 | 00951f40-00968e00 | 275 | 0.27 | 2 | 1 | 83 | 0 | 37 | vehicleclass, inferiorfailure, reconplane, torpedobomber, torpedoboat, divebomber, cargo | 86:74, 3:43, 2:32, 39:31, 4:20, 1:20; +11 more in JSON |
| 62 | 00968e80-0097c8a0 | 341 | 0.53 | 3 | 62 | 1 | 0 | 26 | callback, player, entity, message, oldlevel, newlevel, ambient | 61:61, 2:47, 30:14, 86:13, 20:10, 40:10; +1 more in JSON |
| 63 | 0097c9b0-0098c510 | 132 | 0.55 | 5 | 63 | 1 | 0 | 3 | repair, player, shiplanded, musicover, hpevent, generate, entitykilled | 62:155, 52:93, 2:34, 35:32, 31:24, 61:20; +7 more in JSON |
| 64 | 0098c630-00996060 | 150 | 0.52 | 1 | 64 | 1 | 0 | 2 | software, bsm_hwd, eidos, mpkg, cast, content, language | 0:11 |
| 65 | 00996120-009f6060 | 840 | 0.9 | 2 | 1 | 83 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:373, 45:266, 3:234, 66:167, 95:123, 0:111; +20 more in JSON |
| 66 | 009f6090-00a00370 | 139 | 0.62 | 2 | 1 | 83 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:33, 38:19, 46:17, 45:15, 3:13, 86:13; +2 more in JSON |
| 67 | 00a00420-00a079b0 | 111 | 0.63 | 2 | 1 | 83 | 0 | 0 | vehicle, class, captureweight | 66:38 |
| 68 | 00a07a60-00a19390 | 130 | 0.65 | 2 | 1 | 83 | 0 | 5 | commandtype, neutral, aivstable_, enemy, target, vehicleclass, s_to_ | 67:45, 70:38, 1:31, 86:25, 66:24, 2:15; +7 more in JSON |
| 69 | 00a19410-00a1fac0 | 129 | 0.45 | 2 | 1 | 83 | 0 | 0 | coordinator, sell, strategicgain, duel, defend, capture, siege | 70:19, 2:10, 1:9 |
| 70 | 00a1fba0-00a3da20 | 272 | 0.28 | 2 | 1 | 83 | 1 | 4 | client, online, player, cNetworkClient, network, reconratio, objectivemembers | 69:73, 68:57, 86:46, 11:26, 66:20, 1:19; +5 more in JSON |
| 71 | 00a3db10-00a41c70 | 64 | 0.56 | 2 | 1 | 83 | 0 | 2 | xenonsystemmanager, online__, changestate, succeed, queue, mnetworkclientxlive, write | 70:29, 41:10 |
| 72 | 00a41ce0-00a625d0 | 188 | 0.49 | 3 | 72 | 1 | 0 | 3 | online, server, player, query, matchmaking, remote, movie | 64:21, 71:17, 11:15, 70:9, 19:9, 30:8 |
| 73 | 00a62660-00a7a2e0 | 333 | 0.63 | 2 | 1 | 83 | 0 | 1 | unexpected, chunk, precompiled, call, complex, expression, many | 74:16 |
| 74 | 00a7a3f0-00a9bb30 | 459 | 0.28 | 2 | 1 | 83 | 0 | 15 | memory, sounjd, sound, stream, streaming, stereo, request | 2:43, 86:35, 94:31, 82:20, 87:20, 97:16; +6 more in JSON |
| 75 | 00a9bb70-00aa0f50 | 67 | 0.61 | 2 | 1 | 83 | 0 | 1 | lockit, scrollright_icon, scrollleft_icon, newhighlightindex, heightplus, dontmovetheitems, centervertical | 76:18, 2:10, 11:9, 74:9 |
| 76 | 00aa0f70-00aacf10 | 217 | 0.53 | 2 | 1 | 83 | 0 | 38 | cGuiManager, widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor | 78:23, 87:23, 79:14, 2:8, 88:8 |
| 77 | 00aad040-00ab0520 | 60 | 0.62 | 3 | 77 | 1 | 0 | 0 | moviename, subtitleswidescreen_text, subtitlesnormal_text, extratitles, subtitles, guidefault, english | 76:13, 86:11 |
| 78 | 00ab0eb0-00ac9640 | 274 | 0.42 | 2 | 1 | 83 | 0 | 12 | texture, simplecolor, mvfm, mshd, shadername, guidefault, verticalalign | 76:82, 87:26, 2:19, 86:14, 88:12, 11:11; +1 more in JSON |
| 79 | 00ac97f0-00ad55c0 | 190 | 0.35 | 2 | 1 | 83 | 0 | 7 | vertical_scrollbar, horizontal_scrollbar, visual_group, scalevector, rotationeuler, playbydefault, modeltextureoverride | 76:50, 78:15 |
| 80 | 00ad56c0-00ada420 | 78 | 0.59 | 2 | 1 | 83 | 0 | 0 | trees_pc, bushes_pc, bush, foliage, effects, terrain, normal | 81:22 |
| 81 | 00ada5a0-00b1bc70 | 731 | 0.33 | 2 | 1 | 83 | 0 | 24 | param, mvfm, mshd, emitter, terrain, particlefloating, additive | 87:59, 85:49, 2:47, 86:33, 97:29, 88:27; +9 more in JSON |
| 82 | 00b1bd20-00b20e90 | 101 | 0.71 | 2 | 1 | 83 | 0 | 0 | instanced | 86:9, 85:8 |
| 83 | 00b20ec0-00b402b0 | 332 | 0.33 | 2 | 1 | 83 | 0 | 8 | pf43cc, debugshader, mvfm, mshd, shaders, handmade, dsprites | 87:19, 82:18, 11:13, 85:11 |
| 84 | 00b402e0-00b4b0a0 | 122 | 0.29 | 0 | 84 | 1 | 0 | 3 | uf44uf44uf44, shadow_passtrough, shfx, default, mvfm |  |
| 85 | 00b4b490-00b659d0 | 259 | 0.29 | 2 | 1 | 83 | 0 | 5 | mshd, cSampleOffsets, posteffectsysobj, posteffectsyscam, oldfilm_dust, cSampleWeights, mvfm | 86:67, 82:23, 87:21, 84:19, 83:15, 2:12; +1 more in JSON |
| 86 | 00b65ac0-00b6d1b0 | 134 | 0.74 | 2 | 1 | 83 | 0 | 0 | dofile, userdata, thread, lightuserdata, dobuffer, fundamentals, scripts | 73:31, 2:14 |
| 87 | 00b6d3c0-00b734d0 | 127 | 0.58 | 2 | 1 | 83 | 0 | 31 | c3dnode, cCamera | 85:20, 91:8 |
| 88 | 00b73800-00b80fd0 | 233 | 0.52 | 2 | 1 | 83 | 0 | 1 | c3dnodeanimator, cLight, cDummy, cAnimTrack, cAmbientLight, c3dobject, cPointLight | 87:25, 36:13 |
| 89 | 00b81000-00b86e10 | 99 | 0.68 | 2 | 1 | 83 | 0 | 1 | cSceneResource, resourcedump_, refcounter, cNullSceneResource | 88:14, 2:8, 82:8 |
| 90 | 00b86e40-00b8c3a0 | 92 | 0.42 | 3 | 90 | 1 | 0 | 7 | zoomfactor, targetname, node, dissolve, channelanimation, cCameraResource, cBoneResource | 88:13 |
| 91 | 00b8c430-00b999f0 | 176 | 0.5 | 2 | 1 | 83 | 0 | 5 | cRenderMeshResource, cGroupParamsResource, cGroup, flare, simplecolor2, quadtreenode, flare2 | 87:25, 88:19, 36:9, 89:8 |
| 92 | 00b99bf0-00bb3ce0 | 214 | 0.77 | 2 | 1 | 83 | 0 | 3 | mvfm, mshd, coast, cCorner3, cCorner2, cCorner1, cCorner0 | 87:71, 88:26, 82:24, 89:20, 1:17, 81:17; +3 more in JSON |
| 93 | 00bb3db0-00bbdec0 | 168 | 0.51 | 3 | 93 | 1 | 0 | 4 | mpak, deviceid, shorewavetexturesource2, shorewavetexturesource1, shorewavetexturesource0, open, oceanheightmap | 2:14 |
| 94 | 00bbdef0-00bd26b0 | 204 | 0.33 | 2 | 1 | 83 | 0 | 3 | mvfm, mshd, cloud, waterdrops, thunder, lightning_002, dynamic_light_3dlightning | 88:25, 87:23, 1:12, 95:8 |
| 95 | 00bd2760-00bd93e0 | 92 | 0.59 | 2 | 1 | 83 | 0 | 2 | streamed, fatal, line, expression, couldn, debug | 86:86, 2:27 |
| 96 | 00bd94d0-00be4410 | 173 | 0.44 | 0 | 96 | 1 | 0 | 4 | nagybetu, long, startup, srch, rejected, perftime, duplicate |  |
| 97 | 00be4460-00bf0c60 | 253 | 0.42 | 2 | 1 | 83 | 0 | 16 | cFileStore, removefile, removed, profile, iterator, remove, long | 95:9 |
| 98 | 00bf0df0-00c30570 | 90 | 0.51 | 3 | 98 | 1 | 0 | 3 | cShaderTextureSource, cPhysicalDirectoryX86, cFileX86, getfiledate, getfileattributesex, cShaderAnimTextureSource, assertion | 86:15 |
