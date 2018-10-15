; General defines
#define PRODUCT_NAME "FreeLAN"
#define AUTHOR "Julien Kauffmann"
#define URL "http://www.freelan.org"

; Tap adapter defines
#define TAP "tap0901"
#define TAPINF "OemVista"

[Setup]
AppId={{3AE669E7-36C2-48DF-985B-6037F9AF69A8}
AppName={#PRODUCT_NAME}
AppVersion={#VERSION}
AppVerName={#PRODUCT_NAME} {#VERSION}
AppPublisher={#AUTHOR}
AppPublisherURL={#URL}
AppSupportURL={#URL}
AppUpdatesURL={#URL}
DefaultDirName={pf}\{#PRODUCT_NAME}
DefaultGroupName={#PRODUCT_NAME}
AllowNoIcons=yes
LicenseFile=LICENSE.txt
OutputDir=.
OutputBaseFilename={#NAME}-{#VERSION}-{#ARCH}-install
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
WizardSmallImageFile=images\wizard_small.bmp
WizardImageFile=images\wizard.bmp

[Languages]
Name: english; MessagesFile: compiler:Default.isl
Name: french; MessagesFile: compiler:Languages\French.isl
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl
Name: german; MessagesFile: compiler:Languages\German.isl

[Files]
Source: ..\..\install\{#XARCH}\Release\bin\freelan.exe; DestDir: {app}\bin; Flags: ignoreversion; Components: binaries
Source: ..\..\install\{#XARCH}\Release\config\freelan.cfg; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Components: configuration
Source: files\{#ARCH}\{#TAPINF}.inf; DestDir: {app}\driver; Flags: ignoreversion; Components: tap_adapter
Source: files\{#ARCH}\{#TAP}.cat; DestDir: {app}\driver; Flags: ignoreversion; Components: tap_adapter
Source: files\{#ARCH}\{#TAP}.sys; DestDir: {app}\driver; Flags: ignoreversion; Components: tap_adapter
Source: {#TAP_SETUP}; DestDir: {app}\bin; Flags: ignoreversion; Components: tap_adapter
Source: files\README.txt; DestDir: {app}; Flags: isreadme ignoreversion

[Dirs]
Name: {app}\log; Flags: deleteafterinstall; Components: binaries

[Tasks]
Name: install_service; Description: Register the Windows service.; Flags: checkedonce; Components: binaries; GroupDescription: Windows Service
Name: install_tap; Description: Install a tap adapter on this system.; Flags: checkedonce; Components: tap_adapter; GroupDescription: Tap adapter

[Components]
Name: binaries; Description: Install the {#PRODUCT_NAME} binaries.; Types: custom compact full
Name: configuration; Description: Install the sample configuration files.; Types: custom compact full
Name: tap_adapter; Description: Install the tap adapter driver and setup scripts.; Types: custom full

[Icons]
Name: {group}\{cm:UninstallProgram,{#PRODUCT_NAME}}; Filename: {uninstallexe}

[UninstallDelete]
Type: files; Name: {app}\log\freelan.log

[Run]
Filename: {app}\bin\tap-setup.exe; Parameters: "install ..\driver\{#TAPINF}.inf {#TAP}"; StatusMsg: "Installing the tap adapter..."; Tasks: install_tap
Filename: {app}\bin\freelan.exe; Parameters: "--install"; StatusMsg: "Installing Windows Service..."; Tasks: install_service; Flags: runhidden

[UninstallRun]
Filename: {app}\bin\freelan.exe; Parameters: "--uninstall"; StatusMsg: "Uninstalling Windows Service..."; Tasks: install_service; Flags: runhidden
Filename: {app}\bin\tap-setup.exe; Parameters: "remove {#TAP}"; StatusMsg: "Uninstalling all tap adapters..."; Tasks: install_tap

[Registry]
Root: HKLM; Subkey: "Software\FreeLAN"; ValueType: string; ValueName: "installation_path"; ValueData: "{app}"; Flags: uninsdeletekey
