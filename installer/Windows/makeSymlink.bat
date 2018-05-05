IF "%SPINE_TYPE%" == "1" SET PARAM=/D

mklink %PARAM% "%SPINE_TARGET%" "%SPINE_SOURCE%"
exit %errorlevel%
