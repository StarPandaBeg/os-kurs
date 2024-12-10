@echo off
if "%~1"=="" (
    echo Usage: checksum.bat [file_path]
    exit /b 1
)

set "file=%~1"

:: Проверка существования файла
if not exist "%file%" (
    echo Error: File "%file%" does not exist.
    exit /b 1
)

:: Вычисление контрольной суммы
for /f "delims=" %%A in ('certutil -hashfile %file% SHA256 ^| findstr /r "^[0-9a-fA-F]"') do set "checksum=%%A"

:: Вывод только контрольной суммы
echo %checksum%
