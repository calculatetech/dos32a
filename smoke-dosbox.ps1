param(
    [int]$TimeoutSeconds = 20
)

$ErrorActionPreference = 'Stop'

function Show-SmokeLogTail {
    param([string]$Path)

    if (Test-Path -LiteralPath $Path) {
        Write-Host "Last DOSBox-X log lines from ${Path}:"
        Get-Content -LiteralPath $Path -Tail 40
    }
}

$dosbox = $env:DOSBOX_X
$repo = $env:DOS32A

if (-not $dosbox) { throw 'DOSBOX_X is not set.' }
if (-not $repo) { throw 'DOS32A is not set.' }
if (-not (Test-Path -LiteralPath $dosbox)) { throw "DOSBox-X was not found at $dosbox." }

$driveZ = $env:DOSBOX_DRIVEZ
if (-not $driveZ) {
    $driveZ = Join-Path (Split-Path -Parent $dosbox) 'drivez'
}
if (-not (Test-Path -LiteralPath $driveZ)) { throw "DOSBox-X drive Z folder was not found at $driveZ." }

$stage = Join-Path $driveZ 'D32SMK'
$log = Join-Path $stage 'SMOKE.LOG'
$dos32a = Join-Path $repo 'binw\dos32a.exe'
$sver = Join-Path $repo 'binw\sver.exe'

if (-not (Test-Path -LiteralPath $dos32a)) { throw "Missing build output: $dos32a." }
if (-not (Test-Path -LiteralPath $sver)) { throw "Missing build output: $sver." }

New-Item -ItemType Directory -Path $stage -Force | Out-Null
Remove-Item -LiteralPath $log -ErrorAction SilentlyContinue
Copy-Item -LiteralPath $dos32a -Destination (Join-Path $stage 'DOS32A.EXE') -Force
Copy-Item -LiteralPath $sver -Destination (Join-Path $stage 'SVER.EXE') -Force

$arguments = @(
    '-set "sdl waitonerror=false"',
    ('-set "log logfile={0}"' -f $log),
    '-set "dos log console=true"',
    '-c "z:"',
    '-c "cd D32SMK"',
    '-c "SVER.EXE DOS32A.EXE"',
    '-c "echo SVER_SMOKE_OK"',
    '-c "DOS32A.EXE"',
    '-c "echo DOS32A_SMOKE_OK"',
    '-c "exit"'
) -join ' '

$process = Start-Process -FilePath $dosbox -ArgumentList $arguments -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    $process.Kill()
    $process.WaitForExit()
    Write-Host "DOSBox-X timed out after $TimeoutSeconds seconds."
    Show-SmokeLogTail -Path $log
    exit 124
}

if ($process.ExitCode -ne 0) {
    Write-Host "DOSBox-X exited with code $($process.ExitCode)."
    Show-SmokeLogTail -Path $log
    exit $process.ExitCode
}

if (-not (Test-Path -LiteralPath $log)) {
    Write-Host "DOSBox-X did not create $log."
    exit 1
}

$logText = Get-Content -LiteralPath $log -Raw
$badPatterns = @(
    'E_Exit:',
    'Illegal descriptor',
    "DYNX86:Can't run code",
    'ERROR CPU:CPU:GRP5:Illegal opcode',
    'ERROR CPU:Illegal Unhandled Interrupt'
)

foreach ($pattern in $badPatterns) {
    if ($logText.Contains($pattern)) {
        Write-Host "DOSBox-X reported crash pattern: $pattern"
        Show-SmokeLogTail -Path $log
        exit 1
    }
}

if (-not $logText.Contains('SVER_SMOKE_OK')) {
    Write-Host 'SVER smoke command did not complete.'
    Show-SmokeLogTail -Path $log
    exit 1
}
if (-not $logText.Contains('DOS32A_SMOKE_OK')) {
    Write-Host 'Standalone DOS/32A smoke command did not return to the DOSBox-X shell.'
    Show-SmokeLogTail -Path $log
    exit 1
}
if (-not $logText.Contains('Version:        9.1.2')) {
    Write-Host 'SVER did not report DOS/32A version 9.1.2.'
    Show-SmokeLogTail -Path $log
    exit 1
}

Select-String -Path $log -Pattern 'SVER -- Version|Version:        9\.1\.2|SVER_SMOKE_OK|DOS32A_SMOKE_OK' |
    ForEach-Object { $_.Line -replace '^\s*DOS CON:\s*', '' }

Write-Host "DOSBox-X smoke passed using staged files in $stage."
exit 0
