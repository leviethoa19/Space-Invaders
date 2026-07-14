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

function New-BigExplosion {
    param([int]$CanvasW = 34, [int]$CanvasH = 20)

    $occupied = New-Object 'bool[,]' $CanvasW, $CanvasH
    $cx = $CanvasW / 2.0
    $cy = $CanvasH / 2.0

    $sources = @(
        @{File = 'playerExplosionA.png'; Scale = 2.0; OffX = 3; OffY = 2 },
        @{File = 'playerExplosionB.png'; Scale = 2.0; OffX = -3; OffY = -1 },
        @{File = 'ufoExplosion.png'; Scale = 1.5; OffX = 0; OffY = 0 }
    )

    foreach ($src in $sources) {
        $img = [System.Drawing.Bitmap]::FromFile((Join-Path $SrcBase $src.File))
        $sw = $img.Width
        $sh = $img.Height
        $scale = $src.Scale
        $dw = [Math]::Round($sw * $scale)
        $dh = [Math]::Round($sh * $scale)
        $baseX = [Math]::Round($cx - $dw / 2.0 + $src.OffX)
        $baseY = [Math]::Round($cy - $dh / 2.0 + $src.OffY)
        $block = [Math]::Ceiling($scale)

        for ($sy = 0; $sy -lt $sh; $sy++) {
            for ($sx = 0; $sx -lt $sw; $sx++) {
                $p = $img.GetPixel($sx, $sy)
                if ($p.A -gt 40) {
                    for ($by = 0; $by -lt $block; $by++) {
                        for ($bx = 0; $bx -lt $block; $bx++) {
                            $tx = [int]($baseX + $sx * $scale + $bx)
                            $ty = [int]($baseY + $sy * $scale + $by)
                            if ($tx -ge 0 -and $tx -lt $CanvasW -and $ty -ge 0 -and $ty -lt $CanvasH) {
                                $occupied[$tx, $ty] = $true
                            }
                        }
                    }
                }
            }
        }
        $img.Dispose()
    }

    $canvas = New-Object System.Drawing.Bitmap $CanvasW, $CanvasH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $maxDist = [Math]::Sqrt($cx * $cx + $cy * $cy)

    for ($y = 0; $y -lt $CanvasH; $y++) {
        for ($x = 0; $x -lt $CanvasW; $x++) {
            if ($occupied[$x, $y]) {
                $dx = $x - $cx
                $dy = $y - $cy
                $dist = [Math]::Sqrt($dx * $dx + $dy * $dy) / $maxDist

                if ($dist -lt 0.35) {
                    $r = 255; $g = 250; $b = 200
                }
                elseif ($dist -lt 0.65) {
                    $t = ($dist - 0.35) / 0.3
                    $r = 255; $g = [int](250 - $t * 90); $b = [int](200 - $t * 170)
                }
                else {
                    $t = [Math]::Min(1.0, ($dist - 0.65) / 0.35)
                    $r = [int](255 - $t * 55); $g = [int](160 - $t * 110); $b = [int](30 - $t * 30)
                }
                $canvas.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $r, $g, [Math]::Max(0, $b)))
            }
        }
    }

    return $canvas
}

$big = New-BigExplosion -CanvasW 34 -CanvasH 20
Save-Asset -Bmp $big -SymbolName 'explosionplayerbig' -PngName 'ExplosionPlayerBig.png'
$big.Dispose()

Write-Output "DONE"
