/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef RC_ITEM_H
#define RC_ITEM_H

#include <wx/dataview.h>
#include <macros.h>
#include <base_struct.h>

class MARKER_BASE;
class EDA_BASE_FRAME;
class RC_ITEM;


/**
 * Provide an abstract interface of a RC_ITEM* list manager.
 * The details of the actual list architecture are hidden from the caller.  Any class that
 * implements this interface can then be used by a RC_TREE_MODEL class without it knowing
 * the actual architecture of the list.
 */
class RC_ITEMS_PROVIDER
{
public:
    virtual void SetSeverities( int aSeverities ) = 0;

    virtual int GetCount( int aSeverity = -1 ) = 0;

    /**
     * Function GetItem
     * retrieves a RC_ITEM by index.
     */
    virtual RC_ITEM* GetItem( int aIndex ) = 0;

    /**
     * Function DeleteItem
     * removes (and optionally deletes) the indexed item from the list.
     * @param aDeep If true, the source item should be deleted as well as its entry in the list.
     */
    virtual void DeleteItem( int aIndex, bool aDeep ) = 0;

    /**
     * Function DeleteAllItems
     * removes and deletes all the items in the list.
     */
    virtual void DeleteAllItems() = 0;

    virtual ~RC_ITEMS_PROVIDER() { }
};


/**
 * RC_ITEM
 * is a holder for a DRC (in Pcbnew) or ERC (in Eeschema) error item.
 * RC_ITEMs can have zero, one, or two related EDA_ITEMs.
 */
class RC_ITEM
{
protected:
    int           m_errorCode;         // the error code's numeric value
    wxString      m_errorMessage;
    MARKER_BASE*  m_parent;            // The marker this item belongs to, if any
    KIID          m_mainItemUuid;
    KIID          m_auxItemUuid;

public:

    RC_ITEM()
    {
        m_errorCode     = 0;
        m_parent        = nullptr;
        m_mainItemUuid  = niluuid;
        m_auxItemUuid   = niluuid;
    }

    RC_ITEM( RC_ITEM* aItem )
    {
        m_errorCode = aItem->m_errorCode;
        m_errorMessage = aItem->m_errorMessage;
        m_parent = aItem->m_parent;
        m_mainItemUuid = aItem->m_mainItemUuid;
        m_auxItemUuid = aItem->m_auxItemUuid;
    }

    virtual ~RC_ITEM() { }

    void SetErrorMessage( const wxString& aMessage ) { m_errorMessage = aMessage; }

    void SetItems( EDA_ITEM* aItem, EDA_ITEM* bItem = nullptr )
    {
        m_mainItemUuid = aItem->m_Uuid;

        if( bItem )
            m_auxItemUuid = bItem->m_Uuid;
    }

    void SetItems( const KIID& aItem, const KIID& bItem = niluuid )
    {
        m_mainItemUuid = aItem;
        m_auxItemUuid = bItem;
    }

    KIID GetMainItemID() const { return m_mainItemUuid; }
    KIID GetAuxItemID() const { return m_auxItemUuid; }

    void SetParent( MARKER_BASE* aMarker ) { m_parent = aMarker; }
    MARKER_BASE* GetParent() const { return m_parent; }


    /**
     * Function ShowReport
     * translates this object into a text string suitable for saving to disk in a report.
     * @return wxString - the simple multi-line report text.
     */
    virtual wxString ShowReport( EDA_UNITS aUnits,
                                 const std::map<KIID, EDA_ITEM*>& aItemMap ) const;

    int GetErrorCode() const { return m_errorCode; }
    void SetErrorCode( int aCode ) { m_errorCode = aCode; }

    /**
     * Function GetErrorText
     * returns the string form of a RC error code
     */
    virtual wxString GetErrorText( int aCode = -1, bool aTranslate = true ) const = 0;

    /**
     * Function GetErrorMessage
     * returns the error message of a RC_ITEM
     */
    virtual wxString GetErrorMessage() const;

    /**
     * Function ShowCoord
     * formats a coordinate or position to text.
     */
    static wxString ShowCoord( EDA_UNITS aUnits, const wxPoint& aPos );
};


class RC_TREE_NODE
{
public:
    enum NODE_TYPE { MARKER, MAIN_ITEM, AUX_ITEM };

    RC_TREE_NODE( RC_TREE_NODE* aParent, RC_ITEM* aRcItem, NODE_TYPE aType ) :
            m_Type( aType ),
            m_RcItem( aRcItem ),
            m_Parent( aParent )
    {}

    ~RC_TREE_NODE()
    {
        for( RC_TREE_NODE* child : m_Children )
            delete child;
    }

    NODE_TYPE                  m_Type;
    RC_ITEM*                   m_RcItem;

    RC_TREE_NODE*              m_Parent;
    std::vector<RC_TREE_NODE*> m_Children;
};


class RC_TREE_MODEL : public wxDataViewModel, wxEvtHandler
{
public:
    static wxDataViewItem ToItem( RC_TREE_NODE const* aNode )
    {
        return wxDataViewItem( const_cast<void*>( static_cast<void const*>( aNode ) ) );
    }

    static RC_TREE_NODE* ToNode( wxDataViewItem aItem )
    {
        return static_cast<RC_TREE_NODE*>( aItem.GetID() );
    }

    static KIID ToUUID( wxDataViewItem aItem );


public:
    RC_TREE_MODEL( EDA_DRAW_FRAME* aParentFrame, wxDataViewCtrl* aView );

    ~RC_TREE_MODEL();

    void SetProvider( RC_ITEMS_PROVIDER* aProvider );
    void SetSeverities( int aSeverities );

    int GetDRCItemCount() const { return m_tree.size(); }

    void ExpandAll();

    bool IsContainer( wxDataViewItem const& aItem ) const override;

    wxDataViewItem GetParent( wxDataViewItem const& aItem ) const override;

    unsigned int GetChildren( wxDataViewItem const& aItem,
                              wxDataViewItemArray&  aChildren ) const override;

    // Simple, single-text-column model
    unsigned int GetColumnCount() const override { return 1; }
    wxString GetColumnType( unsigned int aCol ) const override { return "string"; }
    bool HasContainerColumns( wxDataViewItem const& aItem ) const override { return true; }

    /**
     * Called by the wxDataView to fetch an item's value.
     */
    void GetValue( wxVariant&              aVariant,
                   wxDataViewItem const&   aItem,
                   unsigned int            aCol ) const override;

    /**
     * Called by the wxDataView to edit an item's content.
     */
    bool SetValue( wxVariant const& aVariant,
                   wxDataViewItem const&   aItem,
                   unsigned int            aCol ) override
    {
        // Editing not supported
        return false;
    }

    /**
     * Called by the wxDataView to fetch an item's formatting.  Return true iff the
     * item has non-default attributes.
     */
    bool GetAttr( wxDataViewItem const&   aItem,
                  unsigned int            aCol,
                  wxDataViewItemAttr&     aAttr ) const override;

    void ValueChanged( RC_TREE_NODE* aNode );

    void DeleteCurrentItem( bool aDeep );
    void DeleteAllItems();

private:
    void rebuildModel( RC_ITEMS_PROVIDER* aProvider, int aSeverities );
    void onSizeView( wxSizeEvent& aEvent );

private:
    EDA_DRAW_FRAME*            m_editFrame;
    wxDataViewCtrl*            m_view;
    int                        m_severities;
    RC_ITEMS_PROVIDER*         m_rcItemsProvider;   // I own this, but not its contents

    std::vector<RC_TREE_NODE*> m_tree;              // I own this
};



#endif      // RC_ITEM_H
