<#
  Batch test for ogl_init_triangle. Each combination runs in its own process, so a driver crash
  only fails that row. Output: results\<label>.csv and results\<label>.jsonl

  Examples:
    .\run_matrix.ps1 -Exe ..\..\build\vs2022-x64\ogl_init\Release\ogl_init_triangle.exe -Label baseline
    .\run_matrix.ps1 -Exe ... -Label baseline -Driver mesa=C:\mesa\x64   # folder holding Mesa's opengl32.dll
#>
param(
    [Parameter(Mandatory)] [string]$Exe,
    [string]$Label = 'run',
    # name=folder pairs; the folder's opengl32.dll is copied next to a private copy of the exe
    # (opengl32.dll is not a KnownDLL, so the application directory wins over System32).
    [string[]]$Driver = @(),
    [string[]]$Versions = @('3.3', '4.0', '4.1', '4.2', '4.3', '4.4', '4.5', '4.6'),
    [string[]]$Fb = @('linear', 'srgb', 'hdr'),
    [int[]]$Msaa = @(0, 4),
    [switch]$AllowLegacy,
    [int]$TimeoutSec = 30
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $PSScriptRoot '..\results'
New-Item -ItemType Directory -Force $out | Out-Null
$Exe = (Resolve-Path $Exe).Path

$targets = @([pscustomobject]@{ Name = 'system'; Exe = $Exe })
foreach ($d in $Driver) {
    $name, $dir = $d -split '=', 2
    $work = Join-Path $env:TEMP "ogl_init_$name"
    New-Item -ItemType Directory -Force $work | Out-Null
    Copy-Item $Exe $work -Force
    Copy-Item (Join-Path $dir 'opengl32.dll') $work -Force
    Get-ChildItem $dir -Filter *.dll | Copy-Item -Destination $work -Force
    $targets += [pscustomobject]@{ Name = $name; Exe = (Join-Path $work (Split-Path $Exe -Leaf)) }
}

$rows = New-Object System.Collections.Generic.List[object]
$jsonl = Join-Path $out "$Label.jsonl"
Remove-Item $jsonl -ErrorAction SilentlyContinue
foreach ($t in $targets) { foreach ($v in $Versions) { foreach ($f in $Fb) { foreach ($m in $Msaa) { foreach ($dbg in 0, 1) {
    $argl = @('--batch', '--gl', $v, '--fb', $f, '--msaa', $m)
    if ($dbg) { $argl += '--debug' }
    if ($AllowLegacy) { $argl += '--allow-legacy' }
    $p = Start-Process -FilePath $t.Exe -ArgumentList $argl -NoNewWindow -PassThru `
         -RedirectStandardOutput "$env:TEMP\ogl_init_out.txt" -RedirectStandardError "$env:TEMP\ogl_init_err.txt"
    if (-not $p.WaitForExit($TimeoutSec * 1000)) { $p.Kill(); $code = 'timeout' } else { $code = $p.ExitCode }
    $line = (Get-Content "$env:TEMP\ogl_init_out.txt" -ErrorAction SilentlyContinue | Select-Object -Last 1)
    $j = $null
    if ($line) { try { $j = $line | ConvertFrom-Json } catch { } }
    if ($line) { Add-Content $jsonl ("{`"driver`":`"$($t.Name)`",`"result`":" + $line + '}') }
    $rows.Add([pscustomobject]@{
        driver = $t.Name; req = $v; fb = $f; msaa = $m; debug = $dbg; exit = $code
        initRet = $j.initRet; context = $j.context; got = $j.got; profileMask = $j.profileMask
        legacyFallback = $j.legacyFallback; versionMismatch = $j.versionMismatch
        setPixelFormat = $j.setPixelFormat; setPixelFormatFailed = $j.setPixelFormatFailed
        legacyCreate = $j.legacyCreate; pixelPass = $j.pixelPass; renderer = $j.renderer; error = $j.error })
}}}}}
$csv = Join-Path $out "$Label.csv"
$rows | Export-Csv $csv -NoTypeInformation
$rows | Group-Object driver | ForEach-Object {
    $n = $_.Count; $ok = ($_.Group | Where-Object { $_.pixelPass -eq $true -and -not $_.legacyFallback }).Count
    "{0}: {1}/{2} clean (context + triangle pixel, no legacy fallback)" -f $_.Name, $ok, $n
}
"wrote $csv"
