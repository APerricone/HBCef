#include "hbcef.h"
#include <hbapicls.h>
#include <hbstack.h>

DEFINE_GETCLASSID(REFCOUNTED);

HB_SIZE iObjIdx = 0;
HB_SIZE getObjIdx() {
    if(iObjIdx) return iObjIdx;
    iObjIdx = hb_clsGetVarIndex(GETCLASSID(REFCOUNTED),hb_dynsymGet("pObj"));
    return iObjIdx;
}

HB_FUNC( CEF_REFCOUNTED_DELETE ) {
    PHB_ITEM pSelf = hb_stackSelfItem();
    CefBaseRefCounted* refCnt = (CefBaseRefCounted*)hb_arrayGetPtr(pSelf,getObjIdx());
    if(refCnt) {
        refCnt->Release();
        //hb_arraySet(pSelf, getObjIdx(), hb_itemNew(0));
    }
}

HB_FUNC( CEF_REFCOUNTED_COPY ) {
	PHB_ITEM pItem = hb_param(1, HB_IT_OBJECT);
    PHB_ITEM pSelf = hb_stackSelfItem();
    CefBaseRefCounted* ptr = (CefBaseRefCounted*)hb_arrayGetPtr(pSelf,getObjIdx());
    hb_arraySetPtr(pItem, getObjIdx(), ptr);
    ptr->AddRef();
}

void initCefObj(CefBaseRefCounted* refCnt, HB_USHORT classId) {
    hb_clsAssociate(classId);
    PHB_ITEM pSelf = hb_stackSelfItem();
    hb_arraySetPtr(pSelf, getObjIdx(), refCnt);
    refCnt->AddRef();
    hb_itemCopy(hb_stackReturnItem(),hb_stackSelfItem());
}

CefBaseRefCounted* hb_selfCef(PHB_ITEM pSelf) {
    if(!pSelf) pSelf = hb_stackSelfItem();
    if(!HB_IS_OBJECT(pSelf)) return 0;
    return (CefBaseRefCounted*)hb_arrayGetPtr(pSelf,getObjIdx());
}
