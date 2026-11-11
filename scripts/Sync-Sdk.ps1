param(
    [Parameter(Mandatory = $true)][string]$GLViewRoot
)
$ErrorActionPreference = 'Stop'
$kitRoot = Split-Path $PSScriptRoot -Parent
$sourceRoot = (Resolve-Path -LiteralPath $GLViewRoot).Path
$files = @(
    @{ Source = 'SDK/RenderingTester.h'; Destination = 'src/RenderingTester.h' },
    @{ Source = 'SDK/RenderingTester.cpp'; Destination = 'src/RenderingTester.cpp' },
    @{ Source = 'SDK/include/oevSDK.h'; Destination = 'include/oevSDK.h' }
)
# Validate the complete input before changing any files.
foreach ($file in $files) {
    if (-not (Test-Path -LiteralPath (Join-Path $sourceRoot $file.Source) -PathType Leaf)) {
        throw "Missing SDK source: $($file.Source)"
    }
}
foreach ($file in $files) {
    $content = [IO.File]::ReadAllText((Join-Path $sourceRoot $file.Source))
    if ($file.Source -eq 'SDK/RenderingTester.cpp') {
        $content = $content.Replace('"include/oevSDK.h"', '"oevSDK.h"')
    }
    # Normalize inherited trailing whitespace for clean, repeatable diffs.
    $content = [regex]::Replace($content, '[ \t]+(?=\r?$)', '', [Text.RegularExpressions.RegexOptions]::Multiline)
    [IO.File]::WriteAllText((Join-Path $kitRoot $file.Destination), $content,
        [Text.UTF8Encoding]::new($false))
}
Write-Host 'SDK wrapper and public header refreshed. Review the diff and rebuild before release.'
