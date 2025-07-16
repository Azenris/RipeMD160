@echo OFF
cls
setlocal enableDelayedExpansion

:: TEST 1
for /f "delims=" %%i in ('build\static-mt\builds\debug\ripemd160 test1.txt') do (
	set hash=%%i
)

if %ERRORLEVEL% neq 0 (
	echo RipeMD160 failed with errorcode: %ERRORLEVEL%
	exit /b %ERRORLEVEL%
)

echo   Result: !hash!
echo Expected: 9c1185a5c5e9fc54612808977ee8f548b2258d31

if "%hash%" == "9c1185a5c5e9fc54612808977ee8f548b2258d31" (
	echo SUCCESS. Same Hash!
) else (
	echo FAILED. Different Hash!
)

:: TEST 2
echo(
for /f "delims=" %%i in ('build\static-mt\builds\debug\ripemd160 test2.txt') do (
	set hash=%%i
)

if %ERRORLEVEL% neq 0 (
	echo RipeMD160 failed with errorcode: %ERRORLEVEL%
	exit /b %ERRORLEVEL%
)

echo   Result: !hash!
echo Expected: 0bdc9d2d256b3ee9daae347be6f4dc835a467ffe

if "%hash%" == "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe" (
	echo SUCCESS. Same Hash!
) else (
	echo FAILED. Different Hash!
)

:: TEST 3
echo(
for /f "delims=" %%i in ('build\static-mt\builds\debug\ripemd160 test3.txt') do (
	set hash=%%i
)

if %ERRORLEVEL% neq 0 (
	echo RipeMD160 failed with errorcode: %ERRORLEVEL%
	exit /b %ERRORLEVEL%
)

echo   Result: !hash!
echo Expected: 8eb208f7e05d987a9b044a8e98c6b087f15a0bfc

if "%hash%" == "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc" (
	echo SUCCESS. Same Hash!
) else (
	echo FAILED. Different Hash!
)

:: TEST 4
echo(
for /f "delims=" %%i in ('build\static-mt\builds\debug\ripemd160 test4.txt') do (
	set hash=%%i
)

if %ERRORLEVEL% neq 0 (
	echo RipeMD160 failed with errorcode: %ERRORLEVEL%
	exit /b %ERRORLEVEL%
)

echo   Result: !hash!
echo Expected: 5d0689ef49d2fae572b881b123a85ffa21595f36

if "%hash%" == "5d0689ef49d2fae572b881b123a85ffa21595f36" (
	echo SUCCESS. Same Hash!
) else (
	echo FAILED. Different Hash!
)

:: TEST 5
echo(
for /f "delims=" %%i in ('build\static-mt\builds\debug\ripemd160 test5.txt') do (
	set hash=%%i
)

if %ERRORLEVEL% neq 0 (
	echo RipeMD160 failed with errorcode: %ERRORLEVEL%
	exit /b %ERRORLEVEL%
)

echo   Result: !hash!
echo Expected: 37f332f68db77bd9d7edd4969571ad671cf9dd3b

if "%hash%" == "37f332f68db77bd9d7edd4969571ad671cf9dd3b" (
	echo SUCCESS. Same Hash!
) else (
	echo FAILED. Different Hash!
)