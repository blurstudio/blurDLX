/*
    Copyright(C) Nishant Sivakumar (nish#voidnish.com)
    Please maintain this copyright notice if you distribute this file
    in original or modified form.
*/

#pragma once
#include <atlsmtpconnection.h>
#include <Windns.h>

#pragma comment(lib,"Dnsapi.lib")

class CSMTPConnection2 : public CSMTPConnection
{
public:    
    BOOL Connect(LPCTSTR lpszHostDomain, DWORD dwTimeout = 10000) throw();
private:
    void _GetSMTPList(LPCTSTR lpszHostDomain, CSimpleArray<CString>& arrSMTP);
};