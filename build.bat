@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%~dp0"
if not exist build mkdir build
cl /std:c++17 /EHsc /Fe:build\compiler.exe src\main.cpp src\lexer.cpp src\parser.cpp src\symbols.cpp src\codegen.cpp src\exec_format.cpp src\vm.cpp /I src /nologo
exit /b %ERRORLEVEL%
