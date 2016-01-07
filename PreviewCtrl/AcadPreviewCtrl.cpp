#include "stdafx.h"
#include "AcadPreviewCtrl.h"
#include "dbents.h"


AcadPreviewCtrl::AcadPreviewCtrl()
{
	_hPanCursor = NULL; // �ƶ�ʱ��ͼ��
	_hOrbitCursor = NULL; // ��תͼ��
	_view = NULL; // ͼ��ϵͳ�е���ͼ����������ͼ�ε�����
	_device = NULL; // ͼ��ϵͳ�е��豸��
	_model = NULL;
	_isPanning = false; // �Ƿ����ƶ�ͼ��״̬
	_isOrbiting = false; // �Ƿ�����תͼ��״̬
	_db = NULL; // ��Ԥ���ռ�󶨵����ݿ� 
}

AcadPreviewCtrl::~AcadPreviewCtrl()
{
	_clear();
}

void AcadPreviewCtrl::refresh_vdir(const AcGeVector3d &viewDir, const AcGeVector3d &upVector)
{
	_init_view(viewDir, upVector);
	OnPaint();
}

void AcadPreviewCtrl::refresh_render_mode(AcGsView::RenderMode mode)
{
	_view->setMode(mode);
	OnPaint();
}

void AcadPreviewCtrl::show(AcDbDatabase *db, const AcGeVector3d &vdir, const AcGeVector3d &upvec, AcGsView::RenderMode mode)
{
	_clear();
	_init(db, vdir, upvec, mode);
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
	ON_WM_MBUTTONDBLCLK()
	ON_WM_NCHITTEST()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// fslCadViewCtrl message handlers

void AcadPreviewCtrl::OnPaint()
{
	CPaintDC dc(this);
	//ˢ��ͼ��ϵͳ��ͼ
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
	//���ù����ʽ
	_isOrbiting = true;
	SetCapture();

	::SetClassLong(m_hWnd, GCL_HCURSOR, NULL);
	::SetCursor(_hOrbitCursor);

	_startPt = point;
}

//��������:�����ַŴ���С��ͼ
void AcadPreviewCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (_view == NULL)
		return;
	if (_isPanning)
	{
		//��ɴ��豸����ϵͳ����������ϵͳ��ת��
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

		// �������µ����λ�ü��������ʸ��
		AcGeVector3d last_vector((_startPt.x - centerX) / radius,
			-(_startPt.y - centerY) / radius,
			0.0);

		if (last_vector.lengthSqrd() > 1.0) // �����뾶��Χ
			last_vector.normalize();
		else
			last_vector.z = sqrt(1.0 - last_vector.x * last_vector.x - last_vector.y * last_vector.y);

		AcGeVector3d new_vector((point.x - centerX) / radius,
			-(point.y - centerY) / radius,
			0.0);

		if (new_vector.lengthSqrd() > 1.0) // �����뾶��Χ
			new_vector.normalize();
		else
			new_vector.z = sqrt(1.0 - new_vector.x * new_vector.x - new_vector.y * new_vector.y);

		// ȷ����������ĽǶ�
		AcGeVector3d rotation_vector(last_vector);
		rotation_vector = rotation_vector.crossProduct(new_vector); // rotation_vector = last_vector x new_vector

		AcGeVector3d work_vector(rotation_vector);
		work_vector.z = 0.0f; // rotation_vector��xyƽ���ͶӰ

		double roll_angle = atan2(work_vector.x, work_vector.y); // �������������ʸ���ǳ��ϵ�

		// �������ϵ�ʸ���͹���ʸ���ļн� 
		double length = rotation_vector.length();
		double pi = 3.1415926535897932384626433832795;
		double orbit_y_angle = (length != 0.0) ? acos(rotation_vector.z / length) + (pi / 2.0) : (pi / 2.0); // represents inverse cosine of the dot product of the
		if (length > 1.0f)
			length = 1.0f;

		double rotation_angle = asin(length);

		// view����
		_view->roll(roll_angle);
		_view->orbit(0.0f, orbit_y_angle);
		_view->orbit(rotation_angle, 0.0f);
		_view->orbit(0.0f, -orbit_y_angle);
		_view->roll(-roll_angle);
		Invalidate();
		_startPt = point;
	}
	else
		SetFocus();
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
	//��ʼ�ƶ�
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

void AcadPreviewCtrl::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	_init_view(_view->position() - _view->target(), _view->upVector());
	OnPaint();
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

//��������:��õ�ǰ�ӿڵ���Ϣ��
//�������:height �ӿڸ߶ȣ�width �ӿڿ�ȣ�target �ӿ����ĵ㣬viewDir �ӿڵĹ۲�������twist Ť�����ӿ�
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

//��������:��ʼ��ͼ��ϵͳ
void AcadPreviewCtrl::_init_gs(HINSTANCE hRes)
{
	// - ���ع��
	if (_hPanCursor == NULL)
		_hPanCursor = LoadCursor(hRes, MAKEINTRESOURCE(IDI_PAN));
	if (_hOrbitCursor == NULL)
		_hOrbitCursor = LoadCursor(hRes, MAKEINTRESOURCE(IDI_ORBIT));
	::SetClassLong(m_hWnd, GCL_HCURSOR, NULL);

	// - ��ʼ����ͼ
	// - ���ͼ��ϵͳ������
	AcGsManager *manager = acgsGetGsManager();
	assert(manager);

	// - ���ͼ��ϵͳ�๤��
	AcGsClassFactory *factory = manager->getGSClassFactory();
	assert(factory);

	// - ����ͼ��ϵͳ�豸
	_device = manager->createAutoCADDevice(m_hWnd);
	assert(_device);

	CRect rect;
	GetClientRect(&rect);

	_device->onSize(rect.Width(), rect.Height());
	// - ����ͼ��ϵͳ��ͼ
	_view = factory->createView();
	assert(_view);

	_model = manager->createAutoCADModel();
	assert(_model);

	_device->add(_view);
}

void AcadPreviewCtrl::_init(AcDbDatabase *db, const AcGeVector3d &vdir, const AcGeVector3d &upvec, AcGsView::RenderMode mode)
{
	_db = db;

	// - ��ȡ���ж���
	Acad::ErrorStatus es = Acad::eOk;
	AcDbBlockTableRecord *rcd = NULL;
	AcDbBlockTable *tbl = NULL;
	es = _db->getBlockTable(tbl, AcDb::kForRead);
	if (tbl == NULL)
		return;

	es = tbl->getAt(ACDB_MODEL_SPACE, rcd, AcDb::kForRead);
	tbl->close();
	if (rcd == NULL)
		return;

	AcDbObjectId rcdId = rcd->id();
	AcDbObjectIdArray ents;
	_get_ents(rcdId, ents);

	// - ��ȡ��ǰ��ͼ��Χ
	_get_extents(ents, _extents);

	// - ��ʼ��GS
	_init_gs(_hdllInstance);
	_view->add(rcd, _model);
	rcd->close();

	// - ���ó�ʼ��ͼ������viewdirction, upvector, rendermode��
	_init_view(vdir, upvec); // ���ϵȲ�ͼ
	_view->setMode(mode);

	return;
}

void AcadPreviewCtrl::_init_view(const AcGeVector3d &vdir, const AcGeVector3d &upvec)
{
	// - ��ȡ���ĵ�
	AcGePoint3d target;
	_mid(_extents.maxPoint(), _extents.minPoint(), target);

	// - ��ȡ��ǰDCS
	AcGeVector3d xaxis, yaxis = upvec, zaxis = vdir;
	xaxis = zaxis.crossProduct(yaxis);

	AcGeMatrix3d xform;
	xform.setCoordSystem(target, xaxis, yaxis, zaxis);
	xform.invert();

	// - ���㳤��Χ
	AcDbExtents extents = _extents;
	extents.transformBy(xform);
	double width = extents.maxPoint().x - extents.minPoint().x;
	double height = extents.maxPoint().y - extents.minPoint().y;

	// - ����view
	_view->setView(target + vdir, target, upvec, width * 1.05, height * 1.05);
}

void AcadPreviewCtrl::_mid(const AcGePoint3d& pt1, const AcGePoint3d& pt2, AcGePoint3d& ptMid)
{
	ptMid.x = 0.5 *(pt1.x + pt2.x);
	ptMid.y = 0.5 *(pt1.y + pt2.y);
	ptMid.z = 0.5 *(pt1.z + pt2.z);
}

//��������:��ÿ��е�����ʵ��
void AcadPreviewCtrl::_get_ents(const AcDbObjectId& idBlockRec, AcDbObjectIdArray& IDArray)
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

//��������:���ʵ��ķ�Χ
Acad::ErrorStatus AcadPreviewCtrl::_get_extents(const AcDbObjectId& idEnt, AcDbExtents& extents)
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


void AcadPreviewCtrl::_get_extents(const AcDbObjectIdArray& aridEnt, AcDbExtents& extents)
{
	for (int i = 0; i < aridEnt.length(); i++)
	{
		AcDbExtents tem;
		if (_get_extents(aridEnt[i], tem) == Acad::eOk)
		{
			extents.addExt(tem);
		}
	}
}
