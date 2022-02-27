#pragma once
#include <cef_base.h>
#include <hbapiitm.h>

#include <wrapper/cef_helpers.h>
#include <cef_app.h>
#include <views/cef_window.h>
#include <views/cef_browser_view.h>
//#include <cef_client.h>
#include <cef_parser.h>
#include <list>
#include <base/cef_callback.h>
#include <wrapper/cef_closure_task.h>


CefBaseRefCounted* hb_cefItemGet( PHB_ITEM pItem);
PHB_ITEM hb_cefItemPut( PHB_ITEM pItem, CefBaseRefCounted* cefObject );
CefBaseRefCounted* hb_parCef( int iParam );
void hb_retCef(CefBaseRefCounted* cefObject );

void initCefObj(CefBaseRefCounted* refCnt, HB_USHORT classId);
CefBaseRefCounted* hb_selfCef(PHB_ITEM pSelf = 0);

#define FORWARD_GETCLASSID(objName) HB_USHORT getCEF ## objNameCEF ## _ClassId();
#define GETCLASSID(objName) getCEF ## objNameCEF ## _ClassId()

#define DEFINE_GETCLASSID(objName) \
    HB_FUNC_EXTERN(CEF_ ## objName); \
    HB_USHORT objNameCEF ## _ClassId = 0; \
    HB_USHORT getCEF ## objNameCEF ## _ClassId() { \
        if(objNameCEF ## _ClassId) { return objNameCEF ## _ClassId; } \
	    objNameCEF ## _ClassId = hb_clsFindClass("CEF_" # objName, NULL); \
        if(objNameCEF ## _ClassId) { return objNameCEF ## _ClassId; } \
        HB_FUNC_EXEC(CEF_ ## objName); \
        objNameCEF ## _ClassId = hb_clsFindClass("CEF_"  # objName, NULL); \
        return objNameCEF ## _ClassId; \
    }

class MyBrowserCallbacks {
public:
    virtual void OnContextCreated(  CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context) = 0;
};

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRenderProcessHandler {
public:
    explicit SimpleHandler();
    ~SimpleHandler();

    // Provide access to the single global instance of this object.
    static SimpleHandler* GetInstance();

    // CefClient methods:
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this;}
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    //virtual CefRefPtr<CefRenderHandler> GetRender() { return this; }

    // CefDisplayHandler methods:
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                                const CefString& title) override;

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            ErrorCode errorCode,
                            const CefString& errorText,
                            const CefString& failedUrl) override;

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool force_close);

    bool IsClosing() const { return is_closing_; }

    // Returns true if the Chrome runtime is enabled.
    static bool IsChromeRuntimeEnabled();

    // CefRenderProcessHandler methods:
    void RegisterContextCreated(CefRefPtr<CefBrowser> browser, MyBrowserCallbacks* callback);
    virtual void OnContextCreated(  CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context);

private:
    std::map<CefBrowser*, MyBrowserCallbacks*> registeredCallbacks;

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;

    bool is_closing_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleHandler);
};

CefRefPtr<CefV8Value> HBtoV8Value(const CefString& cName, PHB_ITEM pItem);
CefString hbToString(PHB_ITEM pItem);
CefString hb_parCefString(int p);
