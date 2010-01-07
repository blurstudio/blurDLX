// String Definitions
#define			S_TREEVIEW				_T( "TreeViewPlus")

// Window Classes
#define			TREEVIEW_WINDOWCLASS	WC_TREEVIEW


//---------------------------------------------------------------

class TreeViewItemPlus;
class TreeViewPlus;

visible_class( TreeViewPlus )
class TreeViewPlus : public AppliedControl {
	private:
		BOOL						_amAutoCollapsible;
		BOOL						_amDragging;
		COLORREF					_bkColor;
		HWND						_containerWindow;
		Value*						_compareMethod;
		Value*						_data;
		BOOL						_dragEnabled;
		BOOL						_labelEditingEnabled;
		HWND						_editWindow;
		TVHITTESTINFO				_hitTest;
		HTREEITEM					_hitTarget;
		HIMAGELIST					_images;
		BOOL						_invertSort;
		Value*						_itemEditing;
		BOOL						_multiselection;
		POINTS						_points;
		TreeViewItemPlus*			_root;
		COLORREF					_selectedColor;
		BOOL						_sortingEnabled;
		BOOL						_syncSelection;
		HWND						_treeWindow;
		BOOL						_useCustomToolTips;
		BOOL						_useSelectedColor;

	public:
		//---------------------------------------------------------------
		// Constructor
									TreeViewPlus( Value* name, Value* caption, Value** keyparms, int keyparm_count );
									~TreeViewPlus();

		//---------------------------------------------------------------
		//						Static Methods
		//---------------------------------------------------------------
		static LRESULT CALLBACK		WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
		static RolloutControl*		create( Value* name, Value* caption, Value** keyparms, int keyparm_count )			{ return new TreeViewPlus( name, caption, keyparms, keyparm_count ); }

		//---------------------------------------------------------------
		//						Handle Methods
		//---------------------------------------------------------------
		long						handleBeginDrag( HWND hWndTreeView, LPNMTREEVIEW pNMTV );
		long						handleBeginEditLabel( HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD );
		long						handleCustomDraw( HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD );
		long						handleCustomTooltip( NMTVGETINFOTIP* pTVTipInfo );
		long						handleEndEditLabel( HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD );
		long						handleNotify( HWND hWndDlg, int nIDCtrl, LPNMHDR pNMHDR );

		//---------------------------------------------------------------
		//						TreeView Methods
		//---------------------------------------------------------------
		BOOL						addChild( TreeViewItemPlus* child );
		Value*						addItem( Value* item, BOOL recursive = FALSE );
		COLORREF					bkColor();
		HWND						containerWindow();
		Value*						compareMethod();
		void						deleteAllItems();
		void						deselectAll();
		Value*						data();
		Value*						data( HTREEITEM hItem );
		HWND						editWindow();
		void						endEditLabel();
		int							height();
		Value*						hitTest( HWND hWndTreeView, UINT flags = TVHT_ONITEMLABEL );
		void						invalidate();
		BOOL						isAutoCollapsible();
		BOOL						isDragging();
		BOOL						isDragEnabled();
		BOOL						isLabelEditingEnabled();
		BOOL						isMultiSelection();
		BOOL						isSelectionSynced();
		BOOL						isSortingEnabled();
		BOOL						isSortingInverted();
		BOOL						isUsingCustomToolTips();
		BOOL						isUsingSelectedColor();
		Value*						itemByData( Value* data );
		Array*						itemsByNode( Value* node );
		POINTS						points();
		BOOL						repaint();
		TreeViewItemPlus*			root();
		COLORREF					selectedColor();
		Array*						selectedItems();
		BOOL						setBkColor( COLORREF newBkColor );
		void						setAutoCollapsible( BOOL state );
		void						setCompareMethod( Value* method );
		void						setData( Value* data );
		void						setDragEnabled( BOOL dragEnabled );
		void						setLabelEditingEnabled( BOOL state );
		void						setHeight( int height );
		void						setItems( Array* itemList );
		void						setMultiSelection( BOOL state );
		void						setPoints( POINTS newPoints );
		void						setSelectedColor( COLORREF newSelColor );
		void						setSelectionSynced( BOOL syncSel );
		void						setSortingEnabled( BOOL state );
		void						setSortingInverted( BOOL state );
		void						setSize( Point2 size );
		void						setUseCustomToolTips( BOOL state );
		void						setUseSelectedColor( BOOL useSelColor );
		void						setWidth( int width );
		Point2						size();
		void						sort();
		int							width();
		HWND						window();

		//---------------------------------------------------------------
		//							Events
		//---------------------------------------------------------------
		LRESULT						FocusChanged( void );
		LRESULT						KeyDown( WORD key );
		LRESULT						LButtonClick( void );
		LRESULT						LButtonDblClk( void );
		LRESULT						LButtonDown( void );
		LRESULT						RButtonClick( void );

		//---------------------------------------------------------------
		//						MAXScript Methods
		//---------------------------------------------------------------
		classof_methods( TreeViewPlus, RolloutControl );

		void						collect();
		void						gc_trace();
		void						sprin1( CharStream* s )																{ s->printf( _T("TreeViewPlus:%s"), this->name->to_string()); }
		#define						is_treeviewplus(p) ((p)->tag == class_tag(TreeViewPlus))

		void						add_control( Rollout* ro, HWND parent, HINSTANCE hInstance, int& current_y );
		Value*						applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );
		void						compute_layout( Rollout* ro, layout_data* pos )										{ pos->width = 90; pos->height = 30; }
		LPCTSTR						get_control_class()																	{ return TREEVIEW_WINDOWCLASS; }
		Value*						get_property( Value** arg_list, int count );
		BOOL						handle_message( Rollout* ro, UINT message, WPARAM wParam, LPARAM lParam );
		int							num_controls()																		{ return 1; }
		void						set_enable();
		Value*						set_property( Value** arg_list, int count );

//		def_generic(				get,			"get" );

//#include "defimpfn.h"
//		def_generic(				get_props,		"getPropNames" );
//		def_generic(				show_props,		"showProperties" );
//		def_generic(				show_methods,	"showMethods" );
//		def_generic(				show_events,	"showEvents" );
};

visible_class( TreeViewItemPlus )
class TreeViewItemPlus : public AppliedValue {
	private:
		BOOL				_amEnabled;
		BOOL				_amSelected;
		Array*				_children;
		COLORREF			_color;
		TreeViewPlus*		_control;
		Value*				_data;
		Value*				_extraData;
		int					_iconImage;
		int					_iconSelectedImage;
		int					_iconState;
		TSTR				_name;
		Value*				_node;
		TreeViewItemPlus*	_parent;
		TSTR				_toolTip;
		HTREEITEM			_treeItem;

	public:
		// Constructor
		TreeViewItemPlus( TreeViewPlus* control );
		TreeViewItemPlus( TreeViewPlus* control, TreeViewItemPlus* item );
		~TreeViewItemPlus();

		//---------------------------------------------------------------
		//						TreeViewItemPlus Methods
		//---------------------------------------------------------------
		BOOL				addChild( TreeViewItemPlus* child );
		Value*				addItem( Value* item, BOOL recursive = FALSE );
		void				addToTree( HTREEITEM parent = NULL );
		Array*				children( BOOL recursive = FALSE );
		COLORREF			color( void );
		TreeViewPlus*		control( void );
		Value*				data( void );
		void				deleteChildren();
		void				deleteItem( BOOL repaintUi = TRUE );
		void				deselectAll( BOOL recursive = FALSE );
		void				editLabel( void );
		BOOL				ensureVisible( void );
		void				endEditLabel( void );
		Value*				extraData( void );
		BOOL				hasChild( TreeViewItemPlus* item, BOOL recursive = FALSE );
		int					iconImage( void );
		int					iconSelectedImage( void );
		int					iconState( void );
		BOOL				isCollapsed( void );
		BOOL				isEnabled( void );
		BOOL				isInitialized( void );
		BOOL				isRoot( void );
		BOOL				isSelected( void );
		BOOL				isVisible( void );
		Value*				itemByData( Value* data, BOOL recursive = FALSE );
		Array*				itemsByNode( Value* node, BOOL recursive = FALSE );
		TSTR				name( void );
		Value*				node( void );
		TreeViewItemPlus*	parent( void );
		BOOL				remove( BOOL repaintUi = TRUE );
		TreeViewItemPlus*	root( void );
		BOOL				setCollapsed( BOOL state );
		BOOL				setColor( COLORREF clr );
		BOOL				setControl( TreeViewPlus* ctrl );
		BOOL				setData( Value* dataValue );
		BOOL				setEnabled( BOOL state );
		BOOL				setExtraData( Value* extraDat );
		BOOL				setIconImage( int index );
		BOOL				setIconSelectedImage( int index );
		BOOL				setIconState( int index );
		BOOL				setName( TSTR newName );
		BOOL				setNode( Value* node );
		BOOL				setParent( TreeViewItemPlus* parentItem );
		BOOL				setToolTip( TSTR tip );
		BOOL				setSelected( BOOL state, BOOL recursive = FALSE, BOOL updateUi = TRUE );
		Array*				siblings( void );
		void				sort( BOOL recursive = FALSE );
		BOOL				toggleCollapsed( void );
		TSTR				toolTip( void );
		HTREEITEM			treeItem( void );

		//---------------------------------------------------------------
		//						MAXScript Methods
		//---------------------------------------------------------------
		classof_methods( TreeViewItemPlus, Value );

		void				collect();
		void				sprin1(CharStream* s);
		void				gc_trace();
		#define				is_treeviewitemplus(p) ((p)->tag == class_tag(TreeViewItemPlus))

		Value*				get_property( Value** arg_list, int count );
		Value*				set_property( Value** arg_list, int count );

		Value*				applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );

#include "defimpfn.h"
		def_generic		( lt,			"<" );
		def_generic		( gt,			">" );
//		def_generic(		get_props,		"getPropNames" );
//		def_generic(		show_props,		"showProperties" );
//		def_generic(		show_methods,	"showMethods" );
//		def_generic(		show_events,	"showEvents" );
};