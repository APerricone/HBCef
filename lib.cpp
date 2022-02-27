#include "hbcef.h"
#include <hbapi.h>
#include <hbapiitm.h>
#include <hbapistr.h>
#include <wrapper/cef_helpers.h>
#include <cef_app.h>
#include <views/cef_window.h>
#include <views/cef_browser_view.h>
//#include <cef_client.h>
#include <cef_parser.h>
#include <list>
#include <base/cef_callback.h>
#include <wrapper/cef_closure_task.h>
#include <cef_version.h>
#include <map>
#if defined(CEF_X11)
#include <X11/Xlib.h>
#include "include/base/cef_logging.h"
#endif

PHB_ITEM pOnInitialized = 0;

#if defined(CEF_X11)
namespace {

int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
  LOG(WARNING) << "X error received: "
               << "type " << event->type << ", "
               << "serial " << event->serial << ", "
               << "error_code " << static_cast<int>(event->error_code) << ", "
               << "request_code " << static_cast<int>(event->request_code)
               << ", "
               << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display* display) {
  return 0;
}

}  // namespace
#endif  // defined(CEF_X11)

// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp, public CefBrowserProcessHandler {
public:
    SimpleApp() { }

    // CefApp methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    // CefBrowserProcessHandler methods:
    void OnContextInitialized() override;

private:
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return SimpleHandler::GetInstance();
    }
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleApp);
};

HB_FUNC(CEF_LOOP) {
    if(HB_ISEVALITEM(1)) {
        pOnInitialized = hb_itemNew(hb_param(1, HB_IT_EVALITEM));
    }

    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    void* sandbox_info = nullptr;
#if defined(CEF_USE_SANDBOX)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif
#ifdef OS_WIN
    HINSTANCE hInstance;
    hb_winmainArgGet(&hInstance,nullptr,nullptr);
    CefMainArgs main_args(hInstance);
#else
    int argc = hb_cmdargARGC();
    char ** argv = hb_cmdargARGV();
    CefMainArgs main_args(argc, argv);
#endif
    int exit_code = CefExecuteProcess(main_args, nullptr, sandbox_info);
    if(exit_code >= 0) {
        // The sub-process has completed so return here.
        hb_retni(exit_code);
        return;
    }
    /*
    // Parse command-line arguments for use in this method.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
#ifdef OS_WIN
    command_line->InitFromString(::GetCommandLineW());
#else
    command_line->InitFromArgv(argc, argv);
#endif
    */
    // Specify CEF global settings here.
    CefSettings settings;
    /*if (command_line->HasSwitch("enable-chrome-runtime")) {
        // Enable experimental Chrome runtime. See issue #2969 for details.
        settings.chrome_runtime = true;
    }*/
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif
    settings.log_severity = LOGSEVERITY_VERBOSE;

    // SimpleApp implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefRefPtr<SimpleApp> app(new SimpleApp);

    // Initialize CEF for the browser process.
    CefInitialize(main_args, settings, app.get(), sandbox_info);


    // Run the CEF message loop. This will block until CefQuitMessageLoop() is
    // called.
    CefRunMessageLoop();

    // Shut down CEF.
    CefShutdown();

    hb_retni(0);
    return;
}

void SimpleApp::OnContextInitialized() {
    CEF_REQUIRE_UI_THREAD();
    CefRefPtr<SimpleHandler> handler(new SimpleHandler());
    if(pOnInitialized)
        hb_evalBlock0(pOnInitialized);
}

namespace {

SimpleHandler* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
    return "data:" + mime_type + ";base64," +
            CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
}

}  // namespace

SimpleHandler::SimpleHandler() : is_closing_(false) {
    DCHECK(!g_instance);
    g_instance = this;
}

SimpleHandler::~SimpleHandler() {
    g_instance = nullptr;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
    return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
    CEF_REQUIRE_UI_THREAD();

    // Set the title of the window using the Views framework.
    CefRefPtr<CefBrowserView> browser_view =
        CefBrowserView::GetForBrowser(browser);
    if(browser_view) {
        CefRefPtr<CefWindow> window = browser_view->GetWindow();
        if(window)
            window->SetTitle(title);
    }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    // Add to the list of existing browsers.
    browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed destription of this
    // process.
    if(browser_list_.size() == 1) {
        // Set a flag to indicate that the window close should be allowed.
        is_closing_ = true;
    }

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    // Remove from the list of existing browsers.
    BrowserList::iterator bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit) {
        if((*bit)->IsSame(browser)) {
            browser_list_.erase(bit);
            break;
        }
    }

    if(browser_list_.empty()) {
        // All browser windows have closed. Quit the application message loop.
        CefQuitMessageLoop();
    }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();

    // Allow Chrome to show the error page.
    if(IsChromeRuntimeEnabled())
        return;

    // Don't display an error for downloaded files.
    if(errorCode == ERR_ABORTED)
        return;

    // Display a load error message using a data: URI.
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\">"
            "<h2>Failed to load URL "
            << std::string(failedUrl) << " with error " << std::string(errorText)
            << " (" << errorCode << ").</h2></body></html>";

    frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
    if(!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
                                       force_close));
        return;
    }

    if(browser_list_.empty())
        return;

    BrowserList::const_iterator it = browser_list_.begin();
    for (; it != browser_list_.end(); ++it)
        (*it)->GetHost()->CloseBrowser(force_close);
}

// static
bool SimpleHandler::IsChromeRuntimeEnabled() {
    static int value = -1;
    if(value == -1) {
        CefRefPtr<CefCommandLine> command_line =
            CefCommandLine::GetGlobalCommandLine();
        value = command_line->HasSwitch("enable-chrome-runtime") ? 1 : 0;
    }
    return value == 1;
}

HB_FUNC(CEF_VERSIONSTR) { hb_retc(CEF_VERSION); }

void SimpleHandler::OnContextCreated(  CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context)
{
    auto callback = registeredCallbacks.find(browser.get());
    if(callback!=registeredCallbacks.end()) {
        (callback->second)->OnContextCreated(browser,frame,context);
    }
}

void SimpleHandler::RegisterContextCreated(CefRefPtr<CefBrowser> browser, MyBrowserCallbacks* callback) {
    if(callback==0) {
        auto currCallback = registeredCallbacks.find(browser.get());
        if(currCallback!=registeredCallbacks.end()) {
            registeredCallbacks.erase(browser.get());
        }
    } else {
        registeredCallbacks.insert_or_assign(browser.get(), callback);
    }
}

CefString hb_parCefString(int p) {
    PHB_ITEM pItem = hb_param(p, HB_IT_STRING);
    if(pItem) {
        return hbToString(pItem);
    }
    return CefString();
}

CefString hbToString(PHB_ITEM pItem) {
#if defined(CEF_STRING_TYPE_UTF16) || defined(CEF_STRING_TYPE_WIDE)
    void * hStr;
    HB_SIZE len;
    const HB_WCHAR* hb_str = hb_itemGetStrU16(pItem,HB_CDP_ENDIAN_NATIVE,&hStr,&len);
    CefString val = CefString(hb_str,len);
    hb_strfree(hStr);
    return val;
#elif defined(CEF_STRING_TYPE_UTF8)
    void * hStr;
    HB_SIZE len;
    const char * hb_str = hb_itemGetStrUTF8(pItem,HB_CDP_ENDIAN_NATIVE,&hStr,&len);
    CefString val = CefString(hb_str,len);
    hb_strfree(hStr);
    return val;
#else
#error "unknow CefString"
#endif
}
