#PowerShell Script zum Kopieren von Gamefiles in richtiges Verzeichnis 
#MS Launcher Version
#Autor: Lukas Riedel


$destpath = Join-Path $env:APPDATA ".minecraft"
$object = ".\Craft to Exile 2 (VR Support)" #args[1]
$url = "https://www.youtube.com/watch?v=xvFZjo5PgG0"

if (Test-Path $object) {
    Write-Output "Starte Kopiervorgang ..."
    Copy-Item -Path $object -Destination $destpath -Recurse -Force # ja -rf geht nicht, Bash auf die 1
    Write-Output "Kopiervorgang erfolgreich!"
} else {
    Write-Error "Quelle nicht gefunden: $object"
}
Write-Output "never ever fuehre unbekannte Skripte aus, die du nicht gelesen hast bzw. nicht verstehst"
Write-Output "das Fenster schliesst sich in 10 Sekunden selber"
Start-Process $url
Start-Sleep -Seconds 10