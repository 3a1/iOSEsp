#import "overlay.h"

UIView *g_Overlay = nil;

CAShapeLayer *g_CanvasGreen = nil;
CAShapeLayer *g_CanvasYellow = nil;
CAShapeLayer *g_CanvasRed = nil;

void clear_screen_overlay(void) 
{
    if (!g_Overlay || !g_CanvasGreen || !g_CanvasYellow || !g_CanvasRed) return;

    dispatch_async(dispatch_get_main_queue(), ^{
        g_CanvasGreen.path = NULL;
        g_CanvasYellow.path = NULL;
        g_CanvasRed.path = NULL;
    });
}

void update_screen_overlay(player_info_t* players_info, int player_count) 
{
    /* Sanity check */
    if (!g_Overlay || !g_CanvasGreen || !g_CanvasYellow || !g_CanvasRed) return;

    //
    //  Apple uses the points dimensions for UI. 
    //  Those points scale depends on the device. 
    //  We will just dynamically apply scale to our pixel coordinates. 
    //

    /* Get the screen scale */
    CGFloat scale = [UIScreen mainScreen].scale;

    /* Create 3 separate mutable paths on the background thread */
    CGMutablePathRef green_path = CGPathCreateMutable();
    CGMutablePathRef yellow_path = CGPathCreateMutable();
    CGMutablePathRef red_path = CGPathCreateMutable();

    for (int i = 0; i < player_count; i++) 
    {
        player_info_t player = players_info[i];

        vec2_t top_left = player.screen_box_x;
        vec2_t bottom_right = player.screen_box_y;
        
        /* Skip invalid bounding boxes */
        if ((top_left.x == 0.0 && top_left.y == 0.0) || (bottom_right.x == 0.0 && bottom_right.y == 0.0)) 
            continue;

        /* Scale pixel coordinates to the device points measure */
        CGFloat x1 = top_left.x / scale;
        CGFloat y1 = top_left.y / scale;
        CGFloat x2 = bottom_right.x / scale;
        CGFloat y2 = bottom_right.y / scale;

        /* Create a rect */
        CGRect player_rect = CGRectMake(x1, y1, x2 - x1, y2 - y1);

        //
        //  In modes like TDM player health can be more than 100.
        //  In such cases we will use the max health value to calculate health percentage.
        //  This will allow us correctly apply health color in such dynamic cases. 
        //

        /* Calculate health percentage */
        float max_health = player.max_health;
        float current_health = player.health;
        
        float health_percentage = 0.0f;
        /* Always remember to protect ourselves against division by zero */
        if (max_health > 0.0f)
            health_percentage = (current_health / max_health) * 100.0f;

        /* Add color path based on current health percentage */
        if (health_percentage > 70.0f) {
            CGPathAddRect(green_path, NULL, player_rect);
        } else if (health_percentage > 35.0f) {
            CGPathAddRect(yellow_path, NULL, player_rect);
        } else {
            CGPathAddRect(red_path, NULL, player_rect);
        }
    }

    //
    //  Push the drawing updates atomically on the Main UI Thread.
    //  Without dispatching into main thread it will crash SpringBoard.
    //
    dispatch_async(dispatch_get_main_queue(), ^{
        g_CanvasGreen.path = green_path;
        g_CanvasGreen.strokeColor = [UIColor greenColor].CGColor;

        g_CanvasYellow.path = yellow_path;
        g_CanvasYellow.strokeColor = [UIColor yellowColor].CGColor;

        g_CanvasRed.path = red_path;
        g_CanvasRed.strokeColor = [UIColor redColor].CGColor;
        
        CGPathRelease(green_path);
        CGPathRelease(yellow_path);
        CGPathRelease(red_path);
    });
}