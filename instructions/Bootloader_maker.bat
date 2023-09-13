@echo 
For /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set mydate=%%a_%%b)
For /f "tokens=1-2 delims=/:" %%a in ('time /t') do (set mytime=%%a)
del  "..\Bootloader_Script\Finch_2_3.bin"
copy "..\..\..\..\SAMD\Finch\Finch2_SAMD_Application\Finch_2_3\Debug\Finch_2_3.bin" "..\Bootloader_Script\Finch_2_3.bin"
py -2 "..\Bootloader_Script\bin2hex_finch.py"
del  "..\nrfBoot\examples\peripheral\uart\PROG_data.h"
copy "..\Bootloader_Script\PROG_data.h" "..\nrfBoot\examples\peripheral\uart\PROG_data.h"
C:\Keil_v5\UV4\uVision  -b "..\..\Finch\nrfBoot\examples\peripheral\uart\pca10028\blank\arm5_no_packs\bootloaderFinch.uvprojx"
copy "..\nrfBoot\examples\peripheral\uart\pca10028\blank\arm5_no_packs\_build\nrf51422_xxac.hex" "..\Finch_Boot_%mydate%_%mytime%.hex"
PAUSE