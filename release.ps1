param(
    [string]$Version = "0.0.0"
)

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
$project = Join-Path $root "vs.proj\NppJavaPlugin.vcxproj"
$dist = Join-Path $root "dist"

if (-not (Test-Path $msbuild)) {
    throw "MSBuild not found at: $msbuild"
}

if (Test-Path $dist) {
    Remove-Item $dist -Recurse -Force
}
New-Item -ItemType Directory -Path $dist | Out-Null

function Invoke-Build([string]$platform) {
    & $msbuild $project /t:Clean,Build /p:Configuration=Release /p:Platform=$platform /m
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed: $platform"
    }
}

Invoke-Build "x64"
Invoke-Build "Win32"

function New-Package([string]$arch, [string]$binDir) {
    $packageName = "NppJavaPlugin_v$Version`_$arch"
    $stagingRoot = Join-Path $dist ("staging_" + $arch)
    $packageRoot = Join-Path $stagingRoot "NppJavaPlugin"

    if (Test-Path $stagingRoot) {
        Remove-Item $stagingRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Path $packageRoot | Out-Null

    $files = @(
        "NppJavaPlugin.dll",
        "NppJavaPlugin.ini",
        "cfr-0.152.jar",
        "license.txt"
    )

    foreach ($file in $files) {
        $src = Join-Path $binDir $file
        if (-not (Test-Path $src)) {
            throw "Missing file: $src"
        }
        Copy-Item $src $packageRoot -Force
    }

    $zipPath = Join-Path $dist ($packageName + ".zip")
    Compress-Archive -Path $packageRoot -DestinationPath $zipPath -Force

    Remove-Item $stagingRoot -Recurse -Force
}

New-Package "x64" (Join-Path $root "bin64")
New-Package "x86" (Join-Path $root "bin")

Write-Host "Packages created in: $dist"
Write-Host "- NppJavaPlugin_v$Version`_x64.zip"
Write-Host "- NppJavaPlugin_v$Version`_x86.zip"
