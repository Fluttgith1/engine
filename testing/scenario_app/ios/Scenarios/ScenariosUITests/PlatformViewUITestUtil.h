// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

extern const NSInteger kTimeToWaitForPlatformView;

@interface PlatformViewUITestUtil : NSObject

+ (NSString*)platformName;

+ (BOOL)compareImage:(UIImage*)a toOther:(UIImage*)b;

@end

NS_ASSUME_NONNULL_END
