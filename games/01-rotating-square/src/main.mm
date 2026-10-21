// main.mm — the platform layer: one window, one view, a 60 Hz tick, and the game loop.
//
// Everything here is presentation and timing. The ECS knows nothing about AppKit; it
// hands the render system's output (a RenderList) to -drawRect:, which turns quads
// into pixels.
//
// The game loop is three lines in -tick: advance time, run the systems in order,
// ask for a redraw. That hardcoded order is deliberate (see Docs/PURPOSE.md).

#import <Cocoa/Cocoa.h>

#include "ecs.hpp"

#include <cstdio>

// ---------------------------------------------------------------------------

@interface GameView : NSView
@property(nonatomic) const ecs::RenderList* renderList;
@end

@implementation GameView

- (BOOL)isOpaque {
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
    CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];

    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);

    if (self.renderList == nullptr) {
        return;
    }

    const CGFloat cx = NSMidX(self.bounds);
    const CGFloat cy = NSMidY(self.bounds);

    CGContextSetShouldAntialias(ctx, true);
    for (const ecs::Quad& quad : *self.renderList) {
        CGContextSaveGState(ctx);
        CGContextTranslateCTM(ctx, cx + quad.x, cy + quad.y);
        CGContextRotateCTM(ctx, quad.angle);
        CGContextSetRGBFillColor(ctx, quad.colour.r, quad.colour.g, quad.colour.b,
                                 quad.colour.a);
        CGContextFillRect(ctx, CGRectMake(-quad.halfSize, -quad.halfSize,
                                          2.0 * quad.halfSize, 2.0 * quad.halfSize));
        CGContextRestoreGState(ctx);
    }
}

@end

// ---------------------------------------------------------------------------

@interface GameController : NSObject <NSApplicationDelegate>
@end

@implementation GameController {
    ecs::World _world;
    ecs::RenderList _renderList;
    NSWindow* _window;
    GameView* _view;
    NSTimer* _timer;
    CFAbsoluteTime _lastTick;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    (void)notification;

    // The world is populated once, at startup.
    ecs::spawnSquare(_world, /*halfSize=*/120.0f, /*degreesPerSecond=*/20.0f,
                     ecs::Rgba{0.95f, 0.55f, 0.20f, 1.0f});

    const NSRect content = NSMakeRect(0.0, 0.0, 720.0, 720.0);
    const NSWindowStyleMask style = NSWindowStyleMaskTitled |
                                    NSWindowStyleMaskClosable |
                                    NSWindowStyleMaskMiniaturizable;

    _window = [[NSWindow alloc] initWithContentRect:content
                                          styleMask:style
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    _window.title = @"rotating square";
    _window.backgroundColor = [NSColor blackColor];
    [_window center];

    _view = [[GameView alloc] initWithFrame:content];
    _view.renderList = &_renderList;
    _window.contentView = _view;

    [self installMenu];
    [_window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    _lastTick = CFAbsoluteTimeGetCurrent();
    __weak GameController* weakSelf = self;
    _timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 60.0
                                             repeats:YES
                                               block:^(NSTimer* timer) {
                                                 (void)timer;
                                                 [weakSelf tick];
                                               }];

    std::printf("rotating-square: close the window or press Command-Q to quit.\n");
    std::fflush(stdout);
}

// The game loop. Two systems, one order, no registry.
- (void)tick {
    const CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    float dt = static_cast<float>(now - _lastTick);
    _lastTick = now;

    // Clamp after a stall (window drag, debugger break) so the square does not jump.
    if (dt > 0.05f) {
        dt = 0.05f;
    }

    ecs::rotationSystem(_world, dt);        // mutate: angle += spin * dt
    ecs::renderSystem(_world, _renderList); // read world, produce quads

    [_view setNeedsDisplay:YES];
}

- (void)installMenu {
    NSMenu* menuBar = [[NSMenu alloc] init];
    NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
    [menuBar addItem:appMenuItem];
    [NSApp setMainMenu:menuBar];

    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenu addItemWithTitle:@"Quit rotating square"
                       action:@selector(terminate:)
                keyEquivalent:@"q"];
    appMenuItem.submenu = appMenu;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    (void)sender;
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notification {
    (void)notification;
    [_timer invalidate];
}

@end

// ---------------------------------------------------------------------------

int main(int argc, const char* argv[]) {
    (void)argc;
    (void)argv;

    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        GameController* controller = [[GameController alloc] init];
        app.delegate = controller;

        [app run];
    }
    return 0;
}
