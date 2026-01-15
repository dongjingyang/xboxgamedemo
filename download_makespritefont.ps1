# PowerShell script to download and extract MakeSpriteFont from DirectXTK
# MakeSpriteFont is part of DirectX Tool Kit for DirectX 11

Write-Host "Downloading DirectX Tool Kit for DirectX 11 (contains MakeSpriteFont tool)..." -ForegroundColor Green

$directxtkUrl = "https://github.com/microsoft/DirectXTK/archive/refs/heads/main.zip"
$zipFile = "DirectXTK-main.zip"
$extractDir = "DirectXTK-main"

try {
    # Download DirectXTK
    Write-Host "Downloading..." -ForegroundColor Yellow
    Invoke-WebRequest -Uri $directxtkUrl -OutFile $zipFile -UseBasicParsing
    
    # Extract
    Write-Host "Extracting..." -ForegroundColor Yellow
    Expand-Archive -Path $zipFile -DestinationPath . -Force
    
    # Find MakeSpriteFont project
    $makeSpriteFontPath = Join-Path $extractDir "MakeSpriteFont"
    if (Test-Path $makeSpriteFontPath) {
        Write-Host "`nFound MakeSpriteFont project at: $makeSpriteFontPath" -ForegroundColor Green
        Write-Host "`nTo compile MakeSpriteFont:" -ForegroundColor Cyan
        Write-Host "1. Open Visual Studio" -ForegroundColor White
        Write-Host "2. Open: $makeSpriteFontPath\MakeSpriteFont_Win10_Desktop_2019.sln" -ForegroundColor White
        Write-Host "3. Build Release configuration" -ForegroundColor White
        Write-Host "4. Compiled exe will be at: $makeSpriteFontPath\bin\Desktop_2019\Release\MakeSpriteFont.exe" -ForegroundColor White
        Write-Host "`nOr use command line:" -ForegroundColor Cyan
        Write-Host "  cd $makeSpriteFontPath" -ForegroundColor White
        Write-Host "  msbuild MakeSpriteFont_Win10_Desktop_2019.sln /p:Configuration=Release" -ForegroundColor White
    } else {
        Write-Host "`nWarning: MakeSpriteFont directory not found" -ForegroundColor Yellow
    }
    
    Write-Host "`nDownload complete!" -ForegroundColor Green
} catch {
    Write-Host "`nError: $_" -ForegroundColor Red
    Write-Host "`nManual download method:" -ForegroundColor Yellow
    Write-Host "1. Visit: https://github.com/microsoft/DirectXTK" -ForegroundColor White
    Write-Host "2. Click 'Code' -> 'Download ZIP'" -ForegroundColor White
    Write-Host "3. Extract and find MakeSpriteFont directory" -ForegroundColor White
    Write-Host "4. Compile with Visual Studio" -ForegroundColor White
}
