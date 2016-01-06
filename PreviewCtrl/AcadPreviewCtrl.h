#pragma once
#include "resource.h"
#include "afxwin.h"
#include "acgi.h"
#include <math.h>
#include "gs.h"
#include "acgs.h"
#include "acgsmanager.h"

#define GCL_HCURSOR         (-12)
class AcadPreviewCtrl : public CStatic
{
public:
	AcadPreviewCtrl();
	virtual ~AcadPreviewCtrl();

	BOOL init(AcDbDatabase *pDb);
	void set_view_dir(const AcGeVector3d &vec, const AcGeVector3d &upVector);
	void set_render_mode(AcGsView::RenderMode mode);
private:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	void _clear();
	void _initGS(HINSTANCE hRes); // ��ʼ��ͼ��ϵͳ
	BOOL _initInner(AcDbDatabase *pDb); // �ڲ���ʼ��
	void _init_view(const AcGeVector3d &viewDir, const AcGeVector3d &upVector);
	bool _getActiveViewPortInfo(ads_real &height, ads_real &width, AcGePoint3d &target,
		AcGeVector3d &viewDir, ads_real &viewTwist, bool getViewCenter);
	//��ÿ��е�����ʵ��
	void _getAllEnts(const AcDbObjectId& idBlockRec, AcDbObjectIdArray& IDArray);
	//���ʵ��ķ�Χ
	Acad::ErrorStatus _getEntExtents(const AcDbObjectId& idEnt, AcDbExtents& extents);
	void _getEntExtents(const AcDbObjectIdArray& aridEnt, AcDbExtents& extents);
	void _mid(const AcGePoint3d& pt1, const AcGePoint3d& pt2, AcGePoint3d& ptMid);

private:
	AcDbExtents _extents; // ͼֽ��Χ
	HCURSOR _hPanCursor; // �ƶ�ʱ��ͼ��
	HCURSOR _hOrbitCursor; // ��תͼ��
	AcGsView *_view; // ͼ��ϵͳ�е���ͼ����������ͼ�ε�����
	AcGsDevice *_device; // ͼ��ϵͳ�е��豸��
	AcGsModel *_model;
	bool _isPanning; // �Ƿ����ƶ�ͼ��״̬
	bool _isOrbiting; // �Ƿ�����תͼ��״̬
	AcDbDatabase *_db; // ��Ԥ���ռ�󶨵����ݿ� 
	CPoint _startPt; // �ƶ�����תʱ�����
};

