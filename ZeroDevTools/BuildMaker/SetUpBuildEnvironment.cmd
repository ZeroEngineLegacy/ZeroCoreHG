@echo off

IF EXIST LocalBuildEnvironment.cmd (call LocalBuildEnvironment) ELSE (call DefaultBuildEnvironment)