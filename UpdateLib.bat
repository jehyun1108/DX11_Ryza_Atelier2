@echo off
setlocal

REM ===== 인자 바인딩 (순서 변경됨) =========================================
set "SOL=%~1"
set "SRC_BIN=%~2"
set "DST_CLIENT_BIN=%~3"
set "DST_IMPORTER_BIN=%~4"
set "DST_MAPTOOL_BIN=%~5"
set "DST_SDK_INC=%~6"
set "DST_SDK_LIB=%~7"

echo [LOG] Solution Root     : %SOL%
echo [LOG] Source Bin        : %SRC_BIN%
echo [LOG] Dest Client Bin   : %DST_CLIENT_BIN%
echo [LOG] Dest Importer Bin : %DST_IMPORTER_BIN%
echo [LOG] Dest MapTool Bin  : %DST_MAPTOOL_BIN%
echo [LOG] Dest SDK Include  : %DST_SDK_INC%
echo [LOG] Dest SDK Lib      : %DST_SDK_LIB%

REM ===== 폴더 준비 ===========================================================
mkdir "%DST_CLIENT_BIN%"   >nul 2>&1
mkdir "%DST_IMPORTER_BIN%" >nul 2>&1
mkdir "%DST_MAPTOOL_BIN%"  >nul 2>&1
mkdir "%DST_SDK_INC%"      >nul 2>&1
mkdir "%DST_SDK_LIB%"      >nul 2>&1

REM ===== DLL/PDB -> Client, Importer, MapTool =============================
echo [INFO] copy Engine.dll/pdb -> Client, Importer, and MapTool Bins
copy /Y "%SRC_BIN%\Engine.dll" "%DST_CLIENT_BIN%\"   >nul
copy /Y "%SRC_BIN%\Engine.pdb" "%DST_CLIENT_BIN%\"   >nul
copy /Y "%SRC_BIN%\Engine.dll" "%DST_IMPORTER_BIN%\" >nul
copy /Y "%SRC_BIN%\Engine.pdb" "%DST_IMPORTER_BIN%\" >nul
copy /Y "%SRC_BIN%\Engine.dll" "%DST_MAPTOOL_BIN%\"  >nul
copy /Y "%SRC_BIN%\Engine.pdb" "%DST_MAPTOOL_BIN%\"  >nul

REM ===== Engine.lib & 3rd party libs -> SDK Lib ==============================
echo [INFO] copy Engine.lib -> SDK Lib
copy /Y "%SRC_BIN%\Engine.lib" "%DST_SDK_LIB%\" >nul

echo [INFO] copy ThirdParty .lib -> SDK Lib
if exist "%SOL%\Engine\ThirdPartyLib\" (
  xcopy /Y /Q "%SOL%\Engine\ThirdPartyLib\*.lib" "%DST_SDK_LIB%\" >nul
)

REM ===== Public headers -> SDK Inc (재귀 복사) ===============================
echo [INFO] copy Public headers -> SDK Inc
robocopy "%SOL%\Engine\Public" "%DST_SDK_INC%" *.* /E /NFL /NDL /NJH /NJS /NP >nul

REM ===== Shaders -> Client, Importer, MapTool ===============================
echo [INFO] copy Shader dirs -> Client, Importer, and MapTool
for %%d in (Shaders, ShaderFiles) do (
  if exist "%SRC_BIN%\%%d" (
    robocopy "%SRC_BIN%\%%d" "%DST_CLIENT_BIN%\%%d"   *.* /E /NFL /NDL /NJH /NJS /NP >nul
    robocopy "%SRC_BIN%\%%d" "%DST_IMPORTER_BIN%\%%d" *.* /E /NFL /NDL /NJH /NJS /NP >nul
    robocopy "%SRC_BIN%\%%d" "%DST_MAPTOOL_BIN%\%%d"  *.* /E /NFL /NDL /NJH /NJS /NP >nul
  )
)

echo [DONE]
exit /b 0