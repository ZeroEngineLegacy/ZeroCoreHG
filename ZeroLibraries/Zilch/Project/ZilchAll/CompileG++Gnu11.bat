@echo off
cls
echo ***********************************************************************
echo ****************************** G++Gnu11  ******************************
echo ***********************************************************************
g++ -std=gnu++11 Zilch.cpp main.cpp -o %~n0.exe -l"wsock32" -l"ws2_32" -l"IPHLPAPI" -l"Winmm" -l"Shlwapi" -l"Psapi" > %~n0.log 2>&1
type %~n0.log
echo ****************************** COMPLETED ******************************
pause
