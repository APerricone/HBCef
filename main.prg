
PROC Main()
    cef_Loop({|| CreateWindow()})
return

proc CreateWindow()
    LOCAL oBrowser := cef_TopLevelBrowser():New(800,500)
    LOCAL cPath := StrTran(HB_DirBase(),"\","/")
    oBrowser:bOnContextCreated := @InjectJS()
    //oBrowser:loadUrl('file:///injectJS1.html')
    oBrowser:loadUrl('file:///'+cPath+'injectJS1.html')
return

proc InjectJS(oBrowser)
    LOCAL hProcess := {=>}
    hProcess["harbour"] := hb_Version()
    hProcess["chrome"] := cef_VersionStr()
    ? hb_valToExp(hProcess)
    oBrowser:addJavaScript("process",hProcess)
return

