/*!
	\file		MaxFileProperties.h

	\remarks	Maxscript extension to have access to max file OLE container properties
	
	\author		Diego Garcia Huerta
	\author		Email: diego@blur.com
	\author		Company: Blur Studio
	\date		05/01/07

	\history
				- version 1.0 DGH 05/03/07: Created

	\note
				Copyright (c) 2006, Blur Studio Inc.
				All rights reserved.

				Redistribution and use in source and binary forms, with or without 
				modification, are permitted provided that the following conditions 
				are met:

					* Redistributions of source code must retain the above copyright 
					notice, this list of conditions and the following disclaimer.
					* Redistributions in binary form must reproduce the above 
					copyright notice, this list of conditions and the following 
					disclaimer in the documentation and/or other materials provided 
					with the distribution.
					* Neither the name of the Blur Studio Inc. nor the names of its 
					contributors may be used to endorse or promote products derived 
					from this software without specific prior written permission.

				THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
				"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
				LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
				FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
				COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
				INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
				BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
				LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
				CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
				LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
				ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
				POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _H_MAXFILEPROPERTIES_CLASS
#define _H_MAXFILEPROPERTIES_CLASS


visible_class (MaxFileProperties)


class MaxFileProperties : public Value
{
public:
	Array*	m_summaryInfo;
	Array*	m_documentSummaryInfo;
	Array*	m_general;
	Array*	m_meshTotals;
	Array*	m_sceneTotals;
	Array*	m_externalDependences;
	Array*	m_objects;
	Array*	m_materials;
	Array*	m_usedPlugins;
	Array*	m_renderData;
	Array*	m_userDefinedProperties;

	TSTR m_filename;
	TSTR m_title;
	TSTR m_subject;
	TSTR m_author;
	TSTR m_keywords;
	TSTR m_comments;

	TSTR m_manager;
	TSTR m_company;
	TSTR m_category;
	
	Array* m_UserDefinedProperties;

	MaxFileProperties();

	classof_methods(MaxFileProperties, Value);
	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	void		gc_trace();
#	define		is_MaxFileProperties(p) ((p)->tag == class_tag(MaxFileProperties))

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	def_generic (show_props, "showProperties");
	def_generic (get_props,  "getPropNames");
};

#endif //_H_MAXFILEPROPERTIES_CLASS
