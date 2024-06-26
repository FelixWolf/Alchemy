/*
 * @file llpaneleventinfo.cpp
 * @brief Event info panel
 *
 * Copyright (c) 2014, Cinder Roxley <cinder@sdf.org>
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef LL_PANELEVENTINFO_H
#define LL_PANELEVENTINFO_H

#include "llpanel.h"
#include "lleventnotifier.h"

class LLPanelEventInfo : public LLPanel
{
public:
    LLPanelEventInfo();
    /*virtual*/ BOOL postBuild() override;
    /*virtual*/ void onOpen(const LLSD& key) override;

private:
    ~LLPanelEventInfo();
    bool processEventReply(const LLEventStruct& event);
    std::string formatFromMinutes(U32 time);
    void setEventID(U32 id) { mEventID = id; }
    U32 getEventID() const { return mEventID; }
    void onBtnTeleport();
    void onBtnMap();
    void onBtnRemind();

    LLEventStruct mEvent;
    U32 mEventID;
    boost::signals2::connection mEventNotifierConnection;
};

#endif // LL_PANELEVENTINFO_H
