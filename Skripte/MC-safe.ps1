
$source = "C:\Users\lukas.LUKAS_DESKTOP.000\AppData\Roaming\.minecraft\Woods Of The Past-5.3"
$user = "mast_assassin"
$server = "shadowassassin.de"
$alt = "192.168.178.37"
$destination = "/home/mast_assassin/mc_safe"

if (!(Test-Path $source)) {
    Write-Error "Der Quellordner wurde nicht gefunden: $source"
    exit 1
}
Write-Output "Kopieren nach $user@$server`:$destination"
scp -r "$source" "$user@${server}:${destination}"

if ($LASTEXITCODE -eq 0) {
    Write-Output "Kopieren erfolgreich abgeschlossen."
    exit 0
} else {
    Write-Error "Fehler beim Kopieren. Versuche lokale IPv4-Adresse."
    scp -r "$source" "$user@${alt}:`"${destination}`""
}

if ($LASTEXITCODE -eq 0) {
    Write-Output "Kopieren erfolgreich abgeschlossen."
    exit 0
} else {
    Write-Error "Fehler beim Kopieren. Bitte überprüfe die Verbindung und die Pfade."
    exit 1
}
