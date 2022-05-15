'x64版をインストールします
'
'有限会社サイトー企画さんの以下ソフトウェア（フリーソフト）を改変利用しています
' https://www.maruo.co.jp/_library/libpass.asp?f=hmfcext/hmfilerclassicshlnk103.zip
'
strDllName = "DDSFormat.dll"
Dim objWMI, objShell, colOS, objOS, objFso
If WScript.Arguments.Count=0 Then
  Set objWMI = GetObject("winmgmts:{impersonationLevel=impersonate}!\\.\root\cimv2")
  Set colOS = objWMI.ExecQuery("SELECT * FROM Win32_OperatingSystem")
  fVista = false
  For Each objOS in colOS
      aVer=Split(objOS.Version,".")
      If Int(aVer(0)) >= 6 Then
          fVista = true
      End If
  Next
  Set objShell = CreateObject("Shell.Application")
  If fVista Then
      objShell.ShellExecute "wscript.exe", """" + WScript.ScriptFullName + """ 1","","runas",1
  Else
      objShell.ShellExecute "wscript.exe", """" + WScript.ScriptFullName + """ 1"
  End If
Else
  Set objFso=CreateObject("Scripting.FileSystemObject")
  Set objWsh=CreateObject("WScript.Shell")
  strDir = objFso.GetFile(WScript.ScriptFullName).ParentFolder
  objWsh.run "regsvr32.exe """ + strDir + "\" + strDllName + """"
End If
