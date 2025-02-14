//
//  iTermHyperlinkTrigger.m
//  iTerm2
//
//  Created by leppich on 09.05.18.
//

#import "NSURL+iTerm.h"
#import "iTermHyperlinkTrigger.h"

#import <CoreServices/CoreServices.h>
#import <QuartzCore/QuartzCore.h>

@implementation iTermHyperlinkTrigger

+ (NSString *)title {
    return @"Make Hyperlink…";
}

- (BOOL)takesParameter {
    return YES;
}

- (BOOL)isIdempotent {
    return YES;
}

- (NSString *)triggerOptionalParameterPlaceholderWithInterpolation:(BOOL)interpolation {
    return [self triggerOptionalDefaultParameterValueWithInterpolation:interpolation];
}

- (NSString *)triggerOptionalDefaultParameterValueWithInterpolation:(BOOL)interpolation {
    if (interpolation) {
        return @"https://\\(match0)";
    }
    return @"https://\\0";
}

- (BOOL)performActionWithCapturedStrings:(NSString *const *)capturedStrings
                          capturedRanges:(const NSRange *)capturedRanges
                            captureCount:(NSInteger)captureCount
                               inSession:(id<iTermTriggerSession>)aSession
                                onString:(iTermStringLine *)stringLine
                    atAbsoluteLineNumber:(long long)lineNumber
                        useInterpolation:(BOOL)useInterpolation
                                    stop:(BOOL *)stop {
    const NSRange rangeInString = capturedRanges[0];
    
    // Need to stop the world to get scope, provided it is needed. This is potentially going to be a performance problem for a small number of users.
    [self paramWithBackreferencesReplacedWithValues:capturedStrings
                                              count:captureCount
                                              scope:[aSession triggerSessionVariableScope:self]
                                              owner:aSession
                                   useInterpolation:useInterpolation
                                         completion:^(NSString *urlString) {
        [self performActionWithURLString:urlString
                                   range:rangeInString
                                 session:aSession
                      absoluteLineNumber:lineNumber];
    }];
    return YES;
}

- (void)performActionWithURLString:(NSString *)urlString
                             range:(NSRange)rangeInString
                           session:(id<iTermTriggerSession>)aSession
                absoluteLineNumber:(long long)lineNumber {
    NSURL *url = urlString.length ? [NSURL URLWithUserSuppliedString:urlString] : nil;
    if (!url) {
        return;
    }

    [aSession triggerSession:self
          makeHyperlinkToURL:url
                     inRange:rangeInString
                        line:lineNumber];
}

@end
