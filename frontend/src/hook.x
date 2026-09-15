#import <UIKit/UIKit.h>
#import "overlay.h"

#include "pch.h"

bool g_Initialized = false;

//
//  Hook UIWindow class.
//
%hook UIWindow
    //
    //  Create a hook for UIWindow::initWithFrame routine.
    //  We want to intercept window creation routine.
    //
    - (instancetype)initWithFrame:(CGRect)frame 
    {
        /* Let springboard create desired window and get its instance */
        self = %orig;

        /* Make sure that the window is created */
        if (self && !g_Initialized) 
        {
            /* UIWindow could be called asynchronously so set the flag first to prevent race conditions */
            g_Initialized = true;

            //
            //  We can't use here dispatch_once yet, it will crash.
            //  That's why we are using this flag based initialization, sadly.
            //
            dispatch_async(dispatch_get_main_queue(), ^{

                /* Get screen bounds and size */
                CGRect screen_bounds = [UIScreen mainScreen].bounds;
                CGFloat landscape_width = MAX(screen_bounds.size.width, screen_bounds.size.height);
                CGFloat landscape_height = MIN(screen_bounds.size.width, screen_bounds.size.height);

                //
                //  I implement the drawing that is invisible for the screen recording. 
                //  To achive this we create dummy text field with secure property enabled.
                //  This property will make this field invisible for the screen recorders.
                //  To make our overlay invisible too, we will just add our overlay as subview to this secure container.
                //

                /* Create a dummy UITextField with secure text entry enabled */
                UITextField* secure_text_field = [[UITextField alloc] init];

                /* Made a toggle so you can decide during compilation you want it or not */
                #if HIDE_OVERLAY_FROM_CAPTURE
                    secure_text_field.secureTextEntry = YES;
                #else
                    secure_text_field.secureTextEntry = NO;
                #endif
                
                //
                //  Locate the secure container view inside the text field 
                //  The system renders secure text inside a specific internal subview (usually the first subview) 
                //
                UIView* secure_container = nil;
                if (secure_text_field.subviews.count > 0) 
                {
                    secure_container = secure_text_field.subviews.firstObject;
                } 
                else 
                {
                    secure_container = secure_text_field;
                }

                /* Initialize our overlay */
                g_Overlay = [[UIView alloc] initWithFrame:CGRectMake(0, 0, landscape_width, landscape_height)];
                g_Overlay.backgroundColor = [UIColor clearColor];
                g_Overlay.userInteractionEnabled = NO; // Make our overlay touch through
                g_Overlay.center = CGPointMake(screen_bounds.size.width / 2.0f, screen_bounds.size.height / 2.0f);
                g_Overlay.transform = CGAffineTransformMakeRotation(-M_PI_2);

                /* Initialize the vectored canvases, each for one color */
                g_CanvasGreen = [CAShapeLayer layer];
                g_CanvasYellow = [CAShapeLayer layer];
                g_CanvasRed = [CAShapeLayer layer];

                NSArray* layers = @[g_CanvasGreen, g_CanvasYellow, g_CanvasRed];
                for (CAShapeLayer *layer in layers) 
                {
                    layer.frame = g_Overlay.bounds;
                    layer.strokeColor = [UIColor clearColor].CGColor;
                    layer.fillColor = [UIColor clearColor].CGColor;
                    layer.lineWidth = ESP_LINE_WIDTH; 
                    [g_Overlay.layer addSublayer:layer];
                }

                /* Add our overlay to the secure container */
                [secure_container addSubview:g_Overlay];
                
                /* Ensure text field itself doesn't block interactions or visibility */
                secure_text_field.frame = CGRectMake(0, 0, landscape_width, landscape_height);
                secure_text_field.userInteractionEnabled = NO;
                secure_text_field.backgroundColor = [UIColor clearColor];
                
                /* Add secure text field to the SpringBoard window */
                [self addSubview:secure_text_field];
            });

            /* Dispatch network receiver thread */
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                start_network_server();
            });
        }

        /* Return SpringBoard its created window */
        return self;
    }
%end
