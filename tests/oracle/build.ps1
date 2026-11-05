# Builds and runs the oracle (32-bit only: it hosts the original x86 code).
#   .\tests\oracle\build.ps1            # build + run all tests
#   .\tests\oracle\build.ps1 AlignUp64  # build + run tests whose name contains the filter
#
# Two binaries (see host.cpp for why):
#   oracle_host.exe  CRT-free placeholder linked at 0x00400000
#   oracle.dll       the loader, the runner and the tests, linked at 0x10000000
param([string]$Filter = "")

$root = Resolve-Path "$PSScriptRoot\..\.."
$out = "$root\build\oracle"
New-Item -ItemType Directory -Force $out | Out-Null

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
# vcvarsall.bat calls vswhere by name, so its folder must be on PATH.
$env:PATH = "$(Split-Path $vswhere);$env:PATH"
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$vcvars = "$vs\VC\Auxiliary\Build\vcvarsall.bat"

$sources = @(
    "$root\tests\oracle\main.cpp",
    "$root\tests\oracle\pe_image.cpp",
    "$root\tests\oracle\test_core.cpp",
    "$root\tests\oracle\test_gameapp.cpp",
    "$root\tests\oracle\stubs_gameapp.cpp",
    "$root\decomp\core\ProcessCheck.cpp",
    "$root\decomp\core\GameApp.cpp"
) -join '" "'

# _CRT_SECURE_NO_WARNINGS: decompiled code keeps the CRT calls the original
# makes (strncpy...), so the "use _s versions" warnings do not apply.
$dll = "cl /nologo /std:c++17 /EHsc /O2 /W3 /utf-8 /MT /LD /D_CRT_SECURE_NO_WARNINGS/Fo`"$out\\`" /Fe`"$out\oracle.dll`" `"$sources`" " +
       "/link /BASE:0x10000000 /DYNAMICBASE:NO"
$host_ = "cl /nologo /O1 /GS- /Fo`"$out\\`" /Fe`"$out\oracle_host.exe`" `"$root\tests\oracle\host.cpp`" " +
         "/link /BASE:0x400000 /FIXED /DYNAMICBASE:NO /NODEFAULTLIB /ENTRY:HostEntry " +
         "/SUBSYSTEM:CONSOLE kernel32.lib"
cmd /c "`"$vcvars`" x86 >nul && $dll && $host_"
if ($LASTEXITCODE -ne 0) { Write-Output "build failed"; exit 2 }

& "$out\oracle_host.exe" "$root\ORIGINAL\Manhunt2_nosecu.exe" $Filter
exit $LASTEXITCODE
