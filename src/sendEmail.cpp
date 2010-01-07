/*!
	\file		sendEmail.cpp

	\remarks	Maxscrip extension to send emails
	
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

#include "MAXScrpt.h"
#include "Rollouts.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "MAXclses.h"
#include "Parser.h"


#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

//#include "MaxFileProperties.h"
//#include "Resource.h"

#include "defextfn.h"
	//def_name ( maxfileproperties )

#include "definsfn.h"
	def_name ( sender )
//	def_name ( from )
	def_name ( attachment )

/*
    Copyright(C) Nishant Sivakumar (nish#voidnish.com)
    Please maintain this copyright notice if you distribute this file
    in original or modified form.
*/

//#include "StdAfx.h"
#include "sendEmail.h"

BOOL CSMTPConnection2::Connect(LPCTSTR lpszHostDomain, 
                               DWORD dwTimeout /*= 10000*/) throw()
{
    CSimpleArray<CString> arrSMTP;
    _GetSMTPList(lpszHostDomain, arrSMTP);
    for(int i=0; i<arrSMTP.GetSize(); i++)
    {
        if(CSMTPConnection::Connect(arrSMTP[i], dwTimeout) == TRUE)
            return TRUE;
    }
    return FALSE;
}

void CSMTPConnection2::_GetSMTPList(LPCTSTR lpszHostDomain, 
                                   CSimpleArray<CString>& arrSMTP)
{
    PDNS_RECORD pRec = NULL;
    if(DnsQuery(lpszHostDomain, DNS_TYPE_MX, DNS_QUERY_STANDARD,
        NULL, &pRec, NULL) == ERROR_SUCCESS)
    {
        PDNS_RECORD pRecOrig = pRec;
        while(pRec)
        {
            if(pRec->wType == DNS_TYPE_MX)
                arrSMTP.Add(pRec->Data.MX.pNameExchange);
            pRec = pRec->pNext;
        }
        DnsRecordListFree(pRecOrig,DnsFreeRecordList);
    }
} 

//sendemail "diego@blur.com" "test@blur.com" "testing notifications" "This is a test (body)" "The notification system" "mail.blur.com" ""
def_visible_primitive( sendEmail, "sendEmail");

Value* sendEmail_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (sendEmail, 4, count);

	TCHAR *to			= arg_list[0]->to_string();
	TCHAR *subject		= arg_list[1]->to_string();
	TCHAR *body			= arg_list[2]->to_string();
	TCHAR *domainName	= arg_list[3]->to_string();

	TCHAR *sender		= _T("");
	TCHAR *senderName	= _T("");
	TCHAR *attachment	= _T("");

	Value* senderValue		= key_arg(sender);
	Value* senderNameValue	= key_arg(from);
	Value* attachmentValue	= key_arg(attachment);

	if (senderValue != &unsupplied)
		sender = senderValue->to_string();

	if (senderNameValue != &unsupplied)
		senderName = senderNameValue->to_string();

	CMimeMessage msg;
	msg.SetSender(CA2CT(sender));
	msg.SetSenderName(CA2CT(senderName));
	msg.AddRecipient(CA2CT(to));
	msg.SetPriority(ATL_MIME_HIGH_PRIORITY);

	//Optional
	msg.SetSubject(CA2CT(subject));
	msg.AddText(CA2CT(body));

	if (attachmentValue != &unsupplied)
	{
		if (attachmentValue->is_kind_of(class_tag(Array)))
		{
			Array* attachArray = (Array*)attachmentValue;
			for (int i =0; i < attachArray->size; i++)
			{
				attachment = attachArray->data[i]->to_string();
				msg.AttachFile(attachment);				
			}
		}
		else
		{
			attachment = attachmentValue->to_string();
			msg.AttachFile(attachment);
		}
	}

	CSMTPConnection2 conn;

	//You need to specifwy the domain name of the recipient email address
	BOOL mailSentSucc = FALSE;

	if(conn.Connect(domainName))
	{
		mailSentSucc = conn.SendMessage(msg);
		conn.Disconnect();    
	}

	return mailSentSucc ? &true_value:&false_value;
}

void sendEmailInit()
{
	#include "defimpfn.h"
		def_name ( sender )
//		def_name ( from )
		def_name ( attachment )
}