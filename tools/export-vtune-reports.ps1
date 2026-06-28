# Export all VTune Stochfit analysis results to CSV for review.
# Usage: .\tools\export-vtune-reports.ps1
# Output: $env:TEMP\vtune_exports\*.csv
#
# Prerequisites: Close VTune GUI first (it locks the sqlite-db directories).

$vtune   = "C:\Program Files (x86)\Intel\oneAPI\vtune\2026.0\bin64\vtune.exe"
$project = "C:\Users\Stephen\Documents\VTune\Projects\Stochfit"
$out     = "$env:TEMP\vtune_exports"

New-Item -ItemType Directory -Force -Path $out | Out-Null

# Map result directory suffix → (report type, friendly name)
# Add new entries here as new analyses are collected.
$reports = @(
    [pscustomobject]@{ Dir="r000ps"; Type="hotspots";  Name="perf-snapshot"          },
    [pscustomobject]@{ Dir="r001hs"; Type="hotspots";  Name="hotspots"               },
    [pscustomobject]@{ Dir="r002ue"; Type="hotspots";  Name="microarch-exploration"  },
    [pscustomobject]@{ Dir="r003hpc"; Type="hotspots"; Name="hpc-perf"               },
    [pscustomobject]@{ Dir="r004tr"; Type="hotspots";  Name="threading"              }
)

foreach ($r in $reports) {
    $src = "$project\$($r.Dir)"
    if (-not (Test-Path $src)) {
        Write-Host "SKIP $($r.Dir) — directory not found"
        continue
    }
    $dest = "$out\$($r.Name).csv"
    Write-Host "Exporting $($r.Dir) → $($r.Name).csv ..." -NoNewline
    & $vtune -report $r.Type -r $src -format csv -report-output $dest 2>&1 | Out-Null
    if (Test-Path $dest) {
        $size = (Get-Item $dest).Length
        if ($size -gt 0) {
            Write-Host " OK ($([math]::Round($size/1KB, 1)) KB)"
        } else {
            Write-Host " EMPTY (wrong report type or analysis has no data)"
        }
    } else {
        Write-Host " FAILED"
    }
}

Write-Host "`nExports written to: $out"
Get-ChildItem $out -Filter "*.csv" | Select-Object Name, @{N="KB";E={[math]::Round($_.Length/1KB,1)}}
