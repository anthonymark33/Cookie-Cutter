@ECHO OFF
SET ROOT_DIR=%CD%

powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; wget https://storage.googleapis.com/chrome-infra/depot_tools.zip -OutFile depot_tools.zip "
7z -bd x %ROOT_DIR%\depot_tools.zip -odepot_tools
powershell -Command "depot_tools\update_depot_tools"
SET BUFF_PATH=%PATH%
SET DEPOT_TOOLS=%ROOT_DIR%\depot_tools
set PATH=%DEPOT_TOOLS%;%BUFF_PATH%

mkdir %ROOT_DIR%\src\breakpad
CD %ROOT_DIR%\src\breakpad
powershell -Command "fetch breakpad"
powershell -Command "gclient sync"

CD %ROOT_DIR%\src\breakpad\src\src\client\windows
DEL %CD%\breakpad_client.gyp
DEL %CD%\breakpad_client.sln
DEL %CD%\common.vcxproj
DEL %CD%\common.vcxproj.filters
DEL %CD%\build_all.vcxproj
COPY %ROOT_DIR%\scripts\breakpad_client.gyp %CD%

CD %ROOT_DIR%\src\breakpad\src\src
powershell -Command "tools\gyp\gyp.bat --no-circular-check client\windows\breakpad_client.gyp -Dwin_release_RuntimeLibrary=2 -Dwin_debug_RuntimeLibrary=2  -Dplatform=%ARCH% -Dconfiguration=release"

set PATH=%BUFF_PATH%
msbuild /m %CD%\client\windows\breakpad_client.sln /p:Configuration=release /p:Platform=%ARCH%
CD %ROOT_DIR%
