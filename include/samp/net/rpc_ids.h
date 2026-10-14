#pragma once
// Full RPC enumerations. Ported from samp-source sdk/raknet/SAMPRPC.h.
// Game RPC 0-49 (netrpc.cpp), script RPC 100-189 (scriptrpc.cpp).
// REA note: samp.dll 0.3.DL-R1 has no "RPC" defined-strings (observed absent),
// so these IDs come from the open reference; wire-compat must be verified
// against a live server before claiming interop.
#include <cstdint>

namespace samp { namespace net {

enum class GameRpc : uint8_t {
  ServerJoin = 0, ServerQuit = 1, InitGame = 2, VehicleSpawn = 3,
  VehicleDestroy = 4, VehicleDamage = 5, SetCheckpoint = 6,
  DisableCheckpoint = 7, SetRaceCheckpoint = 8, DisableRaceCheckpoint = 9,
  GameModeRestart = 10, ConnectionRejected = 11, ClientMessage = 12,
  WorldTime = 13, Pickup = 14, DestroyPickup = 15, DestroyWeaponPickup = 16,
  Weather = 17, Instagib = 18, SetTimeEx = 19, ToggleClock = 20, Chat = 21,
  RequestClass = 22, RequestSpawn = 23, Spawn = 24, ClientJoin = 25,
  ServerCommand = 26, SetInteriorId = 27, AdminMapTeleport = 28,
  VehicleDestroyed = 29, PickedUpWeapon = 30, PickedUpPickup = 31,
  Death = 32, EnterVehicle = 33, ExitVehicle = 34, UpdateScoresPingsIPs = 35,
  ScmEvent = 36, MenuSelect = 37, MenuQuit = 38, TypingEvent = 39,
  ClientCheck = 40, Click = 41, WorldAddActor = 42, WorldRemoveActor = 43,
  ActorDamage = 44, CreateLabel = 45, DestroyLabel = 46, ChatBubble = 47,
  SetTimer = 48, ModelRequest = 49,
};

enum class ScriptRpc : uint8_t {
  // Stored as int IDs 100-189 on the wire; enum class keeps names only.
  ScrSetSpawnInfo = 100, SetPlayerTeam = 101, SetPlayerSkin = 102,
  SetPlayerName = 103, SetPlayerPos = 104, SetPlayerPosFindZ = 105,
  SetPlayerHealth = 106, PutPlayerInVehicle = 107,
  RemovePlayerFromVehicle = 108, SetPlayerColor = 109,
  DisplayGameText = 110, SetInterior = 111, SetCameraPos = 112,
  SetCameraLookAt = 113, SetVehiclePos = 114, SetVehicleZAngle = 115,
  VehicleParams = 116, SetCameraBehindPlayer = 117,
  TogglePlayerControllable = 118, PlaySound = 119, SetWorldBounds = 120,
  HaveSomeMoney = 121, SetPlayerFacingAngle = 122, ResetMoney = 123,
  ResetPlayerWeapons = 124, GivePlayerWeapon = 125, RespawnVehicle = 126,
  LinkVehicle = 127, SetPlayerArmour = 128, DeathMessage = 129,
  SetMapIcon = 130, DisableMapIcon = 131, SetWeaponAmmo = 132,
  SetGravity = 133, SetVehicleHealth = 134, AttachTrailerToVehicle = 135,
  DetachTrailerFromVehicle = 136, CreateObject = 137, SetObjectPos = 138,
  SetObjectRotation = 139, DestroyObject = 140, SetPlayerVirtualWorld = 141,
  SetVehicleVirtualWorld = 142, CreateExplosion = 143, ShowNameTag = 144,
  MoveObject = 145, StopObject = 146, SetObjectScale = 147,
  NumberPlate = 148, TogglePlayerSpectating = 149, SetPlayerSpectating = 150,
  PlayerSpectatePlayer = 151, PlayerSpectateVehicle = 152,
  ForceSpawnSelection = 153, RemoveComponent = 154,
  AttachObjectToPlayer = 155, InitMenu = 156, ShowMenu = 157,
  HideMenu = 158, SetPlayerWantedLevel = 159, ShowTextDraw = 160,
  HideTextDraw = 161, EditTextDraw = 162, AddGangZone = 163,
  RemoveGangZone = 164, FlashGangZone = 165, StopFlashGangZone = 166,
  ApplyAnimation = 167, ClearAnimations = 168, SetSpecialAction = 169,
  EnableStuntBonus = 170, SetVehicle = 171, SetPlayer = 172,
  VehicleVelocity = 173, PlayerVelocity = 174, InterpolateCamera = 175,
  VehicleComponent = 176, SetGameSpeed = 177, ToggleChatbox = 178,
  ToggleWidescreen = 179, SetScore = 180, SetShopName = 181,
  AudioStream = 182, PlayCrimeReport = 183, SetActorPos = 184,
  SetActorFacingAngle = 185, SetActorHealth = 186,
  ApplyActorAnimation = 187, ClearActorAnimation = 188,
  DisableVehicleCollision = 189,
};

// RakNet connection packet IDs used by netgame.cpp UpdateNetwork.
enum class ConnPacket : uint8_t {
  Rpc = 81, ConnRequestAccepted = 95, ConnAttemptFailed = 99,
  NoFreeIncoming = 107, DisconnNotify = 110, ConnLost = 115,
  RsaKeyMismatch = 118, ConnBanned = 121, InvalidPassword = 126,
  ModifiedPacket = 130, Timestamp = 133,
};

}} // namespace
