#include "stdafx.h"
#include "AcadPreviewCtrl.h"
#include "dbents.h"


AcadPreviewCtrl::AcadPreviewCtrl()
{
	_hPanCursor = NULL; // 移动时的图标
	_hOrbitCursor = NULL; // 旋转图标
	_view = NULL; // 图形系统中的视图，用来绘制图形的区域
	_device = NULL; // 图形系统中的设备，
	_model = NULL;
	_isPanning = false; // 是否处于移动图形状态
	_isOrbiting = false; // 是否处于旋转图形状态
	_db = NULL; // 该预览空间绑定的数据库 
}

AcadPreviewCtrl::~AcadPreviewCtrl()
{
	_clear();
}

//函数功能:传入数据库指针即可预览数据库中的实体
BOOL AcadPreviewCtrl::init(AcDbDatabase *pDb)
{
	_clear();
	return _initInner(pDb);
}

void AcadPreviewCtrl::set_view_dir(const AcGeVector3d &viewDir, const AcGeVector3d &upVector)
{
	_init_view(viewDir, upVector);
}

void AcadPreviewCtrl::set_render_mode(AcGsView::RenderMode mode)
{
	_view->setMode(mode);
	OnPaint();
}

BEGIN_MESSAGE_MAP(AcadPreviewCtrl, CStatic)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_NCHITTEST()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// fslCadViewCtrl message handlers

void AcadPreviewCtrl::OnPaint()
{
	CPaintDC dc(this);
	//刷新图形系统视图
	if (_view)
	{
		_view->invalidate();
		_view->update();
	}
}

void AcadPreviewCtrl::OnSize(UINT nType, int cx, int cy)
{
	CRect rect;
	if (_device)
	{
		GetClientRect(&rect);
		_device->onSize(rect.Width(), rect.Height());
	}
}

void AcadPreviewCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	//设置光标样式
	_isOrbiting = true;
	SetCapture();

	::SetClassLong(m_hWnd, GCL_HCURSOR, NULL);
	::SetCursor(_hOrbitCursor);

	_startPt = point;
}

//函数功能:鼠标滚轮放大缩小视图
void AcadPreviewCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (_view == NULL)
		return;
	if (_isPanning)
	{
		//完成从设备坐标系统到世界坐标系统的转换
		AcGeVector3d pan_vec(-(point.x - _startPt.x), point.y - _startPt.y, 0);
		pan_vec.transformBy(_view->viewingMatrix() * _view->worldToDeviceMatrix().inverse());
		_view->dolly(pan_vec);
		Invalidate();
		_startPt = point;
	}
	else if (_isOrbiting)
	{
		AcGsDCRect view_rect;
		_view->getViewport(view_rect);

		int nViewportX = (view_rect.m_max.x - view_rect.m_min.x) + 1;
		int nViewportY = (view_rect.m_max.y - view_rect.m_min.y) + 1;

		int centerX = int(nViewportX / 2.0f) + view_rect.m_min.x;
		int centerY = int(nViewportY / 2.0f) + view_rect.m_min.y;

		const double radius = min(nViewportX, nViewportY) * 0.4f;

		// 从最后和新的鼠标位置计算出两个矢量
		AcGeVector3d last_vector((_startPt.x - centerX) / radius,
			-(_startPt.y - centerY) / radius,
			0.0);

		if (last_vector.lengthSqrd() > 1.0) // 超出半径范围
			last_vector.normalize();
		else
			last_vector.z = sqrt(1.0 - last_vector.x * last_vector.x - last_vector.y * last_vector.y);

		AcGeVector3d new_vector((point.x - centerX) / radius,
			-(point.y - centerY) / radius,
			0.0);

		if (new_vector.lengthSqrd() > 1.0) // 超出半径范围
			new_vector.normalize();
		else
			new_vector.z = sqrt(1.0 - new_vector.x * new_vector.x - new_vector.y * new_vector.y);

		// 确定相机操作的角度
		AcGeVector3d rotation_vector(last_vector);
		rotation_vector = rotation_vector.crossProduct(new_vector); // rotation_vector = last_vector x new_vector

		AcGeVector3d work_vector(rotation_vector);
		work_vector.z = 0.0f; // rotation_vector到xy平面的投影

		double roll_angle = atan2(work_vector.x, work_vector.y); // 假设相机的向上矢量是朝上的

		// 计算向上的矢量和工作矢量的夹角 
		double length = rotation_vector.length();
		double pi = 3.1415926535897932384626433832795;
		double orbit_y_angle = (length != 0.0) ? acos(rotation_vector.z / length) + (pi / 2.0) : (pi / 2.0); // represents inverse cosine of the dot product of the
		if (length > 1.0f)
			length = 1.0f;

		double rotation_angle = asin(length);

		// view操作
		_view->roll(roll_angle);
		_view->orbit(0.0f, orbit_y_angle);
		_view->orbit(rotation_angle, 0.0f);
		_view->orbit(0.0f, -orbit_y_angle);
		_view->roll(-roll_angle);
		Invalidate();
		_startPt = point;
	}
}

BOOL AcadPreviewCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (_view)
	{
		if (zDelta < 0)
			_view->zoom(0.5);
		else
			_view->zoom(1.5);

		Invalidate();
	}
	return TRUE;
}

void AcadPreviewCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{
	//开始移动
	_isPanning = true;
	SetCapture();

	::SetClassLong(m_hWnd, GCL_HCURSOR, NULL);
	::SetCursor(_hPanCursor);

	_startPt = point;
}

void AcadPreviewCtrl::OnMButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	_isPanning = false;
}

afx_msg LRESULT AcadPreviewCtrl::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

void AcadPreviewCtrl::OnSetFocus(CWnd* pOldWnd)
{
}

void AcadPreviewCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	_isOrbiting = false;
	ReleaseCapture();
}


void AcadPreviewCtrl::_clear()
{
	AcGsManager *manager = acgsGetGsManager();
	if (manager == NULL)
		return;

	if (_view)
	{
		_view->eraseAll();
		if (_device)
			_device->erase(_view);

		AcGsClassFactory *pFactory = manager->getGSClassFactory();
		pFactory->deleteView(_view);
		_view = NULL;
	}

	if (_model)
	{
		manager->destroyAutoCADModel(_model);
		_model = NULL;
	}

	if (_device)
	{
		manager->destroyAutoCADDevice(_device);
		_device = NULL;
	}

	if (_db)
		_db = NULL;
}

//函数功能:获得当前视口的信息。
//输出参数:height 视口高度，width 视口宽度，target 视口中心点，viewDir 视口的观察向量，twist 扭曲的视口
bool AcadPreviewCtrl::_getActiveViewPortInfo(ads_real &height, ads_real &width,
	AcGePoint3d &target, AcGeVector3d &viewDir,
	ads_real &viewTwist, bool getViewCenter)
{
	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();

	if (pDb == NULL)
		return false;

	AcDbViewportTable *pVTable = NULL;
	Acad::ErrorStatus es = pDb->getViewportTable(pVTable, AcDb::kForRead);

	if (es == Acad::eOk)
	{
		AcDbViewportTableRecord *pViewPortRec = NULL;
		es = pVTable->getAt(_T("*Active"), pViewPortRec, AcDb::kForRead);
		if (es == Acad::eOk)
		{
			height = pViewPortRec->height();
			width = pViewPortRec->width();

			if (getViewCenter == true)
			{
				struct resbuf rb;
				memset(&rb, 0, sizeof(struct resbuf));
				acedGetVar(_T("VIEWCTR"), &rb);

				target = AcGePoint3d(rb.resval.rpoint[X], rb.resval.rpoint[Y], rb.resval.rpoint[Z]);
			}
			else
			{

				target = pViewPortRec->target();
			}

			viewDir = pViewPortRec->viewDirection();

			viewTwist = pViewPortRec->viewTwist();
		}
		pVTable->close();
		pViewPortRec->close();
	}

	return (true);
}

//函数功能:初始化图形系统
void AcadPreviewCtrl::_initGS(HINSTANCE hRes)
{
	// 加载光标
	if (_hPanCursor == NULL)
		_hPanCursor = LoadCursor(hRes, MAKEINTRESOURCE(IDI_PAN));
	if (_hOrbitCursor == NULL)
		_hOrbitCursor = LoadCursor(hRes, MAKEINTRESOURCE(IDI_ORBIT));
	::SetClassLong(m_hWnd, GCL_HCURSOR, NULL);

	// 初始化视图

	// 获得图形系统管理器
	AcGsManager *pGsManager = acgsGetGsManager();
	assert(pGsManager);
	// 获得图形系统类工厂
	AcGsClassFactory *pFactory = pGsManager->getGSClassFactory();
	assert(pFactory);

	// 创建图形系统设备
	_device = pGsManager->createAutoCADDevice(m_hWnd);
	assert(_device);

	CRect rect;
	GetClientRect(&rect);

	_device->onSize(rect.Width(), rect.Height());
	// 创建图形系统视图
	_view = pFactory->createView();
	assert(_view);

	_model = pGsManager->createAutoCADModel();
	assert(_model);

	_device->add(_view);
}

BOOL AcadPreviewCtrl::_initInner(AcDbDatabase *pDb)
{
	if (pDb == NULL)
		return FALSE;
	_db = pDb;

	Acad::ErrorStatus es = Acad::eOk;
	AcDbBlockTableRecord *pRec = NULL;
	AcDbBlockTable *pTab = NULL;
	if ((es = _db->getBlockTable(pTab, AcDb::kForRead)) != Acad::eOk)
		return FALSE;

	if ((es = pTab->getAt(ACDB_MODEL_SPACE, pRec, AcDb::kForRead)) != Acad::eOk)
	{
		pTab->close();
		return FALSE;
	}
	pTab->close();

	AcDbObjectId idRec = pRec->id();
	AcDbObjectIdArray aridEnt;
	_getAllEnts(idRec, aridEnt);

	_getEntExtents(aridEnt, _extents);

	_initGS(_hdllInstance);

	_view->add(pRec, _model);

	pRec->close();

	_init_view(AcGeVector3d(1, -1, 1), AcGeVector3d(0, 0, 1)); // 东南等测图

	_view->setMode(AcGsView::kFlatShadedWithWireframe);

	return TRUE;
}

void AcadPreviewCtrl::_init_view(const AcGeVector3d &viewDir, const AcGeVector3d &upVector)
{
	// - 获取中心点
	AcGePoint3d target;
	_mid(_extents.maxPoint(), _extents.minPoint(), target);

	// - 获取当前DCS
	AcGeVector3d xaxis, yaxis = upVector, zaxis = viewDir;
	xaxis = zaxis.crossProduct(yaxis);

	AcGeMatrix3d xform;
	xform.setCoordSystem(target, xaxis, yaxis, zaxis);
	xform.invert();

	// - 计算长宽范围
	AcDbExtents extents = _extents;
	extents.transformBy(xform);
	double width = extents.maxPoint().x - extents.minPoint().x;
	double height = extents.maxPoint().y - extents.minPoint().y;

	// - 设置view
	_view->setView(target + viewDir, target, upVector, width * 1.05, height * 1.05);

	// - 重绘
	OnPaint();
}

void AcadPreviewCtrl::_mid(const AcGePoint3d& pt1, const AcGePoint3d& pt2, AcGePoint3d& ptMid)
{
	ptMid.x = 0.5 *(pt1.x + pt2.x);
	ptMid.y = 0.5 *(pt1.y + pt2.y);
	ptMid.z = 0.5 *(pt1.z + pt2.z);
}

//函数功能:获得块中的所有实体
void AcadPreviewCtrl::_getAllEnts(const AcDbObjectId& idBlockRec, AcDbObjectIdArray& IDArray)
{
	IDArray.setPhysicalLength(0);

	Acad::ErrorStatus es;

	AcDbBlockTableRecord *pBlkRec = NULL;
	if (Acad::eOk != (es = acdbOpenObject(pBlkRec, idBlockRec, AcDb::kForRead)))
	{
		return;
	}

	AcDbBlockTableRecordIterator *pIt = NULL;
	pBlkRec->newIterator(pIt);
	pBlkRec->close();

	for (pIt->start(); !pIt->done(); pIt->step())
	{
		AcDbObjectId idEnt;
		if (Acad::eOk == pIt->getEntityId(idEnt))
		{
			IDArray.append(idEnt);
		}
	}
	delete pIt;
	pIt = NULL;
}

//函数功能:获得实体的范围
Acad::ErrorStatus AcadPreviewCtrl::_getEntExtents(const AcDbObjectId& idEnt, AcDbExtents& extents)
{
	Acad::ErrorStatus es;
	AcDbEntity *pEnt = NULL;
	if (Acad::eOk == acdbOpenObject(pEnt, idEnt, AcDb::kForRead))
	{
		AcDbBlockReference *pBlkRef = AcDbBlockReference::cast(pEnt);
		if (pBlkRef)
		{
			es = pBlkRef->geomExtentsBestFit(extents);
		}
		else
		{
			es = pEnt->getGeomExtents(extents);
		}
		pEnt->close();
	}

	return (es);
}


void AcadPreviewCtrl::_getEntExtents(const AcDbObjectIdArray& aridEnt, AcDbExtents& extents)
{
	for (int i = 0; i < aridEnt.length(); i++)
	{
		AcDbExtents tem;
		if (_getEntExtents(aridEnt[i], tem) == Acad::eOk)
		{
			extents.addExt(tem);
		}
	}
}
