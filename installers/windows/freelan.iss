#define PRODUCT_NAME "FreeLAN"
#define AUTHOR "Julien Kauffmann"
#define URL "http://www.freelan.org"

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

[Languages]
Name: english; MessagesFile: compiler:Default.isl
Name: french; MessagesFile: compiler:Languages\French.isl
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl
Name: german; MessagesFile: compiler:Languages\German.isl

[Files]
Source: ..\..\freelan\bin\freelan.exe; DestDir: {app}\bin; Flags: ignoreversion; Components: binaries
Source: ..\..\freelan\config\freelan.cfg; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Components: configuration

[Tasks]
Name: install_service; Description: Register the Windows service.; Flags: checkedonce; Components: binaries; GroupDescription: Windows Service
Name: install_tap; Description: Install a tap adapter on this system.; Flags: checkedonce; Components: tap_adapter and binaries; GroupDescription: Tap adapter

[Components]
Name: binaries; Description: Install the {#PRODUCT_NAME} binaries.; Types: custom compact full
Name: configuration; Description: Install the sample configuration files.; Types: custom compact full
Name: tap_adapter; Description: Install the tap adapter driver and setup scripts.; Types: custom full

[Icons]
Name: {group}\{cm:UninstallProgram,{#PRODUCT_NAME}}; Filename: {uninstallexe}
