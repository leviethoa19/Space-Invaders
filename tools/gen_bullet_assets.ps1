Add-Type -AssemblyName System.Drawing

$SrcBase = 'C:\Users\Nguyen\Downloads\space-invaders-master\space-invaders-master\SpaceInvadersProject\Assets\Sprites\Projectiles'
$AssetsOut = 'C:\TouchGFXProjects\SpaceInvaders\TouchGFX\assets\images'
$CppOut = 'C:\TouchGFXProjects\SpaceInvaders\TouchGFX\generated\images\src'

function Get-BitmapBytes {
    param([System.Drawing.Bitmap]$Bmp)
    $w = $Bmp.Width
    $h = $Bmp.Height
    $rect = New-Object System.Drawing.Rectangle 0, 0, $w, $h
    $bmpData = $Bmp.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $byteCount = [Math]::Abs($bmpData.Stride) * $h
    $bytes = New-Object byte[] $byteCount
    [System.Runtime.InteropServices.Marshal]::Copy($bmpData.Scan0, $bytes, 0, $byteCount)
    $Bmp.UnlockBits($bmpData)
    $stride = $bmpData.Stride
    $rowBytes = $w * 4
    $out = New-Object byte[] ($w * $h * 4)
    for ($y = 0; $y -lt $h; $y++) {
        [System.Array]::Copy($bytes, $y * $stride, $out, $y * $rowBytes, $rowBytes)
    }
    return $out
}

function Write-ImageCpp {
    param([string]$SymbolName, [byte[]]$Bytes, [int]$W, [int]$H, [string]$OutPath)
    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine("// Hand-generated: recolored/scaled from the user-provided sprite pack projectile frames.")
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

function Save-Asset {
    param([System.Drawing.Bitmap]$Bmp, [string]$SymbolName, [string]$PngName)
    $bytes = Get-BitmapBytes -Bmp $Bmp
    Write-ImageCpp -SymbolName $SymbolName -Bytes $bytes -W $Bmp.Width -H $Bmp.Height -OutPath (Join-Path $CppOut "image_$SymbolName.cpp")
    $Bmp.Save((Join-Path $AssetsOut $PngName), [System.Drawing.Imaging.ImageFormat]::Png)
    Write-Output "OK  $PngName  ($($Bmp.Width) x $($Bmp.Height))"
}

function New-RecoloredBolt {
    param(
        [string]$SourceFile,
        [int]$Scale,
        [int]$CanvasW,
        [int]$CanvasH,
        [int[]]$CoreColor,
        [int[]]$EdgeColor
    )

    $src = [System.Drawing.Bitmap]::FromFile((Join-Path $SrcBase $SourceFile))
    $sw = $src.Width
    $sh = $src.Height

    $dw = $sw * $Scale
    $dh = $sh * $Scale
    $offX = [int](($CanvasW - $dw) / 2)
    $offY = [int](($CanvasH - $dh) / 2)

    $canvas = New-Object System.Drawing.Bitmap $CanvasW, $CanvasH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $cyMid = $sh / 2.0

    for ($sy = 0; $sy -lt $sh; $sy++) {
        $t = [Math]::Abs($sy - $cyMid) / $cyMid
        $r = [int]($CoreColor[0] + ($EdgeColor[0] - $CoreColor[0]) * $t)
        $g = [int]($CoreColor[1] + ($EdgeColor[1] - $CoreColor[1]) * $t)
        $b = [int]($CoreColor[2] + ($EdgeColor[2] - $CoreColor[2]) * $t)
        $color = [System.Drawing.Color]::FromArgb(255, [Math]::Max(0, [Math]::Min(255, $r)), [Math]::Max(0, [Math]::Min(255, $g)), [Math]::Max(0, [Math]::Min(255, $b)))

        for ($sx = 0; $sx -lt $sw; $sx++) {
            $p = $src.GetPixel($sx, $sy)
            if ($p.A -gt 40) {
                for ($by = 0; $by -lt $Scale; $by++) {
                    for ($bx = 0; $bx -lt $Scale; $bx++) {
                        $tx = $offX + $sx * $Scale + $bx
                        $ty = $offY + $sy * $Scale + $by
                        if ($tx -ge 0 -and $tx -lt $CanvasW -and $ty -ge 0 -and $ty -lt $CanvasH) {
                            $canvas.SetPixel($tx, $ty, $color)
                        }
                    }
                }
            }
        }
    }
    $src.Dispose()
    return $canvas
}

# Rapid Fire tier: warm yellow streak (matches POWERUP_RAPID_FIRE color 255,220,40), same slim size as the base bullet.
$rapid = New-RecoloredBolt -SourceFile 'ProjectileA_1.png' -Scale 1 -CanvasW 4 -CanvasH 9 -CoreColor @(255, 250, 210) -EdgeColor @(255, 205, 30)
Save-Asset -Bmp $rapid -SymbolName 'bulletplayerrapid' -PngName 'BulletPlayerRapid.png'
$rapid.Dispose()

# Double-Shot / power tier: bright cyan, visibly chunkier (matches POWERUP_DOUBLE_SHOT color 60,220,255).
$power = New-RecoloredBolt -SourceFile 'missile_1.png' -Scale 2 -CanvasW 6 -CanvasH 14 -CoreColor @(230, 255, 255) -EdgeColor @(20, 160, 255)
Save-Asset -Bmp $power -SymbolName 'bulletplayerpower' -PngName 'BulletPlayerPower.png'
$power.Dispose()

# Boss ordinance: bigger, menacing crimson beam so it reads as heavier than a regular alien pellet.
$boss = New-RecoloredBolt -SourceFile 'OmegaRay_1.png' -Scale 1 -CanvasW 9 -CanvasH 16 -CoreColor @(255, 235, 190) -EdgeColor @(215, 25, 35)
Save-Asset -Bmp $boss -SymbolName 'bulletboss' -PngName 'BulletBoss.png'
$boss.Dispose()

Write-Output "DONE"
