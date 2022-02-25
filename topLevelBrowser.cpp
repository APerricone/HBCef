#include "hbcef.h"
#include <views/cef_window.h>
#include <views/cef_browser_view.h>
#include <hbxvm.h>
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

HB_FUNC( CEF_TOPLEVELBROWSER_NEW ) {

    // SimpleHandler implements browser-level callbacks.
    CefRefPtr<SimpleHandler> handler(new SimpleHandler());
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
}

HB_FUNC( CEF_TOPLEVELBROWSER_LOADURL ) {
    CefBrowserView* renderer = (CefBrowserView*)hb_selfCef();
    renderer->GetBrowser()->GetMainFrame()->LoadURL(hb_parc(1));
    hb_ret();
}
