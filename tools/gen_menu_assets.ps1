Add-Type -AssemblyName System.Drawing

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
    param(
        [string]$SymbolName,
        [byte[]]$Bytes,
        [int]$W,
        [int]$H,
        [string]$OutPath
    )

    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine("// Hand-generated menu/UI art (procedurally drawn), see tools/gen_menu_assets.ps1.")
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

function New-TitleBackground {
    param([int]$Width = 240, [int]$Height = 320)

    $bmp = New-Object System.Drawing.Bitmap $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias

    $rect = New-Object System.Drawing.Rectangle 0, 0, $Width, $Height
    $bgBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush($rect, [System.Drawing.ColorTranslator]::FromHtml("#04061a"), [System.Drawing.ColorTranslator]::FromHtml("#0c1240"), 90.0)
    $g.FillRectangle($bgBrush, $rect)

    $rnd = New-Object System.Random(1337)
    for ($i = 0; $i -lt 110; $i++) {
        $x = $rnd.Next(0, $Width)
        $y = $rnd.Next(0, [int]($Height * 0.8))
        $size = $rnd.Next(1, 3)
        $alpha = $rnd.Next(110, 255)
        $starBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb($alpha, 255, 255, 255))
        $g.FillEllipse($starBrush, $x, $y, $size, $size)
        $starBrush.Dispose()
    }

    $farPts = [System.Drawing.Point[]]@(
        (New-Object System.Drawing.Point(0, 300)),
        (New-Object System.Drawing.Point(28, 268)),
        (New-Object System.Drawing.Point(60, 288)),
        (New-Object System.Drawing.Point(96, 254)),
        (New-Object System.Drawing.Point(132, 282)),
        (New-Object System.Drawing.Point(168, 260)),
        (New-Object System.Drawing.Point(204, 284)),
        (New-Object System.Drawing.Point(240, 262)),
        (New-Object System.Drawing.Point(240, 320)),
        (New-Object System.Drawing.Point(0, 320))
    )
    $farBrush = New-Object System.Drawing.SolidBrush([System.Drawing.ColorTranslator]::FromHtml("#241a3d"))
    $g.FillPolygon($farBrush, $farPts)

    $nearPts = [System.Drawing.Point[]]@(
        (New-Object System.Drawing.Point(0, 312)),
        (New-Object System.Drawing.Point(40, 296)),
        (New-Object System.Drawing.Point(80, 310)),
        (New-Object System.Drawing.Point(120, 292)),
        (New-Object System.Drawing.Point(160, 308)),
        (New-Object System.Drawing.Point(200, 294)),
        (New-Object System.Drawing.Point(240, 306)),
        (New-Object System.Drawing.Point(240, 320)),
        (New-Object System.Drawing.Point(0, 320))
    )
    $nearBrush = New-Object System.Drawing.SolidBrush([System.Drawing.ColorTranslator]::FromHtml("#150f26"))
    $g.FillPolygon($nearBrush, $nearPts)

    $g.Dispose()
    return $bmp
}

function New-OutlinedLogo {
    param(
        [string]$Text,
        [int]$Width,
        [int]$Height,
        [string]$FillHex,
        [string]$FillHex2,
        [string]$OutlineHex,
        [int]$FontSize = 30,
        [string]$FontFamily = "Arial Black"
    )

    $bmp = New-Object System.Drawing.Bitmap $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.Clear([System.Drawing.Color]::Transparent)

    $family = New-Object System.Drawing.FontFamily($FontFamily)
    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $sf = New-Object System.Drawing.StringFormat
    $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
    $rectF = New-Object System.Drawing.RectangleF(0, 0, $Width, $Height)
    $path.AddString($Text, $family, [int][System.Drawing.FontStyle]::Regular, $FontSize, $rectF, $sf)

    $outlinePen = New-Object System.Drawing.Pen([System.Drawing.ColorTranslator]::FromHtml($OutlineHex), 4)
    $outlinePen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round
    $g.DrawPath($outlinePen, $path)

    $bounds = $path.GetBounds()
    if ($bounds.Height -le 0) { $bounds = New-Object System.Drawing.RectangleF(0, 0, $Width, $Height) }
    $fillBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush($bounds, [System.Drawing.ColorTranslator]::FromHtml($FillHex), [System.Drawing.ColorTranslator]::FromHtml($FillHex2), 90.0)
    $g.FillPath($fillBrush, $path)

    $g.Dispose()
    return $bmp
}

function New-MenuButton {
    param(
        [string]$Text,
        [int]$Width,
        [int]$Height,
        [string]$FillHex,
        [string]$FillHex2,
        [string]$BorderHex,
        [int]$FontSize = 16
    )

    $bmp = New-Object System.Drawing.Bitmap $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.Clear([System.Drawing.Color]::Transparent)

    $outerRect = New-Object System.Drawing.Rectangle 0, 0, $Width, $Height
    $innerRect = New-Object System.Drawing.Rectangle 2, 2, ($Width - 4), ($Height - 4)

    $fillBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush($innerRect, [System.Drawing.ColorTranslator]::FromHtml($FillHex), [System.Drawing.ColorTranslator]::FromHtml($FillHex2), 90.0)
    $g.FillRectangle($fillBrush, $innerRect)

    $borderPen = New-Object System.Drawing.Pen([System.Drawing.ColorTranslator]::FromHtml($BorderHex), 2)
    $g.DrawRectangle($borderPen, 1, 1, ($Width - 3), ($Height - 3))

    $font = New-Object System.Drawing.Font("Consolas", $FontSize, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $sf = New-Object System.Drawing.StringFormat
    $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
    $textBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
    $g.DrawString($Text, $font, $textBrush, (New-Object System.Drawing.RectangleF(0, 0, $Width, $Height)), $sf)

    $g.Dispose()
    return $bmp
}

# 1. Title screen starfield + mountain background
$bg = New-TitleBackground -Width 240 -Height 320
Save-Asset -Bmp $bg -SymbolName 'titlebackground' -PngName 'TitleBackground.png'
$bg.Dispose()

# 2. "SPACE INVADERS" hero logo (green, bold, outlined)
$logo = New-OutlinedLogo -Text "SPACE`nINVADERS" -Width 208 -Height 76 -FillHex "#7CFF6B" -FillHex2 "#20A83A" -OutlineHex "#063d10" -FontSize 26
Save-Asset -Bmp $logo -SymbolName 'logospaceinvaders' -PngName 'LogoSpaceInvaders.png'
$logo.Dispose()

# 3. "GAME OVER" logo (red, bold, outlined)
$gameOver = New-OutlinedLogo -Text "GAME OVER" -Width 196 -Height 34 -FillHex "#ff8a6b" -FillHex2 "#d21f1f" -OutlineHex "#3d0606" -FontSize 22
Save-Asset -Bmp $gameOver -SymbolName 'logogameover' -PngName 'LogoGameOver.png'
$gameOver.Dispose()

# 4. NEW GAME button (green)
$newGameBtn = New-MenuButton -Text "NEW GAME" -Width 180 -Height 36 -FillHex "#3ecf5f" -FillHex2 "#1f8a3a" -BorderHex "#0f5c22" -FontSize 14
Save-Asset -Bmp $newGameBtn -SymbolName 'buttonnewgame' -PngName 'ButtonNewGame.png'
$newGameBtn.Dispose()

# 5. CONTINUE button (cyan)
$continueBtn = New-MenuButton -Text "CONTINUE" -Width 180 -Height 36 -FillHex "#4fc3f7" -FillHex2 "#1478b0" -BorderHex "#0a4a6e" -FontSize 14
Save-Asset -Bmp $continueBtn -SymbolName 'buttoncontinue' -PngName 'ButtonContinue.png'
$continueBtn.Dispose()

Write-Output "DONE"
