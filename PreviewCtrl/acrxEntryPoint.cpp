// (C) Copyright 2002-2012 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//

#include "StdAfx.h"
#include "resource.h"
#include "PreviewDialog.h"

#define szRDS _RXST("")

class CPreviewCtrlApp : public AcRxArxApp 
{
public:
	CPreviewCtrlApp () : AcRxArxApp () {}

	virtual AcRx::AppRetCode On_kInitAppMsg (void *pkt) 
	{
		AcRx::AppRetCode retCode = AcRxArxApp::On_kInitAppMsg(pkt);

		_previewDlg.Create(IDD_DIALOG_PREVIEW);
		_previewDlg.CenterWindow();

		return (retCode) ;
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg (void *pkt) 
	{
		AcRx::AppRetCode retCode =AcRxArxApp::On_kUnloadAppMsg (pkt) ;
		return (retCode) ;
	}

	virtual void RegisterServerComponents () 
	{
	}

	static void viewCtrlShowPreviewDlg() 
	{
		_previewDlg.init();
		_previewDlg.ShowWindow(SW_SHOW);
	}

private:
	static PreviewDialog _previewDlg;
} ;
PreviewDialog CPreviewCtrlApp::_previewDlg;

IMPLEMENT_ARX_ENTRYPOINT(CPreviewCtrlApp)
ACED_ARXCOMMAND_ENTRY_AUTO(CPreviewCtrlApp, viewCtrl, ShowPreviewDlg, ShowPreviewDlgLocal, ACRX_CMD_MODAL, NULL)

