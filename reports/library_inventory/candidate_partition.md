# Candidate partition: disjoint link-order segments

23796 unnamed non-thunk FUN_ candidates grouped into 92 disjoint function-start ranges (Louvain resolution 0.5, smoothing window 12, min segment 60); 865 candidate function-pointer runs found in .rdata. These are not validated vtables.

Waves use every candidate-to-candidate segment dependency with >= 8 unique caller-target relationships (including direct tail jumps). JSON retains all edges; only the table display is shortened. Segments in one SCC share a wave and remain mutually dependent. Wave 0 means no outgoing strong edge to another SCC in this limited graph, not that the code is ready to implement independently.

Named/incomplete functions, indirect calls, shared state and below-threshold dependencies remain outside this scheduling graph. FUN_ entries may have provisional inventory bookmarks. Purity is the largest Louvain community share, not source-module confidence. Lua binding counts are string-reference hints. Segment IDs can change after a new snapshot; delegate explicit function addresses and disjoint files.

| Seg | Function-start range | Cands | Purity | Wave | SCC | SCC size | Lua hints | Pointer runs | Keywords | Strong deps out (segment:relationships) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004133f0 | 187 | 0.11 | 0 | 0 | 1 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413470-0042a480 | 358 | 0.35 | 1 | 1 | 81 | 0 | 0 | mpakscenes, endgroup, crash, avoidzoneg, avoidzone, training, terraingridlayer | 89:34, 0:27, 7:14, 86:9 |
| 2 | 0042a7e0-004486c0 | 430 | 0.3 | 1 | 1 | 81 | 0 | 17 | bsp_chk_save, deviceclass, daytime, unlockto, unlockname, unlockfrom, playtime | 86:206, 1:137, 89:51, 85:27, 29:25, 53:15; +7 more in JSON |
| 3 | 00448700-0044c390 | 75 | 0.45 | 1 | 1 | 81 | 0 | 1 | panel, callback, setpanel, hidepanel, character, pause, message | 1:22, 89:9 |
| 4 | 0044c5a0-00491070 | 1031 | 0.25 | 1 | 1 | 81 | 0 | 18 | entity, alpha, action, cameraposition, activatetime, aa_flak, point | 1:272, 89:129, 86:115, 2:60, 3:54, 56:44; +9 more in JSON |
| 5 | 00491170-004cb130 | 865 | 0.21 | 1 | 1 | 81 | 0 | 8 | soldiertypes, landvehicleclasses, wreckclass, white, tempid, smokeefx, rotationdecline | 4:261, 1:138, 86:103, 89:73, 2:40, 29:26; +8 more in JSON |
| 6 | 004cb160-004e81b0 | 326 | 0.31 | 1 | 1 | 81 | 1 | 6 | ggame, collect, collectgarbage, ingame, universe, scene, skiptitle | 5:218, 1:132, 89:65, 4:43, 86:40, 2:26; +16 more in JSON |
| 7 | 004e8240-0051e730 | 522 | 0.29 | 1 | 1 | 81 | 0 | 6 | vehicleclass, menuitem_text, back, globals, dview, navigate, scroll_menu | 1:304, 86:175, 89:104, 5:61, 6:58, 79:56; +15 more in JSON |
| 8 | 0051e7e0-00525690 | 96 | 0.56 | 1 | 1 | 81 | 0 | 0 | attackmove, showocean, showfoliage, showboundings, scriptwatchrefresh, scriptcheats, rendernodes | 1:22, 29:21, 5:18, 7:16, 2:9, 11:9; +2 more in JSON |
| 9 | 00525720-00532360 | 153 | 0.6 | 1 | 1 | 81 | 0 | 1 | cycle, target, back, globals, text_b_text, text_a_text, submarine_group | 1:79, 8:32, 89:32, 79:28, 5:23, 78:22; +8 more in JSON |
| 10 | 005329c0-005439b0 | 169 | 0.36 | 1 | 1 | 81 | 0 | 0 | mshd, vidm, pg_6s_text, pg_6p_text, pg_6_text, pg_5s_text, pg_5p_text | 1:38, 5:22, 80:14, 29:14, 86:13, 79:13; +7 more in JSON |
| 11 | 00543a30-005525b0 | 175 | 0.41 | 1 | 1 | 81 | 0 | 2 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 1:49, 29:31, 6:21, 5:20, 79:15, 10:13; +6 more in JSON |
| 12 | 00552630-00558640 | 134 | 0.57 | 1 | 1 | 81 | 0 | 1 | setting_group, selectedconflict_text, listbox_framebox, fe_controls_pc_listbox, fe_controls_pc, column_framebox, toggle_text_group | 1:25, 89:17 |
| 13 | 00558680-00563540 | 127 | 0.54 | 1 | 1 | 81 | 0 | 0 | fe_pc, preset, presets, opt_normal, opt_inverted, opt_cancel, globals | 12:71, 1:61, 77:36, 89:20, 79:19, 29:18; +6 more in JSON |
| 14 | 005635f0-005817c0 | 409 | 0.61 | 1 | 1 | 81 | 0 | 1 | globals, server_text, players_text, mode_text, scroll_right_icon, back, wave_icon | 1:93, 77:38, 89:32, 29:32, 80:29, 79:28; +7 more in JSON |
| 15 | 005817d0-0058b520 | 136 | 0.38 | 1 | 1 | 81 | 0 | 2 | select, navigate, globals, back, fe_pc, xsm_requiresprofile, xsm_requiresmultiprivilege | 1:57, 14:39, 77:22, 89:19, 79:17, 86:12; +2 more in JSON |
| 16 | 0058b5d0-00594740 | 120 | 0.71 | 1 | 1 | 81 | 0 | 0 | mission_mappoint_, profile_nameconflict, profile_cantcreate, main_uacps, main_uacp, sec_objective_group, sec_group | 15:63, 1:33, 14:18, 7:10, 89:9 |
| 17 | 00594860-005c5cd0 | 396 | 0.44 | 1 | 1 | 81 | 0 | 1 | globals, bushgroup, visibility, terrain, message, group, mvfm | 1:135, 79:48, 89:41, 80:40, 86:30, 2:28; +17 more in JSON |
| 18 | 005c5da0-005cfb80 | 164 | 0.29 | 1 | 1 | 81 | 0 | 1 | up_icon, helpline, gui_movie, down_icon, datatables, scripts, uniquemultisettings | 1:28, 5:19, 7:16, 86:16, 89:15, 17:14; +2 more in JSON |
| 19 | 005cfc60-005e0db0 | 123 | 0.62 | 1 | 1 | 81 | 0 | 0 | globals, ingame, messagetemplate_text, settings_group, player_name_text, willendsession, unlock_group | 1:86, 5:43, 89:27, 77:25, 43:24, 6:22; +10 more in JSON |
| 20 | 005e10c0-005ef810 | 79 | 0.73 | 2 | 20 | 1 | 0 | 3 | globals, navigate, back, achievement_lock_icon, players_listbox, rank_icon, change | 1:81, 19:65, 77:45, 29:43, 79:34, 5:32; +8 more in JSON |
| 21 | 005ef8d0-00604bc0 | 179 | 0.45 | 1 | 1 | 81 | 0 | 2 | xsm_saveconfirm, fe_xbox, xsm_dlcchanged, arrow_left_icon, ingame, arrow_right_icon, mainlistbox_text | 1:178, 89:60, 6:52, 77:50, 5:43, 56:31; +9 more in JSON |
| 22 | 00604c20-0061f8a0 | 274 | 0.38 | 1 | 1 | 81 | 0 | 3 | usn_point_text, usn_icon, radar_sweep, pumpermanent, player_point_text, circle_full_, section | 1:87, 2:48, 5:39, 48:36, 79:34, 29:30; +9 more in JSON |
| 23 | 0061f920-00639f40 | 133 | 0.4 | 1 | 1 | 81 | 0 | 1 | globals, rank_icon, continue, scoring_unlock_text, playerreview, move_group, debriefing_clipbox | 22:66, 1:65, 86:28, 7:21, 89:18, 5:16; +7 more in JSON |
| 24 | 0063a280-006440c0 | 93 | 0.42 | 2 | 24 | 1 | 0 | 0 | type_icon, commandbuilding_icon, close_group, globals, yellow_icon, white_icon, spectator_text | 1:58, 23:38, 79:17, 89:13 |
| 25 | 006441b0-00651370 | 123 | 0.51 | 1 | 1 | 81 | 0 | 5 | ingame, vehicleclass, sub_depth_arrow_icon, sub_depht_arrow_dest_icon, sub_air_warning_icon, sub_air_arrow_icon, shipcaptain | 1:49, 29:37, 5:21, 43:20, 7:17, 86:17; +10 more in JSON |
| 26 | 006515b0-006596e0 | 111 | 0.44 | 1 | 1 | 81 | 0 | 0 |  | 29:14, 1:10, 43:9 |
| 27 | 00659760-0065cdf0 | 88 | 0.62 | 1 | 1 | 81 | 0 | 0 |  | 26:33, 29:10 |
| 28 | 0065ce20-00680db0 | 240 | 0.7 | 1 | 1 | 81 | 0 | 2 | circle_hl_icon, ingamegui, normal, icon_l_icon, globals, sm_cp, ingame | 1:149, 27:95, 26:61, 89:46, 5:42, 79:38; +15 more in JSON |
| 29 | 00681ba0-00696470 | 266 | 0.39 | 1 | 1 | 81 | 0 | 7 | gvmultimenu, ingame, pushrequestinterface, interface, textures, warning_2_text, missionunique | 1:90, 6:66, 7:46, 86:38, 5:36, 89:36; +10 more in JSON |
| 30 | 006964b0-0069fa40 | 162 | 0.62 | 1 | 1 | 81 | 0 | 0 | swapstickpairs, swapstickmap, swapstickgeneral, press, invio, inputmodifiers, indietro | 1:25, 86:21, 12:12, 89:8 |
| 31 | 0069fb50-006a9ea0 | 127 | 0.71 | 1 | 1 | 81 | 0 | 0 | devinputs, sensitivitysettings, inputsettings, devicetype, deviceidx, slider, down | 30:54, 13:30, 12:28, 1:27, 86:20, 89:14; +2 more in JSON |
| 32 | 006a9f70-006be980 | 211 | 0.37 | 1 | 1 | 81 | 0 | 0 | savedata, scoring, entities, missionid, entidcont, cont1, cont0 | 86:349, 1:166, 89:48, 4:42, 2:31, 43:21; +7 more in JSON |
| 33 | 006beb10-006d6450 | 301 | 0.51 | 1 | 1 | 81 | 0 | 18 | equipment, state, slots, runwaywidth, runwaylength, runwayfailure, hangarfailure | 1:100, 29:81, 32:73, 43:37, 2:33, 4:19; +8 more in JSON |
| 34 | 006d6470-006da5b0 | 99 | 0.43 | 2 | 34 | 1 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 1:20, 89:10 |
| 35 | 006da670-006e06f0 | 88 | 0.59 | 3 | 35 | 1 | 0 | 6 | firedelaytime, erroroffset, delayedfire, calcerrtick, repeattime, markerclasses, launchspeed | 34:47, 86:24, 1:23, 89:8 |
| 36 | 006e0860-006f06f0 | 191 | 0.39 | 1 | 1 | 81 | 0 | 19 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 1:81, 86:52, 43:31, 2:31, 42:31, 29:28; +6 more in JSON |
| 37 | 006f0700-0070c210 | 360 | 0.3 | 1 | 1 | 81 | 1 | 18 | openaftertime, messages, depthcharge, datatables, scripts, mvfm, mshd | 1:123, 86:66, 89:43, 36:37, 29:34, 8:31; +11 more in JSON |
| 38 | 0070cae0-00714030 | 103 | 0.42 | 1 | 1 | 81 | 0 | 0 | unitlist, unitid, shipnumber, shape, num_0, relativeposition, leader | 1:16, 2:8 |
| 39 | 00714060-0071b710 | 104 | 0.52 | 0 | 39 | 1 | 0 | 3 | sphere, mzonedesc, mnote, identifier, armor, points, category |  |
| 40 | 0071ba20-00722fb0 | 122 | 0.66 | 1 | 1 | 81 | 0 | 5 | startmode, followmode, userpath, pathfollowparams, pathcursor, internalclearprimarycommand, emptycommand | 1:48, 43:42, 47:14, 8:14, 29:9 |
| 41 | 00723030-00727c30 | 66 | 0.67 | 2 | 41 | 1 | 0 | 1 | mgeommesh | 40:13, 1:8 |
| 42 | 00727d30-00758e40 | 514 | 0.29 | 1 | 1 | 81 | 0 | 48 | destroyed, speed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed | 1:193, 86:100, 43:79, 89:69, 2:41, 62:33; +18 more in JSON |
| 43 | 00758eb0-007828a0 | 719 | 0.44 | 1 | 1 | 81 | 0 | 30 | p2p_voice__, reconlevel, multiscore, mmultiplayer, lastbanto, player, recondata | 1:771, 44:70, 86:65, 89:48, 5:40, 59:29; +14 more in JSON |
| 44 | 007828b0-00786a80 | 70 | 0.49 | 1 | 1 | 81 | 0 | 3 |  | 1:9, 43:9 |
| 45 | 00786be0-0078e630 | 95 | 0.32 | 1 | 1 | 81 | 0 | 0 | initialization, send_, recv_, moviecameraouter, moviecameradeck, beam, waterparticle | 89:43, 88:28, 1:23 |
| 46 | 0078e790-007a4860 | 224 | 0.4 | 1 | 1 | 81 | 0 | 0 | postype, camera, thetalinearblend, theta, rholinearblend, blendtime, parent | 1:116, 86:61, 2:51, 29:34, 18:19, 89:16; +3 more in JSON |
| 47 | 007a49a0-007b3730 | 143 | 0.55 | 1 | 1 | 81 | 0 | 11 | pathbaseentity, simple, sustainbefore, rotrefentity, pathpoints, pathintf, pathid | 1:82, 46:78, 2:31, 86:27, 43:21, 85:20; +6 more in JSON |
| 48 | 007b38d0-007d3e60 | 333 | 0.66 | 1 | 1 | 81 | 0 | 19 | powerlost, explosion, enginefire, splash, rightspinning, leftspinning, kamikazebulletclass | 1:126, 2:78, 49:72, 43:57, 89:36, 86:28; +12 more in JSON |
| 49 | 007d5890-00809740 | 501 | 0.42 | 1 | 1 | 81 | 1 | 7 | enemy, neutral, state, datatables, scripts, windsound, unlocks | 1:190, 86:91, 2:75, 89:54, 48:49, 43:41; +15 more in JSON |
| 50 | 00809820-0081aa10 | 214 | 0.59 | 1 | 1 | 81 | 0 | 39 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 1:53, 43:52, 2:31, 61:29, 59:21, 40:17; +8 more in JSON |
| 51 | 0081aa60-00828810 | 96 | 0.45 | 1 | 1 | 81 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 50:83, 1:50, 53:18, 59:18, 86:18, 43:15; +4 more in JSON |
| 52 | 00828870-00851f50 | 437 | 0.28 | 1 | 1 | 81 | 2 | 7 | gameunit, classid, stock, torpedoavoidance, object, state, torpedoenabled | 1:92, 86:64, 4:45, 43:41, 5:37, 40:36; +16 more in JSON |
| 53 | 00851fa0-0086af80 | 296 | 0.46 | 1 | 1 | 81 | 0 | 13 | radius, submarine, torpedo, unlimitedair, tvertangle, turninggun, thorzangle | 1:73, 86:67, 2:48, 36:19, 89:17, 43:16; +12 more in JSON |
| 54 | 0086afc0-00877e50 | 168 | 0.42 | 1 | 1 | 81 | 0 | 3 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 86:76, 89:60, 1:49, 53:27, 82:11, 5:11; +3 more in JSON |
| 55 | 00877fa0-0088b120 | 245 | 0.27 | 1 | 1 | 81 | 0 | 2 | damage, scripts, debugtrap, datatables, yellow, weaponsystems, weapondirectorthinktime | 86:84, 1:82, 32:51, 89:34, 54:26, 36:21; +4 more in JSON |
| 56 | 0088b190-008e62a0 | 890 | 0.74 | 1 | 1 | 81 | 528 | 2 | luakod, szurkenyil, options, hardwarereported, english, xboxcompatibilitymode, vsync | 86:3873, 1:1851, 89:603, 55:437, 43:129, 2:64; +22 more in JSON |
| 57 | 008e6400-008ee990 | 106 | 0.54 | 2 | 57 | 1 | 0 | 1 | pup_gain, pum1stget, uspumicon, uselimit, unitclassindex, targettype, targetfilter | 1:58, 56:50, 86:25, 89:22, 22:15, 43:12; +2 more in JSON |
| 58 | 008ee9b0-00922b70 | 577 | 0.24 | 1 | 1 | 81 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 1:215, 86:94, 2:92, 89:66, 7:45, 15:45; +14 more in JSON |
| 59 | 00922b90-0092dfe0 | 182 | 0.43 | 1 | 1 | 81 | 0 | 79 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 86:43, 1:41, 4:19, 89:18, 29:12, 32:12; +3 more in JSON |
| 60 | 0092e0b0-00943c00 | 208 | 0.34 | 1 | 1 | 81 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 1:81, 59:31, 86:29, 89:25, 2:17, 53:14; +3 more in JSON |
| 61 | 00943d60-00953ec4 | 183 | 0.44 | 1 | 1 | 81 | 0 | 7 | resourceusage, ownerplayer, inferiorfailure, supportmanager, velocitysi, effect, camocolor | 86:70, 1:68, 58:35, 60:22, 89:22, 5:19; +5 more in JSON |
| 62 | 00953f60-00968e00 | 244 | 0.17 | 1 | 1 | 81 | 0 | 31 | vehicleclass, reconplane, torpedobomber, torpedoboat, divebomber, cargo, fighter | 86:78, 1:76, 2:62, 42:35, 61:30, 4:22; +9 more in JSON |
| 63 | 00968e80-00970f90 | 166 | 0.45 | 2 | 63 | 1 | 0 | 18 | callback, entity, oldlevel, newlevel, message, ingame, party | 62:39, 1:25, 89:11, 29:8 |
| 64 | 00970ff0-00978c70 | 103 | 0.55 | 3 | 64 | 1 | 0 | 7 | player, message, plane, other, ambient, callback, entity | 1:65, 63:42, 62:21, 89:15, 43:8, 17:8 |
| 65 | 00978ca0-0098c820 | 206 | 0.28 | 4 | 65 | 1 | 0 | 4 | repair, player, shiplanded, musicover, hpevent, generate, entitykilled | 1:150, 64:99, 55:93, 89:69, 63:68, 36:32; +10 more in JSON |
| 66 | 0098c870-00996270 | 150 | 0.52 | 1 | 1 | 81 | 0 | 2 | software, mpkg, cast, bsm_hwd, content, language, eidos | 1:35, 89:15, 0:11 |
| 67 | 00996390-009f69c0 | 846 | 0.84 | 1 | 1 | 81 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:419, 48:266, 2:234, 68:167, 0:111, 89:105; +22 more in JSON |
| 68 | 009f6a20-009fe0b0 | 86 | 0.76 | 1 | 1 | 81 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:41, 40:19, 49:17, 2:15, 86:13, 29:13; +2 more in JSON |
| 69 | 009fe0f0-00a0fde0 | 183 | 0.69 | 1 | 1 | 81 | 0 | 0 | neutral, aivstable_, enemy, vehicle, class, vehicleclass, s_to_ | 1:38, 2:18, 86:17, 71:14, 89:14, 68:12; +1 more in JSON |
| 70 | 00a102d0-00a1fa60 | 230 | 0.63 | 1 | 1 | 81 | 0 | 5 | coordinator, commandtype, capture, target, sell, strategicgain, duel | 1:80, 71:47, 69:28, 89:19, 43:14, 29:12; +2 more in JSON |
| 71 | 00a1fa90-00a371a0 | 193 | 0.79 | 1 | 1 | 81 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 70:120, 1:48, 69:36, 86:27, 5:23, 89:16; +2 more in JSON |
| 72 | 00a371c0-00a42cf0 | 155 | 0.53 | 1 | 1 | 81 | 0 | 6 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 86:19, 1:16, 44:13, 73:10, 71:9, 89:8 |
| 73 | 00a42d70-00a625d0 | 178 | 0.44 | 1 | 1 | 81 | 0 | 3 | online, server, player, query, matchmaking, remote, movie | 1:23, 72:22, 66:21, 5:15, 89:10, 14:10; +1 more in JSON |
| 74 | 00a62660-00a7a440 | 335 | 0.63 | 1 | 1 | 81 | 0 | 1 | unexpected, chunk, precompiled, call, complex, expression, many | 1:24, 76:16, 89:13 |
| 75 | 00a7a460-00a82b70 | 128 | 0.66 | 1 | 1 | 81 | 0 | 7 | sounjd, memory, sound, volume, play, event, system | 86:35, 1:33, 89:19 |
| 76 | 00a82c60-00a9a4a0 | 301 | 0.24 | 1 | 1 | 81 | 0 | 6 | memory, sounjd, stream, sound, streaming, stereo, request | 1:89, 89:49, 75:39, 84:20, 86:19, 91:13; +3 more in JSON |
| 77 | 00a9a5a0-00aa0ff0 | 97 | 0.53 | 1 | 1 | 81 | 0 | 3 | heightplus, dontmovetheitems, centervertical, autocontrol, lockit, linedistance, items | 1:24, 79:18, 5:9 |
| 78 | 00aa1040-00aa6820 | 103 | 0.57 | 1 | 1 | 81 | 0 | 0 | cGuiManager, camerastore, scrollbar, safezone_43_framebox, safezone_169_framebox, progbar, mouseptrgui_icon | 80:28, 1:16, 79:12, 81:8, 89:8 |
| 79 | 00aa6870-00aace40 | 111 | 0.42 | 1 | 1 | 81 | 0 | 38 | widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor, label | 1:17, 87:13, 86:13, 78:10 |
| 80 | 00aacf10-00acd6b0 | 406 | 0.28 | 1 | 1 | 81 | 0 | 13 | texture, simplecolor, mvfm, mshd, guidefault, shadername, vertical_scrollbar | 1:137, 79:108, 89:62, 86:42, 87:32, 78:15; +6 more in JSON |
| 81 | 00acd800-00ad7500 | 153 | 0.25 | 1 | 1 | 81 | 0 | 6 | playbydefault, framesizesy, cGuiSound, borderwidth, soundefx, hastexture, forward | 79:21, 1:19, 82:11, 89:9 |
| 82 | 00ad7520-00af9660 | 421 | 0.34 | 1 | 1 | 81 | 0 | 9 | mshd, terrain, mvfm, visibility, param, group, bush | 1:145, 89:71, 87:43, 86:42, 83:42, 81:27; +5 more in JSON |
| 83 | 00af9d00-00b03940 | 109 | 0.46 | 1 | 1 | 81 | 0 | 3 | param, windsensitivity, verticalspeed, persec, permeter, inheritedspeed, emittedspeed | 1:36, 82:21, 89:18, 84:16 |
| 84 | 00b03970-00b20dc0 | 347 | 0.31 | 1 | 1 | 81 | 0 | 12 | param, additive, mvfm, initialrotation, cShaderDataSource, dynamic_light_, width | 1:104, 83:88, 85:54, 89:47, 86:46, 82:17 |
| 85 | 00b20e70-00b659d0 | 735 | 0.28 | 1 | 1 | 81 | 0 | 19 | mshd, cSampleOffsets, mvfm, pf43cc, shadowmap, posteffectsysobj, posteffectsyscam | 1:161, 86:103, 89:79, 84:57, 87:16, 5:11; +1 more in JSON |
| 86 | 00b65ac0-00b72080 | 233 | 0.55 | 1 | 1 | 81 | 0 | 31 | dofile, userdata, thread, lightuserdata, dobuffer, c3dnode, fundamentals | 1:51, 74:31, 85:22, 89:22 |
| 87 | 00b72140-00b75f00 | 80 | 0.4 | 1 | 1 | 81 | 0 | 0 | cDummy, c3dobject, cMesh | 86:17, 1:8 |
| 88 | 00b762b0-00b86b90 | 297 | 0.37 | 1 | 1 | 81 | 0 | 2 | c3dnodeanimator, cSceneResource, cLight, cAnimTrack, cAmbientLight, resourcedump_, refcounter | 1:48, 89:27, 87:17, 37:12, 86:10, 84:9; +1 more in JSON |
| 89 | 00b86d00-00bd4200 | 907 | 0.21 | 1 | 1 | 81 | 0 | 23 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 1:263, 87:109, 86:103, 88:60, 84:52, 85:28; +7 more in JSON |
| 90 | 00bd4270-00bed990 | 425 | 0.36 | 1 | 1 | 81 | 0 | 20 | cFileStore, long, iterator, removefile, removed, profile, nagybetu | 1:152, 89:85, 86:83, 91:12, 4:8 |
| 91 | 00beda60-00c30570 | 151 | 0.32 | 1 | 1 | 81 | 0 | 5 | cShaderTextureSource, cPhysicalDirectoryX86, cFileX86, protected, getfiledate, getfileattributesex, cShaderAnimTextureSource | 1:45, 89:21, 86:15, 90:10 |
