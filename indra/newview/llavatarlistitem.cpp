/**
 * @file llavatarlistitem.cpp
 * @brief avatar list item source file
 *
 * $LicenseInfo:firstyear=2009&license=viewerlgpl$
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
 */


#include "llviewerprecompiledheaders.h"

#include <boost/signals2.hpp>

#include "llavataractions.h"
#include "llavatarlistitem.h"

#include "llbutton.h"
#include "llfloaterreg.h"
#include "lltextutil.h"

#include "llagent.h"
#include "llavatarnamecache.h"
#include "llavatariconctrl.h"
#include "llnotificationsutil.h"
#include "lloutputmonitorctrl.h"
#include "llslurl.h"
#include "lltooldraganddrop.h"
// [RLVa:KB] - Checked: RLVa-2.0.1
#include "rlvactions.h"
#include "rlvcommon.h"
// [/RLVa:KB]
#include "alavatargroups.h"

bool LLAvatarListItem::sStaticInitialized = false;
S32 LLAvatarListItem::sLeftPadding = 0;
S32 LLAvatarListItem::sNameRightPadding = 0;
S32 LLAvatarListItem::sChildrenWidths[LLAvatarListItem::ALIC_COUNT];

static LLWidgetNameRegistry::StaticRegistrar sRegisterAvatarListItemParams(&typeid(LLAvatarListItem::Params), "avatar_list_item");

LLAvatarListItem::Params::Params()
:   default_style("default_style"),
    voice_call_invited_style("voice_call_invited_style"),
    voice_call_joined_style("voice_call_joined_style"),
    voice_call_left_style("voice_call_left_style"),
    online_style("online_style"),
    offline_style("offline_style"),
    name_right_pad("name_right_pad", 0)
{};


LLAvatarListItem::LLAvatarListItem(bool not_from_ui_factory/* = true*/)
    : LLPanel(),
    LLFriendObserver(),
    mOnlineStatus(E_UNKNOWN),
    mShowInfoBtn(true),
    mShowProfileBtn(true),
// [RLVa:KB] - Checked: RLVa-1.2.0
    mRlvCheckShowNames(false),
// [/RLVa:KB]
    mShowPermissions(SP_NEVER),
//  mShowPermissions(false),
    mShowCompleteName(false),
    mHovered(false),
    mAvatarNameCacheConnection(),
    mGreyOutUsername(""),
    mColorize(false)
{
    if (not_from_ui_factory)
    {
        buildFromFile("panel_avatar_list_item.xml");
    }
    // *NOTE: mantipov: do not use any member here. They can be uninitialized here in case instance
    // is created from the UICtrlFactory
}

LLAvatarListItem::~LLAvatarListItem()
{
    if (mAvatarId.notNull())
    {
        LLAvatarTracker::instance().removeParticularFriendObserver(mAvatarId, this);
    }

    if (mAvatarNameCacheConnection.connected())
    {
        mAvatarNameCacheConnection.disconnect();
    }
}

BOOL  LLAvatarListItem::postBuild()
{
    mAvatarIcon = getChild<LLAvatarIconCtrl>("avatar_icon");
    mAvatarName = getChild<LLTextBox>("avatar_name");
    mTextField = getChild<LLTextBox>("text_field");
    mTextField->setVisible(false);
    mTextField->setRightAlign();

    mIconPermissionOnline = getChild<LLButton>("permission_online_icon");
    mIconPermissionMap = getChild<LLButton>("permission_map_icon");
    mIconPermissionEditMine = getChild<LLButton>("permission_edit_mine_icon");

    mIconPermissionOnline->setClickedCallback(boost::bind(&LLAvatarListItem::onPermissionBtnToggle, this, (S32)LLRelationship::GRANT_ONLINE_STATUS));
    mIconPermissionMap->setClickedCallback(boost::bind(&LLAvatarListItem::onPermissionBtnToggle, this, (S32)LLRelationship::GRANT_MAP_LOCATION));
    mIconPermissionEditMine->setClickedCallback(boost::bind(&LLAvatarListItem::onPermissionBtnToggle, this, (S32)LLRelationship::GRANT_MODIFY_OBJECTS));

    mIconPermissionEditTheirs = getChild<LLIconCtrl>("permission_edit_theirs_icon");
    mIconPermissionMapTheirs = getChild<LLIconCtrl>("permission_map_theirs_icon");
    mIconPermissionOnlineTheirs = getChild<LLIconCtrl>("permission_online_theirs_icon");

    mIconPermissionOnline->setVisible(false);
    mIconPermissionMap->setVisible(false);
    mIconPermissionEditMine->setVisible(false);
    mIconPermissionEditTheirs->setVisible(false);
    mIconPermissionOnlineTheirs->setVisible(false);
    mIconPermissionMapTheirs->setVisible(false);

    mIconHovered = getChild<LLIconCtrl>("hovered_icon");

    mSpeakingIndicator = getChild<LLOutputMonitorCtrl>("speaking_indicator");
    mSpeakingIndicator->setChannelState(LLOutputMonitorCtrl::UNDEFINED_CHANNEL);
    mInfoBtn = getChild<LLButton>("info_btn");
    mProfileBtn = getChild<LLButton>("profile_btn");

    mInfoBtn->setVisible(false);
    mInfoBtn->setClickedCallback(boost::bind(&LLAvatarListItem::onInfoBtnClick, this));

    mProfileBtn->setVisible(false);
    mProfileBtn->setClickedCallback(boost::bind(&LLAvatarListItem::onProfileBtnClick, this));

    if (!sStaticInitialized)
    {
        // Remember children widths including their padding from the next sibling,
        // so that we can hide and show them again later.
        initChildrenWidths(this);

        // Right padding between avatar name text box and nearest visible child.
        sNameRightPadding = LLUICtrlFactory::getDefaultParams<LLAvatarListItem>().name_right_pad;

        sStaticInitialized = true;
    }

    mIconPermissionOnline->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionMap->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionEditMine->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionEditTheirs->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionOnlineTheirs->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionMapTheirs->setEnabled(SP_NEVER != mShowPermissions);

    return TRUE;
}

void LLAvatarListItem::handleVisibilityChange ( BOOL new_visibility )
{
    //Adjust positions of icons (info button etc) when
    //speaking indicator visibility was changed/toggled while panel was closed (not visible)
    if(new_visibility && mSpeakingIndicator->getIndicatorToggled())
    {
        updateChildren();
        mSpeakingIndicator->setIndicatorToggled(false);
    }
}

void LLAvatarListItem::fetchAvatarName()
{
    if (mAvatarId.notNull())
    {
        if (mAvatarNameCacheConnection.connected())
        {
            mAvatarNameCacheConnection.disconnect();
        }
        mAvatarNameCacheConnection = LLAvatarNameCache::get(getAvatarId(), boost::bind(&LLAvatarListItem::onAvatarNameCache, this, _2));
    }
}

S32 LLAvatarListItem::notifyParent(const LLSD& info)
{
    if (info.has("visibility_changed"))
    {
        updateChildren();
        return 1;
    }
    return LLPanel::notifyParent(info);
}

void LLAvatarListItem::onMouseEnter(S32 x, S32 y, MASK mask)
{
    mIconHovered->setVisible(true);
//  mInfoBtn->setVisible(mShowInfoBtn);
//  mProfileBtn->setVisible(mShowProfileBtn);
// [RLVa:KB] - Checked: RLVa-1.2.0
    bool fRlvCanShowName = (!mRlvCheckShowNames) || (RlvActions::canShowName(RlvActions::SNC_DEFAULT, mAvatarId));
    mInfoBtn->setVisible( (mShowInfoBtn) && (fRlvCanShowName) );
    mProfileBtn->setVisible( (mShowProfileBtn) && (fRlvCanShowName) );
// [/RLVa:KB]

    mHovered = true;
    LLPanel::onMouseEnter(x, y, mask);

    refreshPermissions();
    updateChildren();
}

void LLAvatarListItem::onMouseLeave(S32 x, S32 y, MASK mask)
{
    mIconHovered->setVisible( false);
    mInfoBtn->setVisible(false);
    mProfileBtn->setVisible(false);

    mHovered = false;
    LLPanel::onMouseLeave(x, y, mask);

    refreshPermissions();
    updateChildren();
}

// virtual, called by LLAvatarTracker
void LLAvatarListItem::changed(U32 mask)
{
    // no need to check mAvatarId for null in this case
    setOnline(LLAvatarTracker::instance().isBuddyOnline(mAvatarId));

    if (mask & LLFriendObserver::POWERS)
    {
        refreshPermissions();
        updateChildren();
    }
}

void LLAvatarListItem::setOnline(bool online)
{
    // *FIX: setName() overrides font style set by setOnline(). Not an issue ATM.

    if (mOnlineStatus != E_UNKNOWN && (bool) mOnlineStatus == online)
        return;

    mOnlineStatus = (EOnlineStatus) online;

    // Change avatar name font style depending on the new online status.
    setState(online ? IS_ONLINE : IS_OFFLINE);
}

void LLAvatarListItem::setAvatarName(const std::string& name)
{
    setNameInternal(name, mHighlihtSubstring);
}

void LLAvatarListItem::setAvatarToolTip(const std::string& tooltip)
{
    mAvatarName->setToolTip(tooltip);
}

void LLAvatarListItem::setHighlight(const std::string& highlight)
{
    setNameInternal(mAvatarName->getText(), mHighlihtSubstring = highlight);
}

void LLAvatarListItem::setState(EItemState item_style)
{
    const LLAvatarListItem::Params& params = LLUICtrlFactory::getDefaultParams<LLAvatarListItem>();

    switch(item_style)
    {
    default:
    case IS_DEFAULT:
        mAvatarNameStyle = params.default_style();
        break;
    case IS_VOICE_INVITED:
        mAvatarNameStyle = params.voice_call_invited_style();
        break;
    case IS_VOICE_JOINED:
        mAvatarNameStyle = params.voice_call_joined_style();
        break;
    case IS_VOICE_LEFT:
        mAvatarNameStyle = params.voice_call_left_style();
        break;
    case IS_ONLINE:
        mAvatarNameStyle = params.online_style();
        break;
    case IS_OFFLINE:
        mAvatarNameStyle = params.offline_style();
        break;
    }

    if (mColorize)
    {
        mAvatarNameStyle.color = ALAvatarGroups::instance().getAvatarColor(mAvatarId, mAvatarNameStyle.color.getValue(), ALAvatarGroups::COLOR_NEARBY);
    }

    // *NOTE: You cannot set the style on a text box anymore, you must
    // rebuild the text.  This will cause problems if the text contains
    // hyperlinks, as their styles will be wrong.
    setNameInternal(mAvatarName->getText(), mHighlihtSubstring);

    icon_color_map_t& item_icon_color_map = getItemIconColorMap();
    mAvatarIcon->setColor(item_icon_color_map[item_style]);
}

void LLAvatarListItem::setAvatarId(const LLUUID& id, const LLUUID& session_id, bool ignore_status_changes/* = false*/, bool is_resident/* = true*/, bool use_colorizer/* = false*/)
{
    if (mAvatarId.notNull())
        LLAvatarTracker::instance().removeParticularFriendObserver(mAvatarId, this);

    mAvatarId = id;
    mColorize = use_colorizer;
    mSpeakingIndicator->setSpeakerId(id, session_id);

    // We'll be notified on avatar online status changes
    if (!ignore_status_changes && mAvatarId.notNull())
        LLAvatarTracker::instance().addParticularFriendObserver(mAvatarId, this);

    if (is_resident)
    {
        mAvatarIcon->setValue(id);

        // Set avatar name.
        fetchAvatarName();
    }
}

void LLAvatarListItem::setShowPermissions(EShowPermissionType spType)
{
    mShowPermissions = spType;

    // Reenable the controls for updateChildren()
    mIconPermissionOnline->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionMap->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionEditMine->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionEditTheirs->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionOnlineTheirs->setEnabled(SP_NEVER != mShowPermissions);
    mIconPermissionMapTheirs->setEnabled(SP_NEVER != mShowPermissions);

    refreshPermissions();
    updateChildren();
}

void LLAvatarListItem::showTextField(bool show)
{
    mTextField->setVisible(show);
    updateChildren();
}

void LLAvatarListItem::setTextField(const std::string& text)
{
    mTextField->setValue(text);
}

void LLAvatarListItem::setTextFieldDistance(F32 distance)
{
    if (distance == 0)
        mTextField->setValue(LLStringUtil::null);
    else
        mTextField->setValue(fmt::format(FMT_STRING("{:0.1f}m"), distance));
}

void LLAvatarListItem::setTextFieldSeconds(U32 secs_since)
{
    mTextField->setValue(formatSeconds(secs_since));
}

void LLAvatarListItem::setShowInfoBtn(bool show)
{
    mShowInfoBtn = show;
}

void LLAvatarListItem::setShowProfileBtn(bool show)
{
    mShowProfileBtn = show;
}

void LLAvatarListItem::showSpeakingIndicator(bool visible)
{
    // Already done? Then do nothing.
    if (mSpeakingIndicator->getVisible() == (BOOL)visible)
        return;
// Disabled to not contradict with SpeakingIndicatorManager functionality. EXT-3976
// probably this method should be totally removed.
//  mSpeakingIndicator->setVisible(visible);
//  updateChildren();
}

void LLAvatarListItem::setAvatarIconVisible(bool visible)
{
    // Already done? Then do nothing.
    if (mAvatarIcon->getVisible() == (BOOL)visible)
    {
        return;
    }

    // Show/hide avatar icon.
    mAvatarIcon->setVisible(visible);
    updateChildren();
}

void LLAvatarListItem::onInfoBtnClick()
{
    LLFloaterReg::showInstance("inspect_avatar", LLSD().with("avatar_id", mAvatarId));
}

void LLAvatarListItem::onProfileBtnClick()
{
    LLAvatarActions::showProfile(mAvatarId);
}

void LLAvatarListItem::onPermissionBtnToggle(S32 toggleRight)
{
    LLRelationship* pRelationship = const_cast<LLRelationship*>(LLAvatarTracker::instance().getBuddyInfo(mAvatarId));
    if (!pRelationship)
        return;

    if (LLRelationship::GRANT_MODIFY_OBJECTS != toggleRight)
    {
        S32 rights = pRelationship->getRightsGrantedTo();
        if ( (rights & toggleRight) == toggleRight)
        {
            rights &= ~toggleRight;
            // Revoke the permission locally until we hear back from the region
            pRelationship->revokeRights(toggleRight, LLRelationship::GRANT_NONE);
        }
        else
        {
            rights |= toggleRight;
            // Grant the permission locally until we hear back from the region
            pRelationship->grantRights(toggleRight, LLRelationship::GRANT_NONE);
        }

        LLAvatarPropertiesProcessor::getInstance()->sendFriendRights(mAvatarId, rights);
        refreshPermissions();
        updateChildren();
    }
    else
    {
        LLSD args;
        args["NAME"] = LLSLURL("agent", mAvatarId, "fullname").getSLURLString();

        if (!pRelationship->isRightGrantedTo(LLRelationship::GRANT_MODIFY_OBJECTS))
        {
            LLNotificationsUtil::add("GrantModifyRights", args, LLSD(),
                boost::bind(&LLAvatarListItem::onModifyRightsConfirmationCallback, this, _1, _2, true));
        }
        else
        {
            LLNotificationsUtil::add("RevokeModifyRights", args, LLSD(),
                boost::bind(&LLAvatarListItem::onModifyRightsConfirmationCallback, this, _1, _2, false));
        }
    }
}

void LLAvatarListItem::onModifyRightsConfirmationCallback(const LLSD& notification, const LLSD& response, bool fGrant)
{
    S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
    if (option == 0)
    {
        LLRelationship* pRelationship = const_cast<LLRelationship*>(LLAvatarTracker::instance().getBuddyInfo(mAvatarId));
        if (!pRelationship)
            return;

        S32 rights = pRelationship->getRightsGrantedTo();
        if (!fGrant)
        {
            rights &= ~LLRelationship::GRANT_MODIFY_OBJECTS;
            // Revoke the permission locally until we hear back from the region
            pRelationship->revokeRights(LLRelationship::GRANT_MODIFY_OBJECTS, LLRelationship::GRANT_NONE);
        }
        else
        {
            rights |= LLRelationship::GRANT_MODIFY_OBJECTS;
            // Grant the permission locally until we hear back from the region
            pRelationship->grantRights(LLRelationship::GRANT_MODIFY_OBJECTS, LLRelationship::GRANT_NONE);
        }

        LLAvatarPropertiesProcessor::getInstance()->sendFriendRights(mAvatarId, rights);
        refreshPermissions();
        updateChildren();
    }
}

BOOL LLAvatarListItem::handleDoubleClick(S32 x, S32 y, MASK mask)
{
//  if(mInfoBtn->getRect().pointInRect(x, y))
// [RVLa:KB] - Checked: RLVa-1.2.2
    if ( (mInfoBtn->getVisible()) && (mInfoBtn->getEnabled()) && (mInfoBtn->getRect().pointInRect(x, y)) )
// [/SL:KB]
    {
        onInfoBtnClick();
        return TRUE;
    }
//  if(mProfileBtn->getRect().pointInRect(x, y))
// [RLVa:KB] - Checked: RLVa-1.2.2
    if ( (mProfileBtn->getVisible()) && (mProfileBtn->getEnabled()) && (mProfileBtn->getRect().pointInRect(x, y)) )
// [/SL:KB]
    {
        onProfileBtnClick();
        return TRUE;
    }
    return LLPanel::handleDoubleClick(x, y, mask);
}

void LLAvatarListItem::setValue( const LLSD& value )
{
    if (!value.isMap()) return;;
    if (!value.has("selected")) return;
    getChildView("selected_icon")->setVisible( value["selected"]);
}

const LLUUID& LLAvatarListItem::getAvatarId() const
{
    return mAvatarId;
}

std::string LLAvatarListItem::getAvatarName() const
{
    return mAvatarName->getValue();
}

std::string LLAvatarListItem::getAvatarToolTip() const
{
    return mAvatarName->getToolTip();
}

void LLAvatarListItem::updateAvatarName()
{
    fetchAvatarName();
}

//== PRIVATE SECITON ==========================================================

void LLAvatarListItem::setNameInternal(const std::string& name, const std::string& highlight)
{
    if(mShowCompleteName && highlight.empty())
    {
        LLTextUtil::textboxSetGreyedVal(mAvatarName, mAvatarNameStyle, name, mGreyOutUsername);
    }
    else
    {
        LLTextUtil::textboxSetHighlightedVal(mAvatarName, mAvatarNameStyle, name, highlight);
    }
}

void LLAvatarListItem::onAvatarNameCache(const LLAvatarName& av_name)
{
    mAvatarNameCacheConnection.disconnect();

    mGreyOutUsername.clear();
    std::string name_string = mShowCompleteName? av_name.getCompleteName(false) : av_name.getDisplayName();
    if(av_name.getCompleteName() != av_name.getUserName())
    {
        mGreyOutUsername = "[ " + av_name.getUserName(true) + " ]";
        LLStringUtil::toLower(mGreyOutUsername);
    }
//  setAvatarName(name_string);
//  setAvatarToolTip(av_name.getUserName());
// [RLVa:KB] - Checked: RLVa-1.2.2
    bool fRlvCanShowName = (!mRlvCheckShowNames) || (RlvActions::canShowName(RlvActions::SNC_DEFAULT, mAvatarId));

    setAvatarName( (fRlvCanShowName) ?  name_string : RlvStrings::getAnonym(av_name) );
    setAvatarToolTip( (fRlvCanShowName) ? av_name.getUserName() : RlvStrings::getAnonym(av_name) );
    // TODO-RLVa: bit of a hack putting this here. Maybe find a better way?
    mAvatarIcon->setDrawTooltip(fRlvCanShowName);
// [/RLVa:KB]

    //requesting the list to resort
    notifyParent(LLSD().with("sort", LLSD()));
}

// Convert given number of seconds to a string like "23 minutes", "15 hours" or "3 years",
// taking i18n into account. The format string to use is taken from the panel XML.
std::string LLAvatarListItem::formatSeconds(U32 secs)
{
    static const U32 LL_ALI_MIN     = 60;
    static const U32 LL_ALI_HOUR    = LL_ALI_MIN    * 60;
    static const U32 LL_ALI_DAY     = LL_ALI_HOUR   * 24;
    static const U32 LL_ALI_WEEK    = LL_ALI_DAY    * 7;
    static const U32 LL_ALI_MONTH   = LL_ALI_DAY    * 30;
    static const U32 LL_ALI_YEAR    = LL_ALI_DAY    * 365;

    std::string fmt;
    U32 count = 0;

    if (secs >= LL_ALI_YEAR)
    {
        fmt = "FormatYears"; count = secs / LL_ALI_YEAR;
    }
    else if (secs >= LL_ALI_MONTH)
    {
        fmt = "FormatMonths"; count = secs / LL_ALI_MONTH;
    }
    else if (secs >= LL_ALI_WEEK)
    {
        fmt = "FormatWeeks"; count = secs / LL_ALI_WEEK;
    }
    else if (secs >= LL_ALI_DAY)
    {
        fmt = "FormatDays"; count = secs / LL_ALI_DAY;
    }
    else if (secs >= LL_ALI_HOUR)
    {
        fmt = "FormatHours"; count = secs / LL_ALI_HOUR;
    }
    else if (secs >= LL_ALI_MIN)
    {
        fmt = "FormatMinutes"; count = secs / LL_ALI_MIN;
    }
    else
    {
        fmt = "FormatSeconds"; count = secs;
    }

    LLStringUtil::format_map_t args;
    args["[COUNT]"] = llformat("%u", count);
    return getString(fmt, args);
}

// static
LLAvatarListItem::icon_color_map_t& LLAvatarListItem::getItemIconColorMap()
{
    static icon_color_map_t item_icon_color_map;
    if (!item_icon_color_map.empty()) return item_icon_color_map;

    item_icon_color_map.insert(
        std::make_pair(IS_DEFAULT,
        LLUIColorTable::instance().getColor("AvatarListItemIconDefaultColor", LLColor4::white)));

    item_icon_color_map.insert(
        std::make_pair(IS_VOICE_INVITED,
        LLUIColorTable::instance().getColor("AvatarListItemIconVoiceInvitedColor", LLColor4::white)));

    item_icon_color_map.insert(
        std::make_pair(IS_VOICE_JOINED,
        LLUIColorTable::instance().getColor("AvatarListItemIconVoiceJoinedColor", LLColor4::white)));

    item_icon_color_map.insert(
        std::make_pair(IS_VOICE_LEFT,
        LLUIColorTable::instance().getColor("AvatarListItemIconVoiceLeftColor", LLColor4::white)));

    item_icon_color_map.insert(
        std::make_pair(IS_ONLINE,
        LLUIColorTable::instance().getColor("AvatarListItemIconOnlineColor", LLColor4::white)));

    item_icon_color_map.insert(
        std::make_pair(IS_OFFLINE,
        LLUIColorTable::instance().getColor("AvatarListItemIconOfflineColor", LLColor4::white)));

    return item_icon_color_map;
}

// static
void LLAvatarListItem::initChildrenWidths(LLAvatarListItem* avatar_item)
{
    //speaking indicator width + padding
    S32 speaking_indicator_width = avatar_item->getRect().getWidth() - avatar_item->mSpeakingIndicator->getRect().mLeft;

    // Text field textbox width + padding
    S32 text_field_width = avatar_item->mSpeakingIndicator->getRect().mLeft - avatar_item->mTextField->getRect().mLeft;

    //profile btn width + padding
    S32 profile_btn_width = avatar_item->mTextField->getRect().mLeft - avatar_item->mProfileBtn->getRect().mLeft;

    //info btn width + padding
    S32 info_btn_width = avatar_item->mProfileBtn->getRect().mLeft - avatar_item->mInfoBtn->getRect().mLeft;

    // online permission icon width + padding
    S32 permission_online_width = avatar_item->mInfoBtn->getRect().mLeft - avatar_item->mIconPermissionOnline->getRect().mLeft;

    // map permission icon width + padding
    S32 permission_map_width = avatar_item->mIconPermissionOnline->getRect().mLeft - avatar_item->mIconPermissionMap->getRect().mLeft;

    // edit my objects permission icon width + padding
    S32 permission_edit_mine_width = avatar_item->mIconPermissionMap->getRect().mLeft - avatar_item->mIconPermissionEditMine->getRect().mLeft;

    // edit their objects permission icon width + padding
    S32 permission_edit_theirs_width = avatar_item->mIconPermissionEditMine->getRect().mLeft - avatar_item->mIconPermissionEditTheirs->getRect().mLeft;

    S32 permission_map_theirs_width = avatar_item->mIconPermissionEditTheirs->getRect().mLeft - avatar_item->mIconPermissionMapTheirs->getRect().mLeft;
    S32 permission_online_theirs_width = avatar_item->mIconPermissionMapTheirs->getRect().mLeft - avatar_item->mIconPermissionOnlineTheirs->getRect().mLeft;


    // avatar icon width + padding
    S32 icon_width = avatar_item->mAvatarName->getRect().mLeft - avatar_item->mAvatarIcon->getRect().mLeft;

    sLeftPadding = avatar_item->mAvatarIcon->getRect().mLeft;

    S32 index = ALIC_COUNT;
    sChildrenWidths[--index] = icon_width;
    sChildrenWidths[--index] = 0; // for avatar name we don't need its width, it will be calculated as "left available space"
    sChildrenWidths[--index] = permission_online_theirs_width;
    sChildrenWidths[--index] = permission_map_theirs_width;
    sChildrenWidths[--index] = permission_edit_theirs_width;
    sChildrenWidths[--index] = permission_edit_mine_width;
    sChildrenWidths[--index] = permission_map_width;
    sChildrenWidths[--index] = permission_online_width;
    sChildrenWidths[--index] = info_btn_width;
    sChildrenWidths[--index] = profile_btn_width;
    sChildrenWidths[--index] = text_field_width;
    sChildrenWidths[--index] = speaking_indicator_width;
    llassert(index == 0);
}

void LLAvatarListItem::updateChildren()
{
    LL_DEBUGS("AvatarItemReshape") << LL_ENDL;
    LL_DEBUGS("AvatarItemReshape") << "Updating for: " << getAvatarName() << LL_ENDL;

    S32 name_new_width = getRect().getWidth();
    S32 ctrl_new_left = name_new_width;
    S32 name_new_left = sLeftPadding;

    // iterate through all children and set them into correct position depend on each child visibility
    // assume that child indexes are in back order: the first in Enum is the last (right) in the item
    // iterate & set child views starting from right to left
    for (S32 i = 0; i < ALIC_COUNT; ++i)
    {
        // skip "name" textbox, it will be processed out of loop later
        if (ALIC_NAME == i) continue;

        LLView* control = getItemChildView((EAvatarListItemChildIndex)i);

        LL_DEBUGS("AvatarItemReshape") << "Processing control: " << control->getName() << LL_ENDL;
        // skip invisible views
        if (!control->getVisible()) continue;

        S32 ctrl_width = sChildrenWidths[i]; // including space between current & left controls

        // decrease available for
        name_new_width -= ctrl_width;
        LL_DEBUGS("AvatarItemReshape") << "width: " << ctrl_width << ", name_new_width: " << name_new_width << LL_ENDL;

        LLRect control_rect = control->getRect();
        LL_DEBUGS("AvatarItemReshape") << "rect before: " << control_rect << LL_ENDL;

        if (ALIC_ICON == i)
        {
            // assume that this is the last iteration,
            // so it is not necessary to save "ctrl_new_left" value calculated on previous iterations
            ctrl_new_left = sLeftPadding;
            name_new_left = ctrl_new_left + ctrl_width;
        }
        else
        {
            ctrl_new_left -= ctrl_width;
        }

        LL_DEBUGS("AvatarItemReshape") << "ctrl_new_left: " << ctrl_new_left << LL_ENDL;

        control_rect.setLeftTopAndSize(
            ctrl_new_left,
            control_rect.mTop,
            control_rect.getWidth(),
            control_rect.getHeight());

        LL_DEBUGS("AvatarItemReshape") << "rect after: " << control_rect << LL_ENDL;
        control->setShape(control_rect);
    }

    // set size and position of the "name" child
    LLView* name_view = getItemChildView(ALIC_NAME);
    LLRect name_view_rect = name_view->getRect();
    LL_DEBUGS("AvatarItemReshape") << "name rect before: " << name_view_rect << LL_ENDL;

    // apply paddings
    name_new_width -= sLeftPadding;
    name_new_width -= sNameRightPadding;

    name_view_rect.setLeftTopAndSize(
        name_new_left,
        name_view_rect.mTop,
        name_new_width,
        name_view_rect.getHeight());

    name_view->setShape(name_view_rect);

    LL_DEBUGS("AvatarItemReshape") << "name rect after: " << name_view_rect << LL_ENDL;
}

bool LLAvatarListItem::refreshPermissions()
{
    static const std::string strUngrantedOverlay = "Permission_Ungranted";

    const LLRelationship* relation = LLAvatarTracker::instance().getBuddyInfo(getAvatarId());

    if( (relation) && (((SP_HOVER == mShowPermissions) && (mHovered)) || (SP_NONDEFAULT == mShowPermissions)) )
    {
        bool fGrantedOnline = relation->isRightGrantedTo(LLRelationship::GRANT_ONLINE_STATUS);
        mIconPermissionOnline->setVisible( (!fGrantedOnline) || (mHovered) );
        mIconPermissionOnline->setImageOverlay( (fGrantedOnline) ? "" : strUngrantedOverlay);

        bool fGrantedMap = relation->isRightGrantedTo(LLRelationship::GRANT_MAP_LOCATION);
        mIconPermissionMap->setVisible( (fGrantedMap) || (mHovered) );
        mIconPermissionMap->setImageOverlay( (fGrantedMap) ? "" : strUngrantedOverlay);

        bool fGrantedEditMine = relation->isRightGrantedTo(LLRelationship::GRANT_MODIFY_OBJECTS);
        mIconPermissionEditMine->setVisible( (fGrantedEditMine) || (mHovered) );
        mIconPermissionEditMine->setImageOverlay( (fGrantedEditMine) ? "" : strUngrantedOverlay);

        mIconPermissionEditTheirs->setVisible(relation->isRightGrantedFrom(LLRelationship::GRANT_MODIFY_OBJECTS));
        mIconPermissionOnlineTheirs->setVisible(relation->isRightGrantedFrom(LLRelationship::GRANT_ONLINE_STATUS));
        mIconPermissionMapTheirs->setVisible(relation->isRightGrantedFrom(LLRelationship::GRANT_MAP_LOCATION));
    }
    else
    {
        mIconPermissionOnline->setVisible(false);
        mIconPermissionMap->setVisible(false);
        mIconPermissionEditMine->setVisible(false);
        mIconPermissionEditTheirs->setVisible(false);
        mIconPermissionOnlineTheirs->setVisible(false);
        mIconPermissionMapTheirs->setVisible(false);
    }

    return nullptr != relation;
}

LLView* LLAvatarListItem::getItemChildView(EAvatarListItemChildIndex child_view_index)
{
    LLView* child_view = mAvatarName;

    switch (child_view_index)
    {
    case ALIC_ICON:
        child_view = mAvatarIcon;
        break;
    case ALIC_NAME:
        child_view = mAvatarName;
        break;
    case ALIC_TEXT_FIELD:
        child_view = mTextField;
        break;
    case ALIC_SPEAKER_INDICATOR:
        child_view = mSpeakingIndicator;
        break;
    case ALIC_PERMISSION_ONLINE:
        child_view = mIconPermissionOnline;
        break;
    case ALIC_PERMISSION_MAP:
        child_view = mIconPermissionMap;
        break;
    case ALIC_PERMISSION_EDIT_MINE:
        child_view = mIconPermissionEditMine;
        break;
    case ALIC_PERMISSION_EDIT_THEIRS:
        child_view = mIconPermissionEditTheirs;
        break;
    case ALIC_PERMISSION_MAP_THEIRS:
        child_view = mIconPermissionMapTheirs;
        break;
    case ALIC_PERMISSION_ONLINE_THEIRS:
        child_view = mIconPermissionOnlineTheirs;
        break;
    case ALIC_INFO_BUTTON:
        child_view = mInfoBtn;
        break;
    case ALIC_PROFILE_BUTTON:
        child_view = mProfileBtn;
        break;
    default:
        LL_WARNS("AvatarItemReshape") << "Unexpected child view index is passed: " << child_view_index << LL_ENDL;
        // leave child_view untouched
    }

    return child_view;
}

// EOF
