Add-Type -AssemblyName System.Drawing

$Img = 'C:\TouchGFXProjects\SpaceInvaders\TouchGFX\assets\images'
$Out = 'C:\TouchGFXProjects\SpaceInvaders\tools\preview'
New-Item -ItemType Directory -Force -Path $Out | Out-Null

function Load($name) {
    return [System.Drawing.Image]::FromFile((Join-Path $Img $name))
}

function DrawCenteredText($g, $text, $x, $y, $w, $h, $colorHex, $size) {
    $font = New-Object System.Drawing.Font("Verdana", $size, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $brush = New-Object System.Drawing.SolidBrush([System.Drawing.ColorTranslator]::FromHtml($colorHex))
    $sf = New-Object System.Drawing.StringFormat
    $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
    $g.DrawString($text, $font, $brush, (New-Object System.Drawing.RectangleF($x, $y, $w, $h)), $sf)
}

function DrawLeftText($g, $text, $x, $y, $w, $h, $colorHex, $size) {
    $font = New-Object System.Drawing.Font("Verdana", $size, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $brush = New-Object System.Drawing.SolidBrush([System.Drawing.ColorTranslator]::FromHtml($colorHex))
    $sf = New-Object System.Drawing.StringFormat
    $sf.Alignment = [System.Drawing.StringAlignment]::Near
    $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
    $g.DrawString($text, $font, $brush, (New-Object System.Drawing.RectangleF($x, $y, $w, $h)), $sf)
}

function DrawGameBg($g) {
    $rect = New-Object System.Drawing.Rectangle 0, 0, 240, 320
    $b = New-Object System.Drawing.SolidBrush([System.Drawing.ColorTranslator]::FromHtml("#05070f"))
    $g.FillRectangle($b, $rect)
    $rnd = New-Object System.Random(7)
    for ($i = 0; $i -lt 40; $i++) {
        $x = $rnd.Next(0, 10); $y = $rnd.Next(0, 4)
        $sb = New-Object System.Drawing.SolidBrush([System.Drawing.ColorTranslator]::FromHtml("#3fae4a"))
        $g.FillRectangle($sb, 14 + $x * 21, 40 + $y * 24, 15, 15)
    }
}

# ---------- Screen1 (Title) ----------
$canvas = New-Object System.Drawing.Bitmap 240, 320
$g = [System.Drawing.Graphics]::FromImage($canvas)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.DrawImage((Load 'TitleBackground.png'), 0, 0, 240, 320)
$g.DrawImage((Load 'LogoSpaceInvaders.png'), 16, 104, 208, 76)
DrawLeftText $g "HI 01000" 0 194 240 20 "#ffcd3c" 13
$g.DrawImage((Load 'ButtonNewGame.png'), 30, 236, 180, 36)
$g.Dispose()
$canvas.Save((Join-Path $Out 'Screen1_Title.png'))
$canvas.Dispose()

# ---------- Screen2 Pause overlay ----------
$panelBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(235, 8, 10, 30))
$canvas2 = New-Object System.Drawing.Bitmap 240, 320
$g2 = [System.Drawing.Graphics]::FromImage($canvas2)
$g2.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
DrawGameBg $g2
$g2.FillRectangle($panelBrush, 20, 78, 200, 176)
DrawCenteredText $g2 "PAUSED" 20 86 200 26 "#ffdc28" 20
DrawLeftText $g2 "SCORE 00450" 20 118 200 16 "#ffffff" 12
DrawLeftText $g2 "HI 01000" 20 136 200 16 "#ffcd3c" 12
$g2.DrawImage((Load 'ButtonContinue.png'), 30, 158, 180, 36)
$g2.DrawImage((Load 'ButtonNewGame.png'), 30, 198, 180, 36)
$g2.Dispose()
$canvas2.Save((Join-Path $Out 'Screen2_Pause.png'))
$canvas2.Dispose()

# ---------- Screen2 Game Over overlay ----------
$canvas3 = New-Object System.Drawing.Bitmap 240, 320
$g3 = [System.Drawing.Graphics]::FromImage($canvas3)
$g3.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
DrawGameBg $g3
$g3.FillRectangle($panelBrush, 20, 78, 200, 176)
$g3.DrawImage((Load 'LogoGameOver.png'), 22, 86, 196, 34)
DrawLeftText $g3 "SCORE 01000" 20 118 200 16 "#ffffff" 12
DrawLeftText $g3 "HI 01000" 20 136 200 16 "#ffcd3c" 12
$g3.DrawImage((Load 'ButtonNewGame.png'), 30, 158, 180, 36)
$g3.Dispose()
$canvas3.Save((Join-Path $Out 'Screen2_GameOver.png'))
$canvas3.Dispose()

Write-Output "DONE"
