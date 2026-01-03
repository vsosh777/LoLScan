using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace App
{
    class Payloads
    {
        static public List<string> payloads = [
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
            
            // Certutil - Download/Decode
            "certutil.exe:-urlcache",
            "certutil.exe:-verifyctl",
            "certutil.exe:-decode",
            "certutil.exe:-encode",
            "certutil.exe:-decodehex",
            "certutil.exe:-encodehex",
            
            // Bitsadmin - Download
            "bitsadmin.exe:/transfer",
            "bitsadmin.exe:/create",
            "bitsadmin.exe:/addfile",
            "bitsadmin.exe:/setnotifycmdline",
            
            // Regsvr32 - Squiblydoo
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
            
            // Mshta - HTML Application
            "mshta.exe:vbscript:",
            "mshta.exe:javascript:",
            "mshta.exe:http:",
            "mshta.exe:https:",
            "mshta.exe:.hta",
            
            // MSBuild - Code Execution
            "msbuild.exe:.csproj",
            "msbuild.exe:.xml",
            "msbuild.exe:/p:",
            
            // Installutil - .NET Bypass
            "installutil.exe:/logfile=",
            "installutil.exe:/logtoconsole=false",
            "installutil.exe:/u",
            
            // Regasm/Regsvcs - .NET
            "regasm.exe:/u",
            "regasm.exe:/codebase",
            "regsvcs.exe:/u",
            "regsvcs.exe:/codebase",
            
            // WMIC - Remote Execution
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
            
            // Msiexec - Remote Install
            "msiexec.exe:/i",
            "msiexec.exe:/quiet",
            "msiexec.exe:/q",
            "msiexec.exe:http:",
            "msiexec.exe:https:",
            "msiexec.exe:/y",
            "msiexec.exe:/z",
            
            // Cmstp - UAC Bypass
            "cmstp.exe:/s",
            "cmstp.exe:/au",
            "cmstp.exe:.inf",
            
            // Odbcconf - DLL Execution
            "odbcconf.exe:/a",
            "odbcconf.exe:-f",
            "odbcconf.exe:regsvr",
            
            // IEExec - .NET Download/Execute
            "ieexec.exe:http:",
            "ieexec.exe:https:",
            
            // PsExec-like
            "psexec.exe:-s",
            "psexec.exe:\\\\",
            
            // Cmd.exe Suspicious
            "cmd.exe:/c echo",
            "cmd.exe:/c powershell",
            "cmd.exe:/c start",
            "cmd.exe:/k",
            
            // MSHTAsc/Compiler
            "microsoft.workflow.compiler.exe:",
            
            // DotNet
            "dotnet.exe:build",
            "dotnet.exe:run",
            "dotnet.exe:exec",
            
            // CSC/VBC - Compilers
            "csc.exe:/out:",
            "vbc.exe:/out:",
            "jsc.exe:/out:",
            
            // Mavinject - DLL Injection
            "mavinject.exe:/injectrunning",
            
            // Forfiles - Indirect Execution
            "forfiles.exe:/c",
            
            // PsExec64/PsExec
            "psexec64.exe:-s",
            "psexec64.exe:\\\\",
            
            // At - Scheduled Task (Deprecated)
            "at.exe:\\\\",
            
            // Schtasks - Task Scheduler
            "schtasks.exe:/create",
            "schtasks.exe:/run",
            "schtasks.exe:/s",
            
            // Control.exe - CPL Execution
            "control.exe:.cpl",
            
            // Diskshadow - Volume Shadow Copy
            "diskshadow.exe:/s",
            
            // Esentutl - File Copy
            "esentutl.exe:/y",
            "esentutl.exe:/vss",
            
            // Expand - Extract Files
            "expand.exe:-r",
            
            // Extrac32 - Extract CAB
            "extrac32.exe:/y",
            "extrac32.exe:/c",
            
            // Findstr - ADS
            "findstr.exe:/v",
            "findstr.exe:/s",
            
            // Hh.exe - CHM Execution
            "hh.exe:.chm",
            
            // Makecab/Diantz - Compress
            "makecab.exe:",
            "diantz.exe:",
            
            // Replace - File Replace
            "replace.exe:",
            
            // Pcalua - Program Compatibility
            "pcalua.exe:-a",
            
            // Presentationhost - XBAP
            "presentationhost.exe:",
            
            // Dfsvc - ClickOnce
            "dfsvc.exe:",
            
            // Ieframe - IE DLL
            "ieframe.dll:openurl",
            
            // Mshtml - HTML Application
            "mshtml.dll:",
            
            // Setupapi - INF Execution
            "setupapi.dll:installhinfsection",
            
            // Advpack/Ieadvpack - INF Execution  
            "advpack.dll:launchinfsection",
            "ieadvpack.dll:launchinfsection",
            
            // Shell32 - ShellExecute
            "shell32.dll:shellexec_rundll",
            
            // Syssetup - INF
            "syssetup.dll:setupoperqueuecallback",
            
            // Zipfldr - ZIP Execution
            "zipfldr.dll:routetheCall",
            
            // FTP - Download/Execute
            "ftp.exe:-s:",
            
            // Finger - Download
            "finger.exe:",
            
            // Certreq - Download/Upload
            "certreq.exe:-post",
            "certreq.exe:-config",
            
            // Desktopimgdownldr - Download
            "desktopimgdownldr.exe:/lockscreenurl:",
            
            // AppVLP - App-V
            "appvlp.exe:",
            
            // SyncAppvPublishingServer
            "syncappvpublishingserver.exe:",
            "syncappvpublishingserver.vbs:",
            
            // Bash/WSL
            "bash.exe:-c",
            "wsl.exe:-e",
            "wsl.exe:--exec",
            
            // Verclsid - COM
            "verclsid.exe:/s",
            
            // Xwizard - COM
            "xwizard.exe:rundll",
            
            // Remote - Debugging
            "remote.exe:/s",
            
            // Netsh - Helper DLL
            "netsh.exe:add helper",
            
            // SC - Service Creation
            "sc.exe:create",
            "sc.exe:config",
            
            // Procdump/Sqldumper - Memory Dump
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
            
            // Pubprn - Print Script
            "pubprn.vbs:script:",
            
            // WinRM Script
            "winrm.vbs:",
            
            // Office Applications - Download
            "winword.exe:http:",
            "excel.exe:http:",
            "powerpnt.exe:http:",
            "msaccess.exe:http:",
            "mspub.exe:http:",
            "visio.exe:http:",
            
            // Bginfo - VBS Execution
            "bginfo.exe:",
            
            // Msxsl - XSL Transform
            "msxsl.exe:",
            
            // Squirrel/Update - Package Manager
            "squirrel.exe:--download",
            "update.exe:--download",
            
            // IntelliTrace/DevTools
            "intellitrace.exe:",
            "devtoolslauncher.exe:",
            
            // MSEdge Execution
            "msedge.exe:--headless",
            "msedge_proxy.exe:",
            
            // ClickOnce Deployment
            "dfsvc.exe:",
            "dfshim.dll:shaopendocument",
            
            // Application Whitelist Bypass
            "coregen.exe:",
            "dxcap.exe:",
            "ttdinject.exe:",
            
            // Register-CimProvider
            "register-cimprovider.exe:-path",
            
            // OneDrive
            "onedrivestandaloneupdater.exe:",
            
            // Additional Suspicious
            "replace.exe:",
            "print.exe:",
            "extexport.exe:",
            "ie4uinit.exe:-basesettings",
            "infdefaultinstall.exe:",
            "gpscript.exe:",
            
            // DevTunnel
            "devtunnel.exe:",
            
            // Wt.exe - Windows Terminal
            "wt.exe:-d",
            
            // Tar - Extract with ADS
            "tar.exe:-xf",
            
            // StorDiag
            "stordiag.exe:-collectEtw",
            
            // IMEWDBLD
            "imewdbld.exe:",
            
            // Cmdl32
            "cmdl32.exe:",
            
            // ConfigSecurityPolicy
            "configsecuritypolicy.exe:",
            
            // Cipher - File Destruction
            "cipher.exe:/w",
            
            // FltMC - Filter Manager
            "fltmc.exe:unload",
            
            // Fsutil - USN Journal Deletion
            "fsutil.exe:usn deletejournal",
            "fsutil.exe:behavior set disablelastaccess",
            
            // PrintBRM
            "printbrm.exe:-r",
            
            // Pktmon - Packet Monitor
            "pktmon.exe:start",
            
            // PSR - Screen Recorder
            "psr.exe:/start",
            
            // Msdeploy
            "msdeploy.exe:-verb:sync",
            
            // Bcp - Bulk Copy
            "bcp.exe:queryout",
            
            // Dtutil - SSIS Package
            "dtutil.exe:/copy",
            
            // MpCmdRun - Windows Defender
            "mpcmdrun.exe:-downloadfile",
            
            // Ldifde - LDAP Export
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
        ];
    }
}