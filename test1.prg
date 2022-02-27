
// Basic test, create a Browser windows and opens google
PROC Main()
    cef_Loop({|| CreateWindow()})    
return

proc CreateWindow()    
    LOCAL oBrowser := cef_TopLevelBrowser():New(800,500)
    oBrowser:loadUrl('https://www.google.com/')
return    
