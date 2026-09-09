# Candidate partition: disjoint link-order segments

23927 unnamed non-thunk FUN_ candidates grouped into 92 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 848 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004133f0 | 188 | 0.1 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413470-0042a7e0 | 359 | 0.36 | 1 | 1 | 87 | 0 | 0 | mpakscenes, endgroup, crash, avoidzoneg, avoidzone, training, terraingridlayer | 90:35, 0:29, 9:15, 86:9 |
| 2 | 0042a830-0043f480 | 282 | 0.43 | 1 | 1 | 87 | 0 | 14 | bsp_chk_save, unlockto, unlockname, unlockfrom, playtime, difficulty, united | 86:152, 1:106, 90:37, 85:27, 29:25, 55:19; +4 more in JSON |
| 3 | 0043f4d0-0044c390 | 223 | 0.29 | 1 | 1 | 87 | 0 | 3 | panel, deviceclass, daytime, callback, setpanel, hidepanel, character | 86:55, 1:53, 90:23, 57:9 |
| 4 | 0044c5a0-004553e0 | 112 | 0.47 | 1 | 1 | 87 | 0 | 0 | dialogues, sequence, dialogdefaultpausetime, dialogcharacters, suppressinterruptmsg, requesttime, panelstates | 3:66, 1:41, 86:37, 90:19, 2:8 |
| 5 | 00455520-004ab550 | 1156 | 0.23 | 1 | 1 | 87 | 0 | 21 | soldiertypes, landvehicleclasses, entity, alpha, action, cameraposition, activatetime | 1:276, 86:135, 90:133, 2:52, 63:48, 59:44; +11 more in JSON |
| 6 | 004ab580-004ba080 | 294 | 0.21 | 1 | 1 | 87 | 0 | 4 | wreckclass, timeleft, smoke, wreck, state, timemin, timemax | 86:50, 1:39, 5:28, 90:21, 3:15 |
| 7 | 004ba0a0-004c4ed0 | 211 | 0.26 | 1 | 1 | 87 | 0 | 0 | collect, collectgarbage, traininggrounds, cloudsmall, multi, cloud, player | 6:29, 29:12 |
| 8 | 004c4fe0-004d1840 | 210 | 0.41 | 1 | 1 | 87 | 0 | 1 | white, allbutingame, interface, textures, writestats, userleft, stats | 1:89, 7:82, 90:49, 5:16, 6:12, 29:12; +1 more in JSON |
| 9 | 004d19c0-004f8830 | 404 | 0.28 | 1 | 1 | 87 | 1 | 11 | ggame, collect, collectgarbage, party, ingame, universe, scene | 1:179, 8:137, 7:112, 5:76, 86:57, 90:52; +25 more in JSON |
| 10 | 004f8970-00506ed0 | 212 | 0.62 | 1 | 1 | 87 | 0 | 0 | menuitem_text, vehicleclass, unitlib_nounlock, back, globals, dview, change | 1:110, 86:57, 90:47, 81:32, 80:27, 82:13; +4 more in JSON |
| 11 | 00506f00-0051e6e0 | 156 | 0.53 | 1 | 1 | 87 | 0 | 1 | back, dview, scroll_menu, globals, navigate, vehicleclass, szarnyas_framebox | 10:138, 1:114, 86:102, 90:48, 7:31, 81:31; +8 more in JSON |
| 12 | 0051e730-00527c80 | 109 | 0.72 | 1 | 1 | 87 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 29:31, 1:24, 7:18, 9:10, 2:10, 14:9; +2 more in JSON |
| 13 | 00527cb0-005439b0 | 310 | 0.4 | 1 | 1 | 87 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 1:115, 81:57, 90:43, 80:39, 9:37, 7:34; +8 more in JSON |
| 14 | 00543a30-00558640 | 309 | 0.3 | 1 | 1 | 87 | 0 | 3 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 1:74, 29:31, 90:30, 9:29, 81:21, 7:19; +7 more in JSON |
| 15 | 00558680-00568930 | 190 | 0.28 | 1 | 1 | 87 | 0 | 0 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 1:76, 14:76, 80:53, 81:29, 86:29, 90:27; +7 more in JSON |
| 16 | 00568cb0-0056d9a0 | 92 | 0.7 | 1 | 1 | 87 | 0 | 0 | cucc_group, setting_var_text, tilt_icon, slider_pos_group, setting_template_next_line_group, servers_framebox, servers_fix_group | 15:28, 1:9 |
| 17 | 0056dd80-00584110 | 340 | 0.3 | 1 | 1 | 87 | 0 | 1 | globals, server_text, players_text, mode_text, scroll_right_icon, wave_icon, main_newprofile | 1:72, 16:61, 15:26, 80:24, 90:23, 81:22; +5 more in JSON |
| 18 | 00584170-0058d470 | 72 | 0.56 | 1 | 1 | 87 | 0 | 1 | globals, select, navigate, back, mission_mappoint_, fe_pc, xsm_requiresprofile | 1:66, 17:39, 81:24, 80:24, 90:20, 86:12; +3 more in JSON |
| 19 | 0058d4b0-005c5cd0 | 494 | 0.27 | 1 | 1 | 87 | 0 | 2 | globals, mission_mappoint_, bushgroup, visibility, terrain, message, group | 1:156, 81:64, 18:61, 90:48, 86:47, 17:43; +15 more in JSON |
| 20 | 005c5da0-005ce450 | 132 | 0.36 | 1 | 1 | 87 | 0 | 0 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, uniquemultisettings | 1:20, 9:16, 86:16, 19:14, 90:11, 7:10 |
| 21 | 005ce490-005df510 | 149 | 0.72 | 1 | 1 | 87 | 0 | 1 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 1:94, 7:33, 80:31, 90:31, 9:26, 81:23; +9 more in JSON |
| 22 | 005e09a0-005fd0c0 | 197 | 0.63 | 1 | 1 | 87 | 0 | 5 | xsm_saveconfirm, fe_xbox, globals, back, xsm_dlcchanged, arrow_left_icon, mainlistbox_text | 1:209, 80:84, 21:76, 90:65, 81:63, 9:45; +16 more in JSON |
| 23 | 005fd120-006049f0 | 64 | 0.62 | 1 | 1 | 87 | 0 | 0 | ingame, felkialtojel_text, aaaaaa, paused, title_group, silverline_framebox, secondary_objectives_text | 1:50, 9:34, 80:27, 22:21, 90:17, 8:15; +4 more in JSON |
| 24 | 00604a20-00622990 | 291 | 0.27 | 1 | 1 | 87 | 0 | 3 | showgamercard, globals, usn_point_text, usn_icon, radar_sweep, pumpermanent, playerreview | 1:103, 81:45, 9:42, 2:41, 50:36, 90:33; +11 more in JSON |
| 25 | 00622a10-00654230 | 360 | 0.25 | 1 | 1 | 87 | 0 | 6 | ingame, globals, type_icon, scoring_unlock_text, score_text, medal_icon, commandbuilding_icon | 1:160, 24:83, 81:48, 29:47, 90:39, 9:37; +22 more in JSON |
| 26 | 00654250-006596e0 | 86 | 0.35 | 1 | 1 | 87 | 0 | 0 |  | 29:8 |
| 27 | 00659760-006626e0 | 124 | 0.36 | 1 | 1 | 87 | 0 | 0 | circle_hl_icon, ingamegui, normal, icon_l_icon, circle_small_02_group, circle_small_01_group, number_text | 1:54, 26:31, 78:24, 90:21, 81:20, 29:13 |
| 28 | 00663370-00683fa0 | 241 | 0.46 | 1 | 1 | 87 | 0 | 3 | ingame, sm_cp, globals, unitclass_spawnpoint, sm_support, inferiorfailure, warning_2_text | 27:119, 1:111, 26:55, 81:42, 9:42, 7:41; +15 more in JSON |
| 29 | 00684010-00696470 | 229 | 0.3 | 1 | 1 | 87 | 0 | 6 | gvmultimenu, pushrequestinterface, interface, textures, missionunique, missionhint, unique | 1:77, 9:73, 86:30, 90:30, 8:25, 25:24; +11 more in JSON |
| 30 | 006964b0-0069f940 | 161 | 0.58 | 1 | 1 | 87 | 0 | 0 | swapstickpairs, swapstickmap, swapstickgeneral, press, invio, inputmodifiers, indietro | 1:25, 86:21, 14:12, 90:8 |
| 31 | 0069fa40-006ad120 | 156 | 0.52 | 1 | 1 | 87 | 0 | 0 | sensitivitysettings, inputsettings, devicetype, deviceidx, slider, reverse, devinputs | 30:64, 1:47, 15:45, 14:37, 86:29, 3:28; +3 more in JSON |
| 32 | 006ad150-006be920 | 182 | 0.4 | 1 | 1 | 87 | 0 | 0 | savedata, scoring, missionid, entidcont, cont1, cont0, actunitid | 86:340, 1:146, 90:45, 5:23, 44:19, 3:16; +4 more in JSON |
| 33 | 006be980-006d6450 | 302 | 0.55 | 1 | 1 | 87 | 0 | 17 | equipment, state, slots, runwaywidth, runwaylength, runwayfailure, hangarfailure | 1:100, 29:81, 32:74, 2:30, 46:24, 5:19; +9 more in JSON |
| 34 | 006d6470-006dae20 | 103 | 0.38 | 2 | 34 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 1:22, 90:12 |
| 35 | 006dae90-006deff0 | 67 | 0.69 | 3 | 35 | 1 | 0 | 0 | markerclasses, recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 34:50, 86:10, 1:8 |
| 36 | 006df170-006f1380 | 219 | 0.42 | 1 | 1 | 87 | 0 | 20 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 1:95, 86:74, 42:31, 29:29, 63:29, 44:24; +10 more in JSON |
| 37 | 006f1440-006faf00 | 111 | 0.41 | 1 | 1 | 87 | 1 | 2 | tutorial, singleinvincibletime, shipyardrepairtime, repairtime, repairmedium, repairexpert, repairbasic | 1:41, 12:30, 86:20, 29:14, 90:10, 44:9; +1 more in JSON |
| 38 | 006fb030-0070c210 | 238 | 0.28 | 1 | 1 | 87 | 0 | 10 | openaftertime, messages, whosaysthat, velocity, openstate, dragvert, divedepth | 1:82, 86:67, 36:36, 90:33, 63:22, 5:20; +6 more in JSON |
| 39 | 0070cae0-00717980 | 153 | 0.33 | 1 | 1 | 87 | 0 | 0 | unitlist, unitid, shipnumber, shape, num_0, relativeposition, leader | 1:18 |
| 40 | 00717c70-0071b940 | 82 | 0.62 | 2 | 40 | 1 | 0 | 1 | mzonedesc, sphere, identifier, mnote, armor, points, category | 39:57, 89:13, 91:11 |
| 41 | 0071b9b0-00728fa0 | 214 | 0.45 | 1 | 1 | 87 | 0 | 14 | startmode, followmode, userpath, pathfollowparams, pathcursor, mgeommesh, internalclearprimarycommand | 1:63, 44:27, 46:19, 12:14, 49:12, 29:9 |
| 42 | 007290d0-00741140 | 274 | 0.27 | 1 | 1 | 87 | 0 | 17 | destroyed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed, torpedo | 1:99, 86:54, 90:48, 3:17, 41:13, 44:12; +5 more in JSON |
| 43 | 00741160-00758eb0 | 218 | 0.42 | 1 | 1 | 87 | 0 | 13 | landvehicle, landingship, landfort, sumleaks, sumforces, rampaelfordulas, partraszalltunk | 1:87, 86:48, 44:30, 66:28, 63:25, 5:24; +9 more in JSON |
| 44 | 00758f90-0076ea10 | 434 | 0.73 | 1 | 1 | 87 | 0 | 5 | p2p_voice__, woice, serversendscenescoring, runwaycenter, receive, nonce, myplayer | 1:679, 46:36, 90:12, 33:11, 86:9, 20:9 |
| 45 | 0076eaa0-007788b0 | 125 | 0.44 | 1 | 1 | 87 | 0 | 0 | p2p_voice__, peer, mmultiplayer, xusercheckprivilege, xuserareusersfriends, valid, setmuted | 44:104, 46:44, 1:41, 90:18, 7:14, 21:14; +3 more in JSON |
| 46 | 007788d0-00786a80 | 229 | 0.34 | 1 | 1 | 87 | 0 | 27 | reconlevel, multiscore, lastbanto, recondata, netentity, multiscore_save, hasplrcmd | 1:60, 86:44, 44:34, 63:27, 90:18, 39:17; +3 more in JSON |
| 47 | 00786be0-0078cf20 | 66 | 0.45 | 2 | 47 | 1 | 0 | 0 | send_, recv_, time, client, server | 90:30, 1:16, 88:13 |
| 48 | 0078cff0-007a4860 | 253 | 0.4 | 1 | 1 | 87 | 0 | 0 | postype, camera, thetalinearblend, theta, rholinearblend, blendtime, initialization | 1:123, 86:62, 2:51, 29:38, 90:29, 20:19; +4 more in JSON |
| 49 | 007a49a0-007b2dd0 | 132 | 0.61 | 1 | 1 | 87 | 0 | 8 | pathid, pathbaseentity, simple, paratrooper, soldieranim, slowfactoropened, slowfactorclosed | 48:79, 1:78, 2:28, 86:27, 85:20, 29:18; +6 more in JSON |
| 50 | 007b2e40-007d1d30 | 340 | 0.71 | 1 | 1 | 87 | 0 | 13 | powerlost, explosion, splash, enginefire, rightspinning, pathpoints, leftspinning | 1:127, 51:77, 2:72, 90:36, 44:33, 63:30; +12 more in JSON |
| 51 | 007d1dc0-0080d9b0 | 556 | 0.43 | 1 | 1 | 87 | 1 | 7 | travelspeed, gears, baydoor, wings, enemy, neutral, state | 1:201, 86:115, 2:82, 90:55, 50:54, 29:41; +19 more in JSON |
| 52 | 0080da00-0081a9c0 | 162 | 0.64 | 1 | 1 | 87 | 0 | 38 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 1:46, 66:35, 44:27, 46:21, 41:18, 63:17; +6 more in JSON |
| 53 | 0081aa10-00828810 | 97 | 0.44 | 1 | 1 | 87 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 52:78, 1:50, 55:18, 63:18, 86:18, 90:14; +5 more in JSON |
| 54 | 00828870-00851c70 | 430 | 0.27 | 1 | 1 | 87 | 2 | 7 | gameunit, classid, stock, torpedoavoidance, object, state, torpedoenabled | 1:91, 5:79, 86:60, 41:39, 90:23, 46:22; +15 more in JSON |
| 55 | 00851cb0-0086af80 | 303 | 0.53 | 1 | 1 | 87 | 0 | 10 | radius, submarine, torpedo, unlimitedair, tvertangle, turninggun, thorzangle | 1:74, 86:74, 2:38, 36:21, 90:17, 5:16; +13 more in JSON |
| 56 | 0086afc0-00878340 | 176 | 0.4 | 1 | 1 | 87 | 0 | 4 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 86:77, 90:60, 1:49, 55:27, 82:11, 63:10; +1 more in JSON |
| 57 | 00878350-008828e0 | 109 | 0.43 | 1 | 1 | 87 | 0 | 2 | damage, yellow, weaponsystems, weapondirectorthinktime, warningscrollspeeds, visibletimeout, visibilityrange | 86:40, 56:33, 1:21, 90:12 |
| 58 | 00882ac0-0088b120 | 128 | 0.55 | 1 | 1 | 87 | 0 | 0 | scripts, debugtrap, shallowwater, modelpath, luab, filepath, colormap | 1:61, 32:51, 86:47, 90:24, 36:21 |
| 59 | 0088b190-008e5c40 | 879 | 0.76 | 1 | 1 | 87 | 528 | 2 | luakod, szurkenyil, options, hardwarereported, english, xboxcompatibilitymode, vsync | 86:3876, 1:1850, 90:602, 58:436, 2:55, 32:55; +31 more in JSON |
| 60 | 008e5c50-008ee9b0 | 119 | 0.6 | 1 | 1 | 87 | 0 | 1 | pup_gain, pum1stget, uspumicon, uselimit, unitclassindex, targettype, targetfilter | 1:59, 59:49, 86:25, 90:23, 24:16, 3:16; +2 more in JSON |
| 61 | 008eea80-008f10b0 | 71 | 0.58 | 1 | 1 | 87 | 0 | 0 | vec3array, intarray, floatarray, bytes, none, lua_s | 2:11, 65:9 |
| 62 | 008f10c0-00922c80 | 507 | 0.31 | 1 | 1 | 87 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 1:218, 86:94, 17:78, 2:77, 90:67, 61:62; +16 more in JSON |
| 63 | 00922de0-0092dfe0 | 180 | 0.47 | 1 | 1 | 87 | 0 | 78 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 86:43, 1:38, 5:19, 90:17, 29:12, 32:12; +1 more in JSON |
| 64 | 0092e0b0-00943d60 | 209 | 0.34 | 1 | 1 | 87 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 1:81, 86:38, 63:31, 90:24, 55:14, 2:13; +2 more in JSON |
| 65 | 00943d80-00951d00 | 147 | 0.42 | 1 | 1 | 87 | 0 | 1 | resourceusage, ownerplayer, supportmanager, velocitysi, effect, camocolor, autoattacktarget | 86:69, 1:68, 62:32, 64:23, 90:22, 5:18; +2 more in JSON |
| 66 | 00951d20-00968e00 | 279 | 0.29 | 1 | 1 | 87 | 0 | 37 | vehicleclass, inferiorfailure, reconplane, torpedobomber, torpedoboat, divebomber, cargo | 86:79, 1:76, 2:45, 5:24, 43:19, 3:18; +10 more in JSON |
| 67 | 00968e80-0098c9c0 | 479 | 0.33 | 1 | 1 | 87 | 0 | 29 | player, callback, entity, message, oldlevel, newlevel, ambient | 1:240, 90:95, 58:94, 66:81, 36:34, 86:31; +14 more in JSON |
| 68 | 0098d400-00996510 | 148 | 0.44 | 1 | 1 | 87 | 0 | 2 | mpkg, cast, bsm_hwd, content, language, eidos, software | 1:38, 90:15, 0:12 |
| 69 | 009965d0-009f69c0 | 844 | 0.83 | 1 | 1 | 87 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:417, 50:266, 2:233, 70:167, 0:113, 90:105; +21 more in JSON |
| 70 | 009f6a20-009ffad0 | 117 | 0.63 | 1 | 1 | 87 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:41, 51:19, 41:19, 2:15, 50:15, 86:13; +2 more in JSON |
| 71 | 009ffb40-00a07d40 | 130 | 0.59 | 1 | 1 | 87 | 0 | 0 | vehicle, class, captureweight | 70:34, 1:19 |
| 72 | 00a07e40-00a19410 | 126 | 0.59 | 1 | 1 | 87 | 0 | 5 | commandtype, neutral, aivstable_, enemy, target, vehicleclass, s_to_ | 1:62, 71:51, 86:25, 75:19, 74:19, 90:18; +7 more in JSON |
| 73 | 00a19480-00a1fba0 | 129 | 0.41 | 1 | 1 | 87 | 0 | 0 | coordinator, sell, strategicgain, duel, defend, capture, siege | 1:37, 74:17, 90:11 |
| 74 | 00a1fdb0-00a335d0 | 189 | 0.38 | 1 | 1 | 87 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 73:73, 72:50, 1:48, 71:27, 86:27, 7:20; +3 more in JSON |
| 75 | 00a371a0-00a427a0 | 154 | 0.42 | 1 | 1 | 87 | 0 | 6 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 86:19, 1:16, 46:13, 74:9, 90:8, 76:8 |
| 76 | 00a428d0-00a625d0 | 180 | 0.51 | 1 | 1 | 87 | 0 | 3 | online, server, player, query, matchmaking, remote, movie | 1:23, 75:22, 68:21, 7:11, 90:10, 17:9; +1 more in JSON |
| 77 | 00a62660-00a798c0 | 329 | 0.64 | 1 | 1 | 87 | 0 | 1 | unexpected, chunk, precompiled, complex, expression, call, many | 1:24, 79:16, 90:13 |
| 78 | 00a79910-00a82b70 | 134 | 0.67 | 1 | 1 | 87 | 0 | 7 | memory, sounjd, sound, play, event, system, init | 86:35, 1:33, 90:19 |
| 79 | 00a82c60-00a98400 | 291 | 0.29 | 1 | 1 | 87 | 0 | 6 | memory, sounjd, stream, sound, streaming, stereo, request | 1:82, 90:48, 78:46, 84:20, 86:20, 91:15; +2 more in JSON |
| 80 | 00a98cc0-00aa5b40 | 202 | 0.38 | 1 | 1 | 87 | 0 | 3 | cGuiManager, heightplus, dontmovetheitems, centervertical, camerastore, autocontrol, lockit | 1:45, 82:39, 81:31, 90:15, 7:11 |
| 81 | 00aa5d70-00aaf4a0 | 164 | 0.43 | 1 | 1 | 87 | 0 | 38 | widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor, label | 1:42, 80:34, 86:31, 90:16 |
| 82 | 00aaf4e0-00af9660 | 935 | 0.3 | 1 | 1 | 87 | 0 | 27 | mshd, texture, mvfm, shadername, guidefault, simplecolor, terrain | 1:278, 86:172, 81:148, 90:132, 91:58, 84:32; +9 more in JSON |
| 83 | 00af9d00-00aff690 | 61 | 0.62 | 1 | 1 | 87 | 0 | 0 | persec, permeter, sphereemitter, smartareaemitter, renderpriority, particleemission, partemissiontype | 1:25, 84:17, 90:11 |
| 84 | 00aff700-00b20910 | 390 | 0.26 | 1 | 1 | 87 | 0 | 10 | param, additive, emitter, particle, mvfm, initialrotation, emittedspeed | 1:115, 83:62, 85:56, 90:54, 86:52, 82:31; +1 more in JSON |
| 85 | 00b20a80-00b65ac0 | 748 | 0.27 | 1 | 1 | 87 | 0 | 13 | mshd, cSampleOffsets, mvfm, pf43cc, shadowmap, posteffectsysobj, posteffectsyscam | 1:166, 86:123, 90:82, 84:44, 7:12, 91:9 |
| 86 | 00b65ba0-00b75d80 | 311 | 0.46 | 1 | 1 | 87 | 0 | 31 | dofile, userdata, thread, lightuserdata, dobuffer, cDummy, c3dobject | 1:59, 77:31, 85:24, 90:17, 88:8 |
| 87 | 00b75de0-00b78ed0 | 68 | 0.69 | 1 | 1 | 87 | 0 | 0 | c3dnodeanimator, cAnimTrack, cOptimized3dNodeAnimator, cCameraAnimator | 1:8 |
| 88 | 00b78f60-00b866c0 | 252 | 0.41 | 1 | 1 | 87 | 0 | 1 | cLight, cAmbientLight, boundingbox, resource, resourcedump_, refcounter, matrix | 1:57, 90:29, 91:25, 86:19, 87:17, 84:9 |
| 89 | 00b86720-00b91000 | 186 | 0.47 | 1 | 1 | 87 | 0 | 3 | cSceneResource, cGroupParamsResource, cGroup, flare, zoomfactor, targetname, node | 86:29, 1:28, 91:22, 90:13, 87:11 |
| 90 | 00b911d0-00bd4200 | 746 | 0.22 | 1 | 1 | 87 | 0 | 96 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 1:244, 86:201, 91:80, 84:50, 88:35, 13:28; +5 more in JSON |
| 91 | 00bd4270-00c30570 | 607 | 0.34 | 1 | 1 | 87 | 0 | 23 | long, iterator, cFileStore, removefile, removed, profile, nagybetu | 1:217, 90:114, 86:98, 0:19, 14:11, 5:9; +1 more in JSON |
