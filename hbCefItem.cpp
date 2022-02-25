#include "hbcef.h"
#include <hbapierr.h>
#include <hbstack.h>

static HB_GARBAGE_FUNC( hb_cef_destructor )
{
   CefBaseRefCounted ** ppCef = ( CefBaseRefCounted ** ) Cargo;

   if( *ppCef )
   {
       (*ppCef)->Release();
        *ppCef = NULL;
   }
}

static const HB_GC_FUNCS s_gcCefFuncs =
{
   hb_cef_destructor,
   hb_gcDummyMark
};


CefBaseRefCounted* hb_cefItemGet( PHB_ITEM pItem) {
   CefBaseRefCounted ** ppCef = ( CefBaseRefCounted ** ) hb_itemGetPtrGC( pItem, &s_gcCefFuncs );
   return ppCef ? *ppCef : NULL;
}

PHB_ITEM hb_cefItemPut( PHB_ITEM pItem, CefBaseRefCounted* cefObject ) {
   CefBaseRefCounted ** ppCef = ( CefBaseRefCounted ** ) hb_gcAllocate( sizeof( CefBaseRefCounted * ), &s_gcCefFuncs );

   *ppCef = cefObject;
   cefObject->AddRef();
   return hb_itemPutPtrGC( pItem, ppCef );

}

CefBaseRefCounted* hb_parCef( int iParam ) {
    CefBaseRefCounted ** ppCef = ( CefBaseRefCounted ** ) hb_parptrGC( &s_gcCefFuncs, iParam );

    if( ppCef && *ppCef )
        return *ppCef;

    hb_errRT_BASE( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
    return NULL;
} 

void hb_retCef(CefBaseRefCounted* cefObject ) {
    hb_cefItemPut( hb_stackReturnItem(), cefObject );
}
