/**
 * @file slplugin-objc.h
 * @brief Header file for slplugin-objc.mm.
 *
 * @cond
 *
 * $LicenseInfo:firstyear=2010&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 *
 * @endcond
 */

//Protos for ObjectiveC classes (cannot import cocoa here due to BOOL conflict)
#ifndef __OBJC__
class NSWindow;
#endif // __OBJC__

/* Defined in slplugin-objc.mm: */

class LLCocoaPlugin
{
public:
    LLCocoaPlugin();
    void setupCocoa();
    void setupGroup();
    void updateWindows();
    void processEvents();
public:
    //EventTargetRef mEventTarget;
    NSWindow* mFrontWindow;
    NSWindow* mPluginWindow;
    int mHackState;
};


