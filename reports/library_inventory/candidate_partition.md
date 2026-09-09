# Candidate partition: disjoint link-order segments

23992 untagged FUN_ candidates cut into 87 disjoint address segments (Louvain resolution 0.5, smoothing window 12, min segment 60); 847 vtables found in .rdata. Wave 0 segments have no strong outbound dependency (>= 8 calls) on unfinished segments; segments in one cycle group share a wave.

| Seg | Range | Cands | Purity | Wave | Lua bind | Vtables | Keywords | Strong deps out (seg:calls) |
|---|---|---:|---:|---:|---:|---:|---|---|
| 0 | 00401010-004132d0 | 185 | 0.03 | 0 | 0 | 0 | stateindex, undefined, state |  |
| 1 | 00413330-0042a7e0 | 362 | 0.41 | 1 | 0 | 0 | mpakscenes, endgroup, crash, avoidzoneg, avoidzone, training, terraingridlayer | 84:35, 0:24, 7:15, 80:9 |
| 2 | 0042a830-004486a0 | 429 | 0.31 | 1 | 0 | 17 | bsp_chk_save, deviceclass, daytime, unlockto, unlockname, unlockfrom, playtime | 80:199, 1:137, 84:51, 79:27, 24:25, 49:12 |
| 3 | 004486c0-00453de0 | 182 | 0.49 | 2 | 0 | 1 | panel, sequence, message, callback, suppressinterruptmsg, setpanel, requesttime | 1:62, 80:38, 84:26, 2:21 |
| 4 | 00453ed0-004b9680 | 1450 | 0.08 | 1 | 0 | 25 | soldiertypes, landvehicleclasses, entity, alpha, action, weight, cameraposition | 1:316, 80:160, 84:156, 2:110, 57:53, 56:48 |
| 5 | 004b96a0-004c8260 | 282 | 0.3 | 1 | 0 | 1 | white, collect, collectgarbage, traininggrounds, cloudsmall, multi, cloud | 4:42, 1:30, 84:18, 24:15, 73:9 |
| 6 | 004c8280-004d4150 | 190 | 0.38 | 1 | 0 | 0 | allbutingame, interface, textures, writestats, userleft, stats, sonar | 1:72, 5:71, 84:42, 4:37, 80:17, 24:12 |
| 7 | 004d41a0-004f8830 | 359 | 0.26 | 1 | 1 | 11 | ggame, collect, collectgarbage, party, ingame, scene, ocean | 1:171, 6:123, 5:111, 4:93, 56:58, 84:48 |
| 8 | 004f8970-0051e4d0 | 366 | 0.64 | 2 | 0 | 1 | menuitem_text, vehicleclass, back, globals, dview, navigate, scroll_menu | 1:224, 80:147, 74:102, 84:95, 2:40, 5:38 |
| 9 | 0051e650-00527c80 | 111 | 0.73 | 2 | 0 | 0 | attackmove, cycle, target, stearring, showocean, showfoliage, showboundings | 24:31, 1:24, 5:18, 2:12, 7:11, 4:10 |
| 10 | 00527cb0-00543a30 | 311 | 0.36 | 2 | 0 | 1 | back, mshd, globals, vidm, text_b_text, text_a_text, submarine_group | 1:115, 74:91, 84:43, 5:39, 7:37, 75:31 |
| 11 | 00543a60-00558ed0 | 315 | 0.29 | 1 | 0 | 3 | button_framebox, basicship2, basicplane3, basicplane2, basicsub, basicship, basicplane | 1:77, 24:36, 84:32, 7:30, 74:28, 5:23 |
| 12 | 00558f80-00568930 | 183 | 0.26 | 1 | 0 | 0 | fe_pc, preset, presets, globals, opt_normal, opt_inverted, opt_cancel | 11:76, 74:76, 1:73, 24:31, 80:29, 84:25 |
| 13 | 00568cb0-0056f320 | 102 | 0.65 | 1 | 0 | 1 | globals, setting_2_text, nike_icon, cucc_group, setting_var_text, setting_value_text, tilt_icon | 12:31, 1:25, 74:20 |
| 14 | 00570300-00584110 | 330 | 0.3 | 1 | 0 | 0 | server_text, players_text, mode_text, scroll_right_icon, wave_icon, main_newprofile, titlelogo_icon | 13:71, 1:56, 74:36, 12:23, 84:19, 75:18 |
| 15 | 00584170-0058d470 | 72 | 0.57 | 3 | 0 | 1 | globals, select, navigate, back, mission_mappoint_, fe_pc, xsm_requiresprofile | 1:66, 74:41, 14:39, 84:20, 75:15, 8:14 |
| 16 | 0058d4b0-005caff0 | 555 | 0.13 | 4 | 0 | 2 | globals, mission_mappoint_, bushgroup, visibility, terrain, message, group | 1:163, 74:77, 15:62, 75:56, 84:51, 14:49 |
| 17 | 005cb0a0-005ce340 | 69 | 0.39 | 1 | 0 | 0 | up_icon, gui_movie, down_icon, tempeditcursor_icon, tempeditarrow_icon, targetnames_text, rotationspeeddefaultlimit | 1:13, 7:9, 5:9, 74:8, 84:8, 80:8 |
| 18 | 005ce410-005fb080 | 334 | 0.73 | 2 | 0 | 6 | globals, back, xsm_saveconfirm, fe_xbox, mainlistbox_text, select, main_listbox | 1:303, 74:188, 84:96, 5:86, 24:81, 7:70 |
| 19 | 005fc620-006049f0 | 78 | 0.64 | 2 | 0 | 0 | ingame, felkialtojel_text, aaaaaa, paused, title_group, silverline_framebox, secondary_objectives_text | 1:50, 74:36, 7:29, 6:20, 84:17, 5:14 |
| 20 | 00604a20-0060de90 | 87 | 0.51 | 2 | 0 | 1 | turbo_group, turbo_effect, ship_speed_num3_icon, ship_payload_2_icon, ship_payload_1_icon, repairzone_text, pleasewait | 2:41, 44:36, 1:35, 7:28, 5:26, 24:23 |
| 21 | 0060dfb0-00622990 | 204 | 0.33 | 2 | 0 | 2 | showgamercard, globals, usn_point_text, usn_icon, radar_sweep, pumpermanent, playerreview | 1:68, 74:43, 84:27, 75:15, 80:15, 2:13 |
| 22 | 00622a10-00654650 | 364 | 0.12 | 3 | 0 | 6 | ingame, globals, type_icon, scoring_unlock_text, score_text, medal_icon, commandbuilding_icon | 1:160, 21:83, 74:68, 24:53, 84:39, 7:37 |
| 23 | 00654a70-00664120 | 210 | 0.45 | 1 | 0 | 0 | circle_hl_icon, ingamegui, normal, icon_l_icon, circle_small_02_group, circle_small_01_group, number_text | 1:60, 72:24, 74:23, 24:21, 84:21 |
| 24 | 006641b0-0069a9e0 | 516 | 0.2 | 1 | 0 | 9 | ingame, gvmultimenu, sm_cp, globals, interface, textures, datatables | 1:191, 23:175, 7:115, 80:75, 5:68, 74:65 |
| 25 | 0069aa10-006a0aa0 | 119 | 0.43 | 1 | 0 | 0 | invio, indietro, cs_unknown, fe_pc | 1:24, 84:9 |
| 26 | 006a0ba0-006a7be0 | 116 | 0.59 | 1 | 0 | 0 | sensitivitysettings, inputsettings, devicetype, deviceidx, slider, reverse, sensitivities | 25:52, 12:26, 11:25, 2:21, 80:20, 1:18 |
| 27 | 006a9850-006be980 | 215 | 0.4 | 1 | 0 | 0 | savedata, scoring, entities, missionid, entidcont, cont1, cont0 | 80:349, 1:173, 84:50, 4:36, 2:34, 26:20 |
| 28 | 006beb10-006d6450 | 301 | 0.54 | 1 | 0 | 17 | equipment, state, slots, runwaywidth, runwaylength, runwayfailure, hangarfailure | 1:100, 24:81, 27:73, 2:33, 41:24, 4:19 |
| 29 | 006d6470-006da670 | 100 | 0.41 | 2 | 0 | 0 | recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 1:20, 84:10 |
| 30 | 006da6b0-006deca0 | 67 | 0.76 | 3 | 0 | 0 | markerclasses, recursiveguihighlights, positionmarkers, guihighlights, entitymarkers | 29:46, 80:10, 1:9 |
| 31 | 006dee40-006faf00 | 333 | 0.41 | 1 | 1 | 22 | orgammo, bulletbase, flytime, bulletclass, bomb, owner, bullets | 1:137, 80:72, 24:43, 2:38, 57:34, 36:33 |
| 32 | 006fb030-0070bba0 | 227 | 0.26 | 2 | 0 | 8 | openaftertime, messages, whosaysthat, velocity, openstate, dragvert, divedepth | 1:79, 80:45, 31:32, 84:32, 2:30, 57:21 |
| 33 | 0070bc30-00717980 | 164 | 0.34 | 1 | 0 | 2 | unitlist, unitid, shouldblast, shipnumber, shape, num_0, mindist2 | 1:21, 2:10, 82:9, 31:8 |
| 34 | 00717c70-0071b8d0 | 81 | 0.64 | 2 | 0 | 1 | mzonedesc, sphere, identifier, mnote, armor, points, category | 33:57, 83:13, 86:11 |
| 35 | 0071b940-00728fa0 | 215 | 0.5 | 3 | 0 | 14 | startmode, followmode, userpath, pathfollowparams, pathcursor, mgeommesh, internalclearprimarycommand | 1:63, 39:27, 41:19, 44:14, 9:14, 24:9 |
| 36 | 007290d0-00741140 | 274 | 0.27 | 1 | 0 | 17 | destroyed, sounddevice, memsize, gpudeviceid, barreldelaytime, cpuspeed, torpedo | 1:99, 84:48, 80:46, 2:24, 62:15, 4:14 |
| 37 | 00741160-0074e540 | 158 | 0.4 | 2 | 0 | 10 | landvehicle, landingship, landfort, rampaelfordulas, partraszalltunk, partraszallas, nyitzartimer | 1:64, 80:43, 60:27, 39:22, 57:21, 41:20 |
| 38 | 0074e5d0-00758f90 | 61 | 0.48 | 5 | 0 | 6 | sumleaks, sumforces, iswater, water, launchairstrike, elevator_2, runwayfailure | 1:23, 46:11, 4:9, 28:8, 39:8 |
| 39 | 00758fe0-0076bd70 | 365 | 0.84 | 1 | 0 | 3 | runwaycenter, maxlandingplanesonboard, liftexitpoint, deckcamera, carrierescort, elevator_2, vertangle | 1:663, 84:12, 28:11, 80:8 |
| 40 | 0076be10-007788b0 | 193 | 0.56 | 3 | 0 | 1 | p2p_voice__, mmultiplayer, peer, gamestate, dropped, tick, just | 39:74, 41:74, 1:57, 5:23, 18:18, 84:18 |
| 41 | 007788d0-00786a80 | 229 | 0.34 | 1 | 0 | 27 | reconlevel, multiscore, lastbanto, recondata, netentity, multiscore_save, hasplrcmd | 1:60, 80:44, 39:30, 57:27, 84:18, 33:17 |
| 42 | 00786be0-0078cf20 | 66 | 0.48 | 2 | 0 | 0 | send_, recv_, time, client, server | 84:28, 1:16, 83:15 |
| 43 | 0078cff0-007a4860 | 253 | 0.4 | 1 | 0 | 0 | postype, camera, thetalinearblend, theta, rholinearblend, blendtime, initialization | 1:123, 80:61, 2:55, 24:38, 84:28, 17:17 |
| 44 | 007a49a0-007d1dc0 | 473 | 0.7 | 1 | 0 | 21 | powerlost, explosion, pathbaseentity, splash, simple, enginefire, sustainbefore | 1:205, 2:107, 43:79, 45:78, 39:46, 84:44 |
| 45 | 007d1e50-0080d9b0 | 555 | 0.44 | 1 | 1 | 7 | travelspeed, gears, baydoor, wings, enemy, neutral, state | 1:201, 80:99, 2:95, 44:58, 84:55, 4:44 |
| 46 | 0080da00-0081aa10 | 163 | 0.69 | 4 | 0 | 38 | steeringjam, enginejam, periscope, torpedostock, thrust, attackmove, explosion | 1:46, 60:35, 39:27, 41:21, 35:18, 57:17 |
| 47 | 0081aa60-00828810 | 96 | 0.44 | 5 | 0 | 9 | camocolorgun, shipyardlaunch, camocolor, steeringjam, enginejam, explosion, weapons | 46:77, 1:50, 57:18, 49:16, 80:16, 84:14 |
| 48 | 00828870-00858660 | 491 | 0.24 | 4 | 2 | 12 | gameunit, classid, stock, torpedo, torpedoavoidance, object, submarine | 1:128, 4:93, 80:69, 2:46, 35:40, 49:30 |
| 49 | 00858700-0086af80 | 242 | 0.55 | 1 | 0 | 5 | radius, tvertangle, turninggun, thorzangle, horzrotdir, vertangle, horzangle | 80:40, 1:38, 2:31, 45:12, 50:12, 84:11 |
| 50 | 0086afc0-00877e50 | 168 | 0.39 | 1 | 0 | 2 | effects, minlifetime, maxlifetime, lightning, particle, widthwave, widthscaler | 80:66, 84:60, 1:49, 49:27, 76:11, 81:10 |
| 51 | 00877fa0-00882ac0 | 118 | 0.43 | 2 | 0 | 2 | damage, yellow, weaponsystems, weapondirectorthinktime, warningscrollspeeds, visibletimeout, visibilityrange | 80:33, 1:30, 50:26, 84:14, 4:11, 2:8 |
| 52 | 008840b0-0088b120 | 127 | 0.54 | 2 | 0 | 0 | scripts, debugtrap, luab, missions, fundamentals, autoload, global | 1:52, 27:51, 80:46, 84:22, 31:21 |
| 53 | 0088b190-008ddf90 | 748 | 0.84 | 3 | 528 | 0 | luakod, options, hardwarereported, english, xboxcompatibilitymode, vsync, texturedetail | 80:3870, 1:1780, 84:592, 52:436, 2:64, 27:57 |
| 54 | 008ddfe0-008e5c40 | 131 | 0.37 | 4 | 0 | 2 | szurkenyil, secobjprefix, pinged, missionglobals, flagprocess, quiet, objectivelist | 1:70, 53:55, 84:10, 24:9, 40:8 |
| 55 | 008e5c50-008ee5f0 | 116 | 0.53 | 5 | 0 | 1 | pup_gain, pum1stget, uspumicon, uselimit, unitclassindex, targettype, targetfilter | 1:59, 54:49, 80:25, 84:23, 2:18, 21:16 |
| 56 | 008ee670-00922c80 | 581 | 0.17 | 1 | 1 | 2 | bulletthrowmul, vertangleerror, torpedobot, thinktimeleft, tailgunnerbot, pilotbot, horzangleerror | 1:218, 80:94, 2:93, 14:78, 84:67, 4:48 |
| 57 | 00922de0-0092dfe0 | 180 | 0.46 | 1 | 0 | 78 | party, entity, timing, thinkfunction, roleavailable, gameentity, deadmeat | 80:40, 1:38, 4:20, 84:17, 24:12, 27:12 |
| 58 | 0092e0b0-00943d80 | 210 | 0.34 | 2 | 0 | 1 | cSmoothMapZoomLevel, periszkop, hajobelso, fizika_, cStaticShot_Size_OffsetX_OffsetY, enginejam, utkozoje | 1:81, 57:31, 84:24, 81:22, 2:17, 82:16 |
| 59 | 00943e40-00951d00 | 146 | 0.4 | 3 | 0 | 1 | resourceusage, ownerplayer, supportmanager, velocitysi, effect, camocolor, autoattacktarget | 80:69, 1:68, 56:35, 58:24, 84:22, 4:20 |
| 60 | 00951d20-00968e00 | 279 | 0.27 | 2 | 0 | 37 | vehicleclass, inferiorfailure, reconplane, torpedobomber, torpedoboat, divebomber, cargo | 1:76, 80:74, 2:63, 4:30, 37:16, 57:14 |
| 61 | 00968e80-0098c510 | 473 | 0.31 | 3 | 0 | 29 | player, callback, entity, message, oldlevel, newlevel, ambient | 1:240, 84:95, 52:94, 60:81, 31:34, 4:31 |
| 62 | 0098c630-009965d0 | 155 | 0.5 | 1 | 0 | 2 | software, bsm_hwd, eidos, mpkg, cast, content, language | 1:38, 84:15, 0:12, 2:8 |
| 63 | 00996670-009f6060 | 835 | 0.83 | 5 | 0 | 9 | follow, moveto, state_moveto, targetlock, prepare, goaway, attackrun | 1:417, 44:269, 2:232, 64:167, 0:113, 84:105 |
| 64 | 009f6090-009fe940 | 104 | 0.72 | 4 | 1 | 1 | projtime, precision, bullpos, vehicle, torpedobomb, squadronfreeattacktargets, nonfightergun | 1:41, 45:19, 35:19, 2:15, 44:15, 80:13 |
| 65 | 009fe9c0-00a079b0 | 146 | 0.56 | 2 | 0 | 0 | vehicle, class, captureweight | 1:18 |
| 66 | 00a07a60-00a19480 | 132 | 0.58 | 3 | 0 | 5 | commandtype, neutral, aivstable_, enemy, target, vehicleclass, s_to_ | 1:63, 65:54, 80:25, 69:19, 68:19, 84:18 |
| 67 | 00a19500-00a1fac0 | 127 | 0.46 | 3 | 0 | 0 | coordinator, sell, strategicgain, duel, defend, capture, siege | 1:37, 68:17, 84:11 |
| 68 | 00a1fba0-00a335d0 | 190 | 0.41 | 3 | 1 | 0 | reconratio, objectivemembers, autogrouping, unittypes, members, leader, party | 67:70, 66:56, 1:48, 80:27, 65:25, 5:20 |
| 69 | 00a371a0-00a427a0 | 154 | 0.42 | 3 | 0 | 6 | client, online, xenonsystemmanager, online__, network, player, mnetworkclientxlive | 80:19, 1:16, 41:13, 68:9, 84:8, 70:8 |
| 70 | 00a428d0-00a625d0 | 180 | 0.49 | 3 | 0 | 3 | online, server, player, query, matchmaking, remote, movie | 1:23, 69:22, 62:21, 5:15, 24:10, 84:10 |
| 71 | 00a62660-00a798c0 | 329 | 0.64 | 1 | 0 | 1 | unexpected, chunk, precompiled, complex, expression, call, many | 1:24, 73:16, 84:13 |
| 72 | 00a79910-00a82b70 | 134 | 0.69 | 1 | 0 | 7 | memory, sounjd, sound, play, event, system, init | 80:35, 1:33, 84:19 |
| 73 | 00a82c60-00a9b720 | 324 | 0.28 | 1 | 0 | 8 | memory, sounjd, stream, sound, streaming, stereo, request | 1:91, 84:50, 72:46, 77:20, 86:17, 82:10 |
| 74 | 00a9b740-00aabee0 | 266 | 0.33 | 1 | 0 | 39 | cGuiManager, widescreenalign, visible, rotate, mousehit, mouseblock, lowcolor | 1:57, 75:45, 84:20, 5:16, 82:15, 81:15 |
| 75 | 00aabf80-00ada900 | 641 | 0.25 | 1 | 0 | 19 | texture, mshd, guidefault, simplecolor, mvfm, shadername, hastexture | 1:191, 74:172, 84:90, 82:82, 76:37, 80:25 |
| 76 | 00ada990-00b1bc70 | 735 | 0.3 | 1 | 0 | 18 | param, mvfm, mshd, emitter, terrain, additive, particlefloating | 1:240, 84:115, 82:71, 86:49, 79:49, 81:40 |
| 77 | 00b1bd20-00b20dc0 | 99 | 0.63 | 1 | 0 | 0 | instanced | 1:21, 80:9, 84:9, 79:8 |
| 78 | 00b20e70-00b4b0a0 | 479 | 0.36 | 1 | 0 | 12 | mvfm, pf43cc, mshd, shadowmap, debugshader, shfx, uterrain4 | 1:83, 84:41, 79:24, 77:20, 82:11, 5:11 |
| 79 | 00b4b490-00b659d0 | 264 | 0.26 | 1 | 0 | 1 | mshd, cSampleOffsets, posteffectsysobj, posteffectsyscam, oldfilm_dust, cSampleWeights, mvfm | 1:84, 80:67, 84:41, 78:34, 82:33, 77:23 |
| 80 | 00b65ac0-00b6d7b0 | 136 | 0.78 | 1 | 0 | 0 | dofile, userdata, thread, lightuserdata, dobuffer, fundamentals, scripts | 1:39, 71:31, 84:12 |
| 81 | 00b6d820-00b70fe0 | 76 | 0.55 | 1 | 0 | 31 | c3dnode | 1:12, 82:8, 79:8 |
| 82 | 00b71120-00b811a0 | 336 | 0.51 | 1 | 0 | 1 | c3dnodeanimator, cLight, cDummy, cAnimTrack, cAmbientLight, c3dobject, boundingbox | 1:57, 81:31, 86:27, 84:23, 83:22, 79:14 |
| 83 | 00b811c0-00b999f0 | 396 | 0.64 | 1 | 0 | 4 | cSceneResource, cRenderMeshResource, cGroupParamsResource, cGroup, flare, boundingsphere, mvfm | 82:73, 86:70, 1:61, 81:29, 84:26, 31:12 |
| 84 | 00b99bf0-00bd4200 | 640 | 0.37 | 1 | 0 | 93 | mvfm, mshd, cCorner3, cCorner2, cCorner1, cCorner0, coast | 1:234, 82:154, 81:61, 83:41, 76:40, 86:35 |
| 85 | 00bd4270-00bdb1e0 | 98 | 0.4 | 1 | 0 | 2 |  | 80:83, 1:40, 84:21 |
| 86 | 00bdb2e0-00c30570 | 536 | 0.36 | 1 | 0 | 20 | cFileStore, long, iterator, cPhysicalDirectoryX86, openfileoverlapped, removefile, removed | 1:202, 84:108, 85:39, 0:19, 80:15, 11:12 |
