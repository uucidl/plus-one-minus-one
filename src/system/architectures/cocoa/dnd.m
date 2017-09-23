/* a10 295
 * Copyright (c) 2001-2003 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('./system/architectures/cocoa/dnd.m')
 *with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 295 */

#include <SDL.h>
#import <Cocoa/Cocoa.h>
#include <AppKit/NSWindow.h>
#include <system/demo.h>
#include <system/event.h>
#include <system/event_listener.h>
#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(DND);

static event_listener_t *event_listener = NULL;

static void dnd_send_event(const char *uri)
{
    drop_event_t drop_event;
    drop_event_instantiate_toplevel(&drop_event);
    drop_event.hierarchy[2].verb = (atom_t)strdup(uri);

    DEBUG2("received uri: %s", uri);

    event_listener->accept(event_listener, &drop_event);

    event_destroy(&drop_event);
    event_retire(&drop_event);
}

/*
  Extends all the NSWindow of the application to support our dnd.
*/

@interface NSWindow (DndEnabled)
- (void)setupDnd;
- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender;
@end

@implementation NSWindow (DndEnabled)
- (void)setupDnd
{
    [self
        registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType,
                                                          nil]];
    TRACE1("initialized dnd.");
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    NSPasteboard *pboard;
    pboard = [sender draggingPasteboard];

    if ([[pboard types] containsObject:NSFilenamesPboardType]) {
        return NSDragOperationGeneric;
    }

    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSPasteboard *pboard;
    pboard = [sender draggingPasteboard];

    if ([[pboard types] containsObject:NSFilenamesPboardType]) {
        NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
        int n = [files count];
        int i;
        for (i = 0; i < n; i++) {
            dnd_send_event([[files objectAtIndex:i] UTF8String]);
        }
    }
    return YES;
}
@end

void dnd_init(event_listener_t *el)
{
    event_listener = el;
    NSArray *wins;
    wins = [NSApp windows];
    int n = [wins count];
    int i;
    for (i = 0; i < n; i++) {
        [[wins objectAtIndex:i] setupDnd];
    }
}
