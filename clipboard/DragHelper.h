#pragma once

#include <ShObjIdl.h>
#include <string>

/*
拖动辅助类，
author:churx

开始拖动通用步骤:
CreateDataObject
SetData|SetText|SetHtml ...
[SetDragImage]
[SetDragTip]
DoDrag

创建拖动的源控件
CControlUI::SetDragSource(true)

创建可接收拖动的控件
CreateDropTarget
CControlUI::SetDropTarget(true)

*/

typedef std::string String;
	
class CDragHelper{
public:
	/** 创建数据对象，表示拖动或复制粘贴时传输的对象 */
	static IDataObject* CreateDataObject();

	/** 设置拖动中的图片 */
	static bool SetDragImage(IDataObject* obj, HBITMAP hBmp, int w=0, int h=0);
	static bool SetDragImage(IDataObject* obj, LPCTSTR file, int w=0, int h=0);

	/** 设置拖动中的提示信息， 参数示例：lpMsg="移动到 %1" lpInsert="我的文档" effect=DROPEFFECT_COPY  */
	static bool SetDragTip(IDataObject* obj, LPCTSTR lpMsg, LPCTSTR lpInsert = NULL, DWORD effect=DROPEFFECT_MOVE );

	/** 设置数据对象（格式，数据，数据长度） */
	static bool SetData(IDataObject* obj, CLIPFORMAT cf, const void *pvBlob, UINT cbBlob);

	static bool SetData(IDataObject* obj, LPCTSTR lpType, LPCTSTR lpData);

    static String GetData(IDataObject* obj, LPCTSTR lpType);
	static String GetData(IDataObject* obj, CLIPFORMAT cf);

	static String GetText(IDataObject* obj);

	static String GetHtml(IDataObject* obj);

	/** 获取图片，注意返回的图片需要使用DeleteObject删除 */
	static HBITMAP GetImage(IDataObject* obj);

	/** 获取HDROP（拖动的多个文件），注意返回的对象需要使用DragFinish删除 */
	static HDROP GetFiles(IDataObject* obj);

	static bool SetText(IDataObject* obj, LPCTSTR lpStr);

	static bool SetHtml(IDataObject* obj, LPCTSTR lpStr);

	static bool SetImage(IDataObject* obj, HBITMAP hBmp);
	
	/** 该数据对象是否包含指定的格式 */
	static bool HasData(IDataObject* obj, CLIPFORMAT cf);	

	/** 获取剪切板格式 */
	static CLIPFORMAT GetClipboardFormat(LPCTSTR lpFormat);

	/** 对指定数据对象进行拖动  */ 
	static bool DoDrag(IDataObject* pObj);


	/** 当前时候是否允许拖入，用于dragover中处理，判断数据对象是否支持，再调用是否允许拖入 */
    static void EnableDrop(bool enable);

};

