
PROC Main()
    cef_Loop({|| CreateWindow()})    
return

proc CreateWindow()
    cef_CreateTopLevelBrowser('https://www.google.com/')
return    
