/** 
 * @file llpreviewgesture.cpp
 * @brief Editing UI for inventory-based gestures.
 *
 * Copyright (c) 2004-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

#include "llviewerprecompiledheaders.h"

#include <algorithm>

#include "llpreviewgesture.h"

// libraries
#include "lldatapacker.h"
#include "lldarray.h"
#include "llstring.h"
#include "lldir.h"
#include "llmultigesture.h"
#include "llvfile.h"

// newview
#include "llagent.h"		// todo: remove
#include "llassetuploadresponders.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llfloatergesture.h" // for some label constants
#include "llgesturemgr.h"
#include "llinventorymodel.h"
#include "llkeyboard.h"
#include "lllineeditor.h"
#include "llnotify.h"
#include "llradiogroup.h"
#include "llscrolllistctrl.h"
#include "lltextbox.h"
#include "llvieweruictrlfactory.h"
#include "llviewerinventory.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"		// busycount
#include "viewer.h"			// gVFS

#include "llresmgr.h"

const char NONE_LABEL[] = "---";
const char SHIFT_LABEL[] = "Shift";
const char CTRL_LABEL[] = "Ctrl";

void dialog_refresh_all();

// used for getting

class LLInventoryGestureAvailable : public LLInventoryCompletionObserver
{
public:
	LLInventoryGestureAvailable() {}

protected:
	virtual void done();
};

void LLInventoryGestureAvailable::done()
{
	LLPreview* preview = NULL;
	item_ref_t::iterator it = mComplete.begin();
	item_ref_t::iterator end = mComplete.end();
	for(; it < end; ++it)
	{
		preview = LLPreview::find((*it));
		if(preview)
		{
			preview->refresh();
		}
	}
	gInventory.removeObserver(this);
	delete this;
}

// Used for sorting
struct SortItemPtrsByName
{
	bool operator()(const LLInventoryItem* i1, const LLInventoryItem* i2)
	{
		return (LLString::compareDict(i1->getName(), i2->getName()) < 0);
	}
};

// static
LLPreviewGesture* LLPreviewGesture::show(const std::string& title, const LLUUID& item_id, const LLUUID& object_id, BOOL take_focus)
{
	LLPreviewGesture* previewp = (LLPreviewGesture*)LLPreview::find(item_id);
	if (previewp)
	{
		previewp->open();   /*Flawfinder: ignore*/
		if (take_focus)
		{
			previewp->setFocus(TRUE);
		}
		return previewp;
	}

	LLPreviewGesture* self = new LLPreviewGesture();

	// Finish internal construction
	self->init(item_id, object_id);

	// Builds and adds to gFloaterView
	gUICtrlFactory->buildFloater(self, "floater_preview_gesture.xml");
	self->setTitle(title);

	// Move window to top-left of screen
	LLMultiFloater* hostp = self->getHost();
	if (hostp == NULL)
	{
		LLRect r = self->getRect();
		LLRect screen = gFloaterView->getRect();
		r.setLeftTopAndSize(0, screen.getHeight(), r.getWidth(), r.getHeight());
		self->setRect(r);
	}
	else
	{
		// re-add to host to update title
		hostp->addFloater(self, TRUE);
	}

	// this will call refresh when we have everything.
	LLViewerInventoryItem* item = (LLViewerInventoryItem*)self->getItem();
	if(item && !item->isComplete())
	{
		LLInventoryGestureAvailable* observer;
		observer = new LLInventoryGestureAvailable();
		observer->watchItem(item_id);
		gInventory.addObserver(observer);
		item->fetchFromServer();
	}
	else
	{
		// not sure this is necessary.
		self->refresh();
	}

	if (take_focus)
	{
		self->setFocus(TRUE);
	}

	return self;
}


// virtual
BOOL LLPreviewGesture::handleKeyHere(KEY key, MASK mask,
									  BOOL called_from_parent)
{
	if(getVisible() && getEnabled())
	{
		if(('S' == key) && (MASK_CONTROL == (mask & MASK_CONTROL)))
		{
			saveIfNeeded();
			return TRUE;
		}
	}
	return LLPreview::handleKeyHere(key, mask, called_from_parent);
}


// virtual
BOOL LLPreviewGesture::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
										 EDragAndDropType cargo_type,
										 void* cargo_data,
										 EAcceptance* accept,
										 LLString& tooltip_msg)
{
	BOOL handled = TRUE;
	switch(cargo_type)
	{
	case DAD_ANIMATION:
	case DAD_SOUND:
		{
			// TODO: Don't allow this if you can't transfer the sound/animation

			// make a script step
			LLInventoryItem* item = (LLInventoryItem*)cargo_data;
			if (item
				&& gInventory.getItem(item->getUUID()))
			{
				LLPermissions perm = item->getPermissions();
				if (!((perm.getMaskBase() & PERM_ITEM_UNRESTRICTED) == PERM_ITEM_UNRESTRICTED))
				{
					*accept = ACCEPT_NO;
					if (tooltip_msg.empty())
					{
						tooltip_msg.assign("Only animations and sounds\n"
											"with unrestricted permissions\n"
											"can be added to a gesture.");
					}
					break;
				}
				else if (drop)
				{
					LLScrollListItem* line = NULL;
					if (cargo_type == DAD_ANIMATION)
					{
						line = addStep("Animation");
						LLGestureStepAnimation* anim = (LLGestureStepAnimation*)line->getUserdata();
						anim->mAnimAssetID = item->getAssetUUID();
						anim->mAnimName = item->getName();
					}
					else if (cargo_type == DAD_SOUND)
					{
						line = addStep("Sound");
						LLGestureStepSound* sound = (LLGestureStepSound*)line->getUserdata();
						sound->mSoundAssetID = item->getAssetUUID();
						sound->mSoundName = item->getName();
					}
					updateLabel(line);
					mDirty = TRUE;
					refresh();
				}
				*accept = ACCEPT_YES_COPY_MULTI;
			}
			else
			{
				// Not in user's inventory means it was in object inventory
				*accept = ACCEPT_NO;
			}
			break;
		}
	default:
		*accept = ACCEPT_NO;
		if (tooltip_msg.empty())
		{
			tooltip_msg.assign("Only animations and sounds\n"
								"can be added to a gesture.");
		}
		break;
	}
	return handled;
}


// virtual
BOOL LLPreviewGesture::canClose()
{
	if(!mDirty)
	{
		return TRUE;
	}
	else
	{
		// Bring up view-modal dialog: Save changes? Yes, No, Cancel
		gViewerWindow->alertXml("SaveChanges",
								  handleSaveChangesDialog,
								  this );
		return FALSE;
	}
}

// virtual
void LLPreviewGesture::onClose(bool app_quitting)
{
	gGestureManager.stopGesture(mPreviewGesture);
	LLPreview::onClose(app_quitting);
}

// virtual
void LLPreviewGesture::setMinimized(BOOL minimize)
{
	if (minimize != isMinimized())
	{
		LLFloater::setMinimized(minimize);

		// We're being restored
		if (!minimize)
		{
			refresh();
		}
	}
}


// static
void LLPreviewGesture::handleSaveChangesDialog(S32 option, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;
	switch(option)
	{
	case 0:  // "Yes"
		gGestureManager.stopGesture(self->mPreviewGesture);
		self->mCloseAfterSave = TRUE;
		onClickSave(data);
		break;

	case 1:  // "No"
		gGestureManager.stopGesture(self->mPreviewGesture);
		self->mDirty = FALSE; // Force the dirty flag because user has clicked NO on confirm save dialog...
		self->close();
		break;

	case 2: // "Cancel"
	default:
		// If we were quitting, we didn't really mean it.
		app_abort_quit();
		break;
	}
}


LLPreviewGesture::LLPreviewGesture()
:	LLPreview("Gesture Preview"),
	mTriggerEditor(NULL),
	mModifierCombo(NULL),
	mKeyCombo(NULL),
	mLibraryList(NULL),
	mAddBtn(NULL),
	mUpBtn(NULL),
	mDownBtn(NULL),
	mDeleteBtn(NULL),
	mStepList(NULL),
	mOptionsText(NULL),
	mAnimationRadio(NULL),
	mAnimationCombo(NULL),
	mSoundCombo(NULL),
	mChatEditor(NULL),
	mSaveBtn(NULL),
	mPreviewBtn(NULL),
	mPreviewGesture(NULL),
	mDirty(FALSE)
{
}


LLPreviewGesture::~LLPreviewGesture()
{
	// Userdata for all steps is a LLGestureStep we need to clean up
	std::vector<LLScrollListItem*> data_list = mStepList->getAllData();
	std::vector<LLScrollListItem*>::iterator data_itor;
	for (data_itor = data_list.begin(); data_itor != data_list.end(); ++data_itor)
	{
		LLScrollListItem* item = *data_itor;
		LLGestureStep* step = (LLGestureStep*)item->getUserdata();
		delete step;
		step = NULL;
	}
}


BOOL LLPreviewGesture::postBuild()
{
	LLLineEditor* edit;
	LLComboBox* combo;
	LLButton* btn;
	LLScrollListCtrl* list;
	LLTextBox* text;
	LLCheckBoxCtrl* check;

	edit = LLViewerUICtrlFactory::getLineEditorByName(this, "trigger_editor");
	edit->setKeystrokeCallback(onKeystrokeCommit);
	edit->setCommitCallback(onCommitSetDirty);
	edit->setCommitOnFocusLost(TRUE);
	edit->setCallbackUserData(this);
	edit->setIgnoreTab(TRUE);
	mTriggerEditor = edit;

	text = LLViewerUICtrlFactory::getTextBoxByName(this, "replace_text");
	text->setEnabled(FALSE);
	mReplaceText = text;

	edit = LLViewerUICtrlFactory::getLineEditorByName(this, "replace_editor");
	edit->setEnabled(FALSE);
	edit->setKeystrokeCallback(onKeystrokeCommit);
	edit->setCommitCallback(onCommitSetDirty);
	edit->setCommitOnFocusLost(TRUE);
	edit->setCallbackUserData(this);
	edit->setIgnoreTab(TRUE);
	mReplaceEditor = edit;

	combo = LLViewerUICtrlFactory::getComboBoxByName(this, "modifier_combo");
	combo->setCommitCallback(onCommitSetDirty);
	combo->setCallbackUserData(this);
	mModifierCombo = combo;

	combo = LLViewerUICtrlFactory::getComboBoxByName(this, "key_combo");
	combo->setCommitCallback(onCommitSetDirty);
	combo->setCallbackUserData(this);
	mKeyCombo = combo;

	list = LLViewerUICtrlFactory::getScrollListByName(this, "library_list");
	list->setCommitCallback(onCommitLibrary);
	list->setDoubleClickCallback(onClickAdd);
	list->setCallbackUserData(this);
	mLibraryList = list;

	btn = LLViewerUICtrlFactory::getButtonByName(this, "add_btn");
	btn->setClickedCallback(onClickAdd);
	btn->setCallbackUserData(this);
	btn->setEnabled(FALSE);
	mAddBtn = btn;

	btn = LLViewerUICtrlFactory::getButtonByName(this, "up_btn");
	btn->setClickedCallback(onClickUp);
	btn->setCallbackUserData(this);
	btn->setEnabled(FALSE);
	mUpBtn = btn;

	btn = LLViewerUICtrlFactory::getButtonByName(this, "down_btn");
	btn->setClickedCallback(onClickDown);
	btn->setCallbackUserData(this);
	btn->setEnabled(FALSE);
	mDownBtn = btn;

	btn = LLViewerUICtrlFactory::getButtonByName(this, "delete_btn");
	btn->setClickedCallback(onClickDelete);
	btn->setCallbackUserData(this);
	btn->setEnabled(FALSE);
	mDeleteBtn = btn;

	list = LLViewerUICtrlFactory::getScrollListByName(this, "step_list");
	list->setCommitCallback(onCommitStep);
	list->setCallbackUserData(this);
	mStepList = list;

	// Options
	text = LLViewerUICtrlFactory::getTextBoxByName(this, "options_text");
	text->setBorderVisible(TRUE);
	mOptionsText = text;

	combo = LLViewerUICtrlFactory::getComboBoxByName(this, "animation_list");
	combo->setVisible(FALSE);
	combo->setCommitCallback(onCommitAnimation);
	combo->setCallbackUserData(this);
	mAnimationCombo = combo;

	LLRadioGroup* group;
	group = LLViewerUICtrlFactory::getRadioGroupByName(this, "animation_trigger_type");
	group->setVisible(FALSE);
	group->setCommitCallback(onCommitAnimationTrigger);
	group->setCallbackUserData(this);
	mAnimationRadio = group;

	combo = LLViewerUICtrlFactory::getComboBoxByName(this, "sound_list");
	combo->setVisible(FALSE);
	combo->setCommitCallback(onCommitSound);
	combo->setCallbackUserData(this);
	mSoundCombo = combo;

	edit = LLViewerUICtrlFactory::getLineEditorByName(this, "chat_editor");
	edit->setVisible(FALSE);
	edit->setCommitCallback(onCommitChat);
	//edit->setKeystrokeCallback(onKeystrokeCommit);
	edit->setCommitOnFocusLost(TRUE);
	edit->setCallbackUserData(this);
	edit->setIgnoreTab(TRUE);
	mChatEditor = edit;

	check = LLViewerUICtrlFactory::getCheckBoxByName(this, "wait_anim_check");
	check->setVisible(FALSE);
	check->setCommitCallback(onCommitWait);
	check->setCallbackUserData(this);
	mWaitAnimCheck = check;

	check = LLViewerUICtrlFactory::getCheckBoxByName(this, "wait_time_check");
	check->setVisible(FALSE);
	check->setCommitCallback(onCommitWait);
	check->setCallbackUserData(this);
	mWaitTimeCheck = check;

	edit = LLViewerUICtrlFactory::getLineEditorByName(this, "wait_time_editor");
	edit->setEnabled(FALSE);
	edit->setVisible(FALSE);
	edit->setPrevalidate(LLLineEditor::prevalidateFloat);
//	edit->setKeystrokeCallback(onKeystrokeCommit);
	edit->setCommitOnFocusLost(TRUE);
	edit->setCommitCallback(onCommitWaitTime);
	edit->setCallbackUserData(this);
	edit->setIgnoreTab(TRUE);
	mWaitTimeEditor = edit;

	// Buttons at the bottom
	check = LLViewerUICtrlFactory::getCheckBoxByName(this, "active_check");
	check->setCommitCallback(onCommitActive);
	check->setCallbackUserData(this);
	mActiveCheck = check;

	btn = LLViewerUICtrlFactory::getButtonByName(this, "save_btn");
	btn->setClickedCallback(onClickSave);
	btn->setCallbackUserData(this);
	mSaveBtn = btn;

	btn = LLViewerUICtrlFactory::getButtonByName(this, "preview_btn");
	btn->setClickedCallback(onClickPreview);
	btn->setCallbackUserData(this);
	mPreviewBtn = btn;


	// Populate the comboboxes
	addModifiers();
	addKeys();
	addAnimations();
	addSounds();


	LLInventoryItem* item = getItem();

	if (item) 
	{
		childSetCommitCallback("desc", LLPreview::onText, this);
		childSetText("desc", item->getDescription());
		childSetPrevalidate("desc", &LLLineEditor::prevalidatePrintableNotPipe);
	}

	return TRUE;
}


void LLPreviewGesture::addModifiers()
{
	LLComboBox* combo = mModifierCombo;

	combo->add( NONE_LABEL,  ADD_BOTTOM );
	combo->add( SHIFT_LABEL, ADD_BOTTOM );
	combo->add( CTRL_LABEL,  ADD_BOTTOM );
	combo->setCurrentByIndex(0);
}

void LLPreviewGesture::addKeys()
{
	LLComboBox* combo = mKeyCombo;

	combo->add( NONE_LABEL );
	for (KEY key = KEY_F2; key <= KEY_F12; key++)
	{
		combo->add( LLKeyboard::stringFromKey(key), ADD_BOTTOM );
	}
	combo->setCurrentByIndex(0);
}


// TODO: Sort the legacy and non-legacy together?
void LLPreviewGesture::addAnimations()
{
	LLComboBox* combo = mAnimationCombo;

	combo->removeall();

	combo->add("-- None --", LLUUID::null);

	// Add all the default (legacy) animations
	S32 i;
	for (i = 0; i < gUserAnimStatesCount; ++i)
	{
		// Use the user-readable name
		const char* label = gUserAnimStates[i].mLabel;
		const LLUUID& id = gUserAnimStates[i].mID;
		combo->add(label, id);
	}

	// Get all inventory items that are animations
	LLViewerInventoryCategory::cat_array_t cats;
	LLViewerInventoryItem::item_array_t items;
	LLIsTypeWithPermissions is_copyable_animation(LLAssetType::AT_ANIMATION,
													PERM_ITEM_UNRESTRICTED,
													gAgent.getID(),
													gAgent.getGroupID());
	gInventory.collectDescendentsIf(gAgent.getInventoryRootID(),
									cats,
									items,
									LLInventoryModel::EXCLUDE_TRASH,
									is_copyable_animation);

	// Copy into something we can sort
	std::vector<LLInventoryItem*> animations;

	S32 count = items.count();
	for(i = 0; i < count; ++i)
	{
		animations.push_back( items.get(i) );
	}

	// Do the sort
	std::sort(animations.begin(), animations.end(), SortItemPtrsByName());

	// And load up the combobox
	std::vector<LLInventoryItem*>::iterator it;
	for (it = animations.begin(); it != animations.end(); ++it)
	{
		LLInventoryItem* item = *it;

		combo->add(item->getName(), item->getAssetUUID(), ADD_BOTTOM);
	}
}


void LLPreviewGesture::addSounds()
{
	// Get all inventory items that are sounds
	LLViewerInventoryCategory::cat_array_t cats;
	LLViewerInventoryItem::item_array_t items;
	LLIsTypeWithPermissions is_copyable_sound(LLAssetType::AT_SOUND,
													PERM_ITEM_UNRESTRICTED,
													gAgent.getID(),
													gAgent.getGroupID());
	gInventory.collectDescendentsIf(gAgent.getInventoryRootID(),
									cats,
									items,
									LLInventoryModel::EXCLUDE_TRASH,
									is_copyable_sound);

	// Copy sounds into something we can sort
	std::vector<LLInventoryItem*> sounds;

	S32 i;
	S32 count = items.count();
	for(i = 0; i < count; ++i)
	{
		sounds.push_back( items.get(i) );
	}

	// Do the sort
	std::sort(sounds.begin(), sounds.end(), SortItemPtrsByName());

	// And load up the combobox
	LLComboBox* combo = mSoundCombo;
	combo->removeall();
	std::vector<LLInventoryItem*>::iterator it;
	for (it = sounds.begin(); it != sounds.end(); ++it)
	{
		LLInventoryItem* item = *it;

		combo->add(item->getName(), item->getAssetUUID(), ADD_BOTTOM);
	}
}


void LLPreviewGesture::init(const LLUUID& item_id, const LLUUID& object_id)
{
	// Sets ID and adds to instance list
	setItemID(item_id);
	setObjectID(object_id);
}


void LLPreviewGesture::refresh()
{
	// If previewing or item is incomplete, all controls are disabled
	LLViewerInventoryItem* item = (LLViewerInventoryItem*)getItem();
	bool is_complete = (item && item->isComplete()) ? true : false;
	if (mPreviewGesture || !is_complete)
	{
		
		childSetEnabled("desc", FALSE);
		//mDescEditor->setEnabled(FALSE);
		mTriggerEditor->setEnabled(FALSE);
		mReplaceText->setEnabled(FALSE);
		mReplaceEditor->setEnabled(FALSE);
		mModifierCombo->setEnabled(FALSE);
		mKeyCombo->setEnabled(FALSE);
		mLibraryList->setEnabled(FALSE);
		mAddBtn->setEnabled(FALSE);
		mUpBtn->setEnabled(FALSE);
		mDownBtn->setEnabled(FALSE);
		mDeleteBtn->setEnabled(FALSE);
		mStepList->setEnabled(FALSE);
		mOptionsText->setEnabled(FALSE);
		mAnimationCombo->setEnabled(FALSE);
		mAnimationRadio->setEnabled(FALSE);
		mSoundCombo->setEnabled(FALSE);
		mChatEditor->setEnabled(FALSE);
		mWaitAnimCheck->setEnabled(FALSE);
		mWaitTimeCheck->setEnabled(FALSE);
		mWaitTimeEditor->setEnabled(FALSE);
		mActiveCheck->setEnabled(FALSE);
		mSaveBtn->setEnabled(FALSE);

		// Make sure preview button is enabled, so we can stop it
		mPreviewBtn->setEnabled(TRUE);
		return;
	}

	BOOL modifiable = item->getPermissions().allowModifyBy(gAgent.getID());

	childSetEnabled("desc", modifiable);
	mTriggerEditor->setEnabled(TRUE);
	mLibraryList->setEnabled(modifiable);
	mStepList->setEnabled(modifiable);
	mOptionsText->setEnabled(modifiable);
	mAnimationCombo->setEnabled(modifiable);
	mAnimationRadio->setEnabled(modifiable);
	mSoundCombo->setEnabled(modifiable);
	mChatEditor->setEnabled(modifiable);
	mWaitAnimCheck->setEnabled(modifiable);
	mWaitTimeCheck->setEnabled(modifiable);
	mWaitTimeEditor->setEnabled(modifiable);
	mActiveCheck->setEnabled(TRUE);

	const std::string& trigger = mTriggerEditor->getText();
	BOOL have_trigger = !trigger.empty();

	const std::string& replace = mReplaceEditor->getText();
	BOOL have_replace = !replace.empty();

	LLScrollListItem* library_item = mLibraryList->getFirstSelected();
	BOOL have_library = (library_item != NULL);

	LLScrollListItem* step_item = mStepList->getFirstSelected();
	S32 step_index = mStepList->getFirstSelectedIndex();
	S32 step_count = mStepList->getItemCount();
	BOOL have_step = (step_item != NULL);

	mReplaceText->setEnabled(have_trigger || have_replace);
	mReplaceEditor->setEnabled(have_trigger || have_replace);

	mModifierCombo->setEnabled(TRUE);
	mKeyCombo->setEnabled(TRUE);

	mAddBtn->setEnabled(modifiable && have_library);
	mUpBtn->setEnabled(modifiable && have_step && step_index > 0);
	mDownBtn->setEnabled(modifiable && have_step && step_index < step_count-1);
	mDeleteBtn->setEnabled(modifiable && have_step);

	// Assume all not visible
	mAnimationCombo->setVisible(FALSE);
	mAnimationRadio->setVisible(FALSE);
	mSoundCombo->setVisible(FALSE);
	mChatEditor->setVisible(FALSE);
	mWaitAnimCheck->setVisible(FALSE);
	mWaitTimeCheck->setVisible(FALSE);
	mWaitTimeEditor->setVisible(FALSE);

	if (have_step)
	{
		// figure out the type, show proper options, update text
		LLGestureStep* step = (LLGestureStep*)step_item->getUserdata();
		EStepType type = step->getType();
		switch(type)
		{
		case STEP_ANIMATION:
			{
				LLGestureStepAnimation* anim_step = (LLGestureStepAnimation*)step;
				mOptionsText->setText("Animation to play:");
				mAnimationCombo->setVisible(TRUE);
				mAnimationRadio->setVisible(TRUE);
				mAnimationRadio->setSelectedIndex((anim_step->mFlags & ANIM_FLAG_STOP) ? 1 : 0);
				mAnimationCombo->setCurrentByID(anim_step->mAnimAssetID);
				break;
			}
		case STEP_SOUND:
			{
				LLGestureStepSound* sound_step = (LLGestureStepSound*)step;
				mOptionsText->setText("Sound to play:");
				mSoundCombo->setVisible(TRUE);
				mSoundCombo->setCurrentByID(sound_step->mSoundAssetID);
				break;
			}
		case STEP_CHAT:
			{
				LLGestureStepChat* chat_step = (LLGestureStepChat*)step;
				mOptionsText->setText("Chat to say:");
				mChatEditor->setVisible(TRUE);
				mChatEditor->setText(chat_step->mChatText);
				break;
			}
		case STEP_WAIT:
			{
				LLGestureStepWait* wait_step = (LLGestureStepWait*)step;
				mOptionsText->setText("Wait:");
				mWaitAnimCheck->setVisible(TRUE);
				mWaitAnimCheck->set(wait_step->mFlags & WAIT_FLAG_ALL_ANIM);
				mWaitTimeCheck->setVisible(TRUE);
				mWaitTimeCheck->set(wait_step->mFlags & WAIT_FLAG_TIME);
				mWaitTimeEditor->setVisible(TRUE);
				char buffer[16];		/*Flawfinder: ignore*/
				snprintf(buffer, sizeof(buffer),  "%.1f", (double)wait_step->mWaitSeconds);			/* Flawfinder: ignore */
				mWaitTimeEditor->setText(buffer);
				break;
			}
		default:
			break;
		}
	}
	else
	{
		// no gesture
		mOptionsText->setText("");
	}

	BOOL active = gGestureManager.isGestureActive(mItemUUID);
	mActiveCheck->set(active);

	// Can only preview if there are steps
	mPreviewBtn->setEnabled(step_count > 0);

	// And can only save if changes have been made
	mSaveBtn->setEnabled(mDirty);
	addAnimations();
	addSounds();
}


void LLPreviewGesture::initDefaultGesture()
{
	LLScrollListItem* item;
	item = addStep("Animation");
	LLGestureStepAnimation* anim = (LLGestureStepAnimation*)item->getUserdata();
	anim->mAnimAssetID = ANIM_AGENT_HELLO;
	anim->mAnimName = "Wave";
	updateLabel(item);

	item = addStep("Wait");
	LLGestureStepWait* wait = (LLGestureStepWait*)item->getUserdata();
	wait->mFlags = WAIT_FLAG_ALL_ANIM;
	updateLabel(item);

	item = addStep("Chat");
	LLGestureStepChat* chat_step = (LLGestureStepChat*)item->getUserdata();
	chat_step->mChatText = "Hello, avatar!";
	updateLabel(item);

	// Start with item list selected
	mStepList->selectFirstItem();

	// this is *new* content, so we are dirty
	mDirty = TRUE;
}


void LLPreviewGesture::loadAsset()
{
	LLInventoryItem* item = getItem();
	if (!item) return;

	LLUUID asset_id = item->getAssetUUID();
	if (asset_id.isNull())
	{
		// Freshly created gesture, don't need to load asset.
		// Blank gesture will be fine.
		initDefaultGesture();
		refresh();
		return;
	}

	// TODO: Based on item->getPermissions().allow*
	// could enable/disable UI.

	// Copy the UUID, because the user might close the preview
	// window if the download gets stalled.
	LLUUID* item_idp = new LLUUID(mItemUUID);

	const BOOL high_priority = TRUE;
	gAssetStorage->getAssetData(asset_id,
								LLAssetType::AT_GESTURE,
								onLoadComplete,
								(void**)item_idp,
								high_priority);
	mAssetStatus = PREVIEW_ASSET_LOADING;
}


// static
void LLPreviewGesture::onLoadComplete(LLVFS *vfs,
									   const LLUUID& asset_uuid,
									   LLAssetType::EType type,
									   void* user_data, S32 status)
{
	LLUUID* item_idp = (LLUUID*)user_data;
	LLPreview* preview = LLPreview::find(*item_idp);
	if (preview)
	{
		LLPreviewGesture* self = (LLPreviewGesture*)preview;

		if (0 == status)
		{
			LLVFile file(vfs, asset_uuid, type, LLVFile::READ);
			S32 size = file.getSize();

			char* buffer = new char[size+1];
			file.read((U8*)buffer, size);		/*Flawfinder: ignore*/
			buffer[size] = '\0';

			LLMultiGesture* gesture = new LLMultiGesture();

			LLDataPackerAsciiBuffer dp(buffer, size+1);
			BOOL ok = gesture->deserialize(dp);

			if (ok)
			{
				// Everything has been successful.  Load up the UI.
				self->loadUIFromGesture(gesture);

				self->mStepList->selectFirstItem();

				self->mDirty = FALSE;
				self->refresh();
			}
			else
			{
				llwarns << "Unable to load gesture" << llendl;
			}

			delete gesture;
			gesture = NULL;

			delete [] buffer;
			buffer = NULL;

			self->mAssetStatus = PREVIEW_ASSET_LOADED;
		}
		else
		{
			if( gViewerStats )
			{
				gViewerStats->incStat( LLViewerStats::ST_DOWNLOAD_FAILED );
			}

			if( LL_ERR_ASSET_REQUEST_NOT_IN_DATABASE == status ||
				LL_ERR_FILE_EMPTY == status)
			{
				LLNotifyBox::showXml("GestureMissing");
			}
			else
			{
				LLNotifyBox::showXml("UnableToLoadGesture");
			}

			llwarns << "Problem loading gesture: " << status << llendl;
			self->mAssetStatus = PREVIEW_ASSET_ERROR;
		}
	}
	delete item_idp;
	item_idp = NULL;
}


void LLPreviewGesture::loadUIFromGesture(LLMultiGesture* gesture)
{
	/*LLInventoryItem* item = getItem();


	
	if (item)
	{
		LLLineEditor* descEditor = LLUICtrlFactory::getLineEditorByName(this, "desc");
		descEditor->setText(item->getDescription());
	}*/

	mTriggerEditor->setText(gesture->mTrigger);

	mReplaceEditor->setText(gesture->mReplaceText);

	switch (gesture->mMask)
	{
	default:
	case MASK_NONE:
		mModifierCombo->setSimple( NONE_LABEL );
		break;
	case MASK_SHIFT:
		mModifierCombo->setSimple( SHIFT_LABEL );
		break;
	case MASK_CONTROL:
		mModifierCombo->setSimple( CTRL_LABEL );
		break;
	}

	mKeyCombo->setCurrentByIndex(0);
	if (gesture->mKey != KEY_NONE)
	{
		mKeyCombo->setSimple(LLKeyboard::stringFromKey(gesture->mKey));
	}

	// Make UI steps for each gesture step
	S32 i;
	S32 count = gesture->mSteps.size();
	for (i = 0; i < count; ++i)
	{
		LLGestureStep* step = gesture->mSteps[i];

		LLGestureStep* new_step = NULL;

		switch(step->getType())
		{
		case STEP_ANIMATION:
			{
				LLGestureStepAnimation* anim_step = (LLGestureStepAnimation*)step;
				LLGestureStepAnimation* new_anim_step =
					new LLGestureStepAnimation(*anim_step);
				new_step = new_anim_step;
				break;
			}
		case STEP_SOUND:
			{
				LLGestureStepSound* sound_step = (LLGestureStepSound*)step;
				LLGestureStepSound* new_sound_step =
					new LLGestureStepSound(*sound_step);
				new_step = new_sound_step;
				break;
			}
		case STEP_CHAT:
			{
				LLGestureStepChat* chat_step = (LLGestureStepChat*)step;
				LLGestureStepChat* new_chat_step =
					new LLGestureStepChat(*chat_step);
				new_step = new_chat_step;
				break;
			}
		case STEP_WAIT:
			{
				LLGestureStepWait* wait_step = (LLGestureStepWait*)step;
				LLGestureStepWait* new_wait_step =
					new LLGestureStepWait(*wait_step);
				new_step = new_wait_step;
				break;
			}
		default:
			{
				break;
			}
		}

		if (!new_step) continue;

		// Create an enabled item with this step
		LLSD row;
		row["columns"][0]["value"] = new_step->getLabel();
		row["columns"][0]["font"] = "SANSSERIF_SMALL";
		LLScrollListItem* item = mStepList->addElement(row);
		item->setUserdata(new_step);
	}
}

// Helpful structure so we can look up the inventory item
// after the save finishes.
struct LLSaveInfo
{
	LLSaveInfo(const LLUUID& item_id, const LLUUID& object_id, const LLString& desc,
				const LLTransactionID tid)
		: mItemUUID(item_id), mObjectUUID(object_id), mDesc(desc), mTransactionID(tid)
	{
	}

	LLUUID mItemUUID;
	LLUUID mObjectUUID;
	LLString mDesc;
	LLTransactionID mTransactionID;
};


void LLPreviewGesture::saveIfNeeded()
{
	if (!gAssetStorage)
	{
		llwarns << "Can't save gesture, no asset storage system." << llendl;
		return;
	}

	if (!mDirty)
	{
		return;
	}

	// Copy the UI into a gesture
	LLMultiGesture* gesture = createGesture();

	// Serialize the gesture
	S32 max_size = gesture->getMaxSerialSize();
	char* buffer = new char[max_size];

	LLDataPackerAsciiBuffer dp(buffer, max_size);

	BOOL ok = gesture->serialize(dp);

	if (dp.getCurrentSize() > 1000)
	{
		gViewerWindow->alertXml("GestureSaveFailedTooManySteps");

		delete gesture;
		gesture = NULL;
	}
	else if (!ok)
	{
		gViewerWindow->alertXml("GestureSaveFailedTryAgain");
		delete gesture;
		gesture = NULL;
	}
	else
	{
		// Every save gets a new UUID.  Yup.
		LLTransactionID tid;
		LLAssetID asset_id;
		tid.generate();
		asset_id = tid.makeAssetID(gAgent.getSecureSessionID());

		LLVFile file(gVFS, asset_id, LLAssetType::AT_GESTURE, LLVFile::APPEND);

		S32 size = dp.getCurrentSize();
		file.setMaxSize(size);
		file.write((U8*)buffer, size);

		// Upload that asset to the database
		LLInventoryItem* item = getItem();
		if (item)
		{
			std::string agent_url = gAgent.getRegion()->getCapability("UpdateGestureAgentInventory");
			std::string task_url = gAgent.getRegion()->getCapability("UpdateGestureTaskInventory");
			if (mObjectUUID.isNull() && !agent_url.empty())
			{
				// Saving into agent inventory
				LLSD body;
				body["item_id"] = mItemUUID;
				LLHTTPClient::post(agent_url, body,
					new LLUpdateAgentInventoryResponder(body, asset_id, LLAssetType::AT_GESTURE));
			}
			else if (!mObjectUUID.isNull() && !task_url.empty())
			{
				// Saving into task inventory
				LLSD body;
				body["task_id"] = mObjectUUID;
				body["item_id"] = mItemUUID;
				LLHTTPClient::post(task_url, body,
					new LLUpdateTaskInventoryResponder(body, asset_id, LLAssetType::AT_GESTURE));
			}
			else if (gAssetStorage)
			{
				LLLineEditor* descEditor = LLUICtrlFactory::getLineEditorByName(this, "desc");
				LLSaveInfo* info = new LLSaveInfo(mItemUUID, mObjectUUID, descEditor->getText(), tid);
				gAssetStorage->storeAssetData(tid, LLAssetType::AT_GESTURE, onSaveComplete, info, FALSE);
			}
		}

		// If this gesture is active, then we need to update the in-memory
		// active map with the new pointer.
		if (gGestureManager.isGestureActive(mItemUUID))
		{
			// gesture manager now owns the pointer
			gGestureManager.replaceGesture(mItemUUID, gesture, asset_id);

			// replaceGesture may deactivate other gestures so let the
			// inventory know.
			gInventory.notifyObservers();
		}
		else
		{
			// we're done with this gesture
			delete gesture;
			gesture = NULL;
		}

		mDirty = FALSE;
		refresh();
	}

	delete [] buffer;
	buffer = NULL;
}


// TODO: This is very similar to LLPreviewNotecard::onSaveComplete.
// Could merge code.
// static
void LLPreviewGesture::onSaveComplete(const LLUUID& asset_uuid, void* user_data, S32 status) // StoreAssetData callback (fixed)
{
	LLSaveInfo* info = (LLSaveInfo*)user_data;
	if (info && (status == 0))
	{
		if(info->mObjectUUID.isNull())
		{
			// Saving into user inventory
			LLViewerInventoryItem* item;
			item = (LLViewerInventoryItem*)gInventory.getItem(info->mItemUUID);
			if(item)
			{
				LLPointer<LLViewerInventoryItem> new_item = new LLViewerInventoryItem(item);
				new_item->setDescription(info->mDesc);
				new_item->setTransactionID(info->mTransactionID);
				new_item->setAssetUUID(asset_uuid);
				new_item->updateServer(FALSE);
				gInventory.updateItem(new_item);
				gInventory.notifyObservers();
			}
			else
			{
				llwarns << "Inventory item for gesture " << info->mItemUUID
						<< " is no longer in agent inventory." << llendl
			}
		}
		else
		{
			// Saving into in-world object inventory
			LLViewerObject* object = gObjectList.findObject(info->mObjectUUID);
			LLViewerInventoryItem* item = NULL;
			if(object)
			{
				item = (LLViewerInventoryItem*)object->getInventoryObject(info->mItemUUID);
			}
			if(object && item)
			{
				item->setDescription(info->mDesc);
				item->setAssetUUID(asset_uuid);
				item->setTransactionID(info->mTransactionID);
				object->updateInventory(item, TASK_INVENTORY_ITEM_KEY, false);
				dialog_refresh_all();
			}
			else
			{
				gViewerWindow->alertXml("GestureSaveFailedObjectNotFound");
			}
		}

		// Find our window and close it if requested.
		LLPreviewGesture* previewp = (LLPreviewGesture*)LLPreview::find(info->mItemUUID);
		if (previewp && previewp->mCloseAfterSave)
		{
			previewp->close();
		}
	}
	else
	{
		llwarns << "Problem saving gesture: " << status << llendl;
		LLStringBase<char>::format_map_t args;
		args["[REASON]"] = std::string(LLAssetStorage::getErrorString(status));
		gViewerWindow->alertXml("GestureSaveFailedReason",args);
	}
	delete info;
	info = NULL;
}


LLMultiGesture* LLPreviewGesture::createGesture()
{
	LLMultiGesture* gesture = new LLMultiGesture();

	gesture->mTrigger = mTriggerEditor->getText();
	gesture->mReplaceText = mReplaceEditor->getText();

	const LLString& modifier = mModifierCombo->getSimple();
	if (modifier == CTRL_LABEL)
	{
		gesture->mMask = MASK_CONTROL;
	}
	else if (modifier == SHIFT_LABEL)
	{
		gesture->mMask = MASK_SHIFT;
	}
	else
	{
		gesture->mMask = MASK_NONE;
	}

	if (mKeyCombo->getCurrentIndex() == 0)
	{
		gesture->mKey = KEY_NONE;
	}
	else
	{
		const LLString& key_string = mKeyCombo->getSimple();
		LLKeyboard::keyFromString(key_string.c_str(), &(gesture->mKey));
	}

	std::vector<LLScrollListItem*> data_list = mStepList->getAllData();
	std::vector<LLScrollListItem*>::iterator data_itor;
	for (data_itor = data_list.begin(); data_itor != data_list.end(); ++data_itor)
	{
		LLScrollListItem* item = *data_itor;
		LLGestureStep* step = (LLGestureStep*)item->getUserdata();

		switch(step->getType())
		{
		case STEP_ANIMATION:
			{
				// Copy UI-generated step into actual gesture step
				LLGestureStepAnimation* anim_step = (LLGestureStepAnimation*)step;
				LLGestureStepAnimation* new_anim_step =
					new LLGestureStepAnimation(*anim_step);
				gesture->mSteps.push_back(new_anim_step);
				break;
			}
		case STEP_SOUND:
			{
				// Copy UI-generated step into actual gesture step
				LLGestureStepSound* sound_step = (LLGestureStepSound*)step;
				LLGestureStepSound* new_sound_step =
					new LLGestureStepSound(*sound_step);
				gesture->mSteps.push_back(new_sound_step);
				break;
			}
		case STEP_CHAT:
			{
				// Copy UI-generated step into actual gesture step
				LLGestureStepChat* chat_step = (LLGestureStepChat*)step;
				LLGestureStepChat* new_chat_step =
					new LLGestureStepChat(*chat_step);
				gesture->mSteps.push_back(new_chat_step);
				break;
			}
		case STEP_WAIT:
			{
				// Copy UI-generated step into actual gesture step
				LLGestureStepWait* wait_step = (LLGestureStepWait*)step;
				LLGestureStepWait* new_wait_step =
					new LLGestureStepWait(*wait_step);
				gesture->mSteps.push_back(new_wait_step);
				break;
			}
		default:
			{
				break;
			}
		}
	}

	return gesture;
}


// static
void LLPreviewGesture::updateLabel(LLScrollListItem* item)
{
	LLGestureStep* step = (LLGestureStep*)item->getUserdata();

	LLScrollListCell* cell = item->getColumn(0);
	LLScrollListText* text_cell = (LLScrollListText*)cell;
	std::string label = step->getLabel();
	text_cell->setText(label);
}

// static
void LLPreviewGesture::onCommitSetDirty(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;
	self->mDirty = TRUE;
	self->refresh();
}

// static
void LLPreviewGesture::onCommitLibrary(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* library_item = self->mLibraryList->getFirstSelected();
	if (library_item)
	{
		self->mStepList->deselectAllItems();
		self->refresh();
	}
}


// static
void LLPreviewGesture::onCommitStep(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* step_item = self->mStepList->getFirstSelected();
	if (!step_item) return;

	self->mLibraryList->deselectAllItems();
	self->refresh();
}


// static
void LLPreviewGesture::onCommitAnimation(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* step_item = self->mStepList->getFirstSelected();
	if (step_item)
	{
		LLGestureStep* step = (LLGestureStep*)step_item->getUserdata();
		if (step->getType() == STEP_ANIMATION)
		{
			// Assign the animation name
			LLGestureStepAnimation* anim_step = (LLGestureStepAnimation*)step;
			if (self->mAnimationCombo->getCurrentIndex() == 0)
			{
				anim_step->mAnimName.clear();
				anim_step->mAnimAssetID.setNull();
			}
			else
			{
				anim_step->mAnimName = self->mAnimationCombo->getSimple();
				anim_step->mAnimAssetID = self->mAnimationCombo->getCurrentID();
			}
			//anim_step->mFlags = 0x0;

			// Update the UI label in the list
			updateLabel(step_item);

			self->mDirty = TRUE;
			self->refresh();
		}
	}
}

// static
void LLPreviewGesture::onCommitAnimationTrigger(LLUICtrl* ctrl, void *data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* step_item = self->mStepList->getFirstSelected();
	if (step_item)
	{
		LLGestureStep* step = (LLGestureStep*)step_item->getUserdata();
		if (step->getType() == STEP_ANIMATION)
		{
			LLGestureStepAnimation* anim_step = (LLGestureStepAnimation*)step;
			if (self->mAnimationRadio->getSelectedIndex() == 0)
			{
				// start
				anim_step->mFlags &= ~ANIM_FLAG_STOP;
			}
			else
			{
				// stop
				anim_step->mFlags |= ANIM_FLAG_STOP;
			}
			// Update the UI label in the list
			updateLabel(step_item);

			self->mDirty = TRUE;
			self->refresh();
		}
	}
}

// static
void LLPreviewGesture::onCommitSound(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* step_item = self->mStepList->getFirstSelected();
	if (step_item)
	{
		LLGestureStep* step = (LLGestureStep*)step_item->getUserdata();
		if (step->getType() == STEP_SOUND)
		{
			// Assign the sound name
			LLGestureStepSound* sound_step = (LLGestureStepSound*)step;
			sound_step->mSoundName = self->mSoundCombo->getSimple();
			sound_step->mSoundAssetID = self->mSoundCombo->getCurrentID();
			sound_step->mFlags = 0x0;

			// Update the UI label in the list
			updateLabel(step_item);

			self->mDirty = TRUE;
			self->refresh();
		}
	}
}

// static
void LLPreviewGesture::onCommitChat(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* step_item = self->mStepList->getFirstSelected();
	if (!step_item) return;

	LLGestureStep* step = (LLGestureStep*)step_item->getUserdata();
	if (step->getType() != STEP_CHAT) return;

	LLGestureStepChat* chat_step = (LLGestureStepChat*)step;
	chat_step->mChatText = self->mChatEditor->getText();
	chat_step->mFlags = 0x0;

	// Update the UI label in the list
	updateLabel(step_item);

	self->mDirty = TRUE;
	self->refresh();
}

// static
void LLPreviewGesture::onCommitWait(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* step_item = self->mStepList->getFirstSelected();
	if (!step_item) return;

	LLGestureStep* step = (LLGestureStep*)step_item->getUserdata();
	if (step->getType() != STEP_WAIT) return;

	LLGestureStepWait* wait_step = (LLGestureStepWait*)step;
	U32 flags = 0x0;
	if (self->mWaitAnimCheck->get()) flags |= WAIT_FLAG_ALL_ANIM;
	if (self->mWaitTimeCheck->get()) flags |= WAIT_FLAG_TIME;
	wait_step->mFlags = flags;

	{
		LLLocale locale(LLLocale::USER_LOCALE);

		F32 wait_seconds = (F32)atof(self->mWaitTimeEditor->getText().c_str());
		if (wait_seconds < 0.f) wait_seconds = 0.f;
		if (wait_seconds > 3600.f) wait_seconds = 3600.f;
		wait_step->mWaitSeconds = wait_seconds;
	}

	// Enable the input area if necessary
	self->mWaitTimeEditor->setEnabled(self->mWaitTimeCheck->get());

	// Update the UI label in the list
	updateLabel(step_item);

	self->mDirty = TRUE;
	self->refresh();
}

// static
void LLPreviewGesture::onCommitWaitTime(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* step_item = self->mStepList->getFirstSelected();
	if (!step_item) return;

	LLGestureStep* step = (LLGestureStep*)step_item->getUserdata();
	if (step->getType() != STEP_WAIT) return;

	self->mWaitTimeCheck->set(TRUE);
	onCommitWait(ctrl, data);
}


// static
void LLPreviewGesture::onKeystrokeCommit(LLLineEditor* caller,
										 void* data)
{
	// Just commit every keystroke
	onCommitSetDirty(caller, data);
}

// static
void LLPreviewGesture::onClickAdd(void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* library_item = self->mLibraryList->getFirstSelected();
	if (!library_item) return;

	const LLScrollListCell* library_cell = library_item->getColumn(0);
	const std::string& library_text = library_cell->getText();

	self->addStep(library_text);

	self->mDirty = TRUE;
	self->refresh();
}

LLScrollListItem* LLPreviewGesture::addStep(const std::string& library_text)
{
	LLGestureStep* step = NULL;
	if (!LLString::compareInsensitive(library_text.c_str(), "Animation"))
	{
		step = new LLGestureStepAnimation();
	}
	else if (!LLString::compareInsensitive(library_text.c_str(), "Sound"))
	{
		step = new LLGestureStepSound();
	}
	else if (!LLString::compareInsensitive(library_text.c_str(), "Chat"))
	{
		step = new LLGestureStepChat();
	}
	else if (!LLString::compareInsensitive(library_text.c_str(), "Wait"))
	{
		step = new LLGestureStepWait();
	}
	else
	{
		llerrs << "Unknown step type: " << library_text << llendl;;
		return NULL;
	}

	// Create an enabled item with this step
	LLSD row;
	row["columns"][0]["value"] = step->getLabel();
	row["columns"][0]["font"] = "SANSSERIF_SMALL";
	LLScrollListItem* step_item = mStepList->addElement(row);
	step_item->setUserdata(step);

	// And move selection to the list on the right
	mLibraryList->deselectAllItems();
	mStepList->deselectAllItems();

	step_item->setSelected(TRUE);

	return step_item;
}

// static
void LLPreviewGesture::onClickUp(void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	S32 selected_index = self->mStepList->getFirstSelectedIndex();
	if (selected_index > 0)
	{
		self->mStepList->swapWithPrevious(selected_index);
		self->mDirty = TRUE;
		self->refresh();
	}
}

// static
void LLPreviewGesture::onClickDown(void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	S32 selected_index = self->mStepList->getFirstSelectedIndex();
	if (selected_index < 0) return;

	S32 count = self->mStepList->getItemCount();
	if (selected_index < count-1)
	{
		self->mStepList->swapWithNext(selected_index);
		self->mDirty = TRUE;
		self->refresh();
	}
}

// static
void LLPreviewGesture::onClickDelete(void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	LLScrollListItem* item = self->mStepList->getFirstSelected();
	S32 selected_index = self->mStepList->getFirstSelectedIndex();
	if (item && selected_index >= 0)
	{
		LLGestureStep* step = (LLGestureStep*)item->getUserdata();
		delete step;
		step = NULL;

		self->mStepList->deleteSingleItem(selected_index);

		self->mDirty = TRUE;
		self->refresh();
	}
}

// static
void LLPreviewGesture::onCommitActive(LLUICtrl* ctrl, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;
	if (!gGestureManager.isGestureActive(self->mItemUUID))
	{
		gGestureManager.activateGesture(self->mItemUUID);
	}
	else
	{
		gGestureManager.deactivateGesture(self->mItemUUID);
	}

	// Make sure the (active) label in the inventory gets updated.
	LLViewerInventoryItem* item = gInventory.getItem(self->mItemUUID);
	if (item)
	{
		gInventory.updateItem(item);
		gInventory.notifyObservers();
	}

	self->refresh();
}

// static
void LLPreviewGesture::onClickSave(void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;
	self->saveIfNeeded();
}

// static
void LLPreviewGesture::onClickPreview(void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	if (!self->mPreviewGesture)
	{
		// make temporary gesture
		self->mPreviewGesture = self->createGesture();

		// add a callback
		self->mPreviewGesture->mDoneCallback = onDonePreview;
		self->mPreviewGesture->mCallbackData = self;

		// set the button title
		self->mPreviewBtn->setLabelSelected("Stop");
		self->mPreviewBtn->setLabelUnselected("Stop");

		// play it, and delete when done
		gGestureManager.playGesture(self->mPreviewGesture);

		self->refresh();
	}
	else
	{
		// Will call onDonePreview() below
		gGestureManager.stopGesture(self->mPreviewGesture);

		self->refresh();
	}
}


// static
void LLPreviewGesture::onDonePreview(LLMultiGesture* gesture, void* data)
{
	LLPreviewGesture* self = (LLPreviewGesture*)data;

	self->mPreviewBtn->setLabelSelected("Preview");
	self->mPreviewBtn->setLabelUnselected("Preview");

	delete self->mPreviewGesture;
	self->mPreviewGesture = NULL;

	self->refresh();
}
