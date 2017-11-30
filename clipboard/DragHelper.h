#pragma once

#include <ShObjIdl.h>
#include <string>

/*
�϶������࣬
author:churx

��ʼ�϶�ͨ�ò���:
CreateDataObject
SetData|SetText|SetHtml ...
[SetDragImage]
[SetDragTip]
DoDrag

�����϶���Դ�ؼ�
CControlUI::SetDragSource(true)

�����ɽ����϶��Ŀؼ�
CreateDropTarget
CControlUI::SetDropTarget(true)

*/

typedef std::string String;
	
class CDragHelper{
public:
	/** �������ݶ��󣬱�ʾ�϶�����ճ��ʱ����Ķ��� */
	static IDataObject* CreateDataObject();

	/** �����϶��е�ͼƬ */
	static bool SetDragImage(IDataObject* obj, HBITMAP hBmp, int w=0, int h=0);
	static bool SetDragImage(IDataObject* obj, LPCTSTR file, int w=0, int h=0);

	/** �����϶��е���ʾ��Ϣ�� ����ʾ����lpMsg="�ƶ��� %1" lpInsert="�ҵ��ĵ�" effect=DROPEFFECT_COPY  */
	static bool SetDragTip(IDataObject* obj, LPCTSTR lpMsg, LPCTSTR lpInsert = NULL, DWORD effect=DROPEFFECT_MOVE );

	/** �������ݶ��󣨸�ʽ�����ݣ����ݳ��ȣ� */
	static bool SetData(IDataObject* obj, CLIPFORMAT cf, const void *pvBlob, UINT cbBlob);

	static bool SetData(IDataObject* obj, LPCTSTR lpType, LPCTSTR lpData);

    static String GetData(IDataObject* obj, LPCTSTR lpType);
	static String GetData(IDataObject* obj, CLIPFORMAT cf);

	static String GetText(IDataObject* obj);

	static String GetHtml(IDataObject* obj);

	/** ��ȡͼƬ��ע�ⷵ�ص�ͼƬ��Ҫʹ��DeleteObjectɾ�� */
	static HBITMAP GetImage(IDataObject* obj);

	/** ��ȡHDROP���϶��Ķ���ļ�����ע�ⷵ�صĶ�����Ҫʹ��DragFinishɾ�� */
	static HDROP GetFiles(IDataObject* obj);

	static bool SetText(IDataObject* obj, LPCTSTR lpStr);

	static bool SetHtml(IDataObject* obj, LPCTSTR lpStr);

	static bool SetImage(IDataObject* obj, HBITMAP hBmp);
	
	/** �����ݶ����Ƿ����ָ���ĸ�ʽ */
	static bool HasData(IDataObject* obj, CLIPFORMAT cf);	

	/** ��ȡ���а��ʽ */
	static CLIPFORMAT GetClipboardFormat(LPCTSTR lpFormat);

	/** ��ָ�����ݶ�������϶�  */ 
	static bool DoDrag(IDataObject* pObj);


	/** ��ǰʱ���Ƿ��������룬����dragover�д����ж����ݶ����Ƿ�֧�֣��ٵ����Ƿ��������� */
    static void EnableDrop(bool enable);

};

