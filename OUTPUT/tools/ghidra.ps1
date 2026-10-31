# Runs a Ghidra headless script read-only against the analyzed MH2 project.
#   .\OUTPUT\tools\ghidra.ps1 CallTree.java 0x61c2a1 3 OUTPUT\tree.txt
param([Parameter(Mandatory)][string]$Script, [Parameter(ValueFromRemainingArguments)][string[]]$ScriptArgs)
$root = Resolve-Path "$PSScriptRoot\..\.."
if (-not $env:JAVA_HOME) { $env:JAVA_HOME = "C:\Program Files\Microsoft\jdk-21.0.12.8-hotspot" }
& "$root\TOOLS\ghidra_12.1.2_PUBLIC\support\analyzeHeadless.bat" "$root\OUTPUT\ghidra" MH2 `
    -process Manhunt2_nosecu.exe -noanalysis -readOnly -scriptPath "$root\OUTPUT\tools" `
    -postScript $Script @ScriptArgs *> "$root\OUTPUT\ghidra_script.log"
Select-String -Path "$root\OUTPUT\ghidra_script.log" -Pattern "\.java> " | ForEach-Object { $_.Line -replace '^.*\.java> ', '' -replace ' \(GhidraScript\)\s*$', '' }
