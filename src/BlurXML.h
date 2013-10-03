#ifndef		__BLURXML_H__
#define		__BLURXML_H__

#include "atlbase.h"
#include "msxml2.h"

class XMLNodeList;
class XMLDocument;

visible_class( XMLNode );

class XMLNode : public AppliedValue {
	private:
		CComPtr<IXMLDOMNode> _node;
		IXMLDOMDocument * _document;

	public:
		XMLNode( IXMLDOMNode* node, IXMLDOMDocument* document );
		~XMLNode();

		//------------------------------------------------------------------
		//						C++ METHODS
		//------------------------------------------------------------------
		XMLNode*			addNode( TSTR nodeName );
		XMLNodeList*		attributes();
		XMLNode*			attributeByName( TSTR name );
		TSTR				baseName();
		XMLNodeList*		children();
		XMLNode*			childByIndex( int index );
		XMLNode*			childByName( TSTR name );
		Array*				childrenByName( TSTR name );
		IXMLDOMDocument*	document();
		XMLNode*			firstChild();
		bool				isParsed();
		bool				isProperty();
		bool				isValid() { return _node != NULL; }
		XMLNode*			lastChild();
		TSTR				namespaceURI();
		XMLNode*			nextSibling();
		IXMLDOMNode*		node();
		TSTR				nodeName();
		int					nodeType();
		long				numAttributes();
		long				numChildren();
		long				numNodes();
		long				numProperties();
		XMLNode*			parent();
		TSTR				prefix();
		XMLNode*			previousSibling();
		XMLNodeList*		properties();
		XMLNode*			propertyByName( TSTR name );
		Array*				propertiesByName( TSTR name );

		bool				setAttribute( TSTR name, bool val );
		bool				setAttribute( TSTR name, Color val );
		bool				setAttribute( TSTR name, float val );
		bool				setAttribute( TSTR name, int val );
		bool				setAttribute( TSTR name, TSTR val );
		bool				setAttribute( TSTR name, Value* val );

		bool				setProperty( TSTR name, bool val );
	
		bool				setProperty( TSTR name, Color val );
		bool				setProperty( TSTR name, float val );
		bool				setProperty( TSTR name, int val );
		bool				setProperty( TSTR name, TSTR val );
		bool				setProperty( TSTR name, Value* val );

		bool				setText( TSTR str );

		bool				setValue( bool val );
		bool				setValue( Color val );
		bool				setValue( float val );
		bool				setValue( int val );
		bool				setValue( TSTR val );
		bool				setValue( Value* val );

		TSTR				text();
		IXMLDOMNode*		textNode();
		Value*				value();
		TSTR				writeXML();


		//------------------------------------------------------------------
		//						MAXSCRIPT METHODS
		//------------------------------------------------------------------
		void				collect()	{ delete this; }
		void				sprin1( CharStream* s );
		void				gc_trace();

		classof_methods( XMLNode, Value );
#define is_xmlnode(p) ((p)->tag == class_tag( XMLNode ))

		Value*				applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );
		Value*				get_property( Value** arg_list, int count );
		Value*				set_property( Value** arg_list, int count );

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "macros/define_implementations.h"
#else
	#include "defimpfn.h"
#endif
		def_generic(		get,		"get" );
		//use_generic(		eq,			"=" );

		//def_generic(	show_props,	"showProperties" );
		//def_generic(	get_props,	"getPropNames" );
};

visible_class( XMLNodeList );
class XMLNodeList : public Value {
	private:
		IXMLDOMDocument * _document;
		IXMLDOMNodeList * _nodeList;
		IXMLDOMNamedNodeMap * _attrList;
		Array * _propList;

	public:
		XMLNodeList( IXMLDOMDocument* document, IXMLDOMNodeList* nodeList );
		XMLNodeList( IXMLDOMDocument* document, IXMLDOMNamedNodeMap* attrList );
		XMLNodeList( IXMLDOMDocument* document );
		~XMLNodeList();

		//------------------------------------------------------------------
		//						C++ METHODS
		//------------------------------------------------------------------
		bool				addItem( XMLNode* item );
		IXMLDOMDocument*	document() { return _document; }
		XMLNode*			getItem( int index );
		XMLNode*			getItem( const TCHAR * name );
		bool				isAttrList() { return _attrList != NULL; }
		bool				isNodeList() { return _nodeList != NULL; }
		bool				isPropList() { return _propList != NULL; }
		bool				isValid() { return isNodeList() || isAttrList(); }
		long				length();
		Array*				toArray();

		//------------------------------------------------------------------
		//						MAXSCRIPT METHODS
		//------------------------------------------------------------------
		void		collect()			{ delete this; }
		void		sprin1( CharStream* s );
		void		gc_trace();
#define				is_xmlnodelist(p) ((p)->tag == class_tag( XMLNodeList ) )
		BOOL		_is_collection() { return 1; }

		Value*		get_property( Value** arg_list, int count );
		Value*		set_property( Value** arg_list, int count );
		Value*		get_names( BOOL onlyProps = FALSE, BOOL onlyNodes = FALSE );
		Value*		get_nodes( BOOL onlyProps = FALSE, BOOL onlyNodes = FALSE );
		Value*		map(node_map& m);
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "macros/define_implementations.h"
#else
	#include "defimpfn.h"
#endif

		def_generic( get, "get" );
		use_generic( coerce, "coerce" );

		//def_generic(	show_props,	"showProperties" );
		//def_generic(	get_props,	"getPropNames" );
};

applyable_class( XMLDocument );
class XMLDocument : public AppliedValue {
	private:
		IXMLDOMDocument*	_document;
		XMLNode*			_root;

	public:
		//------------------------------------------------------------------
		//						   C++ METHODS
		//------------------------------------------------------------------
		XMLDocument();
		~XMLDocument();

		XMLNode*			addNode( TSTR nodeName );
		IXMLDOMDocument*	document();
		bool				loadFromFile( TSTR fileName );
		bool				isValid() { return _document != 0; }
		bool				parseXML( TSTR xml );
		XMLNode*			root();
		bool				saveToFile( TSTR fileName );
		TSTR				writeXML();

		//------------------------------------------------------------------
		//						MAXSCRIPT METHODS
		//------------------------------------------------------------------
		void				collect() { delete this; }
		void				sprin1( CharStream* s );
		void				gc_trace();

		classof_methods( XMLDocument, Value );
#define						is_xmldocument(p) ((p)->tag == class_tag( XMLDocument ))

		Value*				applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );
		Value*				get_property( Value** arg_list, int count );
		Value*				set_property( Value** arg_list, int count );

		//def_generic(		show_props,	"showProperties" );
		//def_generic(		get_props,	"getPropNames" );
};

#endif		__BLURXML_H__