#include "bsp/mission_lua_host.hpp"

#include "bsp/mission_scene_load.hpp"

#include <algorithm>
#include <cstring>

namespace bsp {
namespace {

// Table at 00e0b7b8, in table order, terminated by the null function pointer at
// 00e0c938. 560 rows, every name distinct; SetMotionBlurParams/SetMBP,
// AddAirBaseStock/AddAirBasePlanes and MissionNarrative/MissionNarrativeEnqueue
// are the three address aliases.
constexpr MissionLuaBinding kBindings[] = {
    {"Log", 0x0088BEC0U},
    {"LogToFile", 0x0088BC00U},
    {"DoFileOutsideMPAK", 0x0088BD20U},
    {"GetProperty", 0x0088BF80U},
    {"Assert", 0x0088C3E0U},
    {"SETLOG", 0x0088C620U},
    {"AddWatch", 0x0088D130U},
    {"ShowHint", 0x008D1F50U},
    {"ShowHintForced", 0x008D2190U},
    {"AddStoredHint", 0x008D24F0U},
    {"RemoveStoredHint", 0x008D2340U},
    {"HideHint", 0x0088CEC0U},
    {"IsHintActive", 0x008D2850U},
    {"IsHintStored", 0x008D2BB0U},
    {"IsHintCritical", 0x008D26A0U},
    {"IsSMVisible", 0x0088CD70U},
    {"BannSupportmanager", 0x0088C750U},
    {"PermitSupportmanager", 0x0088C8E0U},
    {"LogToConsole", 0x0088D420U},
    {"SetBPCsodaSquadMereteHack", 0x0088D2B0U},
    {"NetworkLog", 0x0088D6E0U},
    {"RemoveWatch", 0x0088D560U},
    {"random", 0x0088C160U},
    {"UnlockAllMission", 0x0088D000U},
    {"AddDamage", 0x0088E000U},
    {"SetWaterDamage", 0x0088E790U},
    {"SetFireDamage", 0x0088E320U},
    {"ExplodeToParts", 0x0088E1B0U},
    {"GetHpPercentage", 0x0088E9D0U},
    {"SetControlFactor", 0x0088EB50U},
    {"GetControlFactor", 0x0088EDC0U},
    {"SetFailure", 0x0088EFF0U},
    {"GetFailure", 0x0088F1D0U},
    {"ClearFailure", 0x0088F3C0U},
    {"ClearAllShipFailure", 0x0088F5A0U},
    {"SetDamagedGFXLevel", 0x0088F710U},
    {"ActivateSpawnpoint", 0x0088F8B0U},
    {"DeactivateSpawnpoint", 0x0088FAB0U},
    {"GetDamagedGFXLevel", 0x0088FCC0U},
    {"SetNumbering", 0x0088FE30U},
    {"GetWaterLoad", 0x00891680U},
    {"GetLeaks", 0x008918D0U},
    {"SetPump", 0x00893240U},
    {"AddWaterLoad", 0x00893380U},
    {"Sink", 0x00891B20U},
    {"HitTarget", 0x00892F70U},
    {"GetFire", 0x008ACF00U},
    {"AddFire", 0x008ACD10U},
    {"SetRepairLevel", 0x008AD150U},
    {"SetRepairPriority", 0x008AD6F0U},
    {"RepairEnable", 0x008AD330U},
    {"FailureRepairEnable", 0x008AD540U},
    {"SetRepairEffectivity", 0x008AD8D0U},
    {"RepairAddTimeDelay", 0x008ADA70U},
    {"GetRepairLevel", 0x008ADC40U},
    {"RemoveWrecks", 0x008970B0U},
    {"InferiorIsDisabled", 0x00897E40U},
    {"GetGuiName", 0x0088DAA0U},
    {"GetMeasure", 0x0088D8E0U},
    {"SetInvincible", 0x00897A50U},
    {"IsInvincible", 0x00897CB0U},
    {"GetFirepower", 0x0088DC10U},
    {"GetAirBaseStatus", 0x00895BA0U},
    {"GetAirBaseSlotStatus", 0x00896220U},
    {"IsReadyToSendPlanes", 0x00895D20U},
    {"SetAirBaseSlot", 0x00895ED0U},
    {"SetAirBaseSlotCount", 0x008963E0U},
    {"LaunchAirBaseSlot", 0x00896750U},
    {"AddAirBaseStock", 0x00896A90U},
    {"SetAirBaseSlotReady", 0x00896590U},
    {"AddAirBasePlanes", 0x00896A90U},
    {"AddShipyardStock", 0x00896CC0U},
    {"RemoveShipyardStock", 0x00896EE0U},
    {"RemoveAirBasePlanes", 0x008973C0U},
    {"RemoveAllAirBasePlanes", 0x008975B0U},
    {"SetAirBasePlaneLimit", 0x00897720U},
    {"GetAirBasePlaneLimit", 0x008978D0U},
    {"IsInFormation", 0x008996A0U},
    {"IsFormationLeader", 0x00899810U},
    {"IsFormationFollower", 0x00899980U},
    {"GetFormationLeader", 0x00899AF0U},
    {"JoinFormation", 0x00899D10U},
    {"LeaveFormation", 0x00899EB0U},
    {"DisbandFormation", 0x0089A020U},
    {"FillPathPoints", 0x0089A190U},
    {"PlayBinkMovie", 0x0089A480U},
    {"SetThink", 0x00897FB0U},
    {"SetWait", 0x00898150U},
    {"ClearWait", 0x00898320U},
    {"ClearThink", 0x00898490U},
    {"CreateScript", 0x00898750U},
    {"DeleteScript", 0x00898AC0U},
    {"Message", 0x00898C60U},
    {"FindEntity", 0x00898E30U},
    {"FindEntityByID", 0x008990B0U},
    {"DoVector3", 0x00899300U},
    {"TrulyDead", 0x00899500U},
    {"InitAll", 0x00898610U},
    {"Order", 0x0089A660U},
    {"SetRace", 0x008A8720U},
    {"SetParty", 0x008A8930U},
    {"SetPartyID", 0x008A8B40U},
    {"SetCommandBuildingOwnerPlayer", 0x008A8D90U},
    {"SetGuiName", 0x008A8F90U},
    {"SupportManagerEnable", 0x0088CBB0U},
    {"ShowHideSupportManager", 0x0088CA70U},
    {"SetPlayerControl", 0x008A5BD0U},
    {"SetCheatTurbo", 0x008A62A0U},
    {"EnterPlayerToRole", 0x008A6060U},
    {"IsPlayerControlled", 0x008A5EF0U},
    {"ShipUseCatapult", 0x00891D50U},
    {"GetLastCatapulted", 0x00892860U},
    {"GetNumCatapulted", 0x00892AA0U},
    {"GetCatapultStock", 0x00892DE0U},
    {"SetCatapultStock", 0x00892C30U},
    {"GameCameraLookAtTarget", 0x00891FB0U},
    {"FreeCameraGetPos", 0x00892660U},
    {"FreeCameraSetPos", 0x008923B0U},
    {"GetSquadFlyTime", 0x008A5D60U},
    {"SetAirbaseSmallPlanes", 0x008A5A20U},
    {"LaunchSquadron", 0x0089E3C0U},
    {"GetPosition", 0x008A7B00U},
    {"GetBounding", 0x0089B4E0U},
    {"GetDirection", 0x008A7CB0U},
    {"GetRotation", 0x008A7E60U},
    {"RelativePosition", 0x008A8070U},
    {"GetRelativeSpeed", 0x008A8280U},
    {"GetETA", 0x008A8480U},
    {"StartLanding2", 0x0089AC30U},
    {"SetBeachFight", 0x0089ADB0U},
    {"SetCaptureValue", 0x0089B0C0U},
    {"GetCommandBuildingLevel", 0x0089B6D0U},
    {"SetCommandBuildingLevel", 0x0089B260U},
    {"GetCapturePercentage", 0x0089B840U},
    {"IsLandingTrafficActive", 0x0089AF40U},
    {"GetFireTarget", 0x0089C360U},
    {"SetFireTarget", 0x0089A8B0U},
    {"ShipSetTorpedoStock", 0x0089EEE0U},
    {"PlaneReloadBombPlatforms", 0x0089F080U},
    {"PlaneChangeAmmoType", 0x0089F1F0U},
    {"NavigatorDirectMoveToRange", 0x008A2D70U},
    {"NavigatorMoveToRange", 0x008A2F20U},
    {"NavigatorMoveToPos", 0x008A2BC0U},
    {"NavigatorMoveOnPath", 0x008A3600U},
    {"NavigatorForceMoveCloseToTarget", 0x008A3460U},
    {"NavigatorForceTorpedo", 0x008A7200U},
    {"NavigatorStop", 0x008A7400U},
    {"NavigatorCruise", 0x008A75C0U},
    {"NavigatorSetAvoidShipCollision", 0x008A3970U},
    {"NavigatorSetAvoidLandCollision", 0x008A3B10U},
    {"NavigatorSetAvoidAllShipCollision", 0x008D0740U},
    {"NavigatorSetTorpedoEvasion", 0x008A3CD0U},
    {"NavigatorAttackMove", 0x008A30D0U},
    {"NavigatorAllowMaxDepth", 0x008A32C0U},
    {"NavigatorLandAttackerStop", 0x008A2A50U},
    {"PilotMoveTo", 0x008A4150U},
    {"PilotMoveToRange", 0x008A4590U},
    {"PilotMoveOnPath", 0x008A3E70U},
    {"PilotRetreat", 0x008A4300U},
    {"PilotLand", 0x008A47B0U},
    {"PilotCloseToShip", 0x008A4960U},
    {"PilotCloseToShipDrop", 0x008A4B10U},
    {"PilotSetTarget", 0x008A4C90U},
    {"PilotBomb", 0x008A4F00U},
    {"PilotTorpedo", 0x008A5310U},
    {"PilotGunFire", 0x008A50D0U},
    {"PilotStartAttackRun", 0x008A5870U},
    {"PilotStop", 0x008A7780U},
    {"PilotClearOrders", 0x008A7940U},
    {"PlaneBombFallingTime", 0x008A54C0U},
    {"AddAnimatedPartEffect", 0x008A5650U},
    {"PlaneSetYawCtrl", 0x0089D9D0U},
    {"PlaneSetPitchCtrl", 0x0089DB70U},
    {"PlaneSetRollCtrl", 0x0089DD10U},
    {"PlaneSetPowerCtrl", 0x0089DEB0U},
    {"PlaneSetTargetRoll", 0x0089D690U},
    {"PlaneSetTargetPitch", 0x0089D830U},
    {"PlaneSetGunfire", 0x0089E050U},
    {"GetPlaneSquadron", 0x0089D250U},
    {"GetSquadronPlanes", 0x0089CC50U},
    {"GetSquadronPlane", 0x0089CFE0U},
    {"GetSquadronLanded", 0x0089CE60U},
    {"GetSquadronLandedBase", 0x0089D470U},
    {"SquadronSetWandererMul", 0x008A1B70U},
    {"SquadronSetWandererEnabled", 0x008A1D60U},
    {"SquadronSetHomeBase", 0x0089E220U},
    {"SquadronSetDropAllBombPercent", 0x008A1F40U},
    {"SquadronLandAndKill", 0x008A20E0U},
    {"SquadronSetTravelAlt", 0x0089F550U},
    {"SquadronSetAttackAlt", 0x008A22B0U},
    {"SquadronSetReleaseAlt", 0x008A24E0U},
    {"SquadronSetForceRelease", 0x008A28B0U},
    {"SquadronSetTravelSpeed", 0x008A0630U},
    {"TorpedoSetSwimDepth", 0x008A2710U},
    {"EntityTurnToEntity", 0x008A0A10U},
    {"EntityTurnToPosition", 0x008A0F10U},
    {"SquadronRestoreTravelSpeed", 0x008A1A00U},
    {"SquadronSetSpeed", 0x0089F780U},
    {"SquadronForceGunnerMode", 0x0089F950U},
    {"SquadronEnablePlayerTakeoff", 0x008A0360U},
    {"SquadronDisablePlayerTakeoff", 0x008A04C0U},
    {"SquadronEnableTerrainAvoidance", 0x0089FE50U},
    {"SquadronEnableVehicleAvoidance", 0x008A0000U},
    {"SquadronEnableGunfireAvoidance", 0x008A01B0U},
    {"SquadronSetBaseUnsupported", 0x0089FAF0U},
    {"SquadronSetAIRetreatEnabled", 0x0089FCB0U},
    {"SquadronForceRelease", 0x0089E850U},
    {"SquadronSetOldStyleBombing", 0x0089EA00U},
    {"PlaneForceRelease", 0x0089EBA0U},
    {"PlayerSetBomberView", 0x008A0850U},
    {"SetVisibility", 0x008A13D0U},
    {"SetShipDebugFilter", 0x008A15D0U},
    {"SetSingleThread", 0x008A1890U},
    {"AvoidZoneDebugLayer", 0x008A1730U},
    {"UnitSetFireStance", 0x008A6490U},
    {"UnitGetFireStance", 0x008A6640U},
    {"UnitFreeAttack", 0x008A67E0U},
    {"UnitFreeFire", 0x008A6950U},
    {"UnitHoldFire", 0x008A6AC0U},
    {"UnitSetPlayerCommandsEnabled", 0x008A6C30U},
    {"UnitGetAttackTarget", 0x008A6DE0U},
    {"NavigatorEnable", 0x008A7060U},
    {"ArtilleryEnable", 0x0089C590U},
    {"TorpedoEnable", 0x0089C8F0U},
    {"AAEnable", 0x0089C740U},
    {"DCEnable", 0x0089CAA0U},
    {"SetFormationShape", 0x0088FFD0U},
    {"GetFormationShape", 0x008901C0U},
    {"SetPlaneGears", 0x00890340U},
    {"ForcePlaneGears", 0x00890840U},
    {"GetPlaneSurelyLanded", 0x00890510U},
    {"GetPlaneLandingAborted", 0x008906A0U},
    {"SetShipMaxSpeed", 0x00890A10U},
    {"GetShipMaxSpeed", 0x00890BB0U},
    {"SetShipSpeed", 0x00890D30U},
    {"GetShipSpeed", 0x00890EF0U},
    {"GetDraught", 0x008936A0U},
    {"EnableEngineSound", 0x008911E0U},
    {"SetSubmarineDepthLevel", 0x00893F40U},
    {"GetSubmarineDepthLevel", 0x00894100U},
    {"GetSubmarineOnSurface", 0x008942C0U},
    {"ForceSubmarinePeriscope", 0x00893DA0U},
    {"SubmarineAttack", 0x00894440U},
    {"SetWeaponDirectorTargetEnable", 0x008945F0U},
    {"IsLandscape", 0x00894820U},
    {"IsShip", 0x008949D0U},
    {"IsAreaEmpty", 0x00894C00U},
    {"GetSubmarineAirSupply", 0x008938E0U},
    {"SetSubmarineAirSupply", 0x00893A60U},
    {"SetUnlimitedAirSupply", 0x00893C00U},
    {"SetCrewLevel", 0x00894F20U},
    {"GetCrewLevel", 0x008950D0U},
    {"SetSkillLevel", 0x00895250U},
    {"GetSkillLevel", 0x008953F0U},
    {"LandConvoyStart", 0x00895570U},
    {"LandConvoyStop", 0x008956E0U},
    {"LandConvoySetSpeed", 0x00895850U},
    {"LandConvoySetPath", 0x008959F0U},
    {"SetAiControl", 0x008A91B0U},
    {"SpawnNew", 0x0094C480U},
    {"SpawnNewIDIsRequested", 0x00946380U},
    {"SpawnNewIDRemove", 0x00946390U},
    {"Spawn", 0x00944680U},
    {"FindHiddenEntity", 0x008A9E10U},
    {"GenerateObject", 0x00944FD0U},
    {"SpawnLight", 0x00945450U},
    {"PutTo", 0x008A9F90U},
    {"PutFormationTo", 0x008AA260U},
    {"PutRelTo", 0x008AA560U},
    {"SetForcedReconLevel", 0x008AA8F0U},
    {"ClearForcedReconLevel", 0x008AAB90U},
    {"ForceRecon", 0x008AADF0U},
    {"ForceSelectUnit", 0x008AAF30U},
    {"GetSelectedUnit", 0x008AB070U},
    {"SetSelectedUnit", 0x008AB260U},
    {"MultiSelectUnit", 0x008AB3D0U},
    {"IsUnitSelectable", 0x008AB580U},
    {"SetRoleAvailable", 0x008AB850U},
    {"GetRoleAvailable", 0x008ABAC0U},
    {"GetPlayerIndexBySlot", 0x008ABC70U},
    {"GetValidPlayerInSlot", 0x008ABDF0U},
    {"GetRoleOwner", 0x008ABF90U},
    {"IsUnitUntouchable", 0x008AC420U},
    {"AddUntouchableUnit", 0x008AC140U},
    {"RemoveUntouchableUnit", 0x008AC2B0U},
    {"Kill", 0x008AC5C0U},
    {"KillBullets", 0x008AE2F0U},
    {"SetDeadMeat", 0x008AC7B0U},
    {"Save", 0x008AC9D0U},
    {"LoadCheckpoint", 0x008ACA30U},
    {"SaveCheckpoint", 0x008ACB70U},
    {"AddMatrixInterpolator", 0x008ADE00U},
    {"GetDifficulty", 0x008AE030U},
    {"KillStationaries", 0x008AE180U},
    {"SetCloudVisibility", 0x008AE480U},
    {"GetAirbaseOrders", 0x008AE760U},
    {"SetAirbaseOrders", 0x008AE900U},
    {"IsLimboGuiActive", 0x008AB6F0U},
    {"SetDeviceReloadEnabled", 0x008C1350U},
    {"SetDeviceReloadTimeMul", 0x008C14B0U},
    {"SetRocketAirGroundTypeDifferent", 0x008C1620U},
    {"SetHP", 0x008C1790U},
    {"OverrideHP", 0x008C1930U},
    {"DisplayUnitHP", 0x008C1B10U},
    {"HideUnitHP", 0x008C1F50U},
    {"DisplayScores", 0x008C20D0U},
    {"HideScoreDisplay", 0x008C24B0U},
    {"SetBorderZoneParty", 0x008AEB20U},
    {"GetClosestBorderZone", 0x008AECD0U},
    {"IsInBorderZone", 0x008AF020U},
    {"GetBorderZone", 0x008AF260U},
    {"GetBorderCross", 0x008AF700U},
    {"EnableInput", 0x008AFBD0U},
    {"Pause", 0x008AFD50U},
    {"MissionNarrativeSize", 0x008B0AC0U},
    {"MissionNarrative", 0x008B0C10U},
    {"MissionNarrativeEnqueue", 0x008B0C10U},
    {"MissionNarrativeUrgent", 0x008B0E10U},
    {"MissionNarrativeOverride", 0x008B1010U},
    {"MissionNarrativeParty", 0x008B1210U},
    {"MissionNarrativePlayer", 0x008B13E0U},
    {"MissionNarrativeClear", 0x008B15B0U},
    {"Countdown", 0x008B16E0U},
    {"CountdownCancel", 0x008B19A0U},
    {"CountdownTimeLeft", 0x008B1B40U},
    {"TraingingGroundEndScene", 0x008B0470U},
    {"EndScene", 0x008B01B0U},
    {"EndSCore", 0x008AFFC0U},
    {"ForceMultiScoreSend", 0x008AFEC0U},
    {"StartDialog", 0x008B0540U},
    {"BreakDialog", 0x008B07C0U},
    {"KillDialog", 0x008B0940U},
    {"GetActDialogIDs", 0x008CB730U},
    {"SetDayTime", 0x008B1CC0U},
    {"GetDayTime", 0x008B1EC0U},
    {"SetWeather", 0x008B21A0U},
    {"SetRainDropNumPercent", 0x008C7F50U},
    {"SetOldCloudAlpha", 0x008C8090U},
    {"SetSimplifiedReconMultiplier", 0x008B2320U},
    {"GetSimplifiedReconMultiplier", 0x008B24A0U},
    {"SetSimplifiedSonarMultiplier", 0x008B25F0U},
    {"GetSimplifiedSonarMultiplier", 0x008B2770U},
    {"SetSkipMovie", 0x008B28C0U},
    {"Force3dVisibility", 0x008B2AD0U},
    {"MovCam_HandMode_Targeted", 0x008B2CC0U},
    {"MovCam_HandMode_Deck", 0x008B2FE0U},
    {"MovCam_HandMode_Overview", 0x008B3270U},
    {"MovCam_RefPos", 0x008B3740U},
    {"MovCam_RefPos_Polar", 0x008B3C70U},
    {"MovCam_RefPos_Radius", 0x008B4270U},
    {"MovCam_PathWalk", 0x008B47E0U},
    {"MovCam_PathWalkPathPointPropertyPotlekokkal", 0x008B4DA0U},
    {"MovCam_ViktorMode", 0x008B52F0U},
    {"MovCam_TompiModeWeak", 0x008B5870U},
    {"MovCam_TompiMode", 0x008B5DF0U},
    {"MovCam_SetSecondaryTarget", 0x008B60C0U},
    {"MovCam_SetZoom", 0x008B65C0U},
    {"MovCam_AddInterpolator", 0x008B6740U},
    {"MovCam_PickSnittDeckPosition", 0x008B6AD0U},
    {"MovCam_SetSpeedLimits", 0x008B6D20U},
    {"MovCam_ResetSpeedLimits", 0x008B6ED0U},
    {"MovCam_SetAccelTime", 0x008B7010U},
    {"MovCam_ResetAccelTime", 0x008B7180U},
    {"MovCam_SetDeviationFactor", 0x008B72C0U},
    {"MovCam_ResetDeviationFactor", 0x008B7460U},
    {"MovCam_SetOverviewHeight", 0x008B75A0U},
    {"MovCam_ResetOverviewHeight", 0x008B7710U},
    {"MovCam_SetPosRotZoom", 0x008BFA70U},
    {"MovCamNew_AddPositions", 0x008B7850U},
    {"MovCamNew_AddPosition", 0x008B79F0U},
    {"MovCamNew_SetFOV", 0x008B7BA0U},
    {"GetCameraPosRot", 0x008B7D30U},
    {"Camera_Shake", 0x008B8040U},
    {"Camera_SetShakeMax", 0x008B81C0U},
    {"Camera_SetShakeBase", 0x008B8340U},
    {"Camera_SetShakeDecay", 0x008B84C0U},
    {"Scoring_SetFinalScoringFunctionName", 0x008B8640U},
    {"Scoring_SetMissionCompleted", 0x008B8AD0U},
    {"Scoring_SetRanking", 0x008B8CA0U},
    {"Scoring_RealPlayTimeRunning", 0x008B87F0U},
    {"Scoring_GetRealPlayTime", 0x008B8970U},
    {"Scoring_ForceRefreshScoringTable", 0x008B8E60U},
    {"Scoring_GetTotalMissionScore", 0x008B8FF0U},
    {"Scoring_SetMissionScore", 0x008B9190U},
    {"Scoring_GetMissionScore", 0x008B93C0U},
    {"Scoring_AddMissionScore", 0x008B95C0U},
    {"Scoring_SetMissionMedal", 0x008B97F0U},
    {"Scoring_SetActionScore", 0x008B9A30U},
    {"Scoring_GetActionScore", 0x008B9C90U},
    {"Scoring_AddActionScore", 0x008B9E90U},
    {"Scoring_SetShipScore", 0x008BA0F0U},
    {"Scoring_GetShipScore", 0x008BA350U},
    {"Scoring_AddShipScore", 0x008BA550U},
    {"Scoring_SetPlaneScore", 0x008BA7B0U},
    {"Scoring_GetPlaneScore", 0x008BAA10U},
    {"Scoring_AddPlaneScore", 0x008BAC10U},
    {"Scoring_SetCommandScore", 0x008BAE70U},
    {"Scoring_GetCommandScore", 0x008BB0D0U},
    {"Scoring_AddCommandScore", 0x008BB2D0U},
    {"Scoring_SetActionMedal", 0x008BB530U},
    {"Scoring_GrantBonus", 0x008BB770U},
    {"Scoring_GrantUnlock", 0x008CB920U},
    {"Scoring_IsUnlocked", 0x008BBC40U},
    {"Scoring_GetPlayerShotDown", 0x008BC9B0U},
    {"ClearPlayerScore", 0x008BC540U},
    {"Scoring_ClearAllMissionsScore", 0x008D2D60U},
    {"SetAchievements", 0x008BC740U},
    {"Scoring_GetUnitTypeShotDown", 0x008D0140U},
    {"Scoring_SetConditionMessage", 0x008BBE00U},
    {"Scoring_SetVictoryMessage", 0x008BC0C0U},
    {"Multi_GetPlayers", 0x008BCD80U},
    {"GetPlayerDetails", 0x008BCF10U},
    {"TompiAction", 0x008C5360U},
    {"TSetCamMode", 0x008C5490U},
    {"TAddCamPos", 0x008C55C0U},
    {"TAddLookatPos", 0x008C56F0U},
    {"ShowSafeZone", 0x008C59D0U},
    {"Objectives_Add", 0x008CD440U},
    {"Objectives_Completed", 0x008BD340U},
    {"Objectives_Failed", 0x008BD900U},
    {"Objectives_AddUnit", 0x008CDD60U},
    {"Objectives_RemoveUnit", 0x008CE510U},
    {"Objectives_ClientRefresh", 0x008BDEC0U},
    {"Hack_DisableBoatFormations", 0x008BE010U},
    {"Hack_ForceColumnFormationNearIslands", 0x008BE180U},
    {"ForceFireDepthCharge", 0x0089B9D0U},
    {"KillTorpedoesFromUnit", 0x0089BB70U},
    {"ForceEquippedOrdnanceType", 0x0089BD20U},
    {"ReplaceDeviceWithEntity", 0x0089BEF0U},
    {"ReplaceSceneBorder", 0x0089C1B0U},
    {"IsGUIActive", 0x008CA010U},
    {"SetGUIHighlight", 0x008CA1F0U},
    {"GetPlayerActWeapon", 0x008BE2F0U},
    {"GunForceFire", 0x008BE440U},
    {"GunForceFireWithAngle", 0x008BE600U},
    {"ForceEnableInput", 0x008CBAD0U},
    {"HasFired", 0x008BE7E0U},
    {"GetPlaneEquipment", 0x008BEA50U},
    {"GetPlaneIsReloading", 0x008BEBE0U},
    {"GetPayloads", 0x008CECC0U},
    {"GetPayload", 0x008CEFC0U},
    {"GetShipTorpedoes", 0x008BED80U},
    {"GetSquadTorpedoes", 0x008BEF80U},
    {"GetGuns", 0x008CF7E0U},
    {"GetGun", 0x008CF350U},
    {"GetCameraState", 0x008BF6A0U},
    {"GetInputOn", 0x008BFD50U},
    {"GetMapCursorPos", 0x008BFEE0U},
    {"ForceMapCursorPos", 0x008C0070U},
    {"GetMapZoom", 0x008C0260U},
    {"GetDistFromCrosshair", 0x008C0560U},
    {"GetConfiguredInput", 0x008CC030U},
    {"MapUnitHighlight", 0x008C27C0U},
    {"MapHideNodeByName", 0x008C03B0U},
    {"HackPressInput", 0x008D0430U},
    {"GetDevice", 0x008C3610U},
    {"GetRawText", 0x008CA450U},
    {"GetFormationPositions", 0x008C3AB0U},
    {"ReloadTorpedoes", 0x008C3CE0U},
    {"Scoring_IgnorePlayersPartyLosses", 0x008C3EA0U},
    {"Scoring_IgnoreEntityKill", 0x008CC330U},
    {"Scoring_AddPartyLoss", 0x008C3FE0U},
    {"IsSwapSticksEnabled", 0x008C4370U},
    {"GetEntitySpeed", 0x008C0820U},
    {"SetRoleHack", 0x008C09A0U},
    {"SetMouseExclusiveRepairCategory", 0x008C0B70U},
    {"ForceArrows", 0x008CBCB0U},
    {"HackSensitiveUnit", 0x008CBEB0U},
    {"GetKamikazeByDummy", 0x008C3880U},
    {"EnableObjectivesToggle", 0x008C0CA0U},
    {"ActivateObjectives", 0x008C0E10U},
    {"FakePing", 0x008C0F90U},
    {"SetInputModifiers", 0x008C1150U},
    {"GetInputModifiers", 0x008C2660U},
    {"GetCameraRelPosL", 0x008BF460U},
    {"GetCameraRelPosW", 0x008BF230U},
    {"SetCounterTimer", 0x008C4200U},
    {"Music_Control_SetLevel", 0x008C4D10U},
    {"Music_Control_Suspend", 0x008C4ED0U},
    {"AddLockitPathToSelection", 0x008C5040U},
    {"LoadSelectedPaths", 0x008C5230U},
    {"Effect", 0x008A9730U},
    {"DestroyEffect", 0x008CB5B0U},
    {"StopEffect", 0x008A9CA0U},
    {"PrepareClass", 0x008C8F70U},
    {"IsClassChanged", 0x008CC4B0U},
    {"UnloadClass", 0x008CC630U},
    {"Mpak", 0x008C5820U},
    {"GameTime", 0x008A9320U},
    {"GetPrimaryTarget", 0x008A9460U},
    {"TempAddFloatsam", 0x008C5B80U},
    {"TempAddAnim", 0x008C5CD0U},
    {"RemoveTempAddedStuff", 0x008C5E20U},
    {"InsertFishSchool", 0x008C2970U},
    {"WipeEntityType", 0x008C2AC0U},
    {"GetTargetInfoTarget", 0x008C2C50U},
    {"StartVehicleAnim", 0x008C2E00U},
    {"StopVehicleAnim", 0x008C3060U},
    {"SetVisibilityTerrainNode", 0x008C3170U},
    {"GetVisibilityTerrainNode", 0x008C3410U},
    {"AddLineOfSightTrigger", 0x008C9460U},
    {"AddProximityTrigger", 0x008C9730U},
    {"AddSquadOrdnanceDroppedTrigger", 0x008C9A10U},
    {"RemoveTrigger", 0x008C5FD0U},
    {"LoadMessageMap", 0x008C61C0U},
    {"UnloadMessageMap", 0x008C6490U},
    {"EnableMessages", 0x008CFE40U},
    {"AddListener", 0x008C6760U},
    {"RemoveListener", 0x008C6990U},
    {"IsListenerActive", 0x008C6BB0U},
    {"DisplayMessage", 0x008D08B0U},
    {"OverrideMessage", 0x008D0EA0U},
    {"SetTipEffect", 0x0089E670U},
    {"IsInPosition", 0x0089ED20U},
    {"SetForceSellableUnits", 0x00891070U},
    {"CheatMaxRepair", 0x008C6DD0U},
    {"CheatRemoveWater", 0x008C7010U},
    {"SoundFade", 0x008D16B0U},
    {"Blackout", 0x008D1340U},
    {"GameOver", 0x008C74B0U},
    {"MusicFade", 0x008C7190U},
    {"BlackBars", 0x008C7300U},
    {"Loading_Start", 0x008CC850U},
    {"Loading_Progress", 0x008C76C0U},
    {"Loading_Finish", 0x008C77F0U},
    {"PerfTimingStart", 0x008D1C50U},
    {"PerfTimingEnd", 0x008D1DD0U},
    {"MemoryStatus", 0x008C7BE0U},
    {"StartMeasure", 0x008C7950U},
    {"StopMeasure", 0x008C7A70U},
    {"dprintf", 0x008C7E10U},
    {"Region", 0x008C7D20U},
    {"debugtrap", 0x008C8390U},
    {"class_enumeration", 0x008C8560U},
    {"SetLight", 0x008C44B0U},
    {"SetSky", 0x008C49A0U},
    {"SetShallowWater", 0x008C4B50U},
    {"AddPowerup", 0x008EE410U},
    {"PreparePowerup", 0x008EA610U},
    {"GetAvailablePowerups", 0x008EB350U},
    {"AICreate", 0x00A37310U},
    {"AIEnable", 0x00A37400U},
    {"AIEnableGrouping", 0x00A37650U},
    {"AIMergeGroups", 0x00A37790U},
    {"AICreateGroup", 0x00A38A50U},
    {"AIGetGroupInfo", 0x00A378C0U},
    {"AISetCommand", 0x00A37A00U},
    {"AISetHintWeight", 0x00A37B30U},
    {"AISetQuickSpawnTargetPos", 0x00A38010U},
    {"AISetDefendResourcePercent", 0x00A37D50U},
    {"AIReloadGlobals", 0x00A38960U},
    {"AISetSpawnSceneUnitsWeightMul", 0x00A37EB0U},
    {"AIGetTargetWeight", 0x00A38200U},
    {"AISetTargetWeight", 0x00A38430U},
    {"SetMotionBlurParams", 0x0088E560U},
    {"SetMBP", 0x0088E560U},
    {"SetDOF", 0x00897210U},
    {"DisablePhysics", 0x00891380U},
    {"BreakShip", 0x008914F0U},
    {"TerminateExecution", 0x008C8250U},
};
constexpr std::size_t kBindingCount = sizeof(kBindings) / sizeof(kBindings[0]);

// 00885110 opens with this mode and 00887220 receives these literal modes.
constexpr int kScriptOpenMode = 2;
constexpr int kChunkResultMode = 2; // 00885110 and 00884be0 pass 2
constexpr int kCallResultMode = 4; // 008879b3 passes 4

// game+1FE4h value that 00887750 and 00887e50 refuse to run in.
constexpr int kRefusedLifecycleState = 2;

} // namespace

const MissionLuaBinding* mission_lua_bindings() noexcept { return kBindings; }

std::size_t mission_lua_binding_count() noexcept { return kBindingCount; }

const MissionLuaBinding* find_mission_lua_binding(const char* name) noexcept
{
    if (name == nullptr) return nullptr;
    for (std::size_t i = 0; i < kBindingCount; ++i) {
        if (std::strcmp(kBindings[i].name, name) == 0) return &kBindings[i];
    }
    return nullptr;
}

bool loading_screen_calls_suppressed(int script_load_depth) noexcept
{
    return script_load_depth > 0;
}

int mission_lua_pushed_value_count(const std::vector<MissionLuaArgument>& arguments) noexcept
{
    int pushed = 0;
    for (const MissionLuaArgument& argument : arguments) {
        if (argument.type != MissionLuaArgumentType::Skipped) ++pushed;
    }
    return pushed;
}

std::vector<std::string> split_lua_entry_point_name(const std::string& name)
{
    std::vector<std::string> segments;
    std::string current;
    for (const char c : name) {
        if (c == kMissionLuaNameSeparator) {
            if (!current.empty()) segments.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(c);
    }
    if (!current.empty()) segments.push_back(current);
    return segments;
}

std::size_t initialise_mission_lua_host(MissionLuaHostServices& host)
{
    host.create_state();
    host.set_panic_function(kMissionLuaPanicFunction);
    host.set_gc_pause(kLuaGcSetPause, kMissionLuaGcPause);
    for (std::size_t i = 0; i < kMissionLuaStandardLibraryCount; ++i) {
        host.open_standard_library(kMissionLuaStandardLibraries[i]);
    }
    // 00884c37: the platform chunk runs before the table is installed.
    run_lua_chunk(host, kMissionLuaPlatformChunk,
        static_cast<int>(std::strlen(kMissionLuaPlatformChunk)), kMissionLuaPlatformChunk, false,
        false, kChunkResultMode);
    for (std::size_t i = 0; i < kBindingCount; ++i) {
        host.register_global_function(kBindings[i]);
    }
    // 00884c94: the bytes come from the singleton at 00884770, so the chunk
    // name is a label rather than a virtual file system path. The host owns the
    // buffer, which is why nothing is passed here.
    return kBindingCount;
}

LuaChunkResult run_lua_chunk(MissionLuaHostServices& host, const char* buffer, int size,
    const char* chunk_name, bool capture_error, bool capture_results, int result_mode)
{
    LuaChunkResult result;
    const int saved_top = host.lua_gettop();
    int status = host.luaL_loadbuffer(buffer, size, chunk_name);
    if (status == 0) {
        status = host.lua_pcall(0, kLuaMultRet, kLuaNoErrorHandler);
        result.status = status == 0 ? LuaChunkStatus::Ok : LuaChunkStatus::RunFailed;
    } else {
        result.status = LuaChunkStatus::LoadFailed;
    }

    const int new_top = host.lua_gettop();
    if (new_top > saved_top) {
        if (lua_chunk_failed(result.status)) {
            // 006b8a5f and 006b8a80: the string is read on both paths and only
            // stored when a sink was supplied.
            std::string message = host.lua_tolstring_at_top();
            if (capture_error) {
                result.error_message = message;
                result.error_message_captured = true;
            }
        } else if (capture_results) {
            result.results_collected = host.collect_results(new_top - saved_top, result_mode);
        }
        host.lua_settop(saved_top);
    }
    return result;
}

LuaChunkResult run_script_file(MissionLuaHostServices& host, const std::string& path)
{
    LuaChunkResult result;
    if (!host.open_script(path)) {
        // 0088528c: the stream is not released on this path either.
        return result;
    }
    const int size = host.script_size();
    std::vector<char> buffer(static_cast<std::size_t>(size < 0 ? 0 : size));
    host.read_script(buffer.empty() ? nullptr : buffer.data(), size);
    // 00885110 passes its own path argument as the chunk name and a stack local
    // as the error sink, then frees that local without reading it.
    result = run_lua_chunk(host, buffer.empty() ? "" : buffer.data(), host.script_size(),
        path.c_str(), true, false, kChunkResultMode);
    result.error_message.clear();
    result.error_message_captured = false;
    host.close_script();
    return result;
}

std::vector<LuaChunkResult> run_script_with_variants(
    MissionLuaHostServices& host, const std::string& path, bool run_variants)
{
    std::vector<LuaChunkResult> results;
    results.push_back(run_script_file(host, path));
    if (!run_variants) return results;
    for (const std::string& variant : host.script_variant_names(path)) {
        results.push_back(run_script_file(host, variant));
    }
    return results;
}

std::vector<LuaChunkResult> run_mission_script(
    MissionLuaHostServices& host, const std::string& scene_name)
{
    return run_script_with_variants(host, mission_script_path(scene_name), true);
}

namespace {

// 008877e0..008879bb, shared by 00887750 and the inline path of 00887e50.
NamedCallOutcome run_named_call_body(
    MissionLuaHostServices& host, const NamedCallRequest& request, NamedCallOutcome outcome)
{
    const int saved_top = host.lua_gettop();

    int forward_first = request.forward_stack_first;
    int forward_last = request.forward_stack_last;
    if (forward_first != 0) {
        const int top = host.lua_gettop();
        if (forward_first < 0) forward_first = top + 1 + forward_first;
        if (forward_last < 0) forward_last = top + 1 + forward_last;
    }

    host.lua_getglobal(kMissionLuaErrorHandler);
    const int error_handler_index = host.lua_gettop();
    host.adjust_reentrancy_depth(1);

    outcome.resolved_path = split_lua_entry_point_name(request.name);
    const char* first = outcome.resolved_path.empty() ? "" : outcome.resolved_path.front().c_str();
    host.lua_getglobal(first);
    for (std::size_t i = 1; i < outcome.resolved_path.size(); ++i) {
        host.lua_pushstring(outcome.resolved_path[i].c_str());
        host.lua_gettable(-2);
        host.lua_remove(-2);
    }

    int nargs = 0;
    if (!request.self_key.empty()) {
        host.lua_getglobal(kMissionLuaSelfTable);
        host.lua_pushstring(request.self_key.c_str());
        host.lua_gettable(-2);
        host.lua_remove(-2);
        nargs = 1;
    }
    for (const MissionLuaArgument& argument : request.arguments) {
        host.push_argument(argument);
    }
    nargs += mission_lua_pushed_value_count(request.arguments);
    if (forward_first != 0 && forward_first <= forward_last) {
        nargs += 1 + (forward_last - forward_first);
        for (int index = forward_first; index <= forward_last; ++index) {
            host.lua_pushvalue(index);
        }
    }
    outcome.arguments_pushed = nargs;

    const int marker = host.lua_gettop();
    host.adjust_call_stack_marker(marker);
    outcome.pcall_status = host.lua_pcall(nargs, kLuaMultRet, error_handler_index);
    host.adjust_call_stack_marker(-marker);

    const int results = host.lua_gettop() - saved_top;
    outcome.results = results;
    if (request.collect_results && results != 0) {
        host.collect_results(results, kCallResultMode);
    }
    host.lua_settop(saved_top);
    host.adjust_reentrancy_depth(-1);
    outcome.dispatched = true;
    return outcome;
}

} // namespace

NamedCallOutcome call_named_entry_point(MissionLuaHostServices& host, const NamedCallRequest& request)
{
    NamedCallOutcome outcome;
    // 00887782: the gate only bites when the caller did not force the call.
    if (!request.run_during_shutdown && host.game_lifecycle_state() == kRefusedLifecycleState) {
        return outcome;
    }
    return run_named_call_body(host, request, outcome);
}

NamedCallOutcome call_named_entry_point_forced(
    MissionLuaHostServices& host, const std::string& name, std::vector<MissionLuaArgument> arguments)
{
    NamedCallRequest request;
    request.name = name;
    request.arguments = std::move(arguments);
    request.forward_stack_first = 0;
    request.forward_stack_last = kLuaMultRet;
    request.collect_results = false;
    request.run_during_shutdown = true;
    return call_named_entry_point(host, request);
}

NamedCallOutcome call_named_entry_point_threadsafe(
    MissionLuaHostServices& host, const std::string& name, std::vector<MissionLuaArgument> arguments)
{
    NamedCallOutcome outcome;
    // 00887e73 and 00887ec8: the lifecycle gate is unconditional here and is
    // tested twice, once before and once after the lock is taken.
    if (host.game_lifecycle_state() == kRefusedLifecycleState) return outcome;
    if (host.on_frame_job_thread()) {
        host.queue_named_call_for_main_thread(name);
        outcome.queued = true;
        return outcome;
    }
    if (host.game_lifecycle_state() == kRefusedLifecycleState) return outcome;

    NamedCallRequest request;
    request.name = name;
    request.arguments = std::move(arguments);
    request.forward_stack_first = 0;
    request.forward_stack_last = kLuaMultRet;
    request.collect_results = false;
    request.run_during_shutdown = true;
    return run_named_call_body(host, request, outcome);
}

NamedCallOutcome call_entry_point_if_defined(MissionLuaHostServices& host, const std::string& name,
    std::vector<MissionLuaArgument> arguments, bool threadsafe)
{
    NamedCallOutcome outcome;
    // 0045f48d and 0045f4b3: the global is fetched, tested and released before
    // anything else happens, and a missing global ends the call.
    if (!host.global_is_defined(name.c_str())) return outcome;
    if (threadsafe) return call_named_entry_point_threadsafe(host, name, std::move(arguments));
    return call_named_entry_point_forced(host, name, std::move(arguments));
}

} // namespace bsp
