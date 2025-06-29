setlocal
mt.exe -nologo -manifest "./xBrowser/resources/win/xBrowser.exe.manifest" "./xBrowser/resources/win/compatibility.manifest" -outputresource:"./x64/Release/xBrowser.exe";#1
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/chrome_elf.dll ./x64/Release/chrome_elf.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/libcef.dll ./x64/Release/libcef.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/libEGL.dll ./x64/Release/libEGL.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/libGLESv2.dll ./x64/Release/libGLESv2.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/snapshot_blob.bin ./x64/Release/snapshot_blob.bin
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/v8_context_snapshot.bin ./x64/Release/v8_context_snapshot.bin
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/vk_swiftshader.dll ./x64/Release/vk_swiftshader.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/vk_swiftshader_icd.json ./x64/Release/vk_swiftshader_icd.json
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/vulkan-1.dll ./x64/Release/vulkan-1.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./deps/d3dcompiler_47.dll ./x64/Release/d3dcompiler_47.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./Resources/chrome_100_percent.pak ./x64/Release/chrome_100_percent.pak
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./Resources/chrome_200_percent.pak ./x64/Release/chrome_200_percent.pak
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./Resources/resources.pak ./x64/Release/resources.pak
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_if_different ./Resources/icudtl.dat ./x64/Release/icudtl.dat
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"cmake.exe" -E copy_directory ./Resources/locales ./x64/Release/locales
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd