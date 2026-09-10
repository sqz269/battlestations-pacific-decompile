# Candidate partition: disjoint link-order segments

23887 unnamed non-thunk FUN_ candidates grouped into 94 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 853 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004138d0 | 191 | 0.07 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413d10-0041db10 | 154 | 0.56 | 1 | 1 | 89 | 0 | 0 | avoidzoneg, avoidzone | 11:11, 0:9 |
| 2 | 0041dd20-0042a480 | 201 | 0.25 | 1 | 1 | 89 | 0 | 0 | mpakscenes, endgroup, crash, training, terraingridlayer, terraingrid, modes | 1:84, 91:31, 0:20, 87:9 |
| 3 | 0042a7e0-004486a0 | 430 | 0.31 | 1 | 1 | 89 | 0 | 17 | bsp_chk_save, deviceclass, daytime, unlockto, unlockname, unlockfrom, playtime | 87:206, 1:72, 2:65, 91:51, 86:27, 29:25; +8 more in JSON |
| 4 | 004486c0-0044c390 | 76 | 0.49 | 1 | 1 | 89 | 0 | 1 | panel, callback, setpanel, hidepanel, character, pause, message | 2:13, 1:9, 91:9 |
| 5 | 0044c5a0-00453f40 | 108 | 0.55 | 1 | 1 | 89 | 0 | 0 | dialogues, sequence, dialogdefaultpausetime, dialogcharacters, suppressinterruptmsg, requesttime, panelstates | 4:54, 87:37, 2:24, 3:20, 91:17, 1:16 |
| 6 | 00454a50-00468600 | 237 | 0.38 | 1 | 1 | 89 | 0 | 1 | action, cameraposition, activatetime, aa_flak, alpha, point, periscope | 2:45, 1:35, 91:32, 58:25, 87:14, 3:11 |
| 7 | 00468660-0047c2a0 | 333 | 0.35 | 1 | 1 | 89 | 0 | 2 | scenebrowsergroups, cloudclass, browsergroup, properties, spawnpoint, hidden, entity | 91:63, 2:57, 1:56, 87:44, 6:39, 3:34; +4 more in JSON |
| 8 | 0047c4d0-0048f670 | 349 | 0.37 | 1 | 1 | 89 | 0 | 15 | weight, soldiertypes, landvehicleclasses, entity, outingdelay, mininrow, maxinrow | 87:23, 1:21, 2:17, 91:17, 61:16, 9:13; +4 more in JSON |
| 9 | 0048f930-004ba080 | 535 | 0.2 | 1 | 1 | 89 | 0 | 7 | soldiertypes, landvehicleclasses, wreckclass, tempid, smokeefx, rotationdecline, gravitymul | 8:224, 87:104, 3:53, 1:44, 91:44, 2:41; +5 more in JSON |
| 10 | 004ba0a0-004c81f0 | 275 | 0.32 | 1 | 1 | 89 | 0 | 1 | white, collect, collectgarbage, traininggrounds, cloudsmall, multi, cloud | 9:37, 91:17, 2:17, 29:15, 1:12, 78:9 |
| 11 | 004c8260-004f8830 | 550 | 0.39 | 1 | 1 | 89 | 1 | 11 | ggame, collect, collectgarbage, ingame, party, textures, universe | 10:181, 2:161, 91:91, 1:83, 87:59, 9:54; +27 more in JSON |
| 12 | 004f8970-00506f00 | 209 | 0.58 | 1 | 1 | 89 | 0 | 0 | menuitem_text, vehicleclass, unitlib_nounlock, back, globals, dview, change | 2:66, 87:57, 79:45, 91:45, 1:41, 80:15; +5 more in JSON |
| 13 | 00506f80-0051e4d0 | 151 | 0.53 | 1 | 1 | 89 | 0 | 1 | back, dview, scroll_menu, globals, navigate, vehicleclass, szarnyas_framebox | 12:133, 87:102, 2:73, 91:48, 79:48, 1:41; +6 more in JSON |
| 14 | 0051e650-00527c80 | 111 | 0.73 | 1 | 1 | 89 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 29:31, 10:18, 1:14, 11:13, 3:12, 2:10; +2 more in JSON |
| 15 | 00527cb0-00544b60 | 336 | 0.36 | 1 | 1 | 89 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 79:90, 2:69, 1:49, 11:44, 91:43, 10:39; +8 more in JSON |
| 16 | 00544e60-00558680 | 284 | 0.23 | 1 | 1 | 89 | 0 | 3 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 2:37, 29:36, 11:36, 1:34, 91:30, 79:26; +5 more in JSON |
| 17 | 005586b0-00568930 | 189 | 0.27 | 1 | 1 | 89 | 0 | 0 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 16:76, 79:74, 2:49, 87:29, 29:28, 1:27; +7 more in JSON |
| 18 | 00568cb0-0056f320 | 102 | 0.65 | 1 | 1 | 89 | 0 | 1 | globals, setting_2_text, nike_icon, cucc_group, setting_var_text, setting_value_text, tilt_icon | 17:31, 79:19, 2:18 |
| 19 | 00570300-0058d3f0 | 400 | 0.29 | 1 | 1 | 89 | 0 | 2 | globals, back, select, navigate, server_text, players_text, mode_text | 2:83, 79:77, 18:71, 1:39, 91:39, 81:25; +13 more in JSON |
| 20 | 0058d430-0059e9d0 | 168 | 0.45 | 1 | 1 | 89 | 0 | 0 | globals, mission_mappoint_, main_checkpoint_dlc, main_checkpoint, checkpoint_available, objective, hidden | 19:105, 2:42, 79:29, 1:15, 91:15, 3:12; +4 more in JSON |
| 21 | 0059ea20-005ce9d0 | 465 | 0.25 | 1 | 1 | 89 | 0 | 1 | bushgroup, visibility, terrain, message, group, datatables, scripts | 2:64, 87:59, 1:55, 20:52, 11:45, 91:44; +14 more in JSON |
| 22 | 005cea60-005df400 | 143 | 0.73 | 1 | 1 | 89 | 0 | 1 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 2:62, 79:50, 10:39, 11:35, 1:30, 91:30; +5 more in JSON |
| 23 | 005df510-005ee440 | 77 | 0.62 | 2 | 23 | 1 | 0 | 3 | globals, navigate, back, achievement_lock_icon, players_listbox, rank_icon, change | 22:78, 79:75, 2:58, 29:45, 19:31, 10:30; +8 more in JSON |
| 24 | 005ee4a0-005fd180 | 123 | 0.63 | 1 | 1 | 89 | 0 | 2 | xsm_saveconfirm, fe_xbox, xsm_dlcchanged, arrow_left_icon, arrow_right_icon, slider0, main_text | 2:85, 79:61, 1:47, 91:45, 58:32, 11:26; +6 more in JSON |
| 25 | 005fd190-006049f0 | 62 | 0.66 | 1 | 1 | 89 | 0 | 0 | ingame, felkialtojel_text, aaaaaa, paused, title_group, silverline_framebox, secondary_objectives_text | 11:49, 79:37, 2:33, 24:24, 1:17, 91:17; +3 more in JSON |
| 26 | 00604a20-0060de90 | 87 | 0.51 | 1 | 1 | 89 | 0 | 1 | turbo_group, turbo_effect, ship_speed_num3_icon, ship_payload_2_icon, ship_payload_1_icon, repairzone_text, pleasewait | 3:41, 48:36, 11:31, 10:26, 1:25, 29:23; +3 more in JSON |
| 27 | 0060dfb0-00622990 | 204 | 0.39 | 1 | 1 | 89 | 0 | 2 | showgamercard, globals, usn_point_text, usn_icon, radar_sweep, pumpermanent, playerreview | 2:42, 79:41, 91:27, 1:26, 29:22, 87:15; +5 more in JSON |
| 28 | 00622a10-00654650 | 364 | 0.22 | 1 | 1 | 89 | 0 | 6 | ingame, globals, type_icon, scoring_unlock_text, score_text, medal_icon, commandbuilding_icon | 2:100, 27:83, 1:60, 79:57, 29:53, 11:43; +21 more in JSON |
| 29 | 00654a70-006980e0 | 708 | 0.3 | 1 | 1 | 89 | 0 | 9 | circle_hl_icon, normal, ingame, ingamegui, gvmultimenu, icon_l_icon, globals | 2:160, 11:150, 1:88, 91:84, 79:83, 10:68; +25 more in JSON |
| 30 | 006981c0-0069fe70 | 133 | 0.4 | 1 | 1 | 89 | 0 | 0 | swapstickpairs, swapstickmap, swapstickgeneral, press, invio, inputmodifiers, indietro | 87:17, 2:17, 29:14, 1:8, 91:8 |
| 31 | 0069ff40-006a7be0 | 120 | 0.57 | 1 | 1 | 89 | 0 | 0 | sensitivitysettings, inputsettings, devicetype, deviceidx, slider, reverse, sensitivities | 30:50, 16:27, 17:26, 3:21, 87:20, 91:12; +3 more in JSON |
| 32 | 006a9850-006c0240 | 257 | 0.42 | 1 | 1 | 89 | 0 | 0 | savedata, scoring, entities, missionid, entidcont, cont1, cont0 | 87:349, 2:112, 1:63, 91:50, 3:35, 31:20; +12 more in JSON |
| 33 | 006c02f0-006ca770 | 107 | 0.52 | 1 | 1 | 89 | 0 | 0 | equipment, state, slots, classid, stock, waitingpos, targetid | 32:99, 29:52, 1:33, 3:18, 44:9, 2:8 |
| 34 | 006ca8e0-006d6450 | 152 | 0.47 | 1 | 1 | 89 | 0 | 18 | runwaywidth, runwaylength, readyplane, loaddelay, justliftedupid, runwayfailure, hangarfailure | 33:46, 32:43, 1:35, 29:34, 2:22, 3:14; +7 more in JSON |
| 35 | 006d6470-006deac0 | 165 | 0.5 | 2 | 35 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers, markerclasses | 1:16, 91:16, 2:13, 87:11, 3:8 |
| 36 | 006dec70-006fa6d0 | 331 | 0.38 | 1 | 1 | 89 | 1 | 28 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 87:94, 2:76, 1:60, 29:43, 41:41, 3:38; +14 more in JSON |
| 37 | 006fac20-00717980 | 394 | 0.23 | 1 | 1 | 89 | 0 | 14 | openaftertime, messages, mvfm, whosaysthat, velocity, unitlist, unitid | 87:74, 2:54, 1:47, 36:42, 3:40, 91:36; +8 more in JSON |
| 38 | 00717c70-0071b940 | 79 | 0.62 | 2 | 38 | 1 | 0 | 1 | mzonedesc, sphere, identifier, mnote, armor, points, category | 37:58, 90:12, 93:11 |
| 39 | 0071b9b0-00722fb0 | 123 | 0.64 | 1 | 1 | 89 | 0 | 5 | startmode, followmode, userpath, pathfollowparams, pathcursor, internalclearprimarycommand, emptycommand | 2:36, 44:21, 43:21, 47:14, 14:14, 1:12; +1 more in JSON |
| 40 | 00723030-007290d0 | 92 | 0.5 | 1 | 1 | 89 | 0 | 9 | mgeommesh | 39:13, 2:13, 43:8 |
| 41 | 007292c0-0074b780 | 401 | 0.26 | 1 | 1 | 89 | 0 | 29 | destroyed, speed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed | 2:91, 87:85, 91:63, 1:58, 3:34, 43:27; +13 more in JSON |
| 42 | 0074ba20-00758e40 | 89 | 0.49 | 1 | 1 | 89 | 0 | 10 | sumleaks, sumforces, iswater, water, soldierbox, rocketer, rampa | 2:20, 87:17, 64:14, 43:13, 1:12, 50:11; +2 more in JSON |
| 43 | 00758eb0-0076a8a0 | 347 | 0.87 | 1 | 1 | 89 | 0 | 5 | runwaycenter, maxlandingplanesonboard, liftexitpoint, deckcamera, carrierescort, elevator_2, vertangle | 2:651, 1:12, 91:12, 34:10, 87:9 |
| 44 | 0076a940-007866b0 | 440 | 0.33 | 1 | 1 | 89 | 0 | 28 | p2p_voice__, reconlevel, multiscore, mmultiplayer, lastbanto, player, recondata | 43:101, 2:81, 87:56, 1:36, 91:36, 10:27; +13 more in JSON |
| 45 | 007868c0-0078cf20 | 68 | 0.47 | 2 | 45 | 1 | 0 | 0 | send_, recv_, time, client, server | 91:30, 89:13, 2:10 |
| 46 | 0078cff0-007a42c0 | 250 | 0.41 | 1 | 1 | 89 | 0 | 0 | camera, thetalinearblend, theta, rholinearblend, blendtime, postype, initialization | 1:79, 87:57, 3:53, 2:44, 29:38, 91:29; +4 more in JSON |
| 47 | 007a44d0-007b34b0 | 144 | 0.57 | 1 | 1 | 89 | 0 | 9 | simple, pathintf, pathid, pathbaseentity, forward, paratrooper, entity | 46:83, 1:52, 87:32, 3:31, 2:28, 86:20; +7 more in JSON |
| 48 | 007b34f0-007d1d30 | 331 | 0.68 | 1 | 1 | 89 | 0 | 12 | powerlost, explosion, splash, enginefire, rightspinning, pathpoints, leftspinning | 1:84, 3:79, 49:72, 2:40, 91:35, 43:33; +11 more in JSON |
| 49 | 007d1dc0-00809330 | 501 | 0.44 | 1 | 1 | 89 | 1 | 16 | travelspeed, gears, baydoor, wings, enemy, neutral, state | 1:115, 87:112, 3:80, 2:79, 91:55, 48:54; +17 more in JSON |
| 50 | 00809380-0081aa10 | 218 | 0.61 | 1 | 1 | 89 | 0 | 39 | steeringjam, enginejam, periscope, repairzonearea, torpedostock, thrust, attackmove | 64:35, 1:31, 3:31, 43:27, 44:25, 61:22; +8 more in JSON |
| 51 | 0081aa60-00828810 | 96 | 0.43 | 1 | 1 | 89 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 50:83, 1:25, 2:25, 54:18, 61:18, 87:18; +6 more in JSON |
| 52 | 00828870-00844fc0 | 255 | 0.58 | 1 | 1 | 89 | 1 | 2 | torpedoavoidance, torpedoenabled, shipcollisionavoidance, landcollisionavoidance, firetargetid, firedamageperfiretick, depthchargeenabled | 87:42, 2:35, 9:34, 39:22, 3:17, 44:16; +7 more in JSON |
| 53 | 008455a0-00851e90 | 179 | 0.27 | 1 | 1 | 89 | 1 | 5 | classid, object, stock, stockvec, slotvec, singleturninggun, namevecsize | 1:24, 87:22, 29:22, 2:20, 52:17, 91:17; +10 more in JSON |
| 54 | 00851ec0-0086af80 | 299 | 0.54 | 1 | 1 | 89 | 0 | 13 | radius, submarine, torpedo, unlimitedair, tvertangle, turninggun, thorzangle | 87:70, 1:50, 3:48, 2:23, 36:21, 91:17; +12 more in JSON |
| 55 | 0086afc0-00877e50 | 168 | 0.42 | 1 | 1 | 89 | 0 | 3 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 87:76, 91:60, 54:27, 1:26, 2:23, 82:11; +4 more in JSON |
| 56 | 00877fa0-00881600 | 112 | 0.47 | 1 | 1 | 89 | 0 | 2 | damage, yellow, weaponsystems, weapondirectorthinktime, warningscrollspeeds, visibletimeout, visibilityrange | 87:37, 55:26, 2:9, 91:9, 1:8 |
| 57 | 00881a70-0088ac20 | 132 | 0.67 | 1 | 1 | 89 | 0 | 0 | scripts, debugtrap, shallowwater, modelpath, luab, filepath, colormap | 87:51, 32:51, 2:42, 91:25, 1:23, 36:20 |
| 58 | 0088b120-008e5c40 | 879 | 0.78 | 1 | 1 | 89 | 528 | 2 | luakod, szurkenyil, options, hardwarereported, english, xboxcompatibilitymode, vsync | 87:3876, 2:1239, 1:611, 91:602, 57:434, 44:83; +28 more in JSON |
| 59 | 008e5c50-008f1790 | 201 | 0.48 | 1 | 1 | 89 | 0 | 1 | pup_gain, pum1stget, vec3array, uspumicon, uselimit, unitclassindex, targettype | 58:56, 2:37, 3:30, 87:25, 1:24, 91:24; +4 more in JSON |
| 60 | 008f17e0-00922c80 | 496 | 0.35 | 1 | 1 | 89 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 2:125, 87:94, 1:91, 19:89, 3:81, 91:66; +17 more in JSON |
| 61 | 00922de0-0092df80 | 179 | 0.46 | 1 | 1 | 89 | 0 | 79 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 87:43, 2:20, 1:18, 91:17, 29:12, 32:12; +4 more in JSON |
| 62 | 0092dfe0-00943c00 | 209 | 0.34 | 1 | 1 | 89 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 2:46, 87:38, 1:35, 61:30, 91:24, 3:17; +2 more in JSON |
| 63 | 00943d60-00951f80 | 154 | 0.5 | 1 | 1 | 89 | 0 | 1 | resourceusage, ownerplayer, supportmanager, velocitysi, effect, camocolor, autoattacktarget | 87:70, 2:39, 60:32, 1:29, 62:22, 91:22; +5 more in JSON |
| 64 | 00951fc0-0096b480 | 321 | 0.21 | 1 | 1 | 89 | 0 | 38 | vehicleclass, inferiorfailure, reconplane, torpedobomber, torpedoboat, divebomber, cargo | 87:78, 3:66, 2:48, 1:30, 41:27, 63:17; +10 more in JSON |
| 65 | 0096b4c0-0098c510 | 425 | 0.35 | 1 | 1 | 89 | 0 | 28 | player, entity, message, callback, oldlevel, newlevel, ambient | 2:137, 1:101, 91:95, 57:94, 64:93, 36:34; +15 more in JSON |
| 66 | 0098c630-009965d0 | 155 | 0.51 | 1 | 1 | 89 | 0 | 2 | software, bsm_hwd, eidos, mpkg, cast, content, language | 2:20, 1:18, 91:15, 0:12, 3:8 |
| 67 | 00996670-009f69c0 | 843 | 0.81 | 1 | 1 | 89 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:374, 48:265, 3:233, 68:167, 0:113, 91:105; +21 more in JSON |
| 68 | 009f6a20-009ffad0 | 117 | 0.62 | 1 | 1 | 89 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:34, 39:19, 49:17, 3:15, 48:15, 87:13; +2 more in JSON |
| 69 | 009ffb40-00a07d40 | 130 | 0.57 | 1 | 1 | 89 | 0 | 0 | vehicle, class, captureweight | 68:34, 1:11, 2:8 |
| 70 | 00a07e40-00a19480 | 127 | 0.6 | 1 | 1 | 89 | 0 | 5 | commandtype, neutral, aivstable_, enemy, target, vehicleclass, s_to_ | 69:51, 1:40, 87:25, 2:22, 74:19, 91:18; +8 more in JSON |
| 71 | 00a19500-00a1fac0 | 127 | 0.43 | 1 | 1 | 89 | 0 | 0 | coordinator, sell, strategicgain, duel, defend, capture, siege | 1:19, 2:18, 73:17, 91:11 |
| 72 | 00a1fba0-00a284e0 | 108 | 0.64 | 1 | 1 | 89 | 0 | 0 | strategicgain, siege, escort, competitive | 71:51, 70:25, 1:13, 69:12, 73:12 |
| 73 | 00a286a0-00a335d0 | 82 | 0.57 | 1 | 1 | 89 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 70:28, 87:25, 72:23, 71:19, 2:17, 1:16; +5 more in JSON |
| 74 | 00a371a0-00a46c30 | 227 | 0.47 | 1 | 1 | 89 | 0 | 8 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 75:43, 87:19, 2:18, 10:16, 44:15, 91:13; +4 more in JSON |
| 75 | 00a46d60-00a625d0 | 107 | 0.43 | 1 | 1 | 89 | 0 | 1 | online, server, player, query, matchmaking, remote, movie | 74:16, 19:9, 66:9 |
| 76 | 00a62660-00a798c0 | 329 | 0.64 | 1 | 1 | 89 | 0 | 1 | unexpected, chunk, precompiled, complex, expression, call, many | 78:16, 91:13, 1:12, 2:12 |
| 77 | 00a79910-00a828b0 | 132 | 0.66 | 1 | 1 | 89 | 0 | 7 | memory, sounjd, sound, play, event, system, init | 87:35, 2:21, 91:19, 1:12 |
| 78 | 00a82ae0-00a98400 | 293 | 0.29 | 1 | 1 | 89 | 0 | 6 | memory, sounjd, stream, sound, streaming, stereo, request | 2:49, 91:47, 77:45, 1:33, 84:20, 87:20; +3 more in JSON |
| 79 | 00a98cc0-00aa8710 | 261 | 0.4 | 1 | 1 | 89 | 0 | 22 | cGuiManager, heightplus, dontmovetheitems, centervertical, camerastore, autocontrol, mouse | 81:36, 2:29, 87:22, 1:18, 91:17, 80:16; +1 more in JSON |
| 80 | 00aa87b0-00ab5f00 | 181 | 0.45 | 1 | 1 | 89 | 0 | 20 | pivot, widescreenalign, visible, rotate, partialdisplaytype, partialdisplayratio, mousehit | 79:42, 2:42, 91:33, 1:32, 87:28 |
| 81 | 00ab6070-00ada420 | 482 | 0.25 | 1 | 1 | 89 | 0 | 18 | texture, simplecolor, mshd, mvfm, guidefault, cOverbrightFactor, shadername | 80:85, 87:71, 2:68, 79:62, 91:57, 1:55; +7 more in JSON |
| 82 | 00ada5a0-00af9660 | 377 | 0.35 | 1 | 1 | 89 | 0 | 9 | mshd, terrain, mvfm, visibility, param, group, atlas | 87:87, 2:63, 1:58, 91:58, 93:42, 84:27; +4 more in JSON |
| 83 | 00af9d00-00aff690 | 61 | 0.62 | 1 | 1 | 89 | 0 | 0 | persec, permeter, sphereemitter, smartareaemitter, renderpriority, particleemission, partemissiontype | 84:17, 1:14, 2:11, 91:11 |
| 84 | 00aff700-00b20bf0 | 392 | 0.26 | 1 | 1 | 89 | 0 | 15 | param, additive, emitter, particle, mvfm, initialrotation, emittedspeed | 83:62, 2:60, 1:55, 91:54, 87:52, 86:50; +2 more in JSON |
| 85 | 00b20c50-00b402b0 | 344 | 0.41 | 1 | 1 | 89 | 0 | 10 | pf43cc, mvfm, shadowmap, debugshader, mshd, uterrain4, uterrain3 | 91:32, 2:32, 1:31, 87:21, 86:18, 84:14; +2 more in JSON |
| 86 | 00b402e0-00b659d0 | 394 | 0.23 | 1 | 1 | 89 | 0 | 9 | mshd, cSampleOffsets, mvfm, posteffectsysobj, posteffectsyscam, oldfilm_dust, cSampleWeights | 87:102, 2:50, 1:48, 91:47, 84:35, 85:20 |
| 87 | 00b65ac0-00b75610 | 307 | 0.5 | 1 | 1 | 89 | 0 | 31 | dofile, userdata, thread, lightuserdata, dobuffer, cDummy, c3dobject | 2:34, 76:31, 1:25, 91:23, 86:21, 89:8 |
| 88 | 00b75720-00b7aa40 | 89 | 0.62 | 1 | 1 | 89 | 0 | 0 | c3dnodeanimator, cAnimTrack, cOptimized3dNodeAnimator, cCameraAnimator | 91:11, 1:8 |
| 89 | 00b7abd0-00b866c0 | 229 | 0.26 | 1 | 1 | 89 | 0 | 2 | cLight, cAmbientLight, resourcedump_, refcounter, cPointLight, cDirectionalLight | 2:26, 87:18, 91:16, 1:15, 84:9 |
| 90 | 00b86720-00b8e580 | 122 | 0.55 | 1 | 1 | 89 | 0 | 5 | cSceneResource, flare, zoomfactor, targetname, node, flare2, dissolve | 93:18, 91:17, 87:16, 1:13, 88:12, 2:12 |
| 91 | 00b8e5a0-00bd4200 | 803 | 0.26 | 1 | 1 | 89 | 0 | 12 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 87:213, 2:128, 1:117, 93:56, 84:50, 89:35; +6 more in JSON |
| 92 | 00bd4270-00be8820 | 350 | 0.33 | 1 | 1 | 89 | 0 | 17 | cFileStore, long, iterator, removefile, removed, nagybetu, remove | 87:83, 2:68, 91:65, 1:54, 0:15, 11:8; +1 more in JSON |
| 93 | 00be88c0-00c30570 | 249 | 0.41 | 1 | 1 | 89 | 0 | 8 | profile, cShaderTextureSource, cPhysicalDirectoryX86, cFileX86, saving_pc, savefailed, protected | 2:47, 91:46, 1:37, 92:18, 87:15 |
