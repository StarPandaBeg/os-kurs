@echo off
setlocal enabledelayedexpansion

if "%~1"=="" (
    echo Error: File size is not specified.
    exit /b
)
if "%~2"=="" (
    echo Error: File name is not specified.
    exit /b
)

:: Присваиваем параметры переменным
set size=%~1
set filename=%~2

:: Проверяем, что размер файла — это число
for /f "delims=0123456789" %%a in ("%size%") do (
    echo Error: File size must be a number
    exit /b
)

:: Удаляем файл, если он существует
if exist %filename% del %filename%

:: Генерация строки 0-9
set digits=0123456789

:: Вычисляем длину строки digits
set length=10

:: Заполнение файла
(
for /l %%i in (1,1,%size%) do (
    set /a index=%%i %% %length%
    <nul set /p =!digits:~%index%,1!
)
) > %filename%

echo File %filename% with size %size% bytes was succesfully created!
