# Candidate partition: disjoint link-order segments

23994 untagged FUN_ candidates cut into 96 disjoint address segments (Louvain resolution 0.5, smoothing window 12, min segment 60); 847 vtables found in .rdata. Wave 0 segments have no strong outbound dependency (>= 8 calls) on unfinished segments; segments in one cycle group share a wave.

| Seg | Range | Cands | Purity | Wave | Lua bind | Vtables | Keywords | Strong deps out (seg:calls) |
|---|---|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004138d0 | 191 | 0.03 | 0 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413d10-0041db10 | 154 | 0.55 | 1 | 0 | 0 | avoidzoneg, avoidzone | 13:11, 0:9 |
| 2 | 0041dd20-0042a7e0 | 202 | 0.19 | 1 | 0 | 0 | mpakscenes, endgroup, crash, training, terraingridlayer, terraingrid, modes | 1:84, 92:32, 0:20, 87:9 |
| 3 | 0042a830-0043f860 | 289 | 0.39 | 1 | 0 | 14 | bsp_chk_save, unlockto, unlockname, unlockfrom, playtime, difficulty, united | 87:152, 1:56, 2:50, 92:37, 86:27, 34:25 |
| 4 | 0043f9e0-004486c0 | 141 | 0.32 | 1 | 0 | 2 | deviceclass, daytime, device, platform, weatherreconmodifiers, weatherampmultipliers, viewmodeampmultipliers | 87:54, 1:16, 2:15, 92:14, 3:13 |
| 5 | 00448700-0044c390 | 75 | 0.49 | 1 | 0 | 1 | panel, callback, setpanel, hidepanel, character, pause, message | 2:13, 1:9, 92:9 |
| 6 | 0044c5a0-00468600 | 345 | 0.17 | 1 | 0 | 1 | action, cameraposition, activatetime, aa_flak, alpha, point, periscope | 2:69, 5:54, 1:51, 87:51, 92:49, 60:25 |
| 7 | 00468660-004ab550 | 923 | 0.23 | 1 | 0 | 20 | soldiertypes, landvehicleclasses, entity, vehicles, tempid, startpt, scenebrowsergroups | 87:121, 1:103, 92:103, 2:94, 63:84, 6:52 |
| 8 | 004ab580-004b9680 | 288 | 0.24 | 1 | 0 | 4 | wreckclass, timeleft, smoke, wreck, state, timemin, timemax | 87:50, 7:28, 92:21, 2:21, 1:18, 4:15 |
| 9 | 004b96a0-004c4ed0 | 217 | 0.28 | 1 | 0 | 0 | collect, collectgarbage, traininggrounds, cloudsmall, multi, cloud, player | 8:26, 34:12 |
| 10 | 004c4fe0-004d17c0 | 209 | 0.43 | 1 | 0 | 1 | white, allbutingame, interface, textures, writestats, userleft, stats | 9:83, 92:49, 1:47, 2:42, 34:15, 77:11 |
| 11 | 004d1840-004d6de0 | 81 | 0.28 | 1 | 0 | 0 | reassignonplayerdrop, library, setted, planerumble, cameramover, universe, scene | 10:66, 9:31, 8:22, 87:19, 2:11, 1:8 |
| 12 | 004d6e20-004e7bb0 | 147 | 0.41 | 1 | 1 | 5 | ggame, collect, collectgarbage, ingame, skiptitle, skiplogos, skipbriefings | 10:68, 9:68, 11:57, 2:54, 92:30, 63:26 |
| 13 | 004e7c00-004f8830 | 177 | 0.33 | 1 | 0 | 6 | ambient, sound, party, noisetexture, landconvoy, stationary, filename | 2:68, 63:50, 7:39, 12:35, 1:19, 87:16 |
| 14 | 004f8970-00505d50 | 192 | 0.62 | 2 | 0 | 0 | menuitem_text, vehicleclass, unitlib_nounlock, back, globals, dview, change | 2:64, 87:57, 92:46, 1:42, 79:26, 78:22 |
| 15 | 00505de0-0051e650 | 175 | 0.55 | 3 | 0 | 1 | back, dview, scroll_menu, globals, navigate, vehicleclass, szarnyas_framebox | 14:136, 87:102, 2:76, 92:49, 1:42, 79:39 |
| 16 | 0051e6e0-00527c80 | 110 | 0.71 | 2 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 34:31, 9:18, 1:14, 3:11, 2:10, 49:10 |
| 17 | 00527cb0-00543a30 | 311 | 0.39 | 2 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 79:74, 2:66, 1:49, 92:43, 9:34, 81:31 |
| 18 | 00543a60-005525b0 | 174 | 0.55 | 3 | 0 | 2 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 2:32, 34:32, 79:20, 17:17, 1:17, 9:15 |
| 19 | 00552630-00558640 | 134 | 0.55 | 1 | 0 | 1 | setting_group, selectedconflict_text, listbox_framebox, fe_controls_pc_listbox, fe_controls_pc, column_framebox, toggle_text_group | 1:17, 92:17, 2:8 |
| 20 | 00558680-00568930 | 190 | 0.37 | 1 | 0 | 0 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 19:71, 2:49, 78:49, 79:33, 87:29, 34:28 |
| 21 | 00568cb0-0056f320 | 102 | 0.65 | 1 | 0 | 1 | globals, setting_2_text, nike_icon, cucc_group, setting_var_text, setting_value_text, tilt_icon | 20:31, 2:18, 79:11, 78:9 |
| 22 | 00570300-00584110 | 330 | 0.3 | 1 | 0 | 0 | server_text, players_text, mode_text, scroll_right_icon, wave_icon, main_newprofile, titlelogo_icon | 21:71, 2:38, 79:23, 20:23, 92:19, 1:18 |
| 23 | 00584170-0058d470 | 72 | 0.56 | 2 | 0 | 1 | globals, select, navigate, back, mission_mappoint_, fe_pc, xsm_requiresprofile | 2:45, 22:39, 78:23, 79:22, 1:21, 92:20 |
| 24 | 0058d4b0-005c5cd0 | 494 | 0.15 | 3 | 0 | 2 | globals, mission_mappoint_, bushgroup, visibility, terrain, message, group | 2:97, 79:65, 23:61, 1:59, 87:55, 81:49 |
| 25 | 005c5da0-005ce9d0 | 137 | 0.35 | 4 | 0 | 0 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, uniquemultisettings | 87:16, 24:14, 13:14, 1:11, 9:11, 92:11 |
| 26 | 005cea60-005df510 | 144 | 0.72 | 2 | 0 | 1 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 2:63, 9:32, 1:31, 92:31, 79:28, 78:25 |
| 27 | 005e09a0-005ee440 | 76 | 0.63 | 3 | 0 | 3 | globals, navigate, back, achievement_lock_icon, players_listbox, rank_icon, change | 26:75, 2:57, 34:45, 79:44, 78:37, 22:31 |
| 28 | 005ee4a0-005fd180 | 123 | 0.63 | 2 | 0 | 2 | xsm_saveconfirm, fe_xbox, xsm_dlcchanged, arrow_left_icon, arrow_right_icon, slider0, main_text | 2:85, 1:47, 92:45, 78:35, 60:32, 79:31 |
| 29 | 005fd190-006049f0 | 62 | 0.66 | 3 | 0 | 0 | ingame, felkialtojel_text, aaaaaa, paused, title_group, silverline_framebox, secondary_objectives_text | 2:33, 28:24, 78:23, 1:17, 92:17, 79:17 |
| 30 | 00604a20-0060de90 | 87 | 0.51 | 7 | 0 | 1 | turbo_group, turbo_effect, ship_speed_num3_icon, ship_payload_2_icon, ship_payload_1_icon, repairzone_text, pleasewait | 3:39, 52:36, 1:25, 34:23, 9:22, 13:20 |
| 31 | 0060dfb0-00622990 | 204 | 0.36 | 2 | 0 | 2 | showgamercard, globals, usn_point_text, usn_icon, radar_sweep, pumpermanent, playerreview | 2:42, 79:39, 92:27, 1:26, 81:15, 87:15 |
| 32 | 00622a10-00654650 | 364 | 0.12 | 3 | 0 | 6 | ingame, globals, type_icon, scoring_unlock_text, score_text, medal_icon, commandbuilding_icon | 2:100, 31:83, 79:64, 1:60, 34:53, 92:39 |
| 33 | 00654a70-00664030 | 208 | 0.3 | 1 | 0 | 0 | circle_hl_icon, ingamegui, normal, icon_l_icon, circle_small_02_group, circle_small_01_group, number_text | 2:38, 76:24, 79:23, 1:22, 34:21, 92:21 |
| 34 | 00664070-006980e0 | 500 | 0.17 | 1 | 0 | 9 | ingame, gvmultimenu, sm_cp, globals, interface, textures, unitclass_spawnpoint | 33:174, 2:122, 1:66, 9:63, 92:63, 87:63 |
| 35 | 006981c0-0069fe70 | 133 | 0.44 | 2 | 0 | 0 | swapstickpairs, swapstickmap, swapstickgeneral, press, invio, inputmodifiers, indietro | 87:17, 2:17, 34:14, 1:8, 92:8 |
| 36 | 0069ff40-006a9f70 | 125 | 0.64 | 3 | 0 | 0 | devinputs, sensitivitysettings, inputsettings, devicetype, deviceidx, slider, down | 35:52, 20:31, 19:28, 4:21, 87:20, 92:14 |
| 37 | 006aa090-006be980 | 210 | 0.4 | 1 | 0 | 0 | savedata, scoring, entities, missionid, entidcont, cont1, cont0 | 87:349, 2:107, 1:59, 92:48, 6:26, 4:23 |
| 38 | 006beb10-006d6310 | 299 | 0.55 | 1 | 0 | 17 | equipment, state, slots, runwaywidth, runwaylength, runwayfailure, hangarfailure | 34:81, 37:73, 1:70, 3:30, 2:30, 63:28 |
| 39 | 006d6350-006da670 | 102 | 0.4 | 2 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 1:10, 92:10, 2:10 |
| 40 | 006da6b0-006deca0 | 67 | 0.76 | 3 | 0 | 0 | markerclasses, recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 39:46, 87:10 |
| 41 | 006dee40-006face0 | 331 | 0.37 | 1 | 1 | 22 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 87:97, 2:76, 1:60, 63:51, 46:43, 34:43 |
| 42 | 006fad70-0070cae0 | 241 | 0.24 | 2 | 0 | 10 | openaftertime, messages, whosaysthat, velocity, openstate, dragvert, divedepth | 87:73, 2:47, 41:43, 1:36, 92:33, 63:28 |
| 43 | 0070cc30-00717980 | 152 | 0.31 | 2 | 0 | 0 | unitlist, unitid, shipnumber, shape, num_0, relativeposition, leader | 1:11, 87:8 |
| 44 | 00717c70-0071b940 | 82 | 0.56 | 3 | 0 | 1 | mzonedesc, sphere, identifier, mnote, armor, points, category | 43:57, 95:11, 90:8 |
| 45 | 0071b9b0-00727bd0 | 188 | 0.51 | 7 | 0 | 5 | startmode, followmode, userpath, pathfollowparams, pathcursor, mgeommesh, internalclearprimarycommand | 2:37, 49:21, 48:21, 1:19, 52:14, 16:14 |
| 46 | 00727c30-0074e540 | 458 | 0.3 | 1 | 0 | 36 | destroyed, speed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed | 2:108, 87:102, 92:65, 1:62, 48:40, 65:34 |
| 47 | 0074e5d0-00758eb0 | 60 | 0.47 | 3 | 0 | 5 | sumleaks, sumforces, iswater, water, launchairstrike, elevator_2, runwayfailure | 2:15, 54:11, 7:8, 38:8, 48:8, 1:8 |
| 48 | 00758f90-0076f210 | 441 | 0.77 | 1 | 0 | 5 | p2p_voice__, woice, serversendscenescoring, runwaycenter, receive, nonce, myplayer | 2:667, 50:33, 1:13, 92:13, 38:11, 49:10 |
| 49 | 0076f280-007828a0 | 277 | 0.3 | 1 | 0 | 24 | reconlevel, multiscore, lastbanto, p2p_voice__, recondata, peer, netentity | 48:131, 2:57, 87:56, 50:38, 63:37, 92:35 |
| 50 | 007828b0-0078c9b0 | 135 | 0.31 | 1 | 0 | 3 | send_, recv_, time, client, server | 91:22, 2:18, 89:13, 48:10 |
| 51 | 0078cf20-007a4860 | 254 | 0.41 | 5 | 0 | 0 | postype, camera, thetalinearblend, theta, rholinearblend, blendtime, initialization | 1:79, 87:62, 3:51, 2:44, 34:38, 25:19 |
| 52 | 007a49a0-007d1d30 | 472 | 0.67 | 6 | 0 | 21 | powerlost, explosion, pathbaseentity, splash, simple, enginefire, sustainbefore | 1:136, 3:100, 51:84, 53:78, 2:69, 63:49 |
| 53 | 007d1dc0-0080da00 | 557 | 0.43 | 6 | 1 | 7 | travelspeed, gears, baydoor, wings, enemy, neutral, state | 1:122, 87:118, 3:82, 2:79, 52:58, 92:55 |
| 54 | 0080dad0-0081aa10 | 162 | 0.6 | 2 | 0 | 38 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 65:36, 48:27, 49:25, 1:24, 2:22, 63:19 |
| 55 | 0081aa60-008288d0 | 98 | 0.43 | 3 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 54:76, 1:25, 2:25, 63:24, 57:18, 87:18 |
| 56 | 00828930-00851ee0 | 434 | 0.24 | 8 | 2 | 7 | gameunit, classid, stock, torpedoavoidance, object, state, torpedoenabled | 7:68, 87:64, 2:55, 1:37, 45:37, 63:27 |
| 57 | 00851f50-0086af80 | 297 | 0.47 | 1 | 0 | 10 | radius, submarine, torpedo, unlimitedair, tvertangle, turninggun, thorzangle | 87:70, 1:50, 3:38, 2:23, 41:21, 63:18 |
| 58 | 0086afc0-00877e40 | 167 | 0.41 | 1 | 0 | 2 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 87:76, 92:41, 57:27, 1:26, 2:23, 91:19 |
| 59 | 00877e50-0088b120 | 246 | 0.24 | 1 | 0 | 2 | damage, scripts, debugtrap, datatables, yellow, weaponsystems, weapondirectorthinktime | 87:88, 2:51, 37:51, 92:32, 1:31, 58:27 |
| 60 | 0088b190-008e5c10 | 876 | 0.76 | 1 | 528 | 2 | luakod, szurkenyil, options, hardwarereported, english, xboxcompatibilitymode, vsync | 87:3876, 2:1239, 1:611, 92:602, 59:437, 49:75 |
| 61 | 008e5c20-008ee670 | 120 | 0.62 | 3 | 0 | 1 | pup_gain, pum1stget, uspumicon, uselimit, unitclassindex, targettype, targetfilter | 60:52, 2:36, 87:25, 1:23, 92:23, 31:16 |
| 62 | 008ee990-008f1720 | 83 | 0.69 | 2 | 0 | 0 | vec3array, intarray, floatarray, bytes, none, lua_s | 3:12, 65:9 |
| 63 | 008f1790-0092df80 | 676 | 0.27 | 1 | 1 | 80 | bulletthrowmul, entity, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot | 2:145, 87:137, 1:109, 92:83, 3:82, 22:79 |
| 64 | 0092dfe0-00943c00 | 209 | 0.43 | 2 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 2:46, 87:40, 63:38, 1:35, 92:24, 57:14 |
| 65 | 00943d60-00978c70 | 696 | 0.23 | 1 | 0 | 64 | callback, vehicleclass, player, entity, message, inferiorfailure, ambient | 87:148, 2:146, 1:88, 63:58, 92:58, 3:58 |
| 66 | 00978ca0-0098c820 | 206 | 0.27 | 2 | 0 | 3 | repair, player, shiplanded, musicover, hpevent, generate, entitykilled | 65:188, 59:93, 2:78, 1:72, 92:69, 41:32 |
| 67 | 0098c870-00996120 | 149 | 0.52 | 2 | 0 | 2 | software, mpkg, cast, bsm_hwd, content, language, eidos | 2:20, 92:15, 1:14, 0:12 |
| 68 | 00996270-009f69c0 | 847 | 0.86 | 9 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:378, 52:270, 3:234, 69:167, 0:113, 92:105 |
| 69 | 009f6a20-009fe120 | 88 | 0.75 | 8 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:34, 53:19, 45:19, 3:15, 87:13, 34:13 |
| 70 | 009fe130-00a16030 | 240 | 0.55 | 2 | 0 | 5 | commandtype, neutral, aivstable_, enemy, vehicle, class, target | 1:46, 72:31, 87:27, 2:27, 92:18, 65:18 |
| 71 | 00a16050-00a1fa60 | 171 | 0.62 | 2 | 0 | 0 | coordinator, capture, sell, strategicgain, duel, defend, siege | 72:30, 1:24, 2:21, 92:15, 70:12 |
| 72 | 00a1fa90-00a371a0 | 193 | 0.77 | 2 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 71:110, 70:45, 1:29, 87:27, 9:21, 2:19 |
| 73 | 00a371c0-00a453c0 | 198 | 0.37 | 3 | 0 | 8 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 74:35, 87:19, 50:15, 2:14, 92:12, 1:10 |
| 74 | 00a45510-00a61040 | 128 | 0.52 | 3 | 0 | 1 | online, server, player, query, matchmaking, remote, movie | 73:31, 67:14, 2:9, 22:9, 9:8, 34:8 |
| 75 | 00a62150-00a798c0 | 336 | 0.65 | 1 | 0 | 1 | unexpected, chunk, precompiled, complex, closed, attempt, expression | 77:16, 92:13, 1:12, 2:12 |
| 76 | 00a79910-00a82b70 | 134 | 0.67 | 1 | 0 | 7 | memory, sounjd, sound, play, event, system, init | 87:35, 2:21, 92:19, 1:12 |
| 77 | 00a82c60-00a98400 | 291 | 0.28 | 1 | 0 | 6 | memory, sounjd, stream, sound, streaming, stereo, request | 2:49, 92:48, 76:46, 1:33, 84:20, 87:20 |
| 78 | 00a98cc0-00aa0ff0 | 107 | 0.6 | 1 | 0 | 3 | heightplus, dontmovetheitems, centervertical, autocontrol, lockit, linedistance, items | 2:22, 79:21, 1:9, 92:9 |
| 79 | 00aa1040-00aabc70 | 188 | 0.27 | 1 | 0 | 37 | cGuiManager, widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor | 81:35, 87:29, 2:20, 1:15, 92:13, 78:11 |
| 80 | 00aabd60-00ab5f00 | 149 | 0.54 | 1 | 0 | 3 | partialdisplaytype, partialdisplayratio, label, dynamicvb, delayedtextureload, autorotate, moviename | 2:31, 79:30, 92:28, 1:27, 87:21 |
| 81 | 00ab6070-00ad8a00 | 464 | 0.25 | 1 | 0 | 17 | simplecolor, texture, mshd, guidefault, mvfm, cOverbrightFactor, cLowColor | 79:141, 87:101, 2:66, 92:54, 1:52, 80:25 |
| 82 | 00ad8a90-00af90a0 | 403 | 0.33 | 1 | 0 | 8 | terrain, mshd, mvfm, visibility, param, group, atlas | 87:100, 1:66, 2:65, 92:65, 95:45, 84:31 |
| 83 | 00af9660-00aff690 | 62 | 0.63 | 1 | 0 | 0 | persec, permeter, sphereemitter, smartareaemitter, renderpriority, particleemission, partemissiontype | 84:17, 1:15, 2:14, 92:12, 82:9 |
| 84 | 00aff700-00b20c50 | 399 | 0.32 | 1 | 0 | 10 | param, additive, emitter, particle, mvfm, initialrotation, emittedspeed | 83:62, 2:60, 1:56, 92:55, 87:52, 86:49 |
| 85 | 00b20d30-00b4c6a0 | 490 | 0.35 | 1 | 0 | 12 | mvfm, pf43cc, mshd, shadowmap, debugshader, shfx, uterrain4 | 2:43, 92:41, 1:40, 86:24, 87:21, 84:19 |
| 86 | 00b4c6d0-00b659d0 | 255 | 0.15 | 1 | 0 | 1 | mshd, cSampleOffsets, posteffectsysobj, posteffectsyscam, oldfilm_dust, cSampleWeights, mvfm | 87:112, 1:42, 2:42, 84:41, 92:41, 85:29 |
| 87 | 00b65ac0-00b75610 | 310 | 0.48 | 1 | 0 | 31 | dofile, userdata, thread, lightuserdata, dobuffer, cDummy, c3dobject | 2:34, 75:31, 1:25, 86:21, 92:15, 91:8 |
| 88 | 00b75720-00b78e80 | 72 | 0.65 | 0 | 0 | 0 | c3dnodeanimator, cAnimTrack, cOptimized3dNodeAnimator, cCameraAnimator |  |
| 89 | 00b78ed0-00b868b0 | 259 | 0.25 | 1 | 0 | 1 | cLight, cAmbientLight, boundingbox, resource, resourcedump_, refcounter, matrix | 2:35, 95:25, 92:24, 1:23, 87:21, 88:17 |
| 90 | 00b86930-00b8e6c0 | 123 | 0.5 | 2 | 0 | 3 | cSceneResource, flare, zoomfactor, targetname, node, flare2, dissolve | 95:22, 87:18, 2:13, 1:13, 89:13, 92:12 |
| 91 | 00b8e6f0-00bb82f0 | 464 | 0.19 | 1 | 0 | 1 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 87:191, 2:69, 1:66, 84:54, 92:50, 89:48 |
| 92 | 00bb83a0-00bd4200 | 356 | 0.31 | 1 | 0 | 94 | mvfm, mshd, cloud, waterdrops, thunder, streamed, lightning_002 | 87:66, 2:63, 1:55, 95:28, 84:11, 89:11 |
| 93 | 00bd4270-00bdb1e0 | 98 | 0.46 | 1 | 0 | 2 | files | 87:83, 2:28, 92:21, 1:12 |
| 94 | 00bdb2e0-00be8820 | 268 | 0.34 | 1 | 0 | 13 | cFileStore, long, iterator, removefile, removed, nagybetu, fileblock | 2:56, 92:55, 1:50, 93:32, 0:15 |
| 95 | 00be88c0-00c30570 | 270 | 0.42 | 1 | 0 | 7 | cPhysicalDirectoryX86, readfile, profile, cShaderTextureSource, cFileX86, openfileoverlapped, code | 2:56, 92:51, 1:43, 87:15, 94:12 |
