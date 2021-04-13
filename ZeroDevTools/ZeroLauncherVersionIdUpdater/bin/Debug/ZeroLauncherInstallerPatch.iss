#define MyAppName "ZeroLauncherPatch"
#define MyAppNameVisual "Zero Launcher Patch"
#define MyAppPublisher "DigiPen Institute of Technology"
#define MyAppURL "http://zero.digipen.edu/"
#define MyAppVersion "0.0.1.0"                                         

#define IconPath "{app}\"

#ifndef ZeroSource
#define ZeroSource GetEnv("ZERO_SOURCE")    
#endif

#ifndef OutputFiles
#define OutputFiles ZeroSource + "\BuildOutput\Out\Win32\Release\Temp\ZeroLauncherPackage"
#endif

[Setup]
AppId={{E947C414-25CB-44AB-98B9-552D6958B943}          
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={localappdata}\ZeroLauncher_1.0
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename=ZeroLauncherPatch
SetupIconFile="{#ZeroSource}\Projects\Win32Shared\ZeroLauncherIcon.ico"
Compression=lzma
SolidCompression=yes
WizardImageFile="{#ZeroSource}\Build\ZeroInstall.bmp"
ChangesEnvironment=no
ChangesAssociations=no
PrivilegesRequired=none
Uninstallable=no

[Files]
Source: "{#OutputFiles}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs    

[InstallDelete]
Type: filesandordirs; Name: "{app}";


