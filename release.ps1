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
    try {
        Remove-Item $dist -Recurse -Force -ErrorAction Stop
    } catch {
        $dist = Join-Path $root ("dist_" + (Get-Date -Format "yyyyMMdd_HHmmss"))
    }
}
New-Item -ItemType Directory -Path $dist -Force | Out-Null

function Test-Arm64Toolset() {
    $vcRoot = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC"
    if (-not (Test-Path $vcRoot)) { return $false }
    foreach ($dir in Get-ChildItem -Path $vcRoot -Directory -ErrorAction SilentlyContinue) {
        if (Test-Path (Join-Path $dir.FullName "bin\Hostx64\arm64")) { return $true }
        if (Test-Path (Join-Path $dir.FullName "bin\Hostx86\arm64")) { return $true }
    }
    return $false
}

function Invoke-Build([string]$platform) {
    & $msbuild $project /t:Clean,Build /p:Configuration=Release /p:Platform=$platform /m
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed: $platform"
    }
}

Invoke-Build "x64"
Invoke-Build "Win32"

$arm64Enabled = Test-Arm64Toolset
if ($arm64Enabled) {
    Invoke-Build "ARM64"
} else {
    Write-Host "ARM64 toolset not found. Skipping ARM64 build."
}

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
if ($arm64Enabled) {
    New-Package "arm64" (Join-Path $root "binARM64")
}

Write-Host "Packages created in: $dist"
Write-Host "- NppJavaPlugin_v$Version`_x64.zip"
Write-Host "- NppJavaPlugin_v$Version`_x86.zip"
if ($arm64Enabled) {
    Write-Host "- NppJavaPlugin_v$Version`_arm64.zip"
}
