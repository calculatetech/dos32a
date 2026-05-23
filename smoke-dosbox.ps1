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

$stageTag = ('{0:X4}' -f (Get-Random -Maximum 0x10000))
$standName = "D32S$stageTag"
$appName = "D32A$stageTag"
$batchName = "D32B$stageTag.BAT"
$standStage = Join-Path $driveZ $standName
$appStage = Join-Path $driveZ $appName
$appBin = Join-Path $appStage 'BINW'
$log = Join-Path $driveZ "$standName.LOG"
$batch = Join-Path $driveZ $batchName
$process = $null

function Clear-SmokeStage {
    foreach ($path in @($standStage, $appStage, $log, $batch)) {
        Remove-Item -LiteralPath $path -Recurse -Force -ErrorAction SilentlyContinue
    }
}

function Exit-Smoke {
    param([int]$Code)

    Clear-SmokeStage
    exit $Code
}
$dos32a = Join-Path $repo 'binw\dos32a.exe'
$sver = Join-Path $repo 'binw\sver.exe'
$hello = Join-Path $repo 'dos32a_800\examples\asm_1\hello.exe'

if (-not (Test-Path -LiteralPath $dos32a)) { throw "Missing build output: $dos32a." }
if (-not (Test-Path -LiteralPath $sver)) { throw "Missing build output: $sver." }
if (-not (Test-Path -LiteralPath $hello)) { throw "Missing DOS/32A sample: $hello." }

foreach ($path in @($standStage, $appStage, $log, $batch)) {
    Remove-Item -LiteralPath $path -Recurse -Force -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Path $standStage, $appBin -Force | Out-Null
Copy-Item -LiteralPath $dos32a -Destination (Join-Path $standStage 'DOS32A.EXE') -Force
Copy-Item -LiteralPath $sver -Destination (Join-Path $standStage 'SVER.EXE') -Force
Copy-Item -LiteralPath $hello -Destination (Join-Path $standStage 'HELLO.EXE') -Force
Copy-Item -LiteralPath $dos32a -Destination (Join-Path $appBin 'DOS32A.EXE') -Force
Copy-Item -LiteralPath $hello -Destination (Join-Path $appStage 'AHELLO.EXE') -Force

$batchLines = @(
    '@echo off',
    'z:',
    "cd $standName",
    'SVER.EXE DOS32A.EXE',
    'echo SVER_SMOKE_OK',
    'set DOS32A=/PRINT:ON',
    'DOS32A.EXE HELLO.EXE',
    'echo DIRECT_LOADER_SMOKE_OK',
    'set DOS32A=',
    'DOS32A.EXE',
    'echo DOS32A_SMOKE_OK',
    'cd ..',
    "cd $appName",
    "set DOS32A=Z:\$appName",
    'AHELLO.EXE',
    'echo APP_SMOKE_OK',
    'exit'
)
Set-Content -LiteralPath $batch -Value $batchLines -Encoding ASCII

$arguments = @(
    '-set "sdl waitonerror=false"',
    ('-set "log logfile={0}"' -f $log),
    '-set "log exec=true"',
    '-set "dos nocachedir=true"',
    '-set "dos log console=true"',
    '-c "z:"',
    ('-c "{0}"' -f $batchName)
) -join ' '

$process = Start-Process -FilePath $dosbox -ArgumentList $arguments -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    $process.Kill()
    $process.WaitForExit()
    Write-Host "DOSBox-X timed out after $TimeoutSeconds seconds."
    Show-SmokeLogTail -Path $log
    Exit-Smoke 124
}

if ($process.ExitCode -ne 0) {
    Write-Host "DOSBox-X exited with code $($process.ExitCode)."
    Show-SmokeLogTail -Path $log
    Exit-Smoke $process.ExitCode
}

if (-not (Test-Path -LiteralPath $log)) {
    Write-Host "DOSBox-X did not create $log."
    Exit-Smoke 1
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
        Exit-Smoke 1
    }
}

if (-not $logText.Contains('SVER_SMOKE_OK')) {
    Write-Host 'SVER smoke command did not complete.'
    Show-SmokeLogTail -Path $log
    Exit-Smoke 1
}
if (-not $logText.Contains('DIRECT_LOADER_SMOKE_OK')) {
    Write-Host 'External DOS/32A loader smoke command did not return to the DOSBox-X shell.'
    Show-SmokeLogTail -Path $log
    Exit-Smoke 1
}
if (-not $logText.Contains('DOS32A_SMOKE_OK')) {
    Write-Host 'Standalone DOS/32A smoke command did not return to the DOSBox-X shell.'
    Show-SmokeLogTail -Path $log
    Exit-Smoke 1
}
if (-not $logText.Contains('Hello world from protected mode!!!')) {
    Write-Host 'DOS/32A did not transfer control to the protected-mode sample.'
    Show-SmokeLogTail -Path $log
    Exit-Smoke 1
}
if (-not $logText.Contains('APP_SMOKE_OK')) {
    Write-Host 'Protected-mode sample did not return to the DOSBox-X shell.'
    Show-SmokeLogTail -Path $log
    Exit-Smoke 1
}
if (-not $logText.Contains('Version:        26.1')) {
    Write-Host 'SVER did not report DOS/32A version 26.1.'
    Show-SmokeLogTail -Path $log
    Exit-Smoke 1
}

Select-String -Path $log -Pattern 'SVER -- Version|Version:        26\.1|SVER_SMOKE_OK|DIRECT_LOADER_SMOKE_OK|DOS32A_SMOKE_OK|Hello world from protected mode!!!|APP_SMOKE_OK' |
    ForEach-Object { $_.Line -replace '^\s*DOS CON:\s*', '' }

Write-Host "DOSBox-X smoke passed using temporary staged files in $standStage and $appStage."
Exit-Smoke 0
