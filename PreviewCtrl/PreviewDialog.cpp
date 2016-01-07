#include "stdafx.h"
#include "PreviewDialog.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(PreviewDialog, CDialogEx)

PreviewDialog::PreviewDialog(CWnd* pParent)
: CAcUiDialog(PreviewDialog::IDD, pParent)
{

}

PreviewDialog::~PreviewDialog()
{
}

void PreviewDialog::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, _ctrl);
}


BEGIN_MESSAGE_MAP(PreviewDialog, CAcUiDialog)
	ON_MESSAGE(WM_ACAD_KEEPFOCUS, OnAcadKeepFocus)
END_MESSAGE_MAP()

//----- Needed for modeless dialogs to keep focus.
//----- Return FALSE to not keep the focus, return TRUE to keep the focus
LRESULT PreviewDialog::OnAcadKeepFocus(WPARAM, LPARAM) 
{
	return (TRUE);
}

void PreviewDialog::init()
{
	_ctrl.show(curDoc()->database());
}

