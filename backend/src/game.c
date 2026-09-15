#include "game.h"

network_package_t network_package;

/* My PlayerUID is 12 wchars (account created in recent seasons, UIDs are linear) */
#define MAX_UID_LENGTH 12 + 1   // To be sure 
#define MIN_UID_LENGTH 5        // Minimal possible account UID length

bool check_uid(uint64_t ttep, uint64_t fstring)
{
    FString uid = { 0 };
    read_buffer(ttep, fstring, &uid, sizeof(uid));

    if (!uid.wstring || uid.length < MIN_UID_LENGTH || uid.length > MAX_UID_LENGTH || uid.length > uid.capacity)
        return false;

    return true;
}

//
//  Pseudocode of UWorld pointer decryption routine from UE4.
//
uint64_t decrypt_uworld_pointer(uint64_t ttep, uint64_t a1)
{
    uint64_t v1; // x9
    uint64_t v2; // x8
    unsigned int *v3; // x10
    unsigned int v4; // t1

    v1 = 0;
    v2 = 0;
    v3 = (unsigned int *)(a1 + 0x80);
    while ( v1 != 64 )
    {
        v4 = read_u32(ttep, (uint64_t)v3);
        v3++;

        v2 |= (uint64_t)read_u8(ttep, a1 + v4) << v1;
        v1 += 8;
    }
    return v2;
}

void broadcast_players_data(int player_count) 
{
    /* Set the package player count */
    network_package.header.player_count = player_count;

    /* Calculate size of players to send */
    size_t network_package_size = sizeof(package_header_t) + ( player_count * sizeof(player_info_t) );

    /* Broadcast the players data */
    broadcast_data((const uint8_t*)&network_package, network_package_size);
}

bool world_to_screen(vec3_t local_position, rot3_t local_rotation, vec3_t world_position, float fov, vec2_t *out_screen)
{
    /* Sanity check to make sure we pass the pointer */
    if (!out_screen)
        return false;

    /* Convert look angles into radians */
    float yaw_rad   = DEG2RAD(local_rotation.yaw);
    float pitch_rad = DEG2RAD(local_rotation.pitch);

    /* Calculate yaw direction ratios */
    float cos_yaw = cosf(yaw_rad);
    float sin_yaw = sinf(yaw_rad);

    /* Calculate pitch direction ratios */
    float cos_pitch = cosf(pitch_rad);
    float sin_pitch = sinf(pitch_rad);

    /* Create 3D direction arrow in front of local look */
    vec3_t forward = { cos_pitch * cos_yaw, cos_pitch * sin_yaw, sin_pitch };

    /* Create horizontal 3D arrow */
    vec3_t right = { -sin_yaw, cos_yaw, 0.0f };
    
    /* Create vertical 3D arrow */
    vec3_t up = {
        forward.y * right.z - forward.z * right.y,
        forward.z * right.x - forward.x * right.z,
        forward.x * right.y - forward.y * right.x
    };

    /* Calculate distance from local position to object position */
    vec3_t delta = {
        world_position.x - local_position.x,
        world_position.y - local_position.y,
        world_position.z - local_position.z
    };

    /* Measuring forward distance to the target */
    float forward_distance = delta.x * forward.x + delta.y * forward.y + delta.z * forward.z;

    // If forward distance is negative, target is behind us. If near zero means that it's too close. 
    // Using this check we are also protecting ourself for division by zero that could happen next. 
    // In case with e.g. a lines ESP that point into the locations behind the screen, clipping will break it. 
    // You may want to remove/ignore that check in this case. 
    if (forward_distance <= 0.1f)
        return false;

    /* Measuring horizontal distance to the target */
    float horizontal_distance = delta.x * right.x + delta.y * right.y + delta.z * right.z;

    /* Measuring vertical distance to the target */
    float vertical_distance = delta.x * up.x + delta.y * up.y + delta.z * up.z;

    /* Calculate half of the fov and convert to radians */
    float half_fov_rad = DEG2RAD(fov / 2.0f);

    /* Calculate the projection multiplayer based on fov */
    float tan_horizontal = tanf(half_fov_rad);

    /* Calculate our screen aspect ratio */
    float aspect_ratio = SCREEN_WIDTH / SCREEN_HEIGHT;

    /* Vertical projection multiplayer will be different due to the screen aspect ratio */
    float tan_vertical = tan_horizontal / aspect_ratio;

    /* Calculate fraction position of 3D position on 2D screen */
    float fraction_x = (horizontal_distance / forward_distance) / tan_horizontal;
    float fraction_y = (vertical_distance / forward_distance)   / tan_vertical;

    /* Convert x and y fraction screen position into actual screen pixel coordinates */
    out_screen->x = (SCREEN_WIDTH / 2.0f) + fraction_x * (SCREEN_WIDTH / 2.0f);
    out_screen->y = (SCREEN_HEIGHT / 2.0f) - fraction_y * (SCREEN_HEIGHT / 2.0f);

    // In case with e.g. a lines ESP that point into the locations behind the screen, clipping will break it. 
    // You may want to remove/ignore that check in this case. 
    if (out_screen->x < 0.0f || out_screen->x > SCREEN_WIDTH || out_screen->y < 0.0f || out_screen->y > SCREEN_HEIGHT)
        return false;

    return true;
}

float calculate_distance(vec3_t v1, vec3_t v2) 
{
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;
    
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

//
//  Modified version of actor array pointer decryption routine from this thread:
//  https://www.unknowncheats.me/forum/pubg-mobile/515295-pubgm-actorsarray-decryption.html
//
//  I have traced couple in-direct calls but not really found anything related to actors array encryption. (iOS)
//  So I endup just using the decryption routine from the thread above. Credits to 0xPrince.
//
uint64_t DecryptActorsArray(uint64_t ttep, uint64_t uLevel, int Actors_Offset, int EncryptedActors_Offset)
{
    struct ActorsEncryption {
        uint64_t Enc_1, Enc_2;
        uint64_t Enc_3, Enc_4;
    };
    struct Encryption_Chunk {
        uint32_t val_1, val_2, val_3, val_4;
        uint32_t val_5, val_6, val_7, val_8;
    };

	if (read_u64(ttep, uLevel + EncryptedActors_Offset) > 0)
		return uLevel + EncryptedActors_Offset;

	struct ActorsEncryption Encryption = { 0 };
	read_buffer(ttep, uLevel + EncryptedActors_Offset + 0x10, &Encryption, sizeof(Encryption));

	if (Encryption.Enc_1 > 0)
	{
		struct Encryption_Chunk Enc = { 0 }; 
		read_buffer(ttep, Encryption.Enc_1 + 0x80, &Enc, sizeof(Enc));

        return (((read_u8(ttep, Encryption.Enc_1 + Enc.val_1)
                | (read_u8(ttep, Encryption.Enc_1 + Enc.val_2) << 8))
                | (read_u8(ttep, Encryption.Enc_1 + Enc.val_3) << 0x10)) & 0xFFFFFF)
                | ((uint64_t)read_u8(ttep, Encryption.Enc_1 + Enc.val_4) << 0x18)
                | (((uint64_t)read_u8(ttep, Encryption.Enc_1 + Enc.val_5) << 0x20) & 0xFFFF00FFFFFFFFFF)
                | ((uint64_t)read_u8(ttep, Encryption.Enc_1 + Enc.val_6) << 0x28)
                | ((uint64_t)read_u8(ttep, Encryption.Enc_1 + Enc.val_7) << 0x30)
                | ((uint64_t)read_u8(ttep, Encryption.Enc_1 + Enc.val_8) << 0x38);
	}
	else if (Encryption.Enc_2 > 0)
	{
		uint64_t Encrypted_Actors = read_u64(ttep, Encryption.Enc_2);
		if (Encrypted_Actors > 0)
		{
			return ((uint16_t)(Encrypted_Actors - 0x400) & 0xFF00)
                    | (uint8_t)(Encrypted_Actors - 0x04)
                    | ((Encrypted_Actors + 0xFC0000) & 0xFF0000)
                    | ((Encrypted_Actors - 0x4000000) & 0xFF000000)
                    | ((Encrypted_Actors + 0xFC00000000) & 0xFF00000000)
                    | ((Encrypted_Actors + 0xFC0000000000) & 0xFF0000000000)
                    | ((Encrypted_Actors + 0xFC000000000000) & 0xFF000000000000)
                    | ((Encrypted_Actors - 0x400000000000000) & 0xFF00000000000000);
		}
	}
	else if (Encryption.Enc_3 > 0)
	{
		uint64_t Encrypted_Actors = read_u64(ttep, Encryption.Enc_3);
		if (Encrypted_Actors > 0)
			return (Encrypted_Actors >> 0x38) | (Encrypted_Actors << (64 - 0x38));
	}
	else if (Encryption.Enc_4 > 0)
	{
		uint64_t Encrypted_Actors = read_u64(ttep, Encryption.Enc_4);
		if (Encrypted_Actors > 0)
			return Encrypted_Actors ^ 0xCDCD00;
	}

	return 0;
}
