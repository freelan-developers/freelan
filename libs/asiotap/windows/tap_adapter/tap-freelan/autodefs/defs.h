// define the TAP version
#define PRODUCT_TAP_ID "fltap0101"
#define PRODUCT_TAP_WIN32_MIN_MAJOR "1"
#define PRODUCT_TAP_WIN32_MIN_MINOR "1"

// Branding
#define PRODUCT_NAME "FreeLAN"
#define PRODUCT_UNIX_NAME "freelan"
#define PRODUCT_FILE_EXT "ovpn"

// tapinstall.exe source code.
// Not needed if DRVBINSRC is defined
// (or if using pre-built mode).
#define TISRC "../tapinstall"

// TAP Adapter parameters.  Note that PRODUCT_TAP_ID is
// defined in version.m4.
#define PRODUCT_TAP_DEVICE_DESCRIPTION "FreeLAN TAP Adapter V1"
#define PRODUCT_TAP_PROVIDER "FreeLAN TAP Provider V1"
#define PRODUCT_TAP_MAJOR_VER 1
#define PRODUCT_TAP_MINOR_VER 1
#define PRODUCT_TAP_RELDATE "11/25/2009"

// TAP adapter icon -- visible=0x81 or hidden=0x89
#define PRODUCT_TAP_CHARACTERISTICS 0x81

// Build debugging version of TAP driver
//!define PRODUCT_TAP_DEBUG

// DDK Version.
// DDK distribution is assumed to be in C:\WINDDK\${DDKVER}
#define DDKVER 7600.16385
#define DDKVER_MAJOR 7600

// Code Signing.
// If undefined, don't sign any files.
#define SIGNTOOL "../signtool"
#define PRODUCT_SIGN_CN "freelan"

// -j parameter passed to make
#define MAKE_JOBS 2

// output directory for built binaries
// and other generated files
#define GENOUT "gen"

// delete GENOUT directory before starting
// set to "yes" or "no"
#define CLEAN "yes"

// Don't strip executables and DLLs
//!define NO_STRIP

// DEBUGGING -- set to something like "-DBG2"
#define OUTFILE_LABEL ""

// DEBUGGING -- set to something like "DEBUG2"
#define TITLE_LABEL ""

#define PACKAGE_VERSION "1.1"
