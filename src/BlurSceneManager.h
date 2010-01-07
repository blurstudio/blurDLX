#ifndef __BLURSCENEMANAGER_H__
#define	__BLURSCENEMANAGER_H__

#include "GenericMethod.h"
#include "BlurXML.h"

class TableOfContents : public AppliedValue {
	private:
		XMLDocument*	_document;
		XMLNode*		_toc;
		float			_version;
		float			_loadVersion;

		Array*			_nodes( XMLNode* xmlNode );
		Array*			_nodesByProperty( TSTR propName, CComBSTR propVal, XMLNode* xmlNode );
		Array*			_propertiesByName( TSTR propName, XMLNode* xmlNode );

	public:
						TableOfContents();
						~TableOfContents();


		XMLDocument*	document()						{ return this->_document; }
		bool			isInitialized()					{ return this->_toc != NULL; }
		Array*			layers();
		bool			loadFromFile( TSTR fileName );
		Array*			nodes();
		Array*			nodesByProperty( TSTR propName, TSTR propValue );
		Array*			propertiesByName( TSTR propName );
		void			recordClassInfo( INode* node, XMLNode* xmlNode );
		void			recordLayerInfo( TimeValue atTime = 0 );
		void			recordNodeInfo( INode* node, XMLNode* xmlParent, TimeValue atTime = 0 );
		void			recordUserInfo();
		bool			saveToFile( TSTR fileName );
		XMLNode*		toc()							{ return this->_toc; }

		//--------------------------	MAXScript Methods	-----------------------------
		void			collect()						{ delete this; }
		void			sprin1( CharStream* s );
		void			gc_trace();
		Value*			applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );
		Value*			get_property( Value** arg_list, int count );
		Value*			set_property( Value** arg_list, int count );
};

#endif	__BLURSCENEMANAGER_H__