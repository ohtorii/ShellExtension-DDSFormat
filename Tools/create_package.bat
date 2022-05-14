@echo off
REM
REM 配布パッケージを作成するバッチファイル
REM
REM ＊使い方
REM create_package.bat version outputDirectory
REM
REM ＊使用例
REM create_package.bat 1.2.3 c:\project\package
REM
REM 出力フォルダの構成
REM + outputDirectory
REM    +- DDSFormat.zip
REM
REM
REM 一時ディレクトリの構成
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
REM 変数設定
REM --------------------------------------------------------------------
set BAT_DIR=%~dp0
set REPOSITORY=https://github.com/ohtorii/ShellExtension-DDSFormat.git
set PACKAGE_NAME=DDSFormat

REM
REM VisualStudiのバージョンに合わせてください
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
REM メイン処理
REM --------------------------------------------------------------------
echo ROOT_DIR=%ROOT_DIR%

set ARG_VERSION=%1
set ARG_OUTDIR=%2

call :Main
if ERRORLEVEL 1 (
    echo ==============================================================
    echo エラー
    echo ==============================================================
) else (
    echo ==============================================================
    echo 成功
    echo ==============================================================
)
exit /b !ERRORLEVEL!



:Main
    REM 引数の確認
    call :CheckArguments
    if ERRORLEVEL 1 (
        exit /b 1
    )
    call "%VSDEVCMD%"
    if ERRORLEVEL 1 (
        exit /b 1
    )

    REM 処理に必要なコマンドが存在するか調べる
    call :CheckCommands
    if ERRORLEVEL 1 (
        exit /b 1
    )

    REM 作業用の一時ディレクトリを作る
    md "%ROOT_DIR%"  > NUL 2>&1
    if not exist "%ROOT_DIR%" (
        echo 一時ディレクトリを作成できませんでした
        exit /b 1
    )
    REM 作業ディレクトリへ移動する
    pushd "%ROOT_DIR%"

    REM メインの処理
    call :CreatePackageDirectory
    if ERRORLEVEL 1 (
        echo CreatePackageDirectory失敗
    )
    set RET=!ERRORLEVEL!
    REM 元のディレクトリに戻る
    popd
    REM 後始末
    rmdir /S /Q "%ROOT_DIR%"

    exit /b !RET!

REM %1 一時ディレクトリ
:SetupRootDir
    setlocal
    set TEMP_TIME=%time: =0%
    set NOW=%date:/=%%TEMP_TIME:~0,2%%TEMP_TIME:~3,2%%TEMP_TIME:~6,2%
    set ROOT_DIR=%1\%NOW%_%random%
    endlocal && set ROOT_DIR=%ROOT_DIR%
    exit /b 0


REM 引数の確認
:CheckArguments
    if "%ARG_VERSION%" == "" (
        echo バージョン番号を指定してください。
        call :Usage
        exit /b 1
    )
    if "%ARG_OUTDIR%" == "" (
        echo 出力ディレクトリを指定してください。
        call :Usage
        exit /b 1
    )
    exit /b 0


REM バッチファイル中で使用するコマンドが存在するか調べる
:CheckCommands
    where git
    if ERRORLEVEL 1 (
        echo gitコマンドが見つかりません
        exit /b 1
    )

    where 7z
    if ERRORLEVEL 1 (
        echo 7zコマンドが見つかりません
        exit /b 1
    )

    where dotnet
    if ERRORLEVEL 1 (
        echo dotnetコマンドが見つかりません
        exit /b 1
    )

    where MSBuild
    if ERRORLEVEL 1 (
        echo MSBuildコマンドが見つかりません
        exit /b 1
    )

    where NuGet
    if ERRORLEVEL 1 (
        echo NuGetコマンドが見つかりません
        exit /b 1
    )
    exit /b 0



:CreatePackageDirectory
    git clone --recursive --depth 1 "%REPOSITORY%"
    if ERRORLEVEL 1 (
        echo gitコマンドが失敗しました。
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
    REM パッケージに必要なファイルを一時ディレクトリ内のフォルダへコピーする
    REM
    call :CopyFileToZipDir
    if ERRORLEVEL 1 (
        exit /b 1
    )

    call :Zip
    exit /b %ERRORLEVEL%


REM 不要なファイルを削除する
:DeleteUnnecessaryFiles
    rmdir /S /Q  "%GIT_ROOT_DIR%\.git" > NUL 2>&1
    if exist     "%GIT_ROOT_DIR%\.git" (
        echo .git ディレクトリの削除に失敗しました。
        set RESULT=1
        exit /b 1
    )
    rmdir /S /Q  "%GIT_ROOT_DIR%\.vs" > NUL 2>&1
    if exist     "%GIT_ROOT_DIR%\.vs" (
        echo .vs ディレクトリの削除に失敗しました。
        set RESULT=1
        exit /b 1
    )
    exit /b 0


:Build
    dotnet restore "%SOLUTION%"
    if ERRORLEVEL 1 (
        echo dotnet restore失敗
        exit /b 1
    )

    nuget restore "%SOLUTION%"
    if ERRORLEVEL 1 (
        echo nuget restore失敗
        exit /b 1
    )

    MSBuild "%SOLUTION%" -maxcpucount:%NUMBER_OF_PROCESSORS% /t:clean;rebuild /p:Configuration=Release;Platform="x64"
    if ERRORLEVEL 1 (
        echo ビルド失敗:x64
        exit /b 1
    )
    MSBuild "%SOLUTION%" -maxcpucount:%NUMBER_OF_PROCESSORS% /t:clean;rebuild /p:Configuration=Release;Platform="x86"
    if ERRORLEVEL 1 (
        echo ビルド失敗:x86
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
        echo 失敗 CopyPackageFiles
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
        echo ARG_OUTDIRディレクトリを作成できませんでした
        exit /b 1
    )
    REM zipで固める
    7z.exe a -mx9 -mmt%NUMBER_OF_PROCESSORS% "%ARG_OUTDIR%\%PACKAGE_NAME%-%ARG_VERSION%.zip" "%ZIP_DIR%\"
    if not exist  "%ARG_OUTDIR%\%PACKAGE_NAME%-%ARG_VERSION%.zip" (
        echo 7zコマンドが失敗しました
        exit /b 1
    )
    exit /b 0


:Usage
    echo;
    echo create_package.bat : Coopyright (c) 2022 ohtorii
    echo;
    echo ＊使い方
    echo create_package.bat バージョン番号 出力先ディレクトリ名
    echo;
    echo ＊使用例
    echo create_package.bat 1.2.3 c:\project\package
    exit /b 0
