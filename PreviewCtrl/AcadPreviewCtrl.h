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
	void _initGS(HINSTANCE hRes); // 初始化图形系统
	BOOL _initInner(AcDbDatabase *pDb); // 内部初始化
	void _init_view(const AcGeVector3d &viewDir, const AcGeVector3d &upVector);
	bool _getActiveViewPortInfo(ads_real &height, ads_real &width, AcGePoint3d &target,
		AcGeVector3d &viewDir, ads_real &viewTwist, bool getViewCenter);
	//获得块中的所有实体
	void _getAllEnts(const AcDbObjectId& idBlockRec, AcDbObjectIdArray& IDArray);
	//获得实体的范围
	Acad::ErrorStatus _getEntExtents(const AcDbObjectId& idEnt, AcDbExtents& extents);
	void _getEntExtents(const AcDbObjectIdArray& aridEnt, AcDbExtents& extents);
	void _mid(const AcGePoint3d& pt1, const AcGePoint3d& pt2, AcGePoint3d& ptMid);

private:
	AcDbExtents _extents; // 图纸范围
	HCURSOR _hPanCursor; // 移动时的图标
	HCURSOR _hOrbitCursor; // 旋转图标
	AcGsView *_view; // 图形系统中的视图，用来绘制图形的区域
	AcGsDevice *_device; // 图形系统中的设备，
	AcGsModel *_model;
	bool _isPanning; // 是否处于移动图形状态
	bool _isOrbiting; // 是否处于旋转图形状态
	AcDbDatabase *_db; // 该预览空间绑定的数据库 
	CPoint _startPt; // 移动或旋转时的起点
};

