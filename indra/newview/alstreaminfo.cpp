/*
 * @file alstreaminfo.cpp
 * @brief Class enables display of audio stream metadata
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

#include "llviewerprecompiledheaders.h"
#include "alstreaminfo.h"

#include "llaudioengine.h"
#include "llnotificationsutil.h"
#include "llstreamingaudio.h"
#include "lltrans.h"
#include "llurlaction.h"
#include "llviewercontrol.h" // LLCachedControl

#include "llfloater.h"
#include "llfloaterreg.h"

#include "llnotifications.h"

ALStreamInfo::ALStreamInfo()
{
    if (gAudiop && gAudiop->getStreamingAudioImpl() && gAudiop->getStreamingAudioImpl()->supportsMetaData())
    {
        mMetadataConnection = gAudiop->getStreamingAudioImpl()->setMetadataUpdatedCallback([this](const LLSD& metadata) { handleMetadataUpdate(metadata); });
    }
}

ALStreamInfo::~ALStreamInfo()
{
    if (mMetadataConnection.connected())
    {
        mMetadataConnection.disconnect();
    }
}

void ALStreamInfo::handleMetadataUpdate(const LLSD& metadata)
{
    static LLCachedControl<bool> show_stream_info(gSavedSettings, "ShowStreamInfo", false);
    if (!show_stream_info) return;

    bool stream_info_to_chat = gSavedSettings.getBOOL("ShowStreamInfoToChat");

    LLFloater* music_ticker = LLFloaterReg::findInstance("music_ticker");
    if (!stream_info_to_chat && music_ticker)
        return;

    if (metadata.size() > 0)
    {
        std::string station = metadata.has("TRSN") ? metadata["TRSN"].asString() : metadata.has("icy-name") ? metadata["icy-name"].asString() : LLTrans::getString("NowPlaying");
        // Some stations get a little ridiculous with the length.
        if (station.length() > 64)
        {
            LLStringUtil::truncate(station, 64);
            station.append("...");
        }
        LLSD args;
        LLSD payload;
        args["STATION"] = station;
        std::stringstream info;
        if (metadata.has("TITLE"))
            info << metadata["TITLE"].asString();
        if (metadata.has("TITLE") && metadata.has("ARTIST"))
            info << "\n";
        if (metadata.has("ARTIST"))
            info << metadata["ARTIST"].asString();
        args["INFO"] = info.str();

        std::string station_url;
        if (metadata.has("URL"))
            station_url = metadata["URL"].asString();
        else if (metadata.has("icy-url"))
            station_url = metadata["icy-url"].asString();

        LLNotification::Params notify_params;
        notify_params.name = "StreamInfo";
        notify_params.substitutions = args;
        if (!station_url.empty())
        {
            notify_params.payload = payload.with("respond_on_mousedown", TRUE);

            LLNotification::Params::Functor functor_p;
            functor_p.function = [=](const LLSD&, const LLSD&) {LLUrlAction::openURL(station_url); };
            notify_params.functor = functor_p;
        }
        else
        {
            notify_params.payload = payload;
        }

        notify_params.force_to_chat = stream_info_to_chat;
        LLNotificationPtr notification = LLNotifications::instance().add(notify_params);
    }
}
