# Haiku OS development

You are working on code for Haiku (the open-source BeOS successor) and its derivative RenkuOS,
usually ported from Linux or BSD programs, but in some cases also old BeOS programs that 
require updating for Haiku.

## Ground rules
- Haiku is not Linux. Never assume glibc, systemd, /proc, /usr, or Linux-only syscalls exist.
- Haiku is not Windows. Filetypes are primarily recognized by their MIME type, not their filename extension.
- The filesystem is package-managed and read-only under /boot/system. Installs go through
  packages (.hpkg); local unpackaged files go in ~/config/non-packaged/.
- Use `finddir` / find_directory() for standard paths. see below for list of targets.
- Never hardcode /usr or /etc.
- Networking lives in libnetwork (not libsocket/libnsl). BSD extras live in libbsd.
- When unsure how Haiku handles something, search the haikuports tree 
  (https://github.com/haikuports/haikuports.git) for prior art before guessing: other 
  ports have usually solved it already.
- A common issue with updating BeOS programs is the location of `makefile-engine`,
  which under Haiku can be found in `/boot/system/develop/etc/`.
- The program will  be compiled under the Haiku Operating System. If necessary, regard 
  cross-platform compatibility as less important than efficient compilation and full functionality
- Do not attempt verification builds. I will do those manually and get any error codes back to you.
  Just create a new branch on github called converttoqt5 for QT4 to QT5 or QT6 conversions, or 
  converttohaiku when updating old BeOS programs, and make the changes there.
- Use the cmake configuration system. If there already is a qmake .pro file or a simple Makefile 
  in place, replace it with an appropiate CmakeLists.txt. Old BeOS programs may have a .proj file. 
  This is a binary format from a BeOS build utility that is no longer available.
- Compilation should take no more than `mkdir build && cd build && cmake .. && make`. 
- `make install` is not required, but `make clean` is.

## Where to look for information
- HaikuPorts tree: https://github.com/haikuports/haikuports.git (recipes in <category>/<port>/, patchsets in patches/)
- HaikuPorter tool: https://github.com/haikuports/haikuporter.git
- API docs: https://api.haiku-os.org (Haiku Book)
- legacy BeBook (https://www.haiku-os.org/legacy-docs/bebook/) for older kits
- RenkuOS wiki: https://wayofrenkuos.miraheze.org/wiki/Main_Page
- QT4: https://dreamswork.github.io/qt4/
- QT5: https://doc.qt.io/archives/qt-5.15/

Do not try to read these as a whole. Use grep and other search tools to find required information.

## QT4 to QT5 conversion-specific information

QT4 has been deprecated on Haiku and is no longer officially available.

Not all the packages below will be installed on the development machine. 
You may assume the presence of qt5, qt5_devel, qt5_tools, qt5_script and qt5_script_devel.
Notify me when any others need to be installed for the build.

### Current QT5-related libraries available: 

Repository  Name                                   Version                Arch  
--------------------------------------------------------------------------------
HaikuPorts  futuresql_qt5                          0.1.1-3                x86_64
HaikuPorts  futuresql_qt5_devel                    0.1.1-3                x86_64
HaikuPorts  futuresql_qt5_source                   0.1.1-3                source
HaikuPorts  gpgme_qt5                              1.24.1-1               x86_64
HaikuPorts  kdsoap_qt5                             2.2.0-1                x86_64
HaikuPorts  kdsoap_qt5_devel                       2.2.0-1                x86_64
HaikuPorts  kdsoap_qt5_source                      2.2.0-1                source
HaikuPorts  kdsoap_ws_discovery_client_qt5         0.4.0-1                x86_64
HaikuPorts  kdsoap_ws_discovery_client_qt5_devel   0.4.0-1                x86_64
HaikuPorts  kdsoap_ws_discovery_client_qt5_source  0.4.0-1                source
HaikuPorts  kseexpr_qt5                            4.0.4.0-1              x86_64
HaikuPorts  kseexpr_qt5_devel                      4.0.4.0-1              x86_64
HaikuPorts  kseexpr_qt5_source                     4.0.4.0-1              source
HaikuPorts  libaccounts_qt5                        1.17-2                 x86_64
HaikuPorts  libaccounts_qt5_devel                  1.17-2                 x86_64
HaikuPorts  libaccounts_qt5_source                 1.17-2                 source
HaikuPorts  libaccounts_qt5_tools                  1.17-2                 x86_64
HaikuPorts  libqt5pas                              1.2.16-1               x86_64
HaikuPorts  libqt5pas_source                       1.2.16-1               source
HaikuPorts  libquotient_qt5                        0.8.2-1                x86_64
HaikuPorts  libquotient_qt5_devel                  0.8.2-1                x86_64
HaikuPorts  libquotient_qt5_source                 0.8.2-1                source
HaikuPorts  poppler23_qt5                          23.12.0-2              x86_64
HaikuPorts  poppler23_qt5_devel                    23.12.0-2              x86_64
HaikuPorts  poppler24_qt5                          24.12.0-2              x86_64
HaikuPorts  poppler24_qt5_devel                    24.12.0-2              x86_64
HaikuPorts  poppler25.12_qt5                       25.12.0-2              x86_64
HaikuPorts  poppler25.12_qt5_devel                 25.12.0-2              x86_64
HaikuPorts  pyqt5                                  5.15.11-1              x86_64
HaikuPorts  pyqt5_python310                        5.15.11-1              x86_64
HaikuPorts  pyqt5_sip                              12.15.0-2              x86_64
HaikuPorts  pyqt5_sip_python310                    12.15.0-2              x86_64
HaikuPorts  pyqt5_sip_source                       12.15.0-2              source
HaikuPorts  pyqt5_source                           5.15.11-1              source
HaikuPorts  python_poppler_qt5                     21.3.0-3               x86_64
HaikuPorts  python_poppler_qt5_python310           21.3.0-3               x86_64
HaikuPorts  python_poppler_qt5_source              21.3.0-3               source
<system>    qca_qt5                                2.3.12-1               x86_64
HaikuPorts  qca_qt5                                2.3.12-1               x86_64
HaikuPorts  qca_qt5_devel                          2.3.12-1               x86_64
HaikuPorts  qca_qt5_source                         2.3.12-1               source
HaikuPorts  qca_qt5_tools                          2.3.12-1               x86_64
HaikuPorts  qcoro_qt5                              0.13.0-1               x86_64
HaikuPorts  qcoro_qt5_debuginfo                    0.13.0-1               x86_64
HaikuPorts  qcoro_qt5_devel                        0.13.0-1               x86_64
HaikuPorts  qcoro_qt5_source                       0.13.0-1               source
HaikuPorts  qgpgme_qt5                             2.1.0-1                x86_64
HaikuPorts  qgpgme_qt5_debuginfo                   2.1.0-1                x86_64
HaikuPorts  qgpgme_qt5_devel                       2.1.0-1                x86_64
HaikuPorts  qgpgme_qt5_source                      2.1.0-1                source
HaikuPorts  qml_box2d_qt5                          20240415-1             x86_64
HaikuPorts  qml_box2d_qt5_source                   20240415-1             source
<system>    qt5                                    5.15.19-2              x86_64
HaikuPorts  qt5                                    5.15.19-2              x86_64
HaikuPorts  qt5_debuginfo                          5.15.19-2              x86_64
<system>    qt5_devel                              5.15.19-2              x86_64
HaikuPorts  qt5_devel                              5.15.19-2              x86_64
HaikuPorts  qt5_docs                               5.15.19-2              x86_64
HaikuPorts  qt5_examples                           5.15.19-2              x86_64
<system>    qt5_script                             5.15.19-2              x86_64
HaikuPorts  qt5_script                             5.15.19-2              x86_64
HaikuPorts  qt5_script_debuginfo                   5.15.19-2              x86_64
<system>    qt5_script_devel                       5.15.19-2              x86_64
HaikuPorts  qt5_script_devel                       5.15.19-2              x86_64
HaikuPorts  qt5_script_source                      5.15.19-2              source
HaikuPorts  qt5_source                             5.15.19-2              source
<system>    qt5_tools                              5.15.19-2              x86_64
HaikuPorts  qt5_tools                              5.15.19-2              x86_64
HaikuPorts  quazip1_qt5                            1.5-1                  x86_64
HaikuPorts  quazip1_qt5_debuginfo                  1.5-1                  x86_64
HaikuPorts  quazip1_qt5_devel                      1.5-1                  x86_64
HaikuPorts  quazip1_qt5_source                     1.5-1                  source
HaikuPorts  signond_qt5                            8.61-1                 x86_64
HaikuPorts  signond_qt5_devel                      8.61-1                 x86_64
HaikuPorts  signond_qt5_source                     8.61-1                 source
HaikuPorts  signond_qt5_tools                      8.61-1                 x86_64

### Snippets saved from previous QT4 to QT5 conversion chats

The error is my version guard: QLibraryInfo::path() was only introduced in Qt 6.0, not 5.15 as I assumed — Haiku's Qt 5.15.3 passes the >= 5.15 check but has no path() member. Fixing the threshold:
Cause: QLibraryInfo::path() was introduced in Qt 6.0, not Qt 5.15 as I had assumed. I wrote the guard as #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0), and since Haiku ships Qt 5.15.x, the condition was true on a Qt that only has the older location() — hence 'path' is not a member of 'QLibraryInfo'.
Fix in src/main.cpp:34: the threshold is now QT_VERSION_CHECK(6, 0, 0), so Qt 6 uses path() and Qt 5 keeps location(). On Qt 5.15 location() is not yet deprecated, so that branch compiles without warnings; the Qt6 path stays available for when the codebase is ported forward.

## finddir targets

B_DESKTOP_DIRECTORY
B_TRASH_DIRECTORY
B_APPS_DIRECTORY
B_PACKAGE_LINKS_DIRECTORY
B_PREFERENCES_DIRECTORY
B_UTILITIES_DIRECTORY
B_SYSTEM_DIRECTORY
B_SYSTEM_ADDONS_DIRECTORY
B_SYSTEM_BOOT_DIRECTORY
B_SYSTEM_FONTS_DIRECTORY
B_SYSTEM_LIB_DIRECTORY
B_SYSTEM_SERVERS_DIRECTORY
B_SYSTEM_APPS_DIRECTORY
B_SYSTEM_BIN_DIRECTORY
B_SYSTEM_DOCUMENTATION_DIRECTORY
B_SYSTEM_PREFERENCES_DIRECTORY
B_SYSTEM_TRANSLATORS_DIRECTORY
B_SYSTEM_MEDIA_NODES_DIRECTORY
B_SYSTEM_SOUNDS_DIRECTORY
B_SYSTEM_DATA_DIRECTORY
B_SYSTEM_DEVELOP_DIRECTORY
B_SYSTEM_PACKAGES_DIRECTORY
B_SYSTEM_HEADERS_DIRECTORY
B_SYSTEM_DESKBAR_DIRECTORY
B_SYSTEM_ETC_DIRECTORY
B_SYSTEM_SETTINGS_DIRECTORY
B_SYSTEM_LOG_DIRECTORY
B_SYSTEM_SPOOL_DIRECTORY
B_SYSTEM_TEMP_DIRECTORY
B_SYSTEM_VAR_DIRECTORY
B_SYSTEM_CACHE_DIRECTORY
B_SYSTEM_NONPACKAGED_DIRECTORY
B_SYSTEM_NONPACKAGED_ADDONS_DIRECTORY
B_SYSTEM_NONPACKAGED_TRANSLATORS_DIRECTORY
B_SYSTEM_NONPACKAGED_MEDIA_NODES_DIRECTORY
B_SYSTEM_NONPACKAGED_BIN_DIRECTORY
B_SYSTEM_NONPACKAGED_DATA_DIRECTORY
B_SYSTEM_NONPACKAGED_FONTS_DIRECTORY
B_SYSTEM_NONPACKAGED_SOUNDS_DIRECTORY
B_SYSTEM_NONPACKAGED_DOCUMENTATION_DIRECTORY
B_SYSTEM_NONPACKAGED_LIB_DIRECTORY
B_SYSTEM_NONPACKAGED_HEADERS_DIRECTORY
B_SYSTEM_NONPACKAGED_DEVELOP_DIRECTORY
B_USER_DIRECTORY
B_USER_CONFIG_DIRECTORY
B_USER_ADDONS_DIRECTORY
B_USER_BOOT_DIRECTORY
B_USER_FONTS_DIRECTORY
B_USER_LIB_DIRECTORY
B_USER_SETTINGS_DIRECTORY
B_USER_DESKBAR_DIRECTORY
B_USER_PRINTERS_DIRECTORY
B_USER_TRANSLATORS_DIRECTORY
B_USER_MEDIA_NODES_DIRECTORY
B_USER_SOUNDS_DIRECTORY
B_USER_DATA_DIRECTORY
B_USER_CACHE_DIRECTORY
B_USER_PACKAGES_DIRECTORY
B_USER_DEVELOP_DIRECTORY
B_USER_DOCUMENTATION_DIRECTORY
B_USER_HEADERS_DIRECTORY
B_USER_SERVERS_DIRECTORY
B_USER_APPS_DIRECTORY
B_USER_BIN_DIRECTORY
B_USER_PREFERENCES_DIRECTORY
B_USER_ETC_DIRECTORY
B_USER_LOG_DIRECTORY
B_USER_SPOOL_DIRECTORY
B_USER_VAR_DIRECTORY
B_USER_NONPACKAGED_DIRECTORY
B_USER_NONPACKAGED_ADDONS_DIRECTORY
B_USER_NONPACKAGED_TRANSLATORS_DIRECTORY
B_USER_NONPACKAGED_MEDIA_NODES_DIRECTORY
B_USER_NONPACKAGED_BIN_DIRECTORY
B_USER_NONPACKAGED_DATA_DIRECTORY
B_USER_NONPACKAGED_FONTS_DIRECTORY
B_USER_NONPACKAGED_SOUNDS_DIRECTORY
B_USER_NONPACKAGED_DOCUMENTATION_DIRECTORY
B_USER_NONPACKAGED_LIB_DIRECTORY
B_USER_NONPACKAGED_HEADERS_DIRECTORY
B_USER_NONPACKAGED_DEVELOP_DIRECTORY
B_BEOS_DIRECTORY
B_BEOS_SYSTEM_DIRECTORY
B_BEOS_ADDONS_DIRECTORY
B_BEOS_BOOT_DIRECTORY
B_BEOS_FONTS_DIRECTORY
B_BEOS_LIB_DIRECTORY
B_BEOS_SERVERS_DIRECTORY
B_BEOS_APPS_DIRECTORY
B_BEOS_BIN_DIRECTORY
B_BEOS_ETC_DIRECTORY
B_BEOS_DOCUMENTATION_DIRECTORY
B_BEOS_PREFERENCES_DIRECTORY
B_BEOS_TRANSLATORS_DIRECTORY
B_BEOS_MEDIA_NODES_DIRECTORY
B_BEOS_SOUNDS_DIRECTORY
B_COMMON_DIRECTORY
B_COMMON_SYSTEM_DIRECTORY
B_COMMON_ADDONS_DIRECTORY
B_COMMON_BIN_DIRECTORY
B_COMMON_DEVELOP_DIRECTORY
B_COMMON_DOCUMENTATION_DIRECTORY
B_COMMON_FONTS_DIRECTORY
B_COMMON_LIB_DIRECTORY
B_COMMON_BOOT_DIRECTORY
B_COMMON_ETC_DIRECTORY
B_COMMON_LOG_DIRECTORY
B_COMMON_SERVERS_DIRECTORY
B_COMMON_SETTINGS_DIRECTORY
B_COMMON_SPOOL_DIRECTORY
B_COMMON_TEMP_DIRECTORY
B_COMMON_VAR_DIRECTORY

