#pragma once
#include "Resource.h"
#include "AcadPreviewCtrl.h"
#include "afxwin.h"


class PreviewDialog : public CAcUiDialog
{
	DECLARE_DYNAMIC(PreviewDialog)

public:
	PreviewDialog(CWnd* pParent = NULL);
	virtual ~PreviewDialog();

	enum { IDD = IDD_DIALOG_PREVIEW };
	void init();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	LRESULT OnAcadKeepFocus(WPARAM, LPARAM);
private:
	AcadPreviewCtrl _ctrl;
};
