#PowerShell Skript zum richtigen Verschieben von MC-Verzeichnissen
#CurseForge Version
#Autor: Lukas Riedel

$destpath = "$env:USERPROFILE\\curseforge\\minecraft\\Instances"
$source = "Craft to Exile 2 (VR Support)"
$url = "https://www.youtube.com/watch?v=xvFZjo5PgG0"

    if (!(Test-Path $destpath)) {

        Write-Host "Zielpfad nicht vorhanden. Abbruch..."
        Write-Output "Dieses Fenster schliesst sich selber in 10 Sekunden."
        Start-Sleep -Seconds 10
        return 1
    }
if (Test-Path $source) {

    Write-Host "Starte Kopiervorgang..."
    Copy-Item $source $destpath -Recurse -Force
    Write-Host "Vorgang abgeschlossen!"
}
else {
    Write-Host "Quellverzeichnis nicht gefunden! Abbruch..."
    Write-Host "Dieses Fenster schliesst sich selber in 10 Sekunden."
    Start-Sleep -Seconds 10
    return 1
}

Write-Host "never ever fuehre fremde Skripte aus, die du nicht voher selber gelesen hast bzw. verstaden hast."
Write-Host "Dieses Fenster schliesst sich selber in 15 Sekunden."
Start-Process $url
Start-Sleep -Seconds 15