#define MyAppName "Fractal Factory"
#define MyAppVersion "1.1.0"
#define MyAppPublisher "The Cosmic Order"

[Setup]
AppId={{4A4B0EB0-6A7D-48A0-9B28-0C5A28E6F0A1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={userdocs}\Resolume\Extra Effects\Fractal Factory
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

[Run]
Filename: "{cmd}"; Parameters: "/C echo Fractal Factory installed. Restart Resolume and add it from Sources."; Flags: runhidden postinstall skipifsilent
