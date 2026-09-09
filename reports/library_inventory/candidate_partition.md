# Candidate partition: disjoint link-order segments

23941 unnamed non-thunk FUN_ candidates grouped into 92 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 847 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004138d0 | 191 | 0.07 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413d10-0041dd40 | 156 | 0.54 | 1 | 1 | 85 | 0 | 0 | avoidzoneg, avoidzone | 11:11, 0:9 |
| 2 | 0041ddf0-0042a830 | 201 | 0.26 | 1 | 1 | 85 | 0 | 0 | mpakscenes, endgroup, crash, training, terraingridlayer, terraingrid, modes | 1:100, 90:29, 0:20, 86:9 |
| 3 | 0042a920-0043f480 | 281 | 0.42 | 1 | 1 | 85 | 0 | 14 | bsp_chk_save, unlockto, unlockname, unlockfrom, playtime, difficulty, united | 86:152, 1:78, 90:37, 2:30, 85:27, 31:25; +5 more in JSON |
| 4 | 0043f4d0-0044c390 | 223 | 0.28 | 1 | 1 | 85 | 0 | 3 | panel, deviceclass, daytime, callback, setpanel, hidepanel, character | 86:55, 1:39, 90:23, 2:14, 58:9 |
| 5 | 0044c5a0-00456670 | 135 | 0.46 | 1 | 1 | 85 | 0 | 0 | dialogues, sequence, dialogdefaultpausetime, dialogcharacters, suppressinterruptmsg, requesttime, panelstates | 4:66, 86:37, 1:36, 90:19, 3:8 |
| 6 | 00456710-00491070 | 896 | 0.28 | 1 | 1 | 85 | 0 | 18 | entity, alpha, action, cameraposition, activatetime, aa_flak, point | 1:182, 90:110, 86:81, 2:49, 60:44, 63:40; +9 more in JSON |
| 7 | 00491170-004ab3f0 | 235 | 0.37 | 1 | 1 | 85 | 0 | 3 | soldiertypes, landvehicleclasses, tempid, trafficglobals, topbox, timetolive, startvelocitydiradd | 6:223, 86:54, 1:39, 90:23, 4:21, 49:18; +3 more in JSON |
| 8 | 004ab430-004b9680 | 290 | 0.26 | 1 | 1 | 85 | 0 | 4 | wreckclass, timeleft, smoke, wreck, state, timemin, timemax | 86:50, 1:35, 6:24, 90:21, 4:15 |
| 9 | 004b96a0-004c5530 | 228 | 0.3 | 1 | 1 | 85 | 0 | 0 | collect, collectgarbage, traininggrounds, cloudsmall, multi, cloud, player | 8:29, 31:14, 90:9, 79:8 |
| 10 | 004c55a0-004e7ba0 | 425 | 0.42 | 1 | 1 | 85 | 1 | 6 | ggame, collect, collectgarbage, ingame, textures, universe, scene | 9:178, 1:111, 90:85, 2:69, 8:45, 6:44; +25 more in JSON |
| 11 | 004e7bb0-004f8830 | 178 | 0.35 | 1 | 1 | 85 | 0 | 6 | ambient, sound, party, noisetexture, landconvoy, stationary, filename | 1:54, 10:39, 6:38, 2:33, 62:29, 63:21; +6 more in JSON |
| 12 | 004f8970-00506f00 | 213 | 0.59 | 1 | 1 | 85 | 0 | 0 | menuitem_text, vehicleclass, unitlib_nounlock, back, globals, dview, change | 1:86, 86:57, 90:47, 81:29, 80:25, 2:24; +5 more in JSON |
| 13 | 00506f80-0051e4d0 | 153 | 0.53 | 1 | 1 | 85 | 0 | 1 | back, dview, scroll_menu, globals, navigate, vehicleclass, szarnyas_framebox | 12:138, 86:102, 1:76, 90:48, 2:40, 81:37; +8 more in JSON |
| 14 | 0051e650-00527c80 | 111 | 0.74 | 1 | 1 | 85 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 31:31, 9:18, 1:17, 3:12, 45:10, 11:9; +1 more in JSON |
| 15 | 00527cb0-00544b60 | 336 | 0.33 | 1 | 1 | 85 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 1:86, 81:75, 90:43, 9:36, 2:32, 83:29; +11 more in JSON |
| 16 | 00544e60-00558640 | 283 | 0.26 | 1 | 1 | 85 | 0 | 3 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 1:54, 31:36, 90:30, 81:27, 10:23, 9:22; +6 more in JSON |
| 17 | 00558680-00568930 | 190 | 0.37 | 1 | 1 | 85 | 0 | 0 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 16:76, 1:49, 80:49, 81:33, 86:29, 31:28; +7 more in JSON |
| 18 | 00568cb0-0056d9a0 | 92 | 0.7 | 1 | 1 | 85 | 0 | 0 | cucc_group, setting_var_text, tilt_icon, slider_pos_group, setting_template_next_line_group, servers_framebox, servers_fix_group | 17:28, 81:8 |
| 19 | 0056dd80-00584110 | 340 | 0.3 | 1 | 1 | 85 | 0 | 1 | globals, server_text, players_text, mode_text, scroll_right_icon, wave_icon, main_newprofile | 18:61, 1:43, 2:29, 81:26, 17:26, 90:23; +5 more in JSON |
| 20 | 00584170-0058d470 | 72 | 0.56 | 1 | 1 | 85 | 0 | 1 | globals, select, navigate, back, mission_mappoint_, fe_pc, xsm_requiresprofile | 1:44, 19:39, 80:23, 2:22, 81:22, 90:20; +4 more in JSON |
| 21 | 0058d4b0-0059f3c0 | 177 | 0.43 | 1 | 1 | 85 | 0 | 1 | globals, mission_mappoint_, main_checkpoint_dlc, main_checkpoint, checkpoint_available, objective, hidden | 20:61, 19:42, 2:29, 1:28, 81:21, 90:15; +5 more in JSON |
| 22 | 0059f3d0-005c5da0 | 318 | 0.29 | 1 | 1 | 85 | 0 | 1 | bushgroup, visibility, terrain, message, group, mvfm, mapmodel | 1:74, 21:60, 81:44, 86:43, 90:33, 83:28; +13 more in JSON |
| 23 | 005c6460-005ce9d0 | 136 | 0.36 | 1 | 1 | 85 | 0 | 0 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, uniquemultisettings | 1:20, 86:16, 22:14, 11:14, 9:11, 90:11; +2 more in JSON |
| 24 | 005cea60-005df400 | 143 | 0.73 | 1 | 1 | 85 | 0 | 1 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 1:58, 9:34, 2:34, 10:32, 90:30, 81:30; +7 more in JSON |
| 25 | 005df510-005fd0c0 | 198 | 0.64 | 1 | 1 | 85 | 0 | 5 | xsm_saveconfirm, fe_xbox, globals, back, xsm_dlcchanged, arrow_left_icon, mainlistbox_text | 1:124, 2:87, 24:79, 81:76, 80:71, 90:66; +14 more in JSON |
| 26 | 005fd120-006049f0 | 64 | 0.66 | 1 | 1 | 85 | 0 | 0 | ingame, felkialtojel_text, aaaaaa, paused, title_group, silverline_framebox, secondary_objectives_text | 10:39, 1:36, 80:23, 25:21, 90:17, 81:17; +5 more in JSON |
| 27 | 00604a20-0060dfb0 | 88 | 0.5 | 1 | 1 | 85 | 0 | 1 | turbo_group, turbo_effect, ship_speed_num3_icon, ship_payload_2_icon, ship_payload_1_icon, repairzone_text, pleasewait | 3:36, 50:36, 1:30, 9:25, 31:23, 11:20; +5 more in JSON |
| 28 | 0060e0e0-00622990 | 203 | 0.32 | 1 | 1 | 85 | 0 | 2 | showgamercard, globals, usn_point_text, usn_icon, radar_sweep, pumpermanent, playerreview | 1:50, 81:39, 90:27, 2:17, 86:15, 83:14; +5 more in JSON |
| 29 | 00622a10-00654650 | 364 | 0.23 | 1 | 1 | 85 | 0 | 6 | ingame, globals, type_icon, scoring_unlock_text, score_text, medal_icon, commandbuilding_icon | 1:105, 28:83, 81:64, 2:55, 31:53, 90:39; +22 more in JSON |
| 30 | 00654a70-00664030 | 208 | 0.42 | 1 | 1 | 85 | 0 | 0 | circle_hl_icon, ingamegui, normal, icon_l_icon, circle_small_02_group, circle_small_01_group, number_text | 1:42, 78:24, 81:23, 31:21, 90:21, 2:18 |
| 31 | 00664070-006980e0 | 500 | 0.3 | 1 | 1 | 85 | 0 | 9 | ingame, gvmultimenu, sm_cp, globals, interface, textures, unitclass_spawnpoint | 30:174, 1:122, 10:89, 2:66, 9:64, 90:63; +32 more in JSON |
| 32 | 006981c0-0069fe70 | 133 | 0.42 | 1 | 1 | 85 | 0 | 0 | swapstickpairs, swapstickmap, swapstickgeneral, press, invio, inputmodifiers, indietro | 86:17, 1:15, 31:14, 2:10, 90:8 |
| 33 | 0069ff40-006a9ea0 | 124 | 0.63 | 1 | 1 | 85 | 0 | 0 | devinputs, sensitivitysettings, inputsettings, devicetype, deviceidx, slider, down | 32:52, 17:30, 16:28, 1:21, 4:21, 86:20; +2 more in JSON |
| 34 | 006a9f70-006c0210 | 252 | 0.35 | 1 | 1 | 85 | 0 | 0 | savedata, scoring, entities, missionid, entidcont, cont1, cont0 | 86:349, 1:90, 2:78, 90:48, 6:28, 4:23; +11 more in JSON |
| 35 | 006c0240-006ca410 | 105 | 0.5 | 1 | 1 | 85 | 0 | 0 | equipment, state, slots, classid, stock, waitingpos, targetid | 34:97, 31:45, 1:34, 3:18, 45:8 |
| 36 | 006ca640-006d6450 | 155 | 0.47 | 1 | 1 | 85 | 0 | 17 | runwaywidth, runwaylength, readyplane, loaddelay, justliftedupid, runwayfailure, hangarfailure | 34:47, 35:42, 1:37, 31:32, 2:20, 6:16; +7 more in JSON |
| 37 | 006d6470-006da6b0 | 101 | 0.42 | 2 | 37 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 1:19, 90:11 |
| 38 | 006dadc0-006deff0 | 69 | 0.7 | 3 | 38 | 1 | 0 | 0 | markerclasses, recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 37:49, 86:10, 1:9 |
| 39 | 006df170-006f1380 | 219 | 0.38 | 1 | 1 | 85 | 0 | 20 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 86:74, 2:50, 1:45, 43:31, 31:29, 63:29; +10 more in JSON |
| 40 | 006f1440-00717980 | 502 | 0.25 | 1 | 1 | 85 | 1 | 12 | mvfm, openaftertime, messages, depthcharge, datatables, scripts, mshd | 1:96, 86:94, 90:46, 2:45, 39:44, 31:40; +13 more in JSON |
| 41 | 00717c70-0071b9b0 | 83 | 0.59 | 2 | 41 | 1 | 0 | 1 | mzonedesc, sphere, identifier, mnote, armor, points, category | 40:60, 89:13, 91:11 |
| 42 | 0071ba20-00728fa0 | 213 | 0.46 | 1 | 1 | 85 | 0 | 14 | startmode, followmode, userpath, pathfollowparams, pathcursor, mgeommesh, internalclearprimarycommand | 2:44, 44:27, 45:21, 1:19, 49:14, 14:14; +1 more in JSON |
| 43 | 007290d0-00758dc0 | 490 | 0.27 | 1 | 1 | 85 | 0 | 31 | destroyed, speed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed | 1:102, 86:102, 2:84, 90:69, 44:42, 65:33; +24 more in JSON |
| 44 | 00758e40-0076e520 | 429 | 0.74 | 1 | 1 | 85 | 0 | 5 | elevator_2, p2p_voice__, woice, serversendscenescoring, runwaycenter, receive, nonce | 2:665, 45:37, 1:14, 90:12, 36:10, 86:9; +2 more in JSON |
| 45 | 0076e710-00786a80 | 361 | 0.26 | 1 | 1 | 85 | 0 | 27 | reconlevel, multiscore, lastbanto, p2p_voice__, recondata, peer, netentity | 44:125, 1:59, 86:56, 2:42, 90:36, 63:27; +12 more in JSON |
| 46 | 00786be0-0078c9b0 | 65 | 0.46 | 2 | 46 | 1 | 0 | 0 | send_, recv_, time, client, server | 90:29, 88:13, 1:9 |
| 47 | 0078cf20-0079c000 | 179 | 0.51 | 1 | 1 | 85 | 0 | 0 | initialization, zoominput, walkvinput, walkhinput, turnvinput, turnhinput, trg_trans | 1:65, 31:30, 3:26, 90:22, 88:15, 2:12; +3 more in JSON |
| 48 | 0079c040-007a4590 | 74 | 0.53 | 1 | 1 | 85 | 0 | 0 | postype, thetalinearblend, theta, rholinearblend, camera, target, vlook | 47:70, 86:54, 2:25, 1:24, 3:22, 23:15; +2 more in JSON |
| 49 | 007a4860-007b38d0 | 145 | 0.56 | 1 | 1 | 85 | 0 | 11 | pathbaseentity, simple, sustainbefore, rotrefentity, pathpoints, pathintf, pathid | 1:64, 47:58, 3:28, 86:27, 48:22, 85:20; +9 more in JSON |
| 50 | 007b3920-007d5890 | 333 | 0.64 | 1 | 1 | 85 | 0 | 10 | powerlost, explosion, enginefire, splash, rightspinning, leftspinning, kamikazebulletclass | 1:92, 51:77, 3:73, 90:36, 2:35, 44:33; +12 more in JSON |
| 51 | 007d5ac0-0080da00 | 552 | 0.4 | 1 | 1 | 85 | 1 | 7 | enemy, neutral, state, datatables, scripts, windsound, unlocks | 1:152, 86:102, 3:81, 90:54, 50:49, 2:45; +18 more in JSON |
| 52 | 0080dad0-0081a9c0 | 161 | 0.56 | 1 | 1 | 85 | 0 | 39 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 65:36, 44:27, 45:25, 1:24, 2:22, 42:18; +7 more in JSON |
| 53 | 0081aa10-00828810 | 97 | 0.44 | 1 | 1 | 85 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 52:77, 1:31, 2:19, 56:18, 63:18, 86:18; +7 more in JSON |
| 54 | 00828870-008455a0 | 256 | 0.45 | 1 | 1 | 85 | 1 | 2 | torpedoavoidance, torpedoenabled, shipcollisionavoidance, landcollisionavoidance, firetargetid, firedamageperfiretick, depthchargeenabled | 86:42, 7:33, 1:30, 6:24, 42:23, 2:18; +6 more in JSON |
| 55 | 00845670-00851ec0 | 179 | 0.26 | 1 | 1 | 85 | 1 | 5 | classid, object, stock, stockvec, slotvec, singleturninggun, namevecsize | 1:33, 86:22, 6:21, 54:17, 90:17, 31:17; +10 more in JSON |
| 56 | 00851ee0-0086af80 | 298 | 0.47 | 1 | 1 | 85 | 0 | 10 | radius, submarine, torpedo, unlimitedair, tvertangle, turninggun, thorzangle | 86:70, 1:54, 3:38, 39:21, 2:19, 90:17; +13 more in JSON |
| 57 | 0086afc0-00877cd0 | 166 | 0.43 | 1 | 1 | 85 | 0 | 2 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 86:76, 90:60, 1:41, 56:27, 63:10, 83:9; +4 more in JSON |
| 58 | 00877e40-008828e0 | 119 | 0.46 | 1 | 1 | 85 | 0 | 2 | damage, yellow, weaponsystems, weapondirectorthinktime, warningscrollspeeds, visibletimeout, visibilityrange | 86:41, 57:28, 1:14, 90:12 |
| 59 | 00882ac0-0088b120 | 128 | 0.57 | 1 | 1 | 85 | 0 | 0 | scripts, debugtrap, shallowwater, modelpath, luab, filepath, colormap | 34:51, 86:47, 1:36, 2:25, 90:24, 39:21 |
| 60 | 0088b190-008e5c30 | 878 | 0.79 | 1 | 1 | 85 | 528 | 2 | luakod, szurkenyil, options, hardwarereported, english, xboxcompatibilitymode, vsync | 86:3876, 1:1189, 2:661, 90:602, 59:436, 45:76; +27 more in JSON |
| 61 | 008e5c40-008f10a0 | 190 | 0.49 | 1 | 1 | 85 | 0 | 1 | pup_gain, pum1stget, vec3array, uspumicon, uselimit, unitclassindex, targettype | 60:57, 1:43, 86:25, 90:23, 28:16, 4:16; +5 more in JSON |
| 62 | 008f10b0-00922c80 | 508 | 0.35 | 1 | 1 | 85 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 1:138, 86:94, 2:80, 19:78, 3:77, 90:67; +17 more in JSON |
| 63 | 00922de0-0092df80 | 179 | 0.47 | 1 | 1 | 85 | 0 | 78 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 86:43, 1:23, 6:19, 90:17, 2:15, 31:12; +2 more in JSON |
| 64 | 0092dfe0-00943c00 | 209 | 0.43 | 1 | 1 | 85 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 1:51, 86:38, 63:30, 2:30, 90:24, 56:14; +3 more in JSON |
| 65 | 00943d60-00973940 | 627 | 0.26 | 1 | 1 | 85 | 0 | 57 | callback, vehicleclass, entity, inferiorfailure, party, oldlevel, newlevel | 86:148, 1:114, 3:56, 2:56, 90:44, 62:39; +21 more in JSON |
| 66 | 00973a10-0097c8a0 | 141 | 0.45 | 2 | 66 | 1 | 0 | 7 | player, plane, other, ambient, message, warningmanager, supply | 65:114, 1:64, 90:38, 2:35, 86:13, 7:9; +1 more in JSON |
| 67 | 0097c9b0-0098d470 | 140 | 0.4 | 3 | 67 | 1 | 0 | 3 | repair, software, player, shiplanded, musicover, hpevent, generate | 66:113, 59:93, 1:82, 65:62, 90:45, 2:33; +10 more in JSON |
| 68 | 0098d4e0-009966c0 | 149 | 0.4 | 1 | 1 | 85 | 0 | 2 | mpkg, cast, content, language, unmount, russian, processornamestring | 1:33, 90:15, 0:12, 3:9 |
| 69 | 00996720-009f69c0 | 841 | 0.82 | 1 | 1 | 85 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:375, 50:265, 3:230, 70:167, 0:113, 90:105; +21 more in JSON |
| 70 | 009f6a20-009fe940 | 96 | 0.75 | 1 | 1 | 85 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:35, 51:19, 42:19, 3:15, 50:15, 86:13; +2 more in JSON |
| 71 | 009fe9c0-00a0f680 | 169 | 0.5 | 1 | 1 | 85 | 0 | 0 | neutral, aivstable_, enemy, vehicle, class, vehicleclass, s_to_ | 1:26, 70:18, 86:16, 65:16, 90:14, 2:12; +3 more in JSON |
| 72 | 00a0f810-00a160b0 | 67 | 0.61 | 1 | 1 | 85 | 0 | 5 | commandtype, target, command | 1:30, 71:22, 75:12, 45:12, 86:11, 31:9; +1 more in JSON |
| 73 | 00a160d0-00a1fa60 | 167 | 0.56 | 1 | 1 | 85 | 0 | 0 | coordinator, capture, sell, strategicgain, duel, defend, siege | 1:33, 74:28, 90:15, 2:12, 71:8 |
| 74 | 00a1fa90-00a335d0 | 192 | 0.57 | 1 | 1 | 85 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 73:110, 1:34, 71:33, 86:27, 9:20, 31:16; +4 more in JSON |
| 75 | 00a371a0-00a45510 | 200 | 0.41 | 1 | 1 | 85 | 0 | 8 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 76:35, 86:19, 1:18, 45:14, 90:12, 9:9; +1 more in JSON |
| 76 | 00a45570-00a625d0 | 134 | 0.51 | 1 | 1 | 85 | 0 | 1 | online, server, player, query, matchmaking, remote, movie | 75:34, 68:14, 1:10, 19:9, 9:8, 31:8 |
| 77 | 00a62660-00a798c0 | 329 | 0.64 | 1 | 1 | 85 | 0 | 1 | unexpected, chunk, precompiled, complex, expression, call, many | 1:21, 79:16, 90:13 |
| 78 | 00a79910-00a82c60 | 135 | 0.67 | 1 | 1 | 85 | 0 | 7 | memory, sounjd, sound, play, event, system, init | 86:35, 1:25, 90:20, 2:9 |
| 79 | 00a82e20-00a98400 | 290 | 0.28 | 1 | 1 | 85 | 0 | 6 | memory, sounjd, stream, sound, streaming, stereo, request | 1:53, 90:47, 78:43, 2:28, 84:20, 86:20; +3 more in JSON |
| 80 | 00a98cc0-00aa0f50 | 105 | 0.53 | 1 | 1 | 85 | 0 | 3 | heightplus, dontmovetheitems, centervertical, autocontrol, lockit, linedistance, items | 81:20, 1:16, 2:15, 90:9 |
| 81 | 00aa0f70-00aabc70 | 190 | 0.32 | 1 | 1 | 85 | 0 | 37 | cGuiManager, widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor | 83:37, 86:29, 1:26, 90:13, 80:10, 2:9; +1 more in JSON |
| 82 | 00aabd60-00ab5700 | 141 | 0.57 | 1 | 1 | 85 | 0 | 3 | partialdisplaytype, partialdisplayratio, label, dynamicvb, delayedtextureload, autorotate, moviename | 1:49, 81:28, 90:26, 86:18 |
| 83 | 00ab57e0-00af6b90 | 842 | 0.27 | 1 | 1 | 85 | 0 | 24 | mshd, mvfm, texture, simplecolor, guidefault, terrain, normal | 1:188, 86:147, 81:140, 90:107, 91:56, 2:49; +8 more in JSON |
| 84 | 00af6c50-00b20bf0 | 483 | 0.22 | 1 | 1 | 85 | 0 | 11 | param, additive, emitter, dynamic_light_, rotationspeed, particle, mvfm | 1:121, 90:74, 86:64, 85:61, 83:48, 2:29; +2 more in JSON |
| 85 | 00b20c50-00b659d0 | 746 | 0.27 | 1 | 1 | 85 | 0 | 13 | mshd, cSampleOffsets, mvfm, pf43cc, shadowmap, posteffectsysobj, posteffectsyscam | 1:141, 86:123, 90:82, 84:60, 2:26, 10:12; +2 more in JSON |
| 86 | 00b65ac0-00b75d80 | 312 | 0.5 | 1 | 1 | 85 | 0 | 31 | dofile, userdata, thread, lightuserdata, dobuffer, cDummy, c3dobject | 1:40, 77:31, 85:23, 2:19, 90:17, 88:8 |
| 87 | 00b75de0-00b7aa40 | 84 | 0.67 | 1 | 1 | 85 | 0 | 0 | c3dnodeanimator, cAnimTrack, cOptimized3dNodeAnimator, cCameraAnimator | 1:15, 90:9, 89:8 |
| 88 | 00b7abd0-00b86390 | 231 | 0.43 | 1 | 1 | 85 | 0 | 1 | cLight, cAmbientLight, boundingbox, resource, resourcedump_, refcounter, matrix | 1:36, 91:25, 90:24, 86:17, 2:14, 40:9; +1 more in JSON |
| 89 | 00b86420-00b91570 | 197 | 0.45 | 1 | 1 | 85 | 0 | 3 | cSceneResource, cGroupParamsResource, cGroup, flare, zoomfactor, targetname, node | 86:38, 91:22, 1:21, 87:13, 90:13, 40:10 |
| 90 | 00b91580-00bd4200 | 740 | 0.27 | 1 | 1 | 85 | 0 | 95 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 1:195, 86:193, 91:80, 84:64, 2:49, 89:31; +7 more in JSON |
| 91 | 00bd4270-00c30570 | 613 | 0.36 | 1 | 1 | 85 | 0 | 23 | long, iterator, cFileStore, removefile, removed, profile, nagybetu | 1:177, 90:119, 86:98, 2:50, 0:19, 10:13; +1 more in JSON |
