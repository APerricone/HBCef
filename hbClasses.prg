#include <hbclass.ch>

class cef_RefCounted
    DATA pObj PROTECTED

    //CONSTRUCTOR New(pObj)
    DESTRUCTOR Delete()

    METHOD COPY OPERATOR ":="
endclass

class cef_TopLevelBrowser inherit cef_RefCounted

    DATA bOnContextCreated

    CONSTRUCTOR new(nWidth,nHeight)

    METHOD loadUrl(cUrl)
    METHOD addJavaScript(cName, oJS)
endclass

