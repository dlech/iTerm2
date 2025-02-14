//
//  VT100ScreenState.h
//  iTerm2SharedARC
//
//  Created by George Nachman on 12/10/21.
//
// All state from VT100Screen should eventually migrate here to facilitate a division between
// mutable and immutable code paths.

#import <Foundation/Foundation.h>

#import "FindContext.h"
#import "IntervalTree.h"
#import "LineBuffer.h"
#import "VT100Grid.h"
#import "VT100ScreenMark.h"
#import "VT100ScreenSideEffects.h"
#import "VT100Terminal.h"
#import "iTermColorMap.h"
#import "iTermIntervalTreeObserver.h"
#import "iTermMark.h"
#import "iTermTemporaryDoubleBufferedGridController.h"

NS_ASSUME_NONNULL_BEGIN

@class IntervalTree;
@class VT100InlineImageHelper;
@class iTermOrderEnforcer;

@protocol VT100ScreenState<NSObject>

@property (nonatomic, readonly) BOOL audibleBell;
@property (nonatomic, readonly) BOOL showBellIndicator;
@property (nonatomic, readonly) BOOL flashBell;
@property (nonatomic, readonly) BOOL postUserNotifications;
@property (nonatomic, readonly) BOOL cursorBlinks;

// When set, strings, newlines, and linefeeds are appended to printBuffer_. When ANSICSI_PRINT
// with code 4 is received, it's sent for printing.
@property (nonatomic, readonly) BOOL collectInputForPrinting;

@property (nullable, nonatomic, strong, readonly) NSString *printBuffer;

// OK to report window title?
@property (nonatomic, readonly) BOOL allowTitleReporting;

@property (nonatomic, readonly) NSTimeInterval lastBell;

// Line numbers containing animated GIFs that need to be redrawn for the next frame.
@property (nonatomic, strong, readonly) NSIndexSet *animatedLines;

// base64 value to copy to pasteboard, being built up bit by bit.
@property (nullable, nonatomic, strong, readonly) NSString *pasteboardString;

// All currently visible marks and notes. Maps an interval of
//   (startx + absstarty * (width+1)) to (endx + absendy * (width+1))
// to an id<IntervalTreeObject>, which is either PTYNoteViewController or VT100ScreenMark.
@property (nonatomic, strong, readonly) id<IntervalTreeReading> intervalTree;

@property (nonatomic, strong, readonly) id<VT100GridReading> primaryGrid;
@property (nullable, nonatomic, strong, readonly) id<VT100GridReading> altGrid;
// Points to either primaryGrid or altGrid.
@property (nonatomic, strong, readonly) id<VT100GridReading> currentGrid;
// When a saved grid is swapped in, this is the live current grid.
@property (nonatomic, strong, readonly) id<VT100GridReading> realCurrentGrid;

// Holds notes on alt/primary grid (the one we're not in). The origin is the top-left of the
// grid.
@property (nullable, nonatomic, strong, readonly) IntervalTree *savedIntervalTree;

// Cached copies of terminal attributes
@property (nonatomic, readonly) BOOL wraparoundMode;
@property (nonatomic, readonly) BOOL ansi;
@property (nonatomic, readonly) BOOL insert;

// This flag overrides maxScrollbackLines:
@property (nonatomic, readonly) BOOL unlimitedScrollback;

@property (nonatomic, strong, readonly) VT100Terminal *terminal;
@property (nonatomic, strong, readonly) FindContext *findContext;
@property (nonatomic, readonly) int scrollbackOverflow;

// Location of the start of the current command, or (-1, -1) for none.
@property (nonatomic, readonly) VT100GridAbsCoord commandStartCoord;

// Maps an absolute line number to a VT100ScreenMark.
@property (nonatomic, strong, readonly) NSDictionary<NSNumber *, id<iTermMark>> *markCache;

// Max size of scrollback buffer
@property (nonatomic, readonly) unsigned int maxScrollbackLines;

// Where the next tail-find needs to begin.
@property (nonatomic, readonly) long long savedFindContextAbsPos;

@property (nonatomic, strong, readonly) NSSet<NSNumber *> *tabStops;

// Indicates which character set (they are represented by numbers) are in line-drawing mode.
// Valid charsets are in 0..<NUM_CHARSETS
@property (nonatomic, strong, readonly) NSSet<NSNumber *> *charsetUsesLineDrawingMode;

// For REP
@property (nonatomic, readonly) screen_char_t lastCharacter;
@property (nonatomic, readonly) BOOL lastCharacterIsDoubleWidth;
@property (nullable, nonatomic, strong, readonly) iTermExternalAttribute *lastExternalAttribute;

@property (nonatomic, readonly) BOOL saveToScrollbackInAlternateScreen;
@property (nonatomic, readonly) BOOL cursorVisible;
@property (nonatomic, readonly) BOOL shellIntegrationInstalled;
@property (nonatomic, readonly) VT100GridAbsCoordRange lastCommandOutputRange;

// Valid while at the command prompt only. Gives the range of the current prompt. Meaningful
// only if the end is not equal to the start.
@property(nonatomic, readonly) VT100GridAbsCoordRange currentPromptRange;

@property (nonatomic, readonly) VT100GridAbsCoord startOfRunningCommandOutput;
@property (nonatomic, readonly) VT100TerminalProtectedMode protectedMode;

// Initial size before calling -restoreFromDictionary… or -1,-1 if invalid.
@property (nonatomic, readonly) VT100GridSize initialSize;

// A rarely reset count of the number of lines lost to scrollback overflow. Adding this to a
// line number gives a unique line number that won't be reused when the linebuffer overflows.
@property (nonatomic, readonly) long long cumulativeScrollbackOverflow;

@property (nonatomic, strong, readonly) id<LineBufferReading> linebuffer;
@property (nonatomic, readonly) BOOL trackCursorLineMovement;
@property (nonatomic, readonly) BOOL appendToScrollbackWithStatusBar;
@property (nonatomic, readonly) iTermUnicodeNormalization normalization;
@property (nonatomic, weak, readonly) id<iTermIntervalTreeObserver> intervalTreeObserver;
@property (nullable, nonatomic, strong, readonly) VT100ScreenMark *lastCommandMark;
@property (nonatomic, strong, readonly) id<iTermColorMapReading> colorMap;
@property (nonatomic, strong, readonly) id<iTermTemporaryDoubleBufferedGridControllerReading> temporaryDoubleBuffer;

// -2: Within command output (inferred)
// -1: Uninitialized
// >= 0: The line the prompt is at
@property (nonatomic, readonly) long long fakePromptDetectedAbsLine;

// Line where last prompt begain
@property (nonatomic, readonly) long long lastPromptLine;
@property (nonatomic, strong, readonly) id<VT100ScreenSideEffectQueueReading> sideEffects;

// Did we get FinalTerm codes that report info about prompt? Used to decide if advanced paste
// can wait for prompts.
@property (nonatomic, readonly) BOOL shouldExpectPromptMarks;
@property (nonatomic, readonly) BOOL needsRedraw;

@end

@interface VT100ScreenMutableState: NSObject<VT100ScreenState, NSCopying>

@property (nonatomic, readwrite) BOOL audibleBell;
@property (nonatomic, readwrite) BOOL showBellIndicator;
@property (nonatomic, readwrite) BOOL flashBell;
@property (nonatomic, readwrite) BOOL postUserNotifications;
@property (nonatomic, readwrite) BOOL cursorBlinks;
@property (nonatomic, readwrite) BOOL collectInputForPrinting;
@property (nullable, nonatomic, strong, readwrite) NSMutableString *printBuffer;
@property (nonatomic, readwrite) BOOL allowTitleReporting;
@property (nullable, nonatomic, strong) VT100InlineImageHelper *inlineImageHelper;
@property (nonatomic, readwrite) NSTimeInterval lastBell;
@property (nonatomic, strong, readwrite) NSMutableIndexSet *animatedLines;
@property (nullable, nonatomic, strong, readwrite) NSMutableString *pasteboardString;
@property (nonatomic, strong, readwrite) iTermOrderEnforcer *setWorkingDirectoryOrderEnforcer;
@property (nonatomic, strong, readwrite) iTermOrderEnforcer *currentDirectoryDidChangeOrderEnforcer;
@property (nonatomic, strong, readwrite) IntervalTree *intervalTree;

@property (nonatomic, strong, readwrite) VT100Grid *primaryGrid;
@property (nullable, nonatomic, strong, readwrite) VT100Grid *altGrid;
@property (nonatomic, strong, readwrite) VT100Grid *currentGrid;
// When a saved grid is swapped in, this is the live current grid.
@property (nullable, nonatomic, strong, readwrite) VT100Grid *realCurrentGrid;
@property (nullable, nonatomic, strong, readwrite) IntervalTree *savedIntervalTree;
@property (nonatomic, strong, readwrite) VT100Terminal *terminal;
@property (nonatomic, readwrite) BOOL wraparoundMode;
@property (nonatomic, readwrite) BOOL ansi;
@property (nonatomic, readwrite) BOOL insert;
@property (nonatomic, readwrite) BOOL unlimitedScrollback;
@property (nonatomic, strong, readwrite) FindContext *findContext;
// How many scrollback lines have been lost due to overflow. Periodically reset with
// -resetScrollbackOverflow.
@property (nonatomic, readwrite) int scrollbackOverflow;
@property (nonatomic, readwrite) VT100GridAbsCoord commandStartCoord;
@property (nonatomic, strong, readwrite) NSMutableDictionary<NSNumber *, id<iTermMark>> *markCache;
@property (nonatomic, readwrite) unsigned int maxScrollbackLines;
@property (nonatomic, readwrite) long long savedFindContextAbsPos;
@property (nonatomic, strong, readwrite) NSMutableSet<NSNumber *> *tabStops;
@property (nonatomic, strong) NSMutableSet<NSNumber *> *charsetUsesLineDrawingMode;
@property (nonatomic, readwrite) screen_char_t lastCharacter;
@property (nonatomic, readwrite) BOOL lastCharacterIsDoubleWidth;
@property (nullable, nonatomic, strong, readwrite) iTermExternalAttribute *lastExternalAttribute;
@property (nonatomic, readwrite) BOOL saveToScrollbackInAlternateScreen;
@property (nonatomic, readwrite) BOOL cursorVisible;
@property (nonatomic, readwrite) BOOL shellIntegrationInstalled;
@property (nonatomic, readwrite) VT100GridAbsCoordRange lastCommandOutputRange;
@property (nonatomic, readwrite) VT100GridAbsCoordRange currentPromptRange;
@property (nonatomic, readwrite) VT100GridAbsCoord startOfRunningCommandOutput;
@property (nonatomic, readwrite) VT100TerminalProtectedMode protectedMode;
@property (nonatomic, readwrite) VT100GridSize initialSize;
@property (nonatomic, readwrite) long long cumulativeScrollbackOverflow;
@property (nonatomic, strong, readwrite) LineBuffer *linebuffer;
@property (nonatomic, readwrite) BOOL trackCursorLineMovement;
@property (nonatomic, readwrite) BOOL appendToScrollbackWithStatusBar;
@property (nonatomic, readwrite) iTermUnicodeNormalization normalization;
#warning TODO: Prevent access to intervalTreeObserver on the mutation queue. It should only be called on the main queue.
@property (nonatomic, weak, readwrite) id<iTermIntervalTreeObserver> intervalTreeObserver;
@property (nullable, nonatomic, strong, readwrite) VT100ScreenMark *lastCommandMark;
@property (nonatomic, strong, readwrite) iTermColorMap *colorMap;
@property (nonatomic, strong, readwrite) iTermTemporaryDoubleBufferedGridController *temporaryDoubleBuffer;
@property (nonatomic, readwrite) long long fakePromptDetectedAbsLine;
@property (nonatomic, readwrite) long long lastPromptLine;
@property (nonatomic, strong, readonly) VT100ScreenSideEffectQueue *sideEffects;
@property (nonatomic, readwrite) BOOL shouldExpectPromptMarks;

#warning TODO: Once this synced properly (see comment in VT100Screen+Mutation) then it can be made nonatomic.
@property (atomic) BOOL needsRedraw;

- (id<VT100ScreenState>)copy;

@end

NS_ASSUME_NONNULL_END
