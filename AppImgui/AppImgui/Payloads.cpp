#include "Payloads.h"

const char* g_payloads[] = {
    // PowerShell
    "powershell.exe:-encodedcommand",
    "powershell.exe:-enc",
    "powershell.exe:-e",
    "powershell.exe:-w hidden",
    "powershell.exe:-windowstyle hidden",
    "powershell.exe:-nop",
    "powershell.exe:-noprofile",
    "powershell.exe:invoke-expression",
    "powershell.exe:iex",
    "powershell.exe:downloadstring",
    "powershell.exe:downloadfile",
    "pwsh.exe:-encodedcommand",
    "pwsh.exe:-enc",
    // Certutil
    "certutil.exe:-urlcache",
    "certutil.exe:-verifyctl",
    "certutil.exe:-decode",
    "certutil.exe:-encode",
    "certutil.exe:-decodehex",
    "certutil.exe:-encodehex",
    // Bitsadmin
    "bitsadmin.exe:/transfer",
    "bitsadmin.exe:/create",
    "bitsadmin.exe:/addfile",
    "bitsadmin.exe:/setnotifycmdline",
    // Regsvr32
    "regsvr32.exe:/s",
    "regsvr32.exe:/u",
    "regsvr32.exe:/i",
    "regsvr32.exe:scrobj.dll",
    // Rundll32
    "rundll32.exe:javascript:",
    "rundll32.exe:vbscript:",
    "rundll32.exe:comsvcs.dll",
    "rundll32.exe:minidump",
    "rundll32.exe:dfshim.dll",
    "rundll32.exe:url.dll",
    "rundll32.exe:RAFlush",
    // Mshta
    "mshta.exe:vbscript:",
    "mshta.exe:javascript:",
    "mshta.exe:http:",
    "mshta.exe:https:",
    "mshta.exe:.hta",
    // MSBuild
    "msbuild.exe:.csproj",
    "msbuild.exe:.xml",
    "msbuild.exe:/p:",
    // Installutil
    "installutil.exe:/logfile=",
    "installutil.exe:/logtoconsole=false",
    "installutil.exe:/u",
    // Regasm/Regsvcs
    "regasm.exe:/u",
    "regasm.exe:/codebase",
    "regsvcs.exe:/u",
    "regsvcs.exe:/codebase",
    // WMIC
    "wmic.exe:process call create",
    "wmic.exe:os get",
    "wmic.exe:format:",
    "wmic.exe:/node:",
    // Cscript/Wscript
    "cscript.exe:.vbs",
    "cscript.exe:.js",
    "wscript.exe:.vbs",
    "wscript.exe:.js",
    "wscript.exe:.wsf",
    // Msiexec
    "msiexec.exe:/i",
    "msiexec.exe:/quiet",
    "msiexec.exe:/q",
    "msiexec.exe:http:",
    "msiexec.exe:https:",
    "msiexec.exe:/y",
    "msiexec.exe:/z",
    // Cmstp
    "cmstp.exe:/s",
    "cmstp.exe:/au",
    "cmstp.exe:.inf",
    // Odbcconf
    "odbcconf.exe:/a",
    "odbcconf.exe:-f",
    "odbcconf.exe:regsvr",
    // IEExec
    "ieexec.exe:http:",
    "ieexec.exe:https:",
    // PsExec
    "psexec.exe:-s",
    "psexec.exe:\\\\",
    // Cmd.exe
    "cmd.exe:/c echo",
    "cmd.exe:/c powershell",
    "cmd.exe:/c start",
    "cmd.exe:/k",
    // Microsoft.Workflow.Compiler
    "microsoft.workflow.compiler.exe:",
    // DotNet
    "dotnet.exe:build",
    "dotnet.exe:run",
    "dotnet.exe:exec",
    // CSC/VBC/JSC
    "csc.exe:/out:",
    "vbc.exe:/out:",
    "jsc.exe:/out:",
    // Mavinject
    "mavinject.exe:/injectrunning",
    // Forfiles
    "forfiles.exe:/c",
    // PsExec64
    "psexec64.exe:-s",
    "psexec64.exe:\\\\",
    // At
    "at.exe:\\\\",
    // Schtasks
    "schtasks.exe:/create",
    "schtasks.exe:/run",
    "schtasks.exe:/s",
    // Control
    "control.exe:.cpl",
    // Diskshadow
    "diskshadow.exe:/s",
    // Esentutl
    "esentutl.exe:/y",
    "esentutl.exe:/vss",
    // Expand
    "expand.exe:-r",
    // Extrac32
    "extrac32.exe:/y",
    "extrac32.exe:/c",
    // Findstr
    "findstr.exe:/v",
    "findstr.exe:/s",
    // Hh.exe
    "hh.exe:.chm",
    // Makecab/Diantz
    "makecab.exe:",
    "diantz.exe:",
    // Replace
    "replace.exe:",
    // Pcalua
    "pcalua.exe:-a",
    // Presentationhost
    "presentationhost.exe:",
    // Dfsvc
    "dfsvc.exe:",
    // Ieframe
    "ieframe.dll:openurl",
    // Mshtml
    "mshtml.dll:",
    // Setupapi
    "setupapi.dll:installhinfsection",
    // Advpack/Ieadvpack
    "advpack.dll:launchinfsection",
    "ieadvpack.dll:launchinfsection",
    // Shell32
    "shell32.dll:shellexec_rundll",
    // Syssetup
    "syssetup.dll:setupoperqueuecallback",
    // Zipfldr
    "zipfldr.dll:routetheCall",
    // FTP
    "ftp.exe:-s:",
    // Finger
    "finger.exe:",
    // Certreq
    "certreq.exe:-post",
    "certreq.exe:-config",
    // Desktopimgdownldr
    "desktopimgdownldr.exe:/lockscreenurl:",
    // AppVLP
    "appvlp.exe:",
    // SyncAppvPublishingServer
    "syncappvpublishingserver.exe:",
    "syncappvpublishingserver.vbs:",
    // Bash/WSL
    "bash.exe:-c",
    "wsl.exe:-e",
    "wsl.exe:--exec",
    // Verclsid
    "verclsid.exe:/s",
    // Xwizard
    "xwizard.exe:rundll",
    // Remote
    "remote.exe:/s",
    // Netsh
    "netsh.exe:add helper",
    // SC
    "sc.exe:create",
    "sc.exe:config",
    // Procdump/Sqldumper
    "procdump.exe:-ma",
    "sqldumper.exe:",
    "rdrleakdiag.exe:",
    "createdump.exe:",
    "dump64.exe:",
    "dumpminitool.exe:",
    "tttracer.exe:-dump",
    // Credential Access
    "comsvcs.dll:minidump",
    "cmdkey.exe:/list",
    "rpcping.exe:-s",
    "ntdsutil.exe:activate instance ntds",
    // Visual Studio Tools
    "msbuild.exe:",
    "vbc.exe:",
    "csc.exe:",
    "jsc.exe:",
    "fsi.exe:",
    "fsianycpu.exe:",
    "csi.exe:",
    "rcsi.exe:",
    "dotnet.exe:",
    "dnx.exe:",
    "vstest.console.exe:",
    "mftrace.exe:",
    "tracker.exe:",
    // Windows Debugging
    "cdb.exe:",
    "ntsd.exe:",
    "windbg.exe:",
    // Misc Execution
    "pcwrun.exe:",
    "runonce.exe:",
    "forfiles.exe:",
    "conhost.exe:0xffffffff -forcev1",
    "explorer.exe:/root",
    // UAC Bypass
    "eventvwr.exe:",
    "computerdefaults.exe:",
    "sdclt.exe:",
    "fodhelper.exe:",
    "wsreset.exe:",
    // Pubprn
    "pubprn.vbs:script:",
    // WinRM
    "winrm.vbs:",
    // Office
    "winword.exe:http:",
    "excel.exe:http:",
    "powerpnt.exe:http:",
    "msaccess.exe:http:",
    "mspub.exe:http:",
    "visio.exe:http:",
    // Bginfo
    "bginfo.exe:",
    // Msxsl
    "msxsl.exe:",
    // Squirrel/Update
    "squirrel.exe:--download",
    "update.exe:--download",
    // IntelliTrace/DevTools
    "intellitrace.exe:",
    "devtoolslauncher.exe:",
    // MSEdge
    "msedge.exe:--headless",
    "msedge_proxy.exe:",
    // ClickOnce
    "dfsvc.exe:",
    "dfshim.dll:shaopendocument",
    // App Whitelist Bypass
    "coregen.exe:",
    "dxcap.exe:",
    "ttdinject.exe:",
    // Register-CimProvider
    "register-cimprovider.exe:-path",
    // OneDrive
    "onedrivestandaloneupdater.exe:",
    // Additional
    "replace.exe:",
    "print.exe:",
    "extexport.exe:",
    "ie4uinit.exe:-basesettings",
    "infdefaultinstall.exe:",
    "gpscript.exe:",
    // DevTunnel
    "devtunnel.exe:",
    // Wt
    "wt.exe:-d",
    // Tar
    "tar.exe:-xf",
    // StorDiag
    "stordiag.exe:-collectEtw",
    // IMEWDBLD
    "imewdbld.exe:",
    // Cmdl32
    "cmdl32.exe:",
    // ConfigSecurityPolicy
    "configsecuritypolicy.exe:",
    // Cipher
    "cipher.exe:/w",
    // FltMC
    "fltmc.exe:unload",
    // Fsutil
    "fsutil.exe:usn deletejournal",
    "fsutil.exe:behavior set disablelastaccess",
    // PrintBRM
    "printbrm.exe:-r",
    // Pktmon
    "pktmon.exe:start",
    // PSR
    "psr.exe:/start",
    // Msdeploy
    "msdeploy.exe:-verb:sync",
    // Bcp
    "bcp.exe:queryout",
    // Dtutil
    "dtutil.exe:/copy",
    // MpCmdRun
    "mpcmdrun.exe:-downloadfile",
    // Ldifde
    "ldifde.exe:-f",
    // Teams
    "teams.exe:--remote-debugging-port",
    // Vsls-agent
    "vsls-agent.exe:",
    // Misc Microsoft Binaries
    "visualuiaverifynative.exe:",
    "acccheckconsole.exe:",
    "applauncherwinapp.exe:",
    "defaultpack.exe:",
    "provlaunch.exe:",
    "runexehelper.exe:",
    "runscripthelper.exe:",
    "scriptrunner.exe:",
    "settingsynchost.exe:",
    "customshellhost.exe:",
    "devicecredentialdeployment.exe:",
    "iediagcmd.exe:",
    "msdt.exe:",
    "offlinescannershell.exe:",
    "wab.exe:/open",
    "workfolders.exe:",
    "wfc.exe:",
    "wfmformat.exe:",
    "testwindowremoteagent.exe:",
    // Scripts
    "cl_loadassembly.ps1:",
    "cl_mutexverifiers.ps1:",
    "cl_invocation.ps1:",
    "launch-vsdevshell.ps1:",
    "manage-bde.wsf:",
    "utilityfunctions.ps1:",
    "pester.bat:",
    // Additional DLLs
    "pcwutl.dll:",
    "photoviewer.dll:imageview_fullscreen",
    "scrobj.dll:",
    "shimgvw.dll:imageview_fullscreen",
    "shdocvw.dll:openurl",
    "url.dll:openurl",
    "url.dll:fileprotocolhandler",
};
const int g_payloadCount = sizeof(g_payloads) / sizeof(g_payloads[0]);
