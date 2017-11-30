#define _WIN32_WINNT 0x0600

#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <ShlObj.h>
#include <comdef.h>
#include <comip.h>
#include <stdio.h>
#include <unordered_map>
#include <string>
#include <gdiplus.h>
#include "DragHelper.h"

#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"gdiplus.lib")

#if defined(__IDropTargetHelper_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IDropTargetHelper, __uuidof(IDropTargetHelper));
#endif
#if defined(__IDragSourceHelper_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IDragSourceHelper, __uuidof(IDragSourceHelper));
#endif
#if defined(__IDragSourceHelper2_INTERFACE_DEFINED__)
_COM_SMARTPTR_TYPEDEF(IDragSourceHelper2, __uuidof(IDragSourceHelper2));
#endif


static bool g_bEnableDrop;



class CommonDataObject:public IDataObject{
public:
	CommonDataObject(){
		//DUITRACE(_T("CommonDataObject()"));
		m_lRefCount = 1;
	}

	~CommonDataObject(){
		//DUITRACE(_T("~CommonDataObject()"));	
		for(auto it = m_stgs.begin() ; it != m_stgs.end(); it++){
			ReleaseStgMedium(&(*it));			
		}		
	}
public:
	// ============================ IUnknown ============================
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject){
		static const QITAB qit[] = {
			QITABENT(CommonDataObject, IDataObject),
			{ 0 }
		};
		return QISearch(this, qit, riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef(){
		return InterlockedIncrement(&m_lRefCount);
	}
	ULONG STDMETHODCALLTYPE Release(){
		LONG lRef = InterlockedDecrement(&m_lRefCount);
		if (0 == lRef) {
			delete this;
		}
		return m_lRefCount;
	}

	

	// ============================ IDataObject ============================
	HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium){
		if(!pformatetcIn || !pmedium )return E_INVALIDARG;
		int index = 0;
		int finded = -1;
		for(auto it = m_fmts.begin() ; it != m_fmts.end(); it++){
			if((*it).cfFormat == pformatetcIn->cfFormat){				
				finded = index;
				break;
			}
			index ++;
		}
		if(finded >= 0){ //该类型数据原来已经存在
			return CopyMedium(pmedium, &m_stgs[finded], pformatetcIn);
		}
		return DV_E_FORMATETC;
	}
        
	HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *pformatetc,STGMEDIUM *pmedium){
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *pformatetc){
		if(!pformatetc)return E_INVALIDARG;
		for(auto it = m_fmts.begin() ; it != m_fmts.end(); it++){
			if((*it).cfFormat == pformatetc->cfFormat){				
				return S_OK;
			}
		}
		return S_FALSE;
	}
        
	HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *pFormatectIn,FORMATETC *pFormatetcOut){
		if(!pFormatectIn || !pFormatetcOut)return  E_INVALIDARG;
        *pFormatetcOut = *pFormatectIn;
        pFormatetcOut->ptd = NULL;
        return DATA_S_SAMEFORMATETC;
	}
        
	HRESULT STDMETHODCALLTYPE SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease){
		if(!pformatetc || !pmedium )return E_INVALIDARG;
		int index = 0;
		int finded = -1;
		for(auto it = m_fmts.begin() ; it != m_fmts.end(); it++){
			if((*it).cfFormat == pformatetc->cfFormat){				
				finded = index;
				break;
			}
			index ++;
		}
		if(finded >= 0){ //该类型数据原来已经存在
			ReleaseStgMedium(&m_stgs[finded]);
			m_fmts[finded] = *pformatetc;			
			if(fRelease){
				m_stgs[finded] = *pmedium;
			}else{
				return CopyMedium(&m_stgs[finded], pmedium, pformatetc);  
			}
		}else{ //该类型数据原来不存在					
			if(fRelease){
				m_fmts.push_back(*pformatetc);	
				m_stgs.push_back(*pmedium);
			}else{
				STGMEDIUM stg={0};
				HRESULT hr = CopyMedium(&stg, pmedium, pformatetc);
				if(FAILED(hr))return hr;
				m_stgs.push_back(stg);
				m_fmts.push_back(*pformatetc);	
			}			
		}
		return S_OK;
	}
        
	HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC **ppenumFormatEtc){
		if ( ppenumFormatEtc &&  dwDirection == DATADIR_GET){
			return SHCreateStdEnumFmtEtc(m_fmts.size(), m_fmts.data() , ppenumFormatEtc);
		}	
		return E_INVALIDARG;	
	}

	HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *pformatetc,DWORD advf,IAdviseSink *pAdvSink,DWORD *pdwConnection){
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection){
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **ppenumAdvise){
		return E_NOTIMPL;
	}

protected:
	HRESULT CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc)  
	{  
		if ( (NULL == pMedDest) || (NULL ==pMedSrc) || (NULL == pFmtSrc) )  
		{  
			return E_INVALIDARG;  
		}  
		switch(pMedSrc->tymed)  
		{  
		case TYMED_HGLOBAL:  
			pMedDest->hGlobal = (HGLOBAL)OleDuplicateData(pMedSrc->hGlobal, pFmtSrc->cfFormat, NULL);  
			break;  
		case TYMED_GDI:  
			pMedDest->hBitmap = (HBITMAP)OleDuplicateData(pMedSrc->hBitmap, pFmtSrc->cfFormat, NULL);  
			break;  
		case TYMED_MFPICT:  
			pMedDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData(pMedSrc->hMetaFilePict, pFmtSrc->cfFormat, NULL);  
			break;  
		case TYMED_ENHMF:  
			pMedDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData(pMedSrc->hEnhMetaFile, pFmtSrc->cfFormat, NULL);  
			break;  
		case TYMED_FILE:  
			pMedSrc->lpszFileName = (LPOLESTR)OleDuplicateData(pMedSrc->lpszFileName, pFmtSrc->cfFormat, NULL);  
			break;  
		case TYMED_ISTREAM:  
			pMedDest->pstm = pMedSrc->pstm;  
			pMedSrc->pstm->AddRef();  
			break;  
		case TYMED_ISTORAGE:  
			pMedDest->pstg = pMedSrc->pstg;  
			pMedSrc->pstg->AddRef();  
			break;  
		case TYMED_NULL:  
		default:  
			break;  
		}  
		pMedDest->tymed = pMedSrc->tymed;  
		pMedDest->pUnkForRelease = NULL;  
		if(pMedSrc->pUnkForRelease != NULL)  
		{  
			pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;  
			pMedSrc->pUnkForRelease->AddRef();  
		}  
		return S_OK;  
	} 

protected:
	LONG m_lRefCount;
	std::vector<FORMATETC> m_fmts;
	std::vector<STGMEDIUM> m_stgs;	
};






void CDragHelper::EnableDrop(bool enable){
	g_bEnableDrop = enable;
}


IDataObject* CDragHelper::CreateDataObject(){	
	IDataObject* result = NULL;
	result = new CommonDataObject();
	//SHCreateDataObject(NULL, 0 , NULL,NULL, IID_IDataObject, (void**)&result);	//xp下不支持
	return result;
}

bool CDragHelper::SetDragImage(IDataObject* obj, HBITMAP hBmp, int w, int h){
	if(w==0||h==0){
		BITMAP bm;
		if(!GetObject(hBmp, sizeof(bm), &bm))return false;
		w = bm.bmWidth;
		h = bm.bmHeight;
	}

	IDragSourceHelperPtr helper;
	IDragSourceHelper2Ptr helper2;
	helper2.CreateInstance(CLSID_DragDropHelper);
	if(helper2){
		helper = helper2;
		helper2->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);
		//SetDragTip(obj, _T(""), NULL, DROPIMAGE_NOIMAGE);
	}else{
		helper.CreateInstance(CLSID_DragDropHelper);
	}
	if(helper){
		SHDRAGIMAGE shdi;
		ZeroMemory(&shdi, sizeof(shdi));
		shdi.sizeDragImage.cx = w;
		shdi.sizeDragImage.cy = h;
		shdi.hbmpDragImage = (HBITMAP)CopyImage(hBmp, IMAGE_BITMAP, w, h, 0);		
		if(!shdi.hbmpDragImage)return false;
		shdi.ptOffset.x = w-12;
		shdi.ptOffset.y = h-12;
		HRESULT hr = helper->InitializeFromBitmap(&shdi, obj);
		if(FAILED(hr))return false;
		return true;
	}
	return false;
	
}

bool CDragHelper::SetDragImage(IDataObject* obj, LPCTSTR file, int w, int h){
	Gdiplus::Bitmap* pImg = Gdiplus::Bitmap::FromFile(file);
	if(pImg && pImg->GetWidth() && pImg->GetHeight()){
		HBITMAP bmp = NULL;
		pImg->GetHBITMAP(Gdiplus::Color::White, &(bmp));
		bool ret = SetDragImage(obj, bmp, w, h);
		DeleteObject(bmp);
		delete pImg;
		return ret;
	}
	return false;
}


bool CDragHelper::SetDragTip(IDataObject* obj, LPCTSTR lpMsg, LPCTSTR lpInsert, DWORD effect){
	static CLIPFORMAT ddtype;
	if(!ddtype)ddtype = GetClipboardFormat(CFSTR_DROPDESCRIPTION);
	//DUITRACE(_T("SetDragTip %s %s"),lpMsg,lpInsert);
	DROPDESCRIPTION dd;
	ZeroMemory(&dd, sizeof(dd));
	if(lpMsg){
		_sntprintf(dd.szMessage, MAX_PATH, _T("%s"), lpMsg);
		if(lpInsert)_sntprintf(dd.szInsert, MAX_PATH, _T("%s"), lpInsert);
		dd.type = (DROPIMAGETYPE)effect;		
		return SetData(obj, ddtype, &dd, sizeof(dd));
	}else{
		dd.type = DROPIMAGE_INVALID;
		return SetData(obj, ddtype, &dd, sizeof(dd));
	}
	return true;
}

CLIPFORMAT CDragHelper::GetClipboardFormat(LPCTSTR lpFormat){
	static std::unordered_map<std::wstring, CLIPFORMAT> typemap;
	if(_tcscmp(lpFormat, _T("text"))==0){
		return CF_UNICODETEXT;
	}
	if(_tcscmp(lpFormat, _T("image"))==0){
		return CF_BITMAP;
	}
	if(_tcscmp(lpFormat, _T("file"))==0 || _tcscmp(lpFormat, _T("files"))==0){
		return CF_HDROP;
	}	
	if(_tcscmp(lpFormat, _T("html"))==0){
		lpFormat = _T("HTML Format");
	}
	auto it = typemap.find(lpFormat);
	if(it != typemap.end()){
		return it->second;
	}else{
		CLIPFORMAT ret = (CLIPFORMAT)RegisterClipboardFormat(lpFormat);
		if(ret){
			typemap[lpFormat] = ret;
		}
		return ret;
	}	
}

bool CDragHelper::SetData(IDataObject* obj, CLIPFORMAT cf, const void *pvBlob, UINT cbBlob){
    void *pv = GlobalAlloc(GPTR, cbBlob);
    HRESULT hr = pv ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr)) {
        CopyMemory(pv, pvBlob, cbBlob);

        FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium = {};
        medium.tymed = TYMED_HGLOBAL;
        medium.hGlobal = pv;
        hr = obj->SetData(&fmte, &medium, TRUE);
        if (FAILED(hr)) {
            GlobalFree(pv);
			return false;
        }
		return true;
    }
    return false;
}

bool CDragHelper::SetData(IDataObject* obj, LPCTSTR type, LPCTSTR str){
	return SetData(obj, GetClipboardFormat(type), str, (lstrlen(str)+1)*sizeof(TCHAR));
}


bool CDragHelper::SetText(IDataObject* obj, LPCTSTR str){
	if(str){
		UINT len = (lstrlen(str) + 1) * sizeof(TCHAR);
#ifdef UNICODE
		CLIPFORMAT cf = CF_UNICODETEXT;		
#else
		CLIPFORMAT cf = CF_TEXT;
#endif
		SetData(obj, cf, str, len);
		return true;
	}	
	return false;
}

bool CDragHelper::SetHtml(IDataObject* obj, LPCTSTR str){
	
	//https://msdn.microsoft.com/en-us/library/windows/desktop/ms649015(v=vs.85).aspx

	if(!str)return false;
	int wlen = _tcslen(str);
	if(!wlen)return false;	

	const char* meta = "Version:0.9\r\nStartHTML:00000097\r\nEndHTML:%08d\r\nStartFragment:00000153\r\nEndFragment:%08d\r\n";
	const char* header = "<!doctype html>\r\n<html>\r\n<body>\r\n<!--StartFragment -->\r\n";
	const char* footer = "\r\n<!--EndFragment -->\r\n</body>\r\n</html>";
	const int metalen = 97;
	const int headerlen = 56;	
	const int footerlen = 39;

	int buflen = metalen + headerlen + (wlen * 3 + 3) + footerlen;
	char* buf = (char* )malloc(buflen);
	if(!buf)return false;
	
	char* fragmentbuf = buf + metalen + headerlen;
	int ret = WideCharToMultiByte(CP_UTF8,0,str,-1,fragmentbuf,(wlen * 3 + 3),NULL,NULL);
	if(ret<=0){free(buf);return false;	}
	int fragmentlen = strlen(fragmentbuf);

	int htmlend = metalen + headerlen + fragmentlen + footerlen;
	int fragmentend = metalen + headerlen + fragmentlen;

	char metabuf[metalen+1];
	ret = _snprintf(metabuf, metalen+1, meta, htmlend, fragmentend);
	memcpy(buf, metabuf, metalen);
	memcpy(buf + metalen, header, headerlen);
	memcpy(buf + fragmentend, footer, footerlen+1);	

	CLIPFORMAT cf = GetClipboardFormat(_T("html"));		
	bool bret = SetData(obj, cf, buf, htmlend+1);
	free(buf);
	return bret;
}

bool CDragHelper::SetImage(IDataObject* obj, HBITMAP hBmp){
    FORMATETC fmte = {CF_BITMAP, NULL, DVASPECT_CONTENT, -1, TYMED_GDI};
    STGMEDIUM medium = {};
    medium.tymed = TYMED_GDI;
    medium.hBitmap = hBmp;
    HRESULT hr = obj->SetData(&fmte, &medium, FALSE);
    if (FAILED(hr)) {
        return false;
    }
	return true;
}


bool CDragHelper::HasData(IDataObject* obj, CLIPFORMAT cf){
	if(!obj)return false;
	if(!cf)return false;
	FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	HRESULT hr = obj->QueryGetData(&fmte);
	if(hr == S_OK)return true;
	return false;
}

String CDragHelper::GetData(IDataObject* obj, LPCTSTR lpType){
	CLIPFORMAT cf = GetClipboardFormat(lpType);
	FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stm = {0};
	String result;
	HRESULT hr = obj->GetData(&fmte, &stm);		
	if(!FAILED(hr) && stm.tymed == TYMED_HGLOBAL){
		result = (LPCSTR) GlobalLock(stm.hGlobal);
		GlobalUnlock(stm.hGlobal);
	}
	if(!FAILED(hr)){
		ReleaseStgMedium(&stm);
	}
	return result;
}

String CDragHelper::GetData(IDataObject* obj, CLIPFORMAT cf) {	
	FORMATETC fmte = { cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stm = { 0 };
	String result;
	HRESULT hr = obj->GetData(&fmte, &stm);
	if (!FAILED(hr) && stm.tymed == TYMED_HGLOBAL) {
		result = (LPCSTR)GlobalLock(stm.hGlobal);
		GlobalUnlock(stm.hGlobal);
	}
	if (!FAILED(hr)) {
		ReleaseStgMedium(&stm);
	}
	return result;
}


HDROP CDragHelper::GetFiles(IDataObject* obj){
	HDROP result = NULL;
	CLIPFORMAT cf = CF_HDROP;
	FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stm = {0};
	HRESULT hr = obj->GetData(&fmte, &stm);		
	if(!FAILED(hr) && stm.tymed == TYMED_HGLOBAL){
		result = (HDROP)stm.hGlobal;
	}
	return result;
}

String CDragHelper::GetHtml(IDataObject* obj){
	String result;
	static CLIPFORMAT cf = GetClipboardFormat(_T("html"));
	FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stm = {0};
	HRESULT hr = obj->GetData(&fmte, &stm);		
	if(!FAILED(hr) && stm.tymed == TYMED_HGLOBAL){
		const char* text = (LPCSTR) GlobalLock(stm.hGlobal);
		const char* startsign = "StartFragment:";
		const char* endsign = "EndFragment:";
		const char* fragmentstart = strstr(text, startsign);
		const char* fragmentend = strstr(text, endsign);
		if(fragmentstart && fragmentend){			
			char* temp = NULL;
			int startsignlen = strlen(startsign);
			int endsignlen = strlen(endsign);
			fragmentstart += startsignlen;
			while(*fragmentstart==' ')fragmentstart ++;
			fragmentend += endsignlen;
			while(*fragmentend==' ')fragmentend ++;
			int posstart = strtol(fragmentstart, &temp, 10);
			int posend = strtol(fragmentend, &temp, 10);
			if(posstart>0 && posend>0 && posend>posstart){
				int textlen = strlen(text);
				if(posend<=textlen){
					result.assign(text+posstart, posend-posstart);
				}
			}
		}			
		GlobalUnlock(stm.hGlobal);		
	}
	return result;
}

static bool GetTextUtil(IDataObject* obj, bool unicode, String& result){
	CLIPFORMAT cf = unicode?CF_UNICODETEXT:CF_TEXT;
	FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stm = {0};
	HRESULT hr = obj->GetData(&fmte, &stm);	
	bool ret = false;
	if(!FAILED(hr) && stm.tymed == TYMED_HGLOBAL){
		void* data =  GlobalLock(stm.hGlobal);
		if(unicode){
			int slen = wcslen((LPCTSTR)data);
			char* buf = (char*)malloc(slen*2+4);
			int len = WideCharToMultiByte(CP_ACP,0, (LPCTSTR)data, -1, buf, slen*2+4,NULL,NULL);
			if (len > 0) {
				result.assign(buf, len);
			}
			free(buf);
			ret = true;
		}else{
			result = (LPCSTR)data;			
		}
		GlobalUnlock(stm.hGlobal);
	}
	return ret;
}

String CDragHelper::GetText(IDataObject* obj){
	String result;
	if(!GetTextUtil(obj, true, result)){
		GetTextUtil(obj, false, result);
	}
	return result;
}


HBITMAP CDragHelper::GetImage(IDataObject* obj){
	HBITMAP result = NULL;
	CLIPFORMAT cf = CF_BITMAP;
	FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_GDI};
	STGMEDIUM stm = {0};
	HRESULT hr = obj->GetData(&fmte, &stm);		
	if(!FAILED(hr) && stm.tymed == TYMED_GDI){
		result = (HBITMAP)OleDuplicateData(stm.hBitmap, CF_BITMAP, 0); 
	}
	return result;
}


bool CDragHelper::DoDrag(IDataObject* pObj){
	DWORD dwEffect = 0;
	DWORD dwEffectAllow = DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK;
	DWORD dwResult = SHDoDragDrop(NULL, pObj, NULL, dwEffectAllow, &dwEffect);		
	if(dwResult != DRAGDROP_S_DROP){
		return true;
	}
	return false;
}