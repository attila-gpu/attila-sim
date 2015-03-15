@echo off
echo ---------------------------------------
echo  DXInterceptor Documentation Generator
echo ---------------------------------------
echo.

if [%1] == [edit] goto edit_cfg
if [%1] == [clean] goto clean_doc

if not exist tools\doxygen\doxygen.exe goto files_not_found
if not exist htmldoc\dxcodegen.cfg goto files_not_found

REM ----------------------------------------------------------------------------
echo * Creating dirs...
if not exist htmldoc\dxcodegen mkdir htmldoc\dxcodegen\

REM ----------------------------------------------------------------------------
echo * Generating documentation for 'DXCodeGenerator'...
tools\doxygen\doxygen.exe htmldoc\dxcodegen.cfg 1> htmldoc\dxcodegen\dxcodegen.log 2> htmldoc\dxcodegen\dxcodegen.err

REM ----------------------------------------------------------------------------
copy /Y htmldoc\note.png htmldoc\dxcodegen\html\ > nul

REM ----------------------------------------------------------------------------
echo * END!
echo.
echo # Read *.log/*.err for a more detailed information.

goto final

:edit_cfg
if [%2] == [] goto edit_cfg_syntax
if not exist htmldoc\%2.cfg goto files_not_found
echo Editing '%2.cfg'...
tools\doxygen\doxywizard htmldoc\%2.cfg
goto final

:clean_doc
echo * Cleaning 'DXCodeGenerator'...
if exist htmldoc\dxcodegen rmdir /S /Q htmldoc\dxcodegen\
goto final

:files_not_found
echo ERROR: One of the needed files was not found!
goto final

:edit_cfg_syntax
echo Syntax: makedoc edit 'project-name' (without .cfg)
goto final

:final
