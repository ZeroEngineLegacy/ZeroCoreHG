@echo off
cls
echo ***********************************************************************
echo ******************************  Em++11   ******************************
echo ***********************************************************************
em++ -std=c++11 -Os Zilch.cpp main.cpp -o %~n0.html > %~n0.log 2>&1
type %~n0.log
echo ****************************** COMPLETED ******************************
pause
