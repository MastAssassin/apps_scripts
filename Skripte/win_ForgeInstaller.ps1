#Skript zur automatisierten Installation von Forge und Verkettung zu Skript fuer Installation von Modpack abhängig von verwendeten Launcher
#Autor: Lukas Riedel

do {
    Write-Host "Welche Launcher wird verwendet? MS Laucher(1) oder CurseForge(2)? `nwarte auf Eingabe...`n" 
    $eingabe = Read-Host
    $valid = $true
    switch ($eingabe) {
        "1" {
            Write-Host "Pruefe auf vorhandene Installationvon Forge 1.20.1 ..."
         
            $mcpath = Join-Path $env:APPDATA ".minecraft"
            $installer = "forge-1.20.1-47.4.16-installer.jar" 
            $installerpath = Join-Path $mcpath $installer
            $path = pwd

            if (Test-Path "$mcpath\\versions\\1.20.1-forge-47.4.16") {
                Write-Host "Forge ist bereits fuer diese Version installiert. Gehe ueber zu naechsten Skript..."
                & ".\win_mslauncher.ps1"
                Start-Sleep 5
            }
            else {
                Copy-Item $installer $mcpath
                cd $mcpath
                java -jar $installerpath --installClient
                Write-Host "Raeume auf..."
                cd $path
                Remove-Item $installerpath
                Remove-Item "$installerpath.log"
                Write-Host "Forge ist nun installiert. Gehe ueber zu naechsten Skript..."
                ".\win_mslauncher.ps1"
                Start-Sleep 5
            }
    
        }


        "2" {
            Write-Host "keine Handeln noetig, CurseForge macht den Rest. Gehe ueber zu naechsten Skript..."
            & ".\win_mslauncher.ps1"
            Start-Sleep 5
        }
        Default {
            Write-Host "Ungueltige Eingabe. Gueltig: 1 oder 2"
            $valid = $false

        }
    }
} until ($valid)



