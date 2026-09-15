#include "pch.h"

int main(int argc, char *argv[])
{
    //
    //  Initialize the jailbreak primitives.
    //  Passing true because we want access to read/write PTEs.
    //  We are doing it only once.
    // 
    int result = jbclient_initialize_primitives_internal( true );
    if ( result == -1 ) {
        ErrorPrint("[-] jailbreak initialize primitives failed");
        return 0; // Exit in this case
    }

    //
    //  Initialize udp network client on 127.0.0.1:8080.
    //  We are also doing it only once.
    //
    if (!init_network_client(SERVER_PORT)) {
        ErrorPrint("[-] failed to initialize network client\n");
        return 0; // Exit in this case
    }

    //
    //  Loop across the game restarts
    //
	while (true) 
	{
		/* Get proccess pid */
		int pid = get_pid_by_name("ShadowTracker");
		if (!pid) {
			ErrorPrint("[-] can't find the process");
			sleep_ms(1000);
			continue;
		}

		/* Get physical translation table of the process memory */
		uint64_t ttep = get_process_ttep(pid);
		if (!ttep) {
			ErrorPrint("[-] get process tteb failed");
			continue;
		}

		/* Get process base */
		uint64_t app_base = get_process_base(pid);
		if (!app_base) {
			ErrorPrint("[-] get process base failed");
			continue;
		}

		/* Decrypt UWorld pointer using UE4 original decryption function */ 
		uint64_t uworld_decrypted = decrypt_uworld_pointer(ttep, app_base + UWorld_Pointer);
		if (!uworld_decrypted) {
			ErrorPrint("[-] decrypting UWorld pointer failed");
			continue;
		}

        /* Enter the actual game reading loop */
        game_loop(ttep, uworld_decrypted);
	}

	return 0;
}

void game_loop(uint64_t ttep, uint64_t uworld_pointer) 
{
    while (true) 
    {
        sleep_ms(30);

        /* Get UWorld pointer */
        uint64_t uworld = read_u64(ttep, uworld_pointer);
        if (!uworld) {
            ErrorPrint("[-] reading UWorld address failed");
            break;
        }

        /* Read UWorld structure */
        UWorld world = { 0 };
        if (!read_buffer(ttep, uworld, &world, sizeof(world))) {
            ErrorPrint("[-] reading UWorld class failed");
            break;
        }

        /* We should have persistent level */
        if (!world.PersistentLevel) { ErrorPrint("[-] empty PersistentLevel"); break; }

        /* Are we in match? */
        while (!world.NetDriver) 
        {
            /* No? Wait a bit and try again. */
            sleep_ms(1000);
            continue;
        }

        //
        //  Good, we are now in match.
        //  Get local player first. 
        //

        uint64_t server_connection = read_u64(ttep, world.NetDriver + UNetDriver_ServerConnection);
        if (!server_connection) { ErrorPrint("[-] failed to read ServerConnection"); continue; }

        uint64_t player_controller = read_u64(ttep, server_connection + UPlayer_PlayerController);
        if (!player_controller) { ErrorPrint("[-] failed to read PlayerController"); continue; }

        uint64_t acknowledged_pawn = read_u64(ttep, player_controller + APlayerController_AcknowledgedPawn);
        if (!acknowledged_pawn) { ErrorPrint("[-] failed to read AcknowledgedPawn"); continue; }

        //
        //  If we are inside a car, local pawn will be car object. 
        //  Cars doesn't have team numbers, so it will be zero.
        //  Due to this we will cache the local team value. 
        //  Sketchy solution? I don't like it either.
        //

        static int cache_localplayer_team = 0;
        int localplayer_team = (int)read_u32(ttep, acknowledged_pawn + AUAECharacter_TeamID);

        if (localplayer_team == 0)
            localplayer_team = cache_localplayer_team;
        else
            cache_localplayer_team = localplayer_team;

        //
        //  Read actor cluster array. 
        //  I'm clipping the max actor count to 4000.
        //  So far I have only saw it reach up to 2000 in Pochinki. 
        //
        //  Sometimes the read can also fail. 
        //

        #define MAX_ACTOR_COUNT 4000

        TArray actors = { 0 };
        if (!read_buffer(ttep, world.PersistentLevel + ULevel_ActorCluster, &actors, sizeof(actors))) {
            ErrorPrint("[-] failed to read ActorCluster");
            continue;
        }
        
        if (!actors.array) 
        {
            //
            //  We don't have array pointer? That means it's probably encrypted. 
            //
            uint64_t actors_array_decrypted = DecryptActorsArray(ttep, world.PersistentLevel, ULevel_ActorCluster, ActorClusterEncryptedOffset);
            if (!actors_array_decrypted) {
                ErrorPrint("[-] failed to decrypt actors array pointer");
                continue;
            }
            
            /* Decrypted? Read it again. */
            read_buffer(ttep, actors_array_decrypted, &actors, sizeof(actors));
        }

        if (!actors.count || actors.count > MAX_ACTOR_COUNT || actors.count > actors.max) { 
            ErrorPrint("[-] actors array sanity check failed"); 
            continue; 
        } 

        //
        //  Sometimes the actors array pointer gets replaced by some PF trap address. 
        //  You can read more about it in my writeup on z3bra.cat. 
        //  Due to this we will cache the entire actors array.
        //  Can we just cache the cluster pointer only? Maybe, I'm too lazy to test it out. 
        //

        static uint64_t cache_actors_array[MAX_ACTOR_COUNT] = { 0 };
        static int cache_actor_count = 0;

        uint64_t local_actors_array[MAX_ACTOR_COUNT];

        if (read_buffer(ttep, actors.array, local_actors_array, actors.count * sizeof(uint64_t))) 
        {
            /* Reading succeed? Saving the actors into the cache */
            memcpy(cache_actors_array, local_actors_array, actors.count * sizeof(uint64_t));
            cache_actor_count = actors.count;
        }
        else 
        {
            /* Reading failed? PF trap is set. Let's use the actors from the cache. */
            ErrorPrint("[-] reading actors array failed; switching to use actors from the cache...");
            memcpy(local_actors_array, cache_actors_array, cache_actor_count * sizeof(uint64_t));
            actors.count = cache_actor_count;
        }

        /* Local player structure for the reading loop */
        typedef struct {
            float max_health;
            float health;
            vec3_t location;
        } player_t;

        /* Create player count */
        int player_count = 0;

        /* Create players array */
        player_t players[MAX_PLAYERS];

        for (int i = 0; i < actors.count; i++) 
        {
            uint64_t actor = local_actors_array[i];

            /* Sanity check */
            if (!actor || actor == acknowledged_pawn) 
                continue;

            /* Filter only players */
            if (!check_uid(ttep, actor + AUAECharacter_PlayerUID))
                continue;

            //
            //  Create local AActor class buffer.
            //  We are only accessing fields up to ASTExtraCharacter class.
            //  So we will just use this class size as our buffer size.
            //  We are reading all inherited bytes to access the child classes fields too.
            //

            #define ASTExtraCharacter_Size 0x12c0

            uint8_t actor_buffer[ASTExtraCharacter_Size];

            if (!read_buffer(ttep, actor, &actor_buffer, sizeof(actor_buffer))) {
                ErrorPrint("[-] reading AActor failed");
                continue;
            }

            /* Player is dead? Skip! */
            uint8_t dead = *(uint8_t*)(actor_buffer + ASTExtraCharacter_bDead);
            if (dead) continue;

            /* Same team as local player? Skip! */
            int team =  *(int*)(actor_buffer + AUAECharacter_TeamID);
            if (team == localplayer_team) continue;

            float health = *(float*)(actor_buffer + ASTExtraCharacter_Health);
            float max_health = *(float*)(actor_buffer + ASTExtraCharacter_HealthMax);

            //
            //  Health depends on the player state (e.g. knocked == 0).
            //  Depends also on the game mode (e.g. tdm == 120).
            //  So checking health for boundaries does not give a sense to me, except MaxHealth sanity check.
            //
            if (health > max_health) continue;

            vec3_t location = { 0 };

            uint64_t current_vehicle = *(uint64_t*)(actor_buffer + ASTExtraCharacter_CurrentVehicle);
            if (current_vehicle)
            {
                //
                //  This player is inside vehicle, read the position from the vehicle instead.
                //

                uint64_t vehicle_root = read_u64(ttep, current_vehicle + AActor_RootComponent);
                if (!vehicle_root) continue;

                read_buffer(ttep, vehicle_root + USceneComponent_RelativeLocation, &location, sizeof(location));
            }
            else 
            {
                //
                //  This player is not inside the vehicle, read position from the player root component.
                //

                uint64_t root_component = *(uint64_t*)(actor_buffer + AActor_RootComponent);
                if (!root_component) continue;

                read_buffer(ttep, root_component + USceneComponent_RelativeLocation, &location, sizeof(location));
            }

            /* Sanity check to prevent buffer overflow */
            if (player_count > MAX_PLAYERS) continue;

            /* Save player information into players array */
            players[player_count].health = health;
            players[player_count].max_health = max_health;
            players[player_count].location = location;

            /* Increase player count */
            player_count++;
        }

        /* There are no players except us? Feel lonely, huh? */
        if (player_count == 0) 
        {
            //
            //  Jokes aside, we want to send the empty package in case if there are no players. 
            //  Let's tell the server that there are currently no players to clear the screen.
            //
            broadcast_players_data(0);
            continue;
        }

        /* Get local player camera */
        uint64_t player_camera_manager = read_u64(ttep, player_controller + APlayerController_PlayerCameraManager);
        if (!player_camera_manager) { ErrorPrint("[-] failed to read PlayerCameraManager"); continue; };

        FCameraCacheEntry camera_cache_entry = { 0 };
        if (!read_buffer(ttep, player_camera_manager + APlayerCameraManager_CameraCache, &camera_cache_entry, sizeof(camera_cache_entry))) {
            ErrorPrint("[-] failed to read CameraCache"); 
            continue; 
        }

        //
        //  Loop for calculating actual screen coordinates of the players. 
        //
        for (int i = 0; i < player_count; i++) 
        {
            player_t player = players[i];

            //
            //  About w2s return values, we are gonna to ignore them. 
            //  Even if the player is behind us or outside the screen, we want to notice the frontend.
            //  It's the frontend decision to render them or not. 
            //

            vec3_t location_x = player.location;
            location_x.z -= 75.0f;
            vec2_t screen_box_x = { 0 };
            world_to_screen(camera_cache_entry.POV.Location, camera_cache_entry.POV.Rotation, location_x, camera_cache_entry.POV.FOV, &screen_box_x);

            vec3_t location_y = player.location;
            location_y.z += 75.0f;
            vec2_t screen_box_y = { 0 };
            world_to_screen(camera_cache_entry.POV.Location, camera_cache_entry.POV.Rotation, location_y, camera_cache_entry.POV.FOV, &screen_box_y);

            float height = { screen_box_x.y - screen_box_y.y };
            
            /* Calculate box width based on height */
            screen_box_x.x += height / 4.0f;
            screen_box_y.x -= height / 4.0f;

            /* Set the player information inside the network package */
            network_package.players_info[i].max_health   = player.max_health;
            network_package.players_info[i].health       = player.health;
            network_package.players_info[i].screen_box_x = screen_box_x;
            network_package.players_info[i].screen_box_y = screen_box_y;
        }

        //
        // Network package prepared, we can send it to the server.
        //
        broadcast_players_data(player_count);
    }
}
