# Collect VTune hotspots + threading analyses on stochfit_profile.exe,
# then export CSVs to $env:TEMP\vtune_exports with a "new_" prefix.
#
# Usage: .\tools\collect-vtune-profile.ps1
#        .\tools\collect-vtune-profile.ps1 -Duration 30 -Config RelWithDebInfo
#
# Prerequisites:
#   - VTune installed at the path below
#   - stochfit_profile.exe built (cmake --build build --preset windows --target stochfit_profile)
#   - Run from the repo root

param(
    [int]   $Duration   = 22,        # total VTune collection window in seconds
    [int]   $StartDelay = 2,         # seconds to skip (app startup / data load)
    [string]$Config     = "RelWithDebInfo",
    [int]   $Iterations = 200000000  # large enough that SA runs past the collection window
)

$vtune   = "C:\Program Files (x86)\Intel\oneAPI\vtune\2026.0\bin64\vtune.exe"
$project = "C:\Users\Stephen\Documents\VTune\Projects\Stochfit"
$out     = "$env:TEMP\vtune_exports"
$exe     = "$PSScriptRoot\..\build\$Config\bin\stochfit_profile.exe"

if (-not (Test-Path $vtune))   { Write-Error "VTune not found at: $vtune"; exit 1 }
if (-not (Test-Path $exe))     { Write-Error "Profile exe not found: $exe`nBuild it first with: cmake --build build --preset windows --target stochfit_profile"; exit 1 }

New-Item -ItemType Directory -Force -Path $out | Out-Null
New-Item -ItemType Directory -Force -Path $project | Out-Null

$analyses = @(
    [pscustomobject]@{ Collect = "hotspots";        Dir = "r005hs_new"; Name = "new_hotspots"  },
    [pscustomobject]@{ Collect = "threading";       Dir = "r006tr_new"; Name = "new_threading" }
)

foreach ($a in $analyses) {
    $resultDir = "$project\$($a.Dir)"

    # Remove stale result dir so VTune doesn't prompt to overwrite
    if (Test-Path $resultDir) {
        Remove-Item $resultDir -Recurse -Force
    }

    Write-Host "`n=== Collecting $($a.Collect) (${Duration}s window, ${StartDelay}s startup skip) ===" -ForegroundColor Cyan

    # -resume-after: skip the first N seconds of app runtime before collecting
    # -duration: collect for this many seconds after the skip (then terminate the app)
    & $vtune `
        -collect $a.Collect `
        -resume-after $StartDelay `
        -duration $Duration `
        -result-dir $resultDir `
        -app-working-dir (Split-Path $exe) `
        -- $exe --iterations=$Iterations

    if ($LASTEXITCODE -ne 0) {
        Write-Warning "VTune exited with code $LASTEXITCODE for $($a.Collect)"
    }

    # Export to CSV
    $dest = "$out\$($a.Name).csv"
    Write-Host "Exporting → $($a.Name).csv ..." -NoNewline
    & $vtune -report hotspots -r $resultDir -format csv -report-output $dest 2>&1 | Out-Null
    if (Test-Path $dest) {
        $kb = [math]::Round((Get-Item $dest).Length / 1KB, 1)
        Write-Host " OK (${kb} KB)" -ForegroundColor Green
    } else {
        Write-Host " FAILED" -ForegroundColor Red
    }
}

Write-Host "`nDone. New exports:" -ForegroundColor Cyan
Get-ChildItem $out -Filter "new_*.csv" |
    Select-Object Name, @{N="KB"; E={[math]::Round($_.Length/1KB, 1)}}
