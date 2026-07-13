Add-Type -AssemblyName System.Drawing

$SrcBase = 'C:\Users\Nguyen\Downloads\space-invaders-master\space-invaders-master\SpaceInvadersProject\Assets\Sprites'
$AssetsOut = 'C:\TouchGFXProjects\SpaceInvaders\TouchGFX\assets\images'
$CppOut = 'C:\TouchGFXProjects\SpaceInvaders\TouchGFX\generated\images\src'

function Get-LetterboxedBytes {
    param(
        [string]$SrcPath,
        [int]$TargetW,
        [int]$TargetH
    )

    $srcBytes = [System.IO.File]::ReadAllBytes($SrcPath)
    $ms = New-Object System.IO.MemoryStream(, $srcBytes)
    $src = [System.Drawing.Bitmap]::FromStream($ms)

    [double]$scale = [Math]::Min([double]$TargetW / $src.Width, [double]$TargetH / $src.Height)
    [int]$drawW = [Math]::Max(1, [Math]::Round($src.Width * $scale))
    [int]$drawH = [Math]::Max(1, [Math]::Round($src.Height * $scale))
    [int]$drawX = [Math]::Floor(($TargetW - $drawW) / 2)
    [int]$drawY = [Math]::Floor(($TargetH - $drawH) / 2)

    $canvas = New-Object System.Drawing.Bitmap $TargetW, $TargetH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($canvas)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
    $g.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceOver
    $g.DrawImage($src, $drawX, $drawY, $drawW, $drawH)
    $g.Dispose()
    $src.Dispose()
    $ms.Dispose()

    $rect = New-Object System.Drawing.Rectangle 0, 0, $TargetW, $TargetH
    $bmpData = $canvas.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $byteCount = [Math]::Abs($bmpData.Stride) * $TargetH
    $bytes = New-Object byte[] $byteCount
    [System.Runtime.InteropServices.Marshal]::Copy($bmpData.Scan0, $bytes, 0, $byteCount)
    $canvas.UnlockBits($bmpData)

    $stride = $bmpData.Stride
    $rowBytes = $TargetW * 4
    $out = New-Object byte[] ($TargetW * $TargetH * 4)
    for ($y = 0; $y -lt $TargetH; $y++) {
        [System.Array]::Copy($bytes, $y * $stride, $out, $y * $rowBytes, $rowBytes)
    }

    return @{ Bytes = $out; Canvas = $canvas }
}

function Write-PreviewPng {
    param($Canvas, [string]$OutPath)
    $Canvas.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
}

function Write-ImageCpp {
    param(
        [string]$SymbolName,
        [byte[]]$Bytes,
        [int]$W,
        [int]$H,
        [string]$OutPath
    )

    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine("// Hand-generated from user-provided sprite pack (space-invaders-master).")
    [void]$sb.AppendLine("// Do not edit by TouchGFX Designer regeneration; keep in sync with BitmapDatabase.")
    [void]$sb.AppendLine("")
    [void]$sb.AppendLine("#include <touchgfx/hal/Config.hpp>")
    [void]$sb.AppendLine("")
    [void]$sb.AppendLine("LOCATION_PRAGMA(`"ExtFlashSection`")")
    [void]$sb.AppendLine("KEEP extern const unsigned char image_$SymbolName[] LOCATION_ATTRIBUTE(`"ExtFlashSection`") = { // ${W}x${H} ARGB8888 pixels.")

    $lineVals = New-Object System.Collections.Generic.List[string]
    for ($i = 0; $i -lt $Bytes.Length; $i++) {
        $lineVals.Add("0x{0:x2}" -f $Bytes[$i])
        if ($lineVals.Count -eq 12) {
            [void]$sb.AppendLine("    " + ($lineVals -join ", ") + ",")
            $lineVals.Clear()
        }
    }
    if ($lineVals.Count -gt 0) {
        [void]$sb.AppendLine("    " + ($lineVals -join ", "))
    }
    [void]$sb.AppendLine("};")

    [System.IO.File]::WriteAllText($OutPath, $sb.ToString())
}

function Process-Sprite {
    param(
        [string]$RelSrc,
        [string]$SymbolName,
        [string]$PngName,
        [int]$W,
        [int]$H
    )

    $srcPath = Join-Path $SrcBase $RelSrc
    $result = Get-LetterboxedBytes -SrcPath $srcPath -TargetW $W -TargetH $H
    Write-ImageCpp -SymbolName $SymbolName -Bytes $result.Bytes -W $W -H $H -OutPath (Join-Path $CppOut "image_$SymbolName.cpp")
    Write-PreviewPng -Canvas $result.Canvas -OutPath (Join-Path $AssetsOut $PngName)
    $result.Canvas.Dispose()
    Write-Output "OK  $PngName  ($W x $H)  <-  $RelSrc"
}

# Aliens: 3 tiers x 2 frames, 15x15 canvas (shrunk to fit 10-wide formation)
Process-Sprite 'Invaders\invaderS1.png' 'aliensmall1'  'AlienSmall1.png'  15 15
Process-Sprite 'Invaders\invaderS2.png' 'aliensmall2'  'AlienSmall2.png'  15 15
Process-Sprite 'Invaders\invaderM1.png' 'alienmedium1' 'AlienMedium1.png' 15 15
Process-Sprite 'Invaders\invaderM2.png' 'alienmedium2' 'AlienMedium2.png' 15 15
Process-Sprite 'Invaders\invaderL1.png' 'alienlarge1'  'AlienLarge1.png'  15 15
Process-Sprite 'Invaders\invaderL2.png' 'alienlarge2'  'AlienLarge2.png'  15 15

# Player ship, native aspect
Process-Sprite 'player.png' 'playership' 'PlayerShip.png' 26 16

# Boss / mystery UFO
Process-Sprite 'Invaders\ufo.png' 'bossship' 'BossShip.png' 70 31

# Shelter: full + 9 damage stages, 28x20 canvas
Process-Sprite 'Shelter\shelter_full.png'      'shelterfull'      'ShelterFull.png'      28 20
Process-Sprite 'Shelter\shelterDamaged_1.png'  'shelterdamaged1'  'ShelterDamaged1.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_2.png'  'shelterdamaged2'  'ShelterDamaged2.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_3.png'  'shelterdamaged3'  'ShelterDamaged3.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_4.png'  'shelterdamaged4'  'ShelterDamaged4.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_5.png'  'shelterdamaged5'  'ShelterDamaged5.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_6.png'  'shelterdamaged6'  'ShelterDamaged6.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_7.png'  'shelterdamaged7'  'ShelterDamaged7.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_8.png'  'shelterdamaged8'  'ShelterDamaged8.png'  28 20
Process-Sprite 'Shelter\shelterDamaged_9.png'  'shelterdamaged9'  'ShelterDamaged9.png'  28 20

# Explosions
Process-Sprite 'Explosions\invaderExplosion.png' 'explosionalien'  'ExplosionAlien.png'  24 15
Process-Sprite 'Explosions\ufoExplosion.png'      'explosionboss'   'ExplosionBoss.png'   40 16
Process-Sprite 'Explosions\playerExplosionA.png'  'explosionplayer' 'ExplosionPlayer.png' 22 12

# Bullets (3px wide so the enemy diamond-chain sprite keeps its detail)
Process-Sprite 'Projectiles\Projectile_Player.png' 'bulletplayer' 'BulletPlayer.png' 3 8
Process-Sprite 'Projectiles\ProjectileA_1.png'      'bulletenemy'  'BulletEnemy.png'  3 8

# Shield power-up icon
Process-Sprite 'PulseShieldIcon.png' 'shieldicon' 'ShieldIcon.png' 16 10

Write-Output "DONE"
