Add-Type -AssemblyName System.Drawing

$SrcBase = 'C:\Users\Nguyen\Downloads\space-invaders-master\space-invaders-master\SpaceInvadersProject\Assets\Sprites\Explosions'
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
    [void]$sb.AppendLine("// Hand-generated: composited/recolored from the user-provided sprite pack explosion frames.")
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

# Recolors a source silhouette (any of the pack's explosion/spark frames) onto a
# transparent canvas using a vertical core->edge gradient. Shared by both the
# rapid-tier kill burst and the three boss "hit spark" variants below, so a
# shot's tier reads consistently everywhere it can visibly land a hit.
function New-TintedBurst {
    param(
        [string]$SourceFile,
        [double]$Scale,
        [int]$CanvasW,
        [int]$CanvasH,
        [int[]]$CoreColor,
        [int[]]$MidColor,
        [int[]]$EdgeColor
    )

    $src = [System.Drawing.Bitmap]::FromFile((Join-Path $SrcBase $SourceFile))
    $sw = $src.Width
    $sh = $src.Height
    $dw = $sw * $Scale
    $dh = $sh * $Scale
    $offX = ($CanvasW - $dw) / 2.0
    $offY = ($CanvasH - $dh) / 2.0
    $block = [Math]::Ceiling($Scale)

    $occupied = New-Object 'bool[,]' $CanvasW, $CanvasH
    for ($sy = 0; $sy -lt $sh; $sy++) {
        for ($sx = 0; $sx -lt $sw; $sx++) {
            $p = $src.GetPixel($sx, $sy)
            if ($p.A -gt 40) {
                for ($by = 0; $by -lt $block; $by++) {
                    for ($bx = 0; $bx -lt $block; $bx++) {
                        $tx = [int]($offX + $sx * $Scale + $bx)
                        $ty = [int]($offY + $sy * $Scale + $by)
                        if ($tx -ge 0 -and $tx -lt $CanvasW -and $ty -ge 0 -and $ty -lt $CanvasH) {
                            $occupied[$tx, $ty] = $true
                        }
                    }
                }
            }
        }
    }
    $src.Dispose()

    $canvas = New-Object System.Drawing.Bitmap $CanvasW, $CanvasH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $cx = $CanvasW / 2.0
    $cy = $CanvasH / 2.0
    $maxDist = [Math]::Sqrt($cx * $cx + $cy * $cy)

    for ($y = 0; $y -lt $CanvasH; $y++) {
        for ($x = 0; $x -lt $CanvasW; $x++) {
            if ($occupied[$x, $y]) {
                $dx = $x - $cx
                $dy = $y - $cy
                $dist = [Math]::Sqrt($dx * $dx + $dy * $dy) / $maxDist

                if ($dist -lt 0.35) {
                    $r = $CoreColor[0]; $g = $CoreColor[1]; $b = $CoreColor[2]
                }
                elseif ($dist -lt 0.65) {
                    $t = ($dist - 0.35) / 0.3
                    $r = [int]($CoreColor[0] + ($MidColor[0] - $CoreColor[0]) * $t)
                    $g = [int]($CoreColor[1] + ($MidColor[1] - $CoreColor[1]) * $t)
                    $b = [int]($CoreColor[2] + ($MidColor[2] - $CoreColor[2]) * $t)
                }
                else {
                    $t = [Math]::Min(1.0, ($dist - 0.65) / 0.35)
                    $r = [int]($MidColor[0] + ($EdgeColor[0] - $MidColor[0]) * $t)
                    $g = [int]($MidColor[1] + ($EdgeColor[1] - $MidColor[1]) * $t)
                    $b = [int]($MidColor[2] + ($EdgeColor[2] - $MidColor[2]) * $t)
                }
                $r = [Math]::Max(0, [Math]::Min(255, $r))
                $g = [Math]::Max(0, [Math]::Min(255, $g))
                $b = [Math]::Max(0, [Math]::Min(255, $b))
                $canvas.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $r, $g, $b))
            }
        }
    }

    return $canvas
}

# 1) Rapid-tier alien kill burst: same invader-explosion silhouette as a normal
#    kill (same size, not bigger - rapid fire isn't "harder", just faster), but
#    a warm yellow/amber palette matching the BULLET_TIER_RAPID bolt color.
$rapidKill = New-TintedBurst -SourceFile 'invaderExplosion.png' -Scale 0.9 -CanvasW 24 -CanvasH 15 `
    -CoreColor @(255, 255, 235) -MidColor @(255, 225, 90) -EdgeColor @(230, 130, 15)
Save-Asset -Bmp $rapidKill -SymbolName 'explosionalienrapid' -PngName 'ExplosionAlienRapid.png'
$rapidKill.Dispose()

# 2) Boss "hit spark" - a tiny, fast burst shown on every non-lethal hit
#    (distinct from the big one-time boss death explosion), one palette per
#    bullet tier so sustained fire visibly communicates shot strength.
$sparkNormal = New-TintedBurst -SourceFile 'playerProjectileExplosion.png' -Scale 1.0 -CanvasW 8 -CanvasH 8 `
    -CoreColor @(255, 250, 235) -MidColor @(255, 200, 120) -EdgeColor @(220, 90, 40)
Save-Asset -Bmp $sparkNormal -SymbolName 'hitsparknormal' -PngName 'HitSparkNormal.png'
$sparkNormal.Dispose()

$sparkRapid = New-TintedBurst -SourceFile 'playerProjectileExplosion.png' -Scale 1.0 -CanvasW 8 -CanvasH 8 `
    -CoreColor @(255, 255, 230) -MidColor @(255, 220, 80) -EdgeColor @(225, 140, 10)
Save-Asset -Bmp $sparkRapid -SymbolName 'hitsparkrapid' -PngName 'HitSparkRapid.png'
$sparkRapid.Dispose()

$sparkPower = New-TintedBurst -SourceFile 'enemyProjectileExplosion.png' -Scale 1.8 -CanvasW 12 -CanvasH 10 `
    -CoreColor @(235, 255, 255) -MidColor @(90, 210, 255) -EdgeColor @(20, 90, 200)
Save-Asset -Bmp $sparkPower -SymbolName 'hitsparkpower' -PngName 'HitSparkPower.png'
$sparkPower.Dispose()

Write-Output "DONE"
