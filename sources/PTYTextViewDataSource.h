// DataSource for PTYTextView.
#import "iTermCursor.h"
#import "iTermFindDriver.h"
#import "iTermLogicalMovementHelper.h"
#import "ScreenChar.h"
#import "LineBuffer.h"
#import "VT100GridTypes.h"

@class iTermColorMap;
@class PTYNoteViewController;
@class PTYSession;
@class PTYTask;
@class SCPPath;
@class VT100Grid;
@class VT100RemoteHost;
@class VT100ScreenMark;
@class VT100Terminal;

@interface PTYTextViewSynchronousUpdateState : NSObject
@property (nonatomic, strong) VT100Grid *grid;
@property (nonatomic) BOOL cursorVisible;
@property (nonatomic, strong) iTermColorMap *colorMap;
@end

@protocol iTermTextDataSource <NSObject>

- (int)width;
- (int)numberOfLines;
- (ScreenCharArray *)screenCharArrayOnLine:(int)line;
- (NSArray<ScreenCharArray *> *)linesInRange:(NSRange)range;
- (long long)totalScrollbackOverflow;

@end


@protocol PTYTextViewDataSource <iTermLogicalMovementHelperDelegate, iTermTextDataSource>

- (VT100Terminal *)terminal;
- (int)height;

// Cursor position is 1-based (the top left is at 1,1).
- (int)cursorX;
- (int)cursorY;

- (const screen_char_t *)getLineAtScreenIndex:(int)theIndex;

// Provide a buffer as large as sizeof(screen_char_t*) * ([SCREEN width] + 1)
- (const screen_char_t *)getLineAtIndex:(int)theIndex withBuffer:(screen_char_t*)buffer;
- (NSArray<ScreenCharArray *> *)linesInRange:(NSRange)range;
- (int)numberOfScrollbackLines;
- (int)scrollbackOverflow;
- (void)resetScrollbackOverflow;
- (long long)absoluteLineNumberOfCursor;
- (BOOL)continueFindAllResults:(NSMutableArray*)results
                     inContext:(FindContext*)context;
- (FindContext*)findContext;

// Initialize the find context.
- (void)setFindString:(NSString*)aString
     forwardDirection:(BOOL)direction
                 mode:(iTermFindMode)mode
          startingAtX:(int)x
          startingAtY:(int)y
           withOffset:(int)offsetof  // Offset in the direction of searching (offset=1 while searching backwards means start one char before x,y)
            inContext:(FindContext*)context
      multipleResults:(BOOL)multipleResults;

// Save the position of the current find context (with the screen appended).
- (void)saveFindContextAbsPos;

// Return a human-readable dump of the screen contents.
- (NSString*)debugString;
- (BOOL)isAllDirty;
- (void)resetAllDirty;
- (void)setRangeOfCharsAnimated:(NSRange)range onLine:(int)line;
- (NSIndexSet *)animatedLines;
- (void)resetAnimatedLines;

// Set the cursor dirty. Cursor coords are different because of how they handle
// being in the WIDTH'th column (it wraps to the start of the next line)
// whereas that wouldn't normally be a legal X value. If possible, the char to the right of the
// cursor is also set dirty to handle DWCs.
- (void)setCharDirtyAtCursorX:(int)x Y:(int)y;
- (void)setLineDirtyAtY:(int)y;

// Check if any the character at x,y has been marked dirty.
- (void)resetDirty;

// Save the current state to a new frame in the dvr.
- (void)saveToDvr:(NSIndexSet *)cleanLines;

// If this returns true then the textview will broadcast iTermTabContentsChanged
// when a dirty char is found.
- (BOOL)shouldSendContentsChangedNotification;

// NOTE: y is a grid index and cannot refer to scrollback history.
- (BOOL)lineIsDirty:(int)y;

- (BOOL)textViewGetAndResetHasScrolled;

// Returns the last modified date for a given line.
- (NSDate *)timestampForLine:(int)y;

- (void)addNote:(PTYNoteViewController *)note inRange:(VT100GridCoordRange)range;
- (void)removeInaccessibleNotes;

// Returns all notes in a range of cells.
- (NSArray *)notesInRange:(VT100GridCoordRange)range;

- (VT100GridCoordRange)coordRangeOfNote:(PTYNoteViewController *)note;
- (NSArray *)charactersWithNotesOnLine:(int)line;
- (VT100ScreenMark *)markOnLine:(int)line;

- (NSString *)workingDirectoryOnLine:(int)line;

- (SCPPath *)scpPathForFile:(NSString *)filename onLine:(int)line;
- (VT100RemoteHost *)remoteHostOnLine:(int)line;
- (VT100GridCoordRange)textViewRangeOfOutputForCommandMark:(VT100ScreenMark *)mark;

// Indicates if we're in alternate screen mode.
- (BOOL)showingAlternateScreen;

- (void)clearBuffer;

// When the cursor is about to be hidden, a copy of the grid is saved. This
// method is used to temporarily swap in the saved grid if one is available. It
// returns a nonnil state if the saved grid was swapped in (only possible if useSavedGrid
// is YES, of course).
- (PTYTextViewSynchronousUpdateState *)setUseSavedGridIfAvailable:(BOOL)useSavedGrid;
- (NSString *)compactLineDumpWithContinuationMarks;

@end
