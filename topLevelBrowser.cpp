#include "hbcef.h"
#include <views/cef_window.h>
#include <views/cef_browser_view.h>
#include <cef_v8.h>
#include <hbxvm.h>
#include <hbapicls.h>
#include <hbapi.h>
#include <hbapierr.h>
FORWARD_GETCLASSID(TOPLEVELBROWSER);

class SimpleHandler;

namespace {

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class SimpleWindowDelegate : public CefWindowDelegate {
public:
    explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view,int _w, int _h) :
        browser_view_(browser_view), w(_w), h(_h)  {}

    void OnWindowCreated(CefRefPtr<CefWindow> window) override {
        // Add the browser view and show the window.
        window->AddChildView(browser_view_);
        window->Show();

        // Give keyboard focus to the browser view.
        browser_view_->RequestFocus();
    }

    void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
        browser_view_ = nullptr;
    }

    bool CanClose(CefRefPtr<CefWindow> window) override {
        // Allow the window to close if the browser says it's OK.
        CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
        if(browser)
            return browser->GetHost()->TryCloseBrowser();
        return true;
    }

    CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
        if(w<=0) w=800;
        if(h<=0) h=600;
        return CefSize(w, h);
    }

private:
    int w,h;
    CefRefPtr<CefBrowserView> browser_view_;

    IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
    DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
public:
    SimpleBrowserViewDelegate(int _w, int _h) : w(_w), h(_h) {}

    bool OnPopupBrowserViewCreated( CefRefPtr<CefBrowserView> browser_view,
                                    CefRefPtr<CefBrowserView> popup_browser_view,
                                    bool is_devtools) override {
        (is_devtools);
        // Create a new top-level Window for the popup. It will show itself after
        // creation.
        CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(popup_browser_view,w,h));

        // We created the Window.
        return true;
    }

private:
    int w,h;

    IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
    DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
};

}  // namespace

class ImplementedBrowserCallbacks : public MyBrowserCallbacks {
public:
    ImplementedBrowserCallbacks(PHB_ITEM _pSelf) : pSelf(_pSelf) {}

    virtual void OnContextCreated(  CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context) override;

private:
    PHB_ITEM pSelf;
};

HB_FUNC( CEF_TOPLEVELBROWSER_NEW ) {

    // SimpleHandler implements browser-level callbacks.
    CefRefPtr<SimpleHandler> handler = SimpleHandler::GetInstance();
    hb_xvmSetLine(__LINE__);
    // Specify CEF browser settings here.
    CefBrowserSettings browser_settings;

    std::string url;
    hb_xvmSetLine(__LINE__);
    // Check if a "--url=" value was provided via the command-line. If so, use
    // that instead of the default URL.
    if(hb_parc(3)) url = hb_parc(3);
    //if(url.empty()) url = "https://www.google.com/";
    hb_xvmSetLine(__LINE__);
    // Create the BrowserView.
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        handler, url, browser_settings, nullptr, nullptr,
        new SimpleBrowserViewDelegate(hb_parni(1),hb_parni(2)));
    hb_xvmSetLine(__LINE__);
    // Create the Window. It will show itself after creation.
    CefRefPtr<CefWindow> window = CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view,hb_parni(1),hb_parni(2)));
    hb_xvmSetLine(__LINE__);
    //hb_retCef((CefBaseRefCounted*)window.get());
    //hb_xvmSetLine(__LINE__);
    initCefObj(browser_view.get(), GETCLASSID(TOPLEVELBROWSER));
    handler->RegisterContextCreated(browser_view->GetBrowser(), new ImplementedBrowserCallbacks(hb_stackSelfItem()));
}

HB_FUNC( CEF_TOPLEVELBROWSER_LOADURL ) {
    CefBrowserView* browserView = (CefBrowserView*)hb_selfCef();
    browserView->GetBrowser()->GetMainFrame()->LoadURL(hb_parCefString(1));
    hb_ret();
}

void ImplementedBrowserCallbacks::OnContextCreated(  CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) {
    static HB_SIZE iOnContextCreated = 0;
    if(iOnContextCreated==0) {
        iOnContextCreated = hb_clsGetVarIndex(GETCLASSID(TOPLEVELBROWSER),hb_dynsymGet("bOnContextCreated"));
    }
    PHB_ITEM pOnInitialized = hb_itemNew( NULL );
    hb_arrayGet(pSelf,iOnContextCreated,pOnInitialized);
    if(pOnInitialized && HB_IS_EVALITEM( pOnInitialized ))
        hb_evalBlock1(pOnInitialized, pSelf);
}

HB_FUNC( CEF_TOPLEVELBROWSER_ADDJAVASCRIPT ) { ///(cName, oJS)
    CefBrowserView* browserView = (CefBrowserView*)hb_selfCef();
    CefRefPtr<CefV8Context> context = browserView->GetBrowser()->GetMainFrame()->GetV8Context();
    CefRefPtr<CefV8Value> object = context->GetGlobal();
    hb_retl(object->SetValue(hb_parc(1), HBtoV8Value(hb_parc(1), hb_param(2, HB_IT_ANY)), V8_PROPERTY_ATTRIBUTE_NONE)? HB_TRUE : HB_FALSE);
}

CefRefPtr<CefV8Value> HBtoV8Value_HASH(PHB_ITEM pItem);
CefRefPtr<CefV8Value> HBtoV8Value_JSON(PHB_ITEM pItem);
CefRefPtr<CefV8Value> HBtoV8Value_ARRAY(PHB_ITEM pItem);

CefRefPtr<CefV8Value> HBtoV8Value(const CefString& cName, PHB_ITEM pItem) {
	HB_TYPE t = hb_itemType(pItem);
	switch(t) {
	case HB_IT_NIL:       //0x00000
		return CefV8Value::CreateUndefined();
	case HB_IT_INTEGER:   //0x00002
	case HB_IT_DOUBLE:    //0x00010
	case HB_IT_LONG:      //0x00008
	case HB_IT_NUMERIC:   //( HB_IT_INTEGER | HB_IT_LONG | HB_IT_DOUBLE )
	case HB_IT_NUMINT:    //( HB_IT_INTEGER | HB_IT_LONG )
		return CefV8Value::CreateDouble(hb_itemGetND(pItem));
	case HB_IT_LOGICAL:   //0x00080
        return CefV8Value::CreateBool(hb_itemGetL(pItem)!=HB_FALSE);
	case HB_IT_STRING:    //0x00400
	case HB_IT_MEMOFLAG:  //0x00800
	case HB_IT_MEMO:      //( HB_IT_MEMOFLAG | HB_IT_STRING )
	case HB_IT_BYREF:     //0x02000
	case HB_IT_MEMVAR:    //0x04000
		return CefV8Value::CreateString(hbToString(pItem));
	case HB_IT_HASH:      //0x00004
		return HBtoV8Value_HASH(pItem);

	//case HB_IT_OBJECT:    //HB_IT_ARRAY
	case HB_IT_ARRAY:     //0x08000
	{
		HB_USHORT clsId = hb_objGetClass(pItem);
		//if(clsId>0 && clsId!=getJSVALUEClassId()) break; //Error
		if(clsId==0) {
            return HBtoV8Value_ARRAY(pItem);
		} else {
			return HBtoV8Value_JSON(pItem); // TODO better
		}
	}
	// unsupported types:
	//case HB_IT_DATE:      //0x00020 CefV8Value::CreateDate
	//case HB_IT_TIMESTAMP: //0x00040 CefV8Value::CreateDate
	//case HB_IT_ALIAS:     //0x00200
	//case HB_IT_SYMBOL:    //0x00100 CefV8Value::CreateFunction
	//case HB_IT_BLOCK:     //0x01000 CefV8Value::CreateFunction
	//case HB_IT_ENUM:      //0x10000
	//case HB_IT_EXTREF:    //0x20000
	//case HB_IT_DEFAULT:   //0x40000
	//case HB_IT_RECOVER:   //0x80000
	//case HB_IT_DATETIME:  //( HB_IT_DATE | HB_IT_TIMESTAMP ) CefV8Value::CreateDate
	}
	hb_errRT_BASE(EG_ARG, 12, "unable to convert",  HB_ERR_FUNCNAME, 1, pItem);
	return CefV8Value::CreateUndefined();
}

CefRefPtr<CefV8Value> HBtoV8Value_JSON(PHB_ITEM pItem) {
    hb_errRT_BASE(EG_ARG, 12, "unable to convert",  HB_ERR_FUNCNAME, 1, pItem);
    return CefV8Value::CreateUndefined();
}

CefRefPtr<CefV8Value> HBtoV8Value_ARRAY(PHB_ITEM pItem) {
    HB_SIZE nLen = hb_itemSize( pItem );
    CefRefPtr<CefV8Value> pRet = CefV8Value::CreateArray(nLen);
    for(int nIndex = 1; nIndex <= nLen; ++nIndex ) {
        PHB_ITEM pElement = hb_arrayGetItemPtr( pItem, nIndex );
        pRet->SetValue(nIndex-1, HBtoV8Value("",pElement));
    }
    return pRet;
}

CefRefPtr<CefV8Value> HBtoV8Value_HASH(PHB_ITEM pItem) {
    HB_SIZE nLen = hb_hashLen( pItem );
    CefRefPtr<CefV8Value> pRet = CefV8Value::CreateObject(nullptr, nullptr);
    for(int nIndex = 1; nIndex <= nLen; ++nIndex ) {
        PHB_ITEM pValue = hb_hashGetValueAt( pItem, nIndex );
        PHB_ITEM pKey = hb_hashGetKeyAt( pItem, nIndex );
        if( HB_IS_STRING( pKey ) ) {
            CefString cKey = hbToString( pKey );
            CefRefPtr<CefV8Value> cefValue = HBtoV8Value(cKey,pValue);
            pRet->SetValue(cKey, cefValue, V8_PROPERTY_ATTRIBUTE_NONE);
        } else if( HB_IS_INTEGER( pKey ) ) {
            pRet->SetValue(hb_itemGetNI(pKey), HBtoV8Value("",pValue));
        }
    }
    return pRet;
}
