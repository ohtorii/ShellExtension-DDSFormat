@echo off
REM
REM �z�z�p�b�P�[�W���쐬����o�b�`�t�@�C��
REM
REM ���g����
REM create_package.bat version outputDirectory
REM
REM ���g�p��
REM create_package.bat 1.2.3 c:\project\package
REM
REM �o�̓t�H���_�̍\��
REM + outputDirectory
REM    +- DDSFormat.zip
REM
REM
REM �ꎞ�f�B���N�g���̍\��
REM +- %TEMP%
REM    +- temp_root_dir                           <- %ROOT_DIR%
REM       +- ShellExtension-DDSFormat             <- %GIT_ROOT_DIR%
REM       +- DDSFormat                            <- %ZIP_DIR%
REM          +- README.MD
REM          +- LICENSE
REM          +- bin_x64
REM             +- Register_x64.bat
REM             +- UnRegister_x64.bat
REM             +- DDSFormat.dll
REM          +- bin_x86
REM             +- Register_x86.bat
REM             +- UnRegister_x86.bat
REM             +- DDSFormat.dll


setlocal enabledelayedexpansion

REM --------------------------------------------------------------------
REM �ϐ��ݒ�
REM --------------------------------------------------------------------
set BAT_DIR=%~dp0
set REPOSITORY=https://github.com/ohtorii/ShellExtension-DDSFormat.git
set PACKAGE_NAME=DDSFormat

REM
REM VisualStudi�̃o�[�W�����ɍ��킹�Ă�������
REM set VSDEVCMD=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat
set VSDEVCMD=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat

REM call :SetupRootDir %TEMP%
call :SetupRootDir d:\temp

set GIT_ROOT_DIR=%ROOT_DIR%\ShellExtension-DDSFormat
set ZIP_DIR=%ROOT_DIR%\%PACKAGE_NAME%
set SOLUTION=%GIT_ROOT_DIR%\Apps\DDSFormat.sln

set BIN_SRC_BACKEND_X64_RELEASE=%GIT_ROOT_DIR%\Apps\x64\Release
set BIN_SRC_BACKEND_X86_RELEASE=%GIT_ROOT_DIR%\Apps\Win32\Release


REM --------------------------------------------------------------------
REM ���C������
REM --------------------------------------------------------------------
echo ROOT_DIR=%ROOT_DIR%

set ARG_VERSION=%1
set ARG_OUTDIR=%2

call :Main
if ERRORLEVEL 1 (
    echo ==============================================================
    echo �G���[
    echo ==============================================================
) else (
    echo ==============================================================
    echo ����
    echo ==============================================================
)
exit /b !ERRORLEVEL!



:Main
    REM �����̊m�F
    call :CheckArguments
    if ERRORLEVEL 1 (
        exit /b 1
    )
    call "%VSDEVCMD%"
    if ERRORLEVEL 1 (
        exit /b 1
    )

    REM �����ɕK�v�ȃR�}���h�����݂��邩���ׂ�
    call :CheckCommands
    if ERRORLEVEL 1 (
        exit /b 1
    )

    REM ��Ɨp�̈ꎞ�f�B���N�g�������
    md "%ROOT_DIR%"  > NUL 2>&1
    if not exist "%ROOT_DIR%" (
        echo �ꎞ�f�B���N�g�����쐬�ł��܂���ł���
        exit /b 1
    )
    REM ��ƃf�B���N�g���ֈړ�����
    pushd "%ROOT_DIR%"

    REM ���C���̏���
    call :CreatePackageDirectory
    if ERRORLEVEL 1 (
        echo CreatePackageDirectory���s
    )
    set RET=!ERRORLEVEL!
    REM ���̃f�B���N�g���ɖ߂�
    popd
    REM ��n��
    rmdir /S /Q "%ROOT_DIR%"

    exit /b !RET!

REM %1 �ꎞ�f�B���N�g��
:SetupRootDir
    setlocal
    set TEMP_TIME=%time: =0%
    set NOW=%date:/=%%TEMP_TIME:~0,2%%TEMP_TIME:~3,2%%TEMP_TIME:~6,2%
    set ROOT_DIR=%1\%NOW%_%random%
    endlocal && set ROOT_DIR=%ROOT_DIR%
    exit /b 0


REM �����̊m�F
:CheckArguments
    if "%ARG_VERSION%" == "" (
        echo �o�[�W�����ԍ����w�肵�Ă��������B
        call :Usage
        exit /b 1
    )
    if "%ARG_OUTDIR%" == "" (
        echo �o�̓f�B���N�g�����w�肵�Ă��������B
        call :Usage
        exit /b 1
    )
    exit /b 0


REM �o�b�`�t�@�C�����Ŏg�p����R�}���h�����݂��邩���ׂ�
:CheckCommands
    where git
    if ERRORLEVEL 1 (
        echo git�R�}���h��������܂���
        exit /b 1
    )

    where 7z
    if ERRORLEVEL 1 (
        echo 7z�R�}���h��������܂���
        exit /b 1
    )

    where dotnet
    if ERRORLEVEL 1 (
        echo dotnet�R�}���h��������܂���
        exit /b 1
    )

    where MSBuild
    if ERRORLEVEL 1 (
        echo MSBuild�R�}���h��������܂���
        exit /b 1
    )

    where NuGet
    if ERRORLEVEL 1 (
        echo NuGet�R�}���h��������܂���
        exit /b 1
    )
    exit /b 0



:CreatePackageDirectory
    git clone --recursive --depth 1 "%REPOSITORY%"
    if ERRORLEVEL 1 (
        echo git�R�}���h�����s���܂����B
        exit /b 1
    )
    
    call :DeleteUnnecessaryFiles
    if ERRORLEVEL 1 (
        exit /b 1
    )
    
    call :Build
    if ERRORLEVEL 1 (
        exit /b 1
    )
    
    REM
    REM �p�b�P�[�W�ɕK�v�ȃt�@�C�����ꎞ�f�B���N�g�����̃t�H���_�փR�s�[����
    REM
    call :CopyFileToZipDir
    if ERRORLEVEL 1 (
        exit /b 1
    )

    call :Zip
    exit /b %ERRORLEVEL%


REM �s�v�ȃt�@�C�����폜����
:DeleteUnnecessaryFiles
    rmdir /S /Q  "%GIT_ROOT_DIR%\.git" > NUL 2>&1
    if exist     "%GIT_ROOT_DIR%\.git" (
        echo .git �f�B���N�g���̍폜�Ɏ��s���܂����B
        set RESULT=1
        exit /b 1
    )
    rmdir /S /Q  "%GIT_ROOT_DIR%\.vs" > NUL 2>&1
    if exist     "%GIT_ROOT_DIR%\.vs" (
        echo .vs �f�B���N�g���̍폜�Ɏ��s���܂����B
        set RESULT=1
        exit /b 1
    )
    exit /b 0


:Build
    dotnet restore "%SOLUTION%"
    if ERRORLEVEL 1 (
        echo dotnet restore���s
        exit /b 1
    )

    nuget restore "%SOLUTION%"
    if ERRORLEVEL 1 (
        echo nuget restore���s
        exit /b 1
    )

    MSBuild "%SOLUTION%" -maxcpucount:%NUMBER_OF_PROCESSORS% /t:clean;rebuild /p:Configuration=Release;Platform="x64"
    if ERRORLEVEL 1 (
        echo �r���h���s:x64
        exit /b 1
    )
    MSBuild "%SOLUTION%" -maxcpucount:%NUMBER_OF_PROCESSORS% /t:clean;rebuild /p:Configuration=Release;Platform="x86"
    if ERRORLEVEL 1 (
        echo �r���h���s:x86
        exit /b 1
    )
    exit /b 0


:CopyFileToZipDir
    xcopy "%BIN_SRC_BACKEND_X64_RELEASE%\DDSFormat.dll" "%ZIP_DIR%\bin_x64\" /F /Y
    xcopy "%BIN_SRC_BACKEND_X64_RELEASE%\DDSFormat.dll" "%ZIP_DIR%\bin_x86\" /F /Y

    copy /B "%GIT_ROOT_DIR%\README.md"   "%ZIP_DIR%"
    copy /B "%GIT_ROOT_DIR%\LICENSE"     "%ZIP_DIR%"
    call :CopyPackageFiles
    if ERRORLEVEL 1 (
        echo ���s CopyPackageFiles
        exit /b 1
    )
    exit /b 0


:CopyPackageFiles
    if not exist "%BAT_DIR%\package_files\" exit /b 0
    xcopy "%BAT_DIR%\package_files\" "%ZIP_DIR%" /Y /I /E
    exit /b %ERRORLEVEL%


:Zip
    md "%ARG_OUTDIR%" > NUL 2>&1
    if not exist "%ARG_OUTDIR%" (
        echo ARG_OUTDIR�f�B���N�g�����쐬�ł��܂���ł���
        exit /b 1
    )
    REM zip�Ōł߂�
    7z.exe a -mx9 -mmt%NUMBER_OF_PROCESSORS% "%ARG_OUTDIR%\%PACKAGE_NAME%-%ARG_VERSION%.zip" "%ZIP_DIR%\"
    if not exist  "%ARG_OUTDIR%\%PACKAGE_NAME%-%ARG_VERSION%.zip" (
        echo 7z�R�}���h�����s���܂���
        exit /b 1
    )
    exit /b 0


:Usage
    echo;
    echo create_package.bat : Coopyright (c) 2022 ohtorii
    echo;
    echo ���g����
    echo create_package.bat �o�[�W�����ԍ� �o�͐�f�B���N�g����
    echo;
    echo ���g�p��
    echo create_package.bat 1.2.3 c:\project\package
    exit /b 0
