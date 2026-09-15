#pragma once

//
//  iOS Internals Offsets
//

/* iOS 16.x.x (Tested on 16.5.1) / 15.x.x (Not tested) */
#define TASK_map       0x28
#define VM_MAP_pmap    0x40 // Could be 0x48 on iOS 15.x.x
#define PMAP_ttep      0x8

//
//  Game Offsets 
//

/* Commonly called GWorld */
#define UWorld_Pointer                          0xacec620 // 4.5.0 0xaa11ea0

/* Format: {ClassName}_{Field} */

#define UNetDriver_ServerConnection             0x78
#define UPlayer_PlayerController                0x30

#define AUAECharacter_PlayerUID                 0x988
#define AUAECharacter_TeamID                    0x998

#define APlayerController_AcknowledgedPawn      0x528
#define APlayerController_PlayerCameraManager   0x548

#define ASTExtraCharacter_bDead                 0xe7c
#define ASTExtraCharacter_Health                0xe60
#define ASTExtraCharacter_HealthMax             0xe64
#define ASTExtraCharacter_CurrentVehicle        0xeb0

#define AActor_RootComponent                    0x208
#define USceneComponent_RelativeLocation        0x1e4
#define APlayerCameraManager_CameraCache        0x520

/* You may not find correct ActorCluster offset inside the dump */
#define ULevel_ActorCluster                     0xa0

/* Magic actors array decryption offset */
#define ActorClusterEncryptedOffset             0x448