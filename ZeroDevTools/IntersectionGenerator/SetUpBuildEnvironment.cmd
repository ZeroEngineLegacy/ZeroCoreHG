@echo off
REM Set the current working directoy to the directoy this file was run from.
SET GENERATOR_SOURCE=%CD%

pushd ..\..\
call SetUpBuildEnvironment
popd

SET GENERATOR_OUTPUT=%ZERO_OUTPUT%

