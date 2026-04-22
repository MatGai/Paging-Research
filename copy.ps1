param(
    [string]$Src = "C:\Users\gagge\source\repos\MatGai\ScouseSystems\out\x64\Debug",
    [string]$Dst = "F:\"
)

$ErrorActionPreference = "Stop"

function Fail([string]$Msg) {
    Write-Host "ERROR: $Msg" -ForegroundColor Red
    exit 1
}

# Normalize destination to a drive root like "F:\"
try {
    $Dst = (Resolve-Path -LiteralPath $Dst).Path
} catch {
    Fail "Destination does not exist: $Dst"
}

# SAFETY: only allow F:\
if ($Dst -notmatch '^[Ff]:\\$') {
    Fail "Refusing to wipe non-F drive root. Dst must be exactly F:\ (got '$Dst')."
}

# SAFETY MARKER: must exist on destination root
$MarkerName = ".SCOUSE_DEPLOY_MARKER"
$MarkerPath = Join-Path $Dst $MarkerName
if (-not (Test-Path -LiteralPath $MarkerPath)) {
    Fail "Safety marker missing: $MarkerPath`nCreate it once to allow wiping F:\ :  New-Item -Path '$MarkerPath' -ItemType File -Force"
}

# Validate source
if (-not (Test-Path -LiteralPath $Src)) {
    Fail "Source does not exist: $Src"
}

Write-Host "Deploying..."
Write-Host "  SRC = $Src"
Write-Host "  DST = $Dst"

# Wipe destination contents (exclude system-protected folders)
$ExcludeNames = @("System Volume Information", '$RECYCLE.BIN')
$MarkerName   = ".SCOUSE_DEPLOY_MARKER"

Get-ChildItem -LiteralPath $Dst -Force |
    Where-Object { $ExcludeNames -notcontains $_.Name -and $_.Name -ne $MarkerName } |
    Remove-Item -Recurse -Force -ErrorAction Stop


# Copy with robocopy (exit codes 0-7 are success)
Write-Host "Copying..." -ForegroundColor Yellow
$robocopyArgs = @(
    $Src, $Dst,
    "/E",              # copy subdirs incl empty
    "/COPY:DAT",       # data, attributes, timestamps
    "/DCOPY:DAT",
    "/R:1", "/W:1",    # retry policy
    "/NFL","/NDL","/NJH","/NJS","/NP"
)

$proc = Start-Process -FilePath "robocopy.exe" -ArgumentList $robocopyArgs -Wait -PassThru
if ($proc.ExitCode -ge 8) {
    Fail "robocopy failed with exit code $($proc.ExitCode)"
}

New-Item -Path (Join-Path $Dst $MarkerName) -ItemType File -Force | Out-Null

Write-Host "DONE (robocopy exit code $($proc.ExitCode))" -ForegroundColor Green
exit 0
