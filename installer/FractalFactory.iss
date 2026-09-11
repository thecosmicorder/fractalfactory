#define MyAppName "Fractal Factory"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "The Cosmic Order"
#define MyAppExeName "FractalFactoryControl.exe"

[Setup]
AppId={{4A4B0EB0-6A7D-48A0-9B28-0C5A28E6F0A1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Fractal Factory
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=output
OutputBaseFilename=FractalFactory_Setup_x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest

[Files]
Source: "..\artifacts\FractalFactory.dll"; DestDir: "{userdocs}\Resolume\Extra Effects\Fractal Factory"; Flags: ignoreversion
Source: "..\artifacts\FractalFactoryControl.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\Fractal Factory Control"; Filename: "{app}\FractalFactoryControl.exe"
Name: "{userdesktop}\Fractal Factory Control"; Filename: "{app}\FractalFactoryControl.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\FractalFactoryControl.exe"; Description: "Launch Fractal Factory Control"; Flags: nowait postinstall skipifsilent
