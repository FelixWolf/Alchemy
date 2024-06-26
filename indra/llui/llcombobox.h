/**
 * @file llcombobox.h
 * @brief LLComboBox base class
 *
 * $LicenseInfo:firstyear=2001&license=viewerlgpl$
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

// A control that displays the name of the chosen item, which when clicked
// shows a scrolling box of choices.

#ifndef LL_LLCOMBOBOX_H
#define LL_LLCOMBOBOX_H

#include "llbutton.h"
#include "lluictrl.h"
#include "llctrlselectioninterface.h"
#include "llrect.h"
#include "llscrolllistctrl.h"
#include "lllineeditor.h"
#include <boost/function.hpp>

// Classes

class LLFontGL;
class LLViewBorder;

class LLComboBox
: public LLUICtrl
, public LLCtrlListInterface
, public ll::ui::SearchableControl
{
public:
    typedef enum e_preferred_position
    {
        ABOVE,
        BELOW
    } EPreferredPosition;

    struct PreferredPositionValues : public LLInitParam::TypeValuesHelper<EPreferredPosition, PreferredPositionValues>
    {
        static void declareValues();
    };


    struct ItemParams : public LLInitParam::Block<ItemParams, LLScrollListItem::Params>
    {
        Optional<std::string>   label;
        ItemParams();
    };

    struct Params
    :   public LLInitParam::Block<Params, LLUICtrl::Params>
    {
        Optional<bool>                      allow_text_entry,
                                            show_text_as_tentative,
                                            allow_new_values;
        Optional<S32>                       max_chars;
        Optional<commit_callback_t>         prearrange_callback,
                                            text_entry_callback,
                                            text_changed_callback;

        Optional<EPreferredPosition, PreferredPositionValues>   list_position;

        // components
        Optional<LLButton::Params>          combo_button;
        Optional<LLScrollListCtrl::Params>  combo_list;
        Optional<LLLineEditor::Params>      combo_editor;

        Optional<LLButton::Params>          drop_down_button;

        Multiple<ItemParams>                items;

        Params();
    };


    virtual ~LLComboBox();
    /*virtual*/ BOOL postBuild() override;

protected:
    friend class LLUICtrlFactory;
    LLComboBox(const Params&);
    void    initFromParams(const Params&);
    void    prearrangeList(std::string filter = "");

    std::string _getSearchText() const override;
    void onSetHighlight() const override;

    void imageLoaded();

public:
    // LLView interface
    void    onFocusLost() override;

    BOOL    handleToolTip(S32 x, S32 y, MASK mask) override;
    BOOL    handleKeyHere(KEY key, MASK mask) override;
    BOOL    handleUnicodeCharHere(llwchar uni_char) override;
    BOOL    handleScrollWheel(S32 x, S32 y, S32 clicks) final override;

    // LLUICtrl interface
    void    clear() override;                   // select nothing
    void    onCommit() override;
    BOOL    acceptsTextInput() const override { return mAllowTextEntry; }
    BOOL    isDirty() const override;           // Returns TRUE if the user has modified this control.
    void    resetDirty() override;              // Clear dirty state

    void    setFocus(BOOL b) override;

    // Selects item by underlying LLSD value, using LLSD::asString() matching.
    // For simple items, this is just the name of the label.
    void    setValue(const LLSD& value) override;

    // Gets underlying LLSD value for currently selected items.  For simple
    // items, this is just the label.
    LLSD    getValue() const override;

    void            setTextEntry(const LLStringExplicit& text);
    void            setKeystrokeOnEsc(BOOL enable);

    LLScrollListItem*   add(const std::string& name, EAddPosition pos = ADD_BOTTOM, BOOL enabled = TRUE);   // add item "name" to menu
    LLScrollListItem*   add(const std::string& name, const LLUUID& id, EAddPosition pos = ADD_BOTTOM, BOOL enabled = TRUE);
    LLScrollListItem*   add(const std::string& name, void* userdata, EAddPosition pos = ADD_BOTTOM, BOOL enabled = TRUE);
    LLScrollListItem*   add(const std::string& name, LLSD value, EAddPosition pos = ADD_BOTTOM, BOOL enabled = TRUE);
    LLScrollListItem*   addSeparator(EAddPosition pos = ADD_BOTTOM);
    BOOL            remove( S32 index );    // remove item by index, return TRUE if found and removed
    void            removeall() { clearRows(); }
    bool            itemExists(const std::string& name);

    void            sortByName(BOOL ascending = TRUE); // Sort the entries in the combobox by name

    // Select current item by name using selectItemByLabel.  Returns FALSE if not found.
    BOOL            setSimple(const LLStringExplicit& name);
    // Get name of current item. Returns an empty string if not found.
    const std::string   getSimple() const;
    // Get contents of column x of selected row
    virtual const std::string getSelectedItemLabel(S32 column = 0) const;

    // Sets the label, which doesn't have to exist in the label.
    // This is probably a UI abuse.
// [SL:KB] - Patch: Control-ComboBox | Checked: Catznip-6.4
    virtual void    setLabel(const LLStringExplicit& name);
// [/SL:KB]
//  void            setLabel(const LLStringExplicit& name);

    // Updates the combobox label to match the selected list item.
// [SL:KB] - Patch: Control-ComboBox | Checked: Catznip-6.4
    virtual void    updateLabel();
// [/SL:KB]
//  void            updateLabel();

    BOOL            remove(const std::string& name);    // remove item "name", return TRUE if found and removed

    BOOL            setCurrentByIndex( S32 index );
    S32             getCurrentIndex() const;

    void            setEnabledByValue(const LLSD& value, BOOL enabled);

    void            createLineEditor(const Params&);

    //========================================================================
    LLCtrlSelectionInterface* getSelectionInterface() override  { return (LLCtrlSelectionInterface*)this; };
    LLCtrlListInterface* getListInterface() override            { return (LLCtrlListInterface*)this; };

    // LLCtrlListInterface functions
    // See llscrolllistctrl.h
    S32     getItemCount() const override;
    // Overwrites the default column (See LLScrollListCtrl for format)
    void    addColumn(const LLSD& column, EAddPosition pos = ADD_BOTTOM) override;
    void    clearColumns() override;
    void    setColumnLabel(std::string_view column, const std::string& label) override;
    LLScrollListItem* addElement(const LLSD& value, EAddPosition pos = ADD_BOTTOM, void* userdata = NULL) override;
    LLScrollListItem* addSimpleElement(const std::string& value, EAddPosition pos = ADD_BOTTOM, const LLSD& id = LLSD()) override;
    void    clearRows() override;
    void    sortByColumn(std::string_view name, BOOL ascending) override;

    // LLCtrlSelectionInterface functions
    BOOL    getCanSelect() const override               { return TRUE; }
    BOOL    selectFirstItem() override                  { return setCurrentByIndex(0); }
    BOOL    selectNthItem( S32 index ) override         { return setCurrentByIndex(index); }
    BOOL    selectItemRange(S32 first, S32 last) override;
    S32     getFirstSelectedIndex() const override      { return getCurrentIndex(); }
    BOOL    setCurrentByID( const LLUUID& id ) override;
    LLUUID  getCurrentID() const override;              // LLUUID::null if no items in menu
    BOOL    setSelectedByValue(const LLSD& value, BOOL selected) override;
    LLSD    getSelectedValue() override;
    BOOL    isSelected(const LLSD& value) const override;
    BOOL    operateOnSelection(EOperation op) override;
    BOOL    operateOnAll(EOperation op) override;

    //========================================================================

    void            setLeftTextPadding(S32 pad);

    void*           getCurrentUserdata();

    void            setPrearrangeCallback( commit_callback_t cb ) { mPrearrangeCallback = cb; }
    void            setTextEntryCallback( commit_callback_t cb ) { mTextEntryCallback = cb; }
    void            setTextChangedCallback( commit_callback_t cb ) { mTextChangedCallback = cb; }

    /**
    * Connects callback to signal called when Return key is pressed.
    */
    boost::signals2::connection setReturnCallback( const commit_signal_t::slot_type& cb ) { return mOnReturnSignal.connect(cb); }

    void            setButtonVisible(BOOL visible);

    void            onButtonMouseDown();
    void            onListMouseUp();
    void            onItemSelected(const LLSD& data);
    void            onTextCommit(const LLSD& data);

    void            updateSelection();
    virtual void    showList();
    virtual void    hideList();

    virtual void    onTextEntry(LLLineEditor* line_editor);

protected:
    LLButton*           mButton;
    LLLineEditor*       mTextEntry;
    LLScrollListCtrl*   mList;
    EPreferredPosition  mListPosition;
    LLPointer<LLUIImage>    mArrowImage;
    LLUIString          mLabel;
    BOOL                mHasAutocompletedText;

private:
    BOOL                mAllowTextEntry;
    BOOL                mAllowNewValues;
    S32                 mMaxChars;
    BOOL                mTextEntryTentative;
    commit_callback_t   mPrearrangeCallback;
    commit_callback_t   mTextEntryCallback;
    commit_callback_t   mTextChangedCallback;
    commit_callback_t   mSelectionCallback;
    boost::signals2::connection mTopLostSignalConnection;
    boost::signals2::connection mImageLoadedConnection;
    commit_signal_t     mOnReturnSignal;
    S32                 mLastSelectedIndex;
};

// A combo box with icons for the list of items.
class LLIconsComboBox
:   public LLComboBox
{
public:
    struct Params
    :   public LLInitParam::Block<Params, LLComboBox::Params>
    {
        Optional<S32>       icon_column,
                            label_column;
        Params();
    };

    /*virtual*/ const std::string getSelectedItemLabel(S32 column = 0) const override;

private:
    enum EColumnIndex
    {
        ICON_COLUMN = 0,
        LABEL_COLUMN
    };

    friend class LLUICtrlFactory;
    LLIconsComboBox(const Params&);
    virtual ~LLIconsComboBox() = default;

    S32         mIconColumnIndex;
    S32         mLabelColumnIndex;
};

#endif
