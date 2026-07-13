# Regenerates Debug/Application/User/generated/subdir.mk to match the current
# set of generated source files (fonts/texts/gui_generated + all image_*.cpp).
$ErrorActionPreference = "Stop"

$proj = "C:/TouchGFXProjects/SpaceInvaders"
$subdirMk = "C:\TouchGFXProjects\SpaceInvaders\STM32CubeIDE\Debug\Application\User\generated\subdir.mk"

$flags = '-mcpu=cortex-m4 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DUSE_BPP=16 -DDEBUG -DSTM32F429xx -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../Drivers/BSP -I../../TouchGFX/target -I../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../TouchGFX/target/generated -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/generated/fonts/include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/generated/images/include -I../../TouchGFX/generated/texts/include -I../../TouchGFX/gui/include -I../../TouchGFX/generated/videos/include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity'

# (relative path under $proj, object base name)
$nonImage = @(
    @("TouchGFX/generated/fonts/src/ApplicationFontProvider.cpp", "ApplicationFontProvider"),
    @("TouchGFX/generated/images/src/BitmapDatabase.cpp", "BitmapDatabase"),
    @("TouchGFX/generated/fonts/src/CachedFont.cpp", "CachedFont"),
    @("TouchGFX/generated/fonts/src/CompressedFontCache.cpp", "CompressedFontCache"),
    @("TouchGFX/generated/fonts/src/CompressedUnmappedFontCache.cpp", "CompressedUnmappedFontCache"),
    @("TouchGFX/generated/fonts/src/FontCache.cpp", "FontCache"),
    @("TouchGFX/generated/fonts/src/Font_verdana_10_4bpp_0.cpp", "Font_verdana_10_4bpp_0"),
    @("TouchGFX/generated/fonts/src/Font_verdana_14_4bpp_0.cpp", "Font_verdana_14_4bpp_0"),
    @("TouchGFX/generated/fonts/src/Font_verdana_20_4bpp_0.cpp", "Font_verdana_20_4bpp_0"),
    @("TouchGFX/generated/fonts/src/Font_verdana_40_4bpp_0.cpp", "Font_verdana_40_4bpp_0"),
    @("TouchGFX/generated/gui_generated/src/common/FrontendApplicationBase.cpp", "FrontendApplicationBase"),
    @("TouchGFX/generated/fonts/src/GeneratedFont.cpp", "GeneratedFont"),
    @("TouchGFX/generated/fonts/src/Kerning_verdana_10_4bpp.cpp", "Kerning_verdana_10_4bpp"),
    @("TouchGFX/generated/fonts/src/Kerning_verdana_14_4bpp.cpp", "Kerning_verdana_14_4bpp"),
    @("TouchGFX/generated/fonts/src/Kerning_verdana_20_4bpp.cpp", "Kerning_verdana_20_4bpp"),
    @("TouchGFX/generated/fonts/src/Kerning_verdana_40_4bpp.cpp", "Kerning_verdana_40_4bpp"),
    @("TouchGFX/generated/texts/src/LanguageGb.cpp", "LanguageGb"),
    @("TouchGFX/generated/images/src/SVGDatabase.cpp", "SVGDatabase"),
    @("TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp", "Screen1ViewBase"),
    @("TouchGFX/generated/gui_generated/src/screen2_screen/Screen2ViewBase.cpp", "Screen2ViewBase"),
    @("TouchGFX/generated/fonts/src/Table_verdana_10_4bpp.cpp", "Table_verdana_10_4bpp"),
    @("TouchGFX/generated/fonts/src/Table_verdana_14_4bpp.cpp", "Table_verdana_14_4bpp"),
    @("TouchGFX/generated/fonts/src/Table_verdana_20_4bpp.cpp", "Table_verdana_20_4bpp"),
    @("TouchGFX/generated/fonts/src/Table_verdana_40_4bpp.cpp", "Table_verdana_40_4bpp"),
    @("TouchGFX/generated/texts/src/Texts.cpp", "Texts"),
    @("TouchGFX/generated/texts/src/TypedTextDatabase.cpp", "TypedTextDatabase"),
    @("TouchGFX/generated/fonts/src/UnmappedDataFont.cpp", "UnmappedDataFont"),
    @("TouchGFX/generated/fonts/src/VectorFontRendererBuffers.cpp", "VectorFontRendererBuffers")
)

$imageSrcDir = "$proj/TouchGFX/generated/images/src"
$imageFiles = Get-ChildItem "C:\TouchGFXProjects\SpaceInvaders\TouchGFX\generated\images\src" -Filter "image_*.cpp" -Recurse |
    Sort-Object Name

$imageEntries = @()
foreach ($f in $imageFiles) {
    $rel = $f.FullName.Substring("C:\TouchGFXProjects\SpaceInvaders\".Length).Replace('\', '/')
    $base = [System.IO.Path]::GetFileNameWithoutExtension($f.Name)
    $imageEntries += @(, @($rel, $base))
}

$all = $nonImage + $imageEntries

$cppSrcs = ($all | ForEach-Object { "$proj/$($_[0])" }) -join " \`n"
$objs = ($all | ForEach-Object { "./Application/User/generated/$($_[1]).o" }) -join " \`n"
$deps = ($all | ForEach-Object { "./Application/User/generated/$($_[1]).d" }) -join " \`n"

$rules = ($all | ForEach-Object {
    $path = "$proj/$($_[0])"
    $base = $_[1]
    "Application/User/generated/$base.o: $path Application/User/generated/subdir.mk`r`n`tarm-none-eabi-g++ `"`$<`" $flags -MMD -MP -MF`"`$(@:%.o=%.d)`" -MT`"`$@`" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o `"`$@`""
}) -join "`r`n"

$cleanLine1Items = @()
$cleanLine2Items = @()
$half = [Math]::Ceiling($all.Count / 2)
for ($i = 0; $i -lt $all.Count; $i++) {
    $base = $all[$i][1]
    $item = "./Application/User/generated/$base.cyclo ./Application/User/generated/$base.d ./Application/User/generated/$base.o ./Application/User/generated/$base.su"
    if ($i -lt $half) { $cleanLine1Items += $item } else { $cleanLine2Items += $item }
}
$cleanLine1 = "`t-`$(RM) " + ($cleanLine1Items -join " ")
$cleanLine2 = "`t-`$(RM) " + ($cleanLine2Items -join " ")

$content = @"
################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
CPP_SRCS += \
$cppSrcs 

OBJS += \
$objs 

CPP_DEPS += \
$deps 


# Each subdirectory must supply rules for building sources it contributes
$rules

clean: clean-Application-2f-User-2f-generated

clean-Application-2f-User-2f-generated:
$cleanLine1
$cleanLine2

.PHONY: clean-Application-2f-User-2f-generated

"@

Set-Content -Path $subdirMk -Value $content -NoNewline -Encoding ASCII
Write-Host "Wrote $subdirMk with $($all.Count) source entries."
