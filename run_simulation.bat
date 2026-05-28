@echo off
setlocal

pushd "%~dp0" || exit /b 1
python .\simulation\run_simulation.py %*
set "SIM_EXIT_CODE=%ERRORLEVEL%"
popd

exit /b %SIM_EXIT_CODE%
