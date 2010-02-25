// rc.cpp:  "Run Command" configuration file, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#include "StringPredicates.h"
#include "log.h"
#include "rc.h"

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

#include <boost/cstdint.hpp>
#include <sys/types.h>
#include <unistd.h> // for getuid()
#include <sys/stat.h>
#include <limits>
#include <cstdlib> // getenv
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/format.hpp>

#ifdef HAIKU_HOST
# include <Path.h>
# include <FindDirectory.h>
#endif

using std::endl;
using std::cout;
using std::cerr;

namespace gnash {

RcInitFile&
RcInitFile::getDefaultInstance()
{
    // TODO: allocate on the heap and provide a destroyDefaultInstance,
    //       for finer control of destruction order
    static RcInitFile rcfile;
    return rcfile;
}


RcInitFile::RcInitFile()
        :
    _delay(0),
    _movieLibraryLimit(8),
    _debug(false),
    _debugger(false),
    _verbosity(-1),
    // will be reset to something else if __OS2__x is defined:
    _urlOpenerFormat("firefox -remote 'openurl(%u)'"),
    _flashVersionString(
        DEFAULT_FLASH_PLATFORM_ID" "\
        DEFAULT_FLASH_MAJOR_VERSION","\
        DEFAULT_FLASH_MINOR_VERSION","\
        DEFAULT_FLASH_REV_NUMBER ",0"),
    // An empty string leaves detection to VM.cpp:
    _flashSystemOS(""),
    _flashSystemManufacturer("Gnash "DEFAULT_FLASH_SYSTEM_OS),
    _actionDump(false),
    _parserDump(false),
    _verboseASCodingErrors(false),
    _verboseMalformedSWF(false),
    _verboseMalformedAMF(false),
    _splashScreen(true),
    _localdomainOnly(false),
    _localhostOnly(false),
    _log("gnash-dbg.log"),
    _writeLog(false),
    _sound(true),
    _pluginSound(true),
    _extensionsEnabled(false),
    _startStopped(false),
    _insecureSSL(false),
    _streamsTimeout(DEFAULT_STREAMS_TIMEOUT),
    _solsandbox(DEFAULT_SOL_SAFEDIR),
    _solreadonly(false),
    _sollocaldomain(false),
    _lcdisabled(false),
    _lctrace(true),
    // TODO: give a  default value, and let 0 mean "disabled" -- 0
    // currently is overridden by libbase/shm.cpp 
    _lcshmkey(0),
    _ignoreFSCommand(true),
    _quality(-1),
    _saveStreamingMedia(false),
    _saveLoadedMedia(false),
    _popups(true),
    _useXv(false),
    _webcamDevice(-1),
    _microphoneDevice(-1),
    _certfile("client.pem"),
    _certdir("/etc/pki/tls"),
    _rootcert("rootcert.pem"),
    _ignoreShowMenu(true)
{
    expandPath(_solsandbox);
    loadFiles();

#ifdef __OS2__x
    _urlOpenerFormat = PrfQueryProfileString(HINI_USER, (PSZ)
            "WPURLDEFAULTSETTINGS",
            (PSZ) "DefaultBrowserExe", NULL,
            (PVOID) browserExe, (LONG)sizeof(browserExe));
    _urlOpenerFormat += " -remote 'openurl(%u)'";
#endif

    // TODO: fetch GNASH_URLOPENER_FORMAT from the environment
}

RcInitFile::~RcInitFile()
{  
}

// Look for a config file in the likely places.
void
RcInitFile::loadFiles()
{
// Note: This used to be bool but the return value was (a) never used
// and (b) only returned true if a gnashrc set by environment variable
// was successfully parsed. It seems we don't care whether a file
// actually exists or not anyway.

    // Check the default system location
    std::string loadfile = SYSCONFDIR;

#if !defined(__OS2__ ) && ! defined(__amigaos4__)
    // On OS/2 only look in %HOME%
    loadfile.append("/gnashrc");
    parseFile(loadfile);
#endif
    
    // Check the users home directory
    const char *home = 0;
    //on AmigaOS we have a GNASH: assign that point to program dir
#if defined (__amigaos4__)
    home = "/gnash";
#elif defined(HAIKU_HOST)
    BPath bp;
    if (B_OK != find_directory(B_USER_SETTINGS_DIRECTORY, &bp)) {
	log_error(_("Failed to find user settings directory"));
    } else {
	bp.Append("Gnash");
	home = bp.Path();
    }
#else
    home = std::getenv("HOME");
#endif
    if (home) {
        loadfile = home;
#ifdef HAIKU_HOST
        loadfile.append("/gnashrc");
#else
        loadfile.append("/.gnashrc");
#endif
        parseFile(loadfile);
    }

    // Check the GNASHRC environment variable
    // Parse each file only once
    char *gnashrc = std::getenv("GNASHRC");
    if (gnashrc)
    {
        std::string paths(gnashrc);
        Tok t(paths, Sep(":"));

        std::list<std::string> l;

        for (Tok::iterator i = t.begin(), e = t.end(); i != e; ++i)
        {
            l.remove(*i);
            l.push_back(*i);
        }

        for (std::list<std::string>::const_iterator i = l.begin(), e = l.end(); i != e; ++i)
        {
            parseFile(*i);
        }
    }
}

bool
RcInitFile::extractSetting(bool &var, const std::string &pattern,
               const std::string &variable, const std::string &value)
{
    
    StringNoCaseEqual noCaseCompare;
    if ( noCaseCompare(variable, pattern) ) {
        if ( noCaseCompare(value, "on") || noCaseCompare(value, "yes") ||
             noCaseCompare(value, "true")) {
            var = true;
        }
        if (noCaseCompare(value, "off") || noCaseCompare(value, "no") ||
            noCaseCompare(value, "false")) {
            var = false;
        }
        return true;
    }

    return false;
}


void
RcInitFile::parseList(PathList &list, const std::string &action,
                     const std::string &items)
{

    if (action == "set") {

        // Clear array of hosts in previously parsed
        // rc files.
        list.clear();

        StringNoCaseEqual noCaseCompare;

        if (noCaseCompare(items, "off") || noCaseCompare(items, "no") ||
            noCaseCompare(items, "false")) {
            // Return empty array (allows disabling of global
            // whitelists in favour of a blacklist)
            return;
        }
    }        

    Tok t(items, Sep(" "));

    for (Tok::iterator i = t.begin(), e = t.end(); i != e; ++i)
    {
        list.push_back(*i);
    }

}

bool
RcInitFile::extractDouble(double& out, const std::string &pattern,
                  const std::string &variable, const std::string &value)
{

    StringNoCaseEqual noCaseCompare;

    if ( noCaseCompare(variable, pattern) ) {
        std::istringstream in(value);
        if (in >> out) return true;
        
        // If the assignment fails, set the double to 0.
        out = 0;
        return true;
    }

    return false;
}

void
RcInitFile::expandPath (std::string& path)
{

// Leaves path unchanged on systems without
// POSIX tilde expansion.

#ifdef HAVE_PWD_H
//Don't build tilde expansion on systems without pwd.h

    //Only if path starts with "~"
    if (path[0] == '~') {

        // Initial "~" followed by "/"
        if (path.substr(1,1) == "/") {
            
            const char *home = std::getenv("HOME");
            if (home) {
                // if HOME set in env, replace ~ with HOME
                path.replace(0, 1, home);
            }

# ifdef HAVE_GETPWNAM //Don't try this on systems without getpwnam

            // HOME not found in env: try using pwd
            else { 
                struct passwd *password = getpwuid(getuid());
                    const char *pwdhome = password->pw_dir;
                    if (home) {
                        path.replace(0, 1, pwdhome);
                    }
                   //If all that fails, leave path alone
                }
            }

        //Initial "~" is not followed by "/"
        else {

            std::string::size_type firstslash =
                  path.find_first_of("/");
            std::string user;
            if (firstslash != std::string::npos) {
                // everything between initial ~ and / 
                user = path.substr(1, firstslash - 1 );
            }
            else user = path.substr(1);

            // find user using pwd
            const char *userhome = NULL;
            struct passwd *password = getpwnam(user.c_str());
            if (password) {
                userhome = password->pw_dir;
                if (userhome) {
                    path.replace(0, firstslash, userhome);
                }
            }
# endif
        }
    }
#endif

}

void
RcInitFile::writeList (const PathList& list, std::ostream& o)
{
    for (PathList::const_iterator it = list.begin();
        it != list.end(); ++it) {
            o << *it << " ";
        }
    o << endl;
}

// Parse the config file and set the variables.
bool
RcInitFile::parseFile(const std::string& filespec)
{

    struct stat stats;
    std::string action;
    std::string variable;
    std::string value;
    std::ifstream in;
    std::string line;

    StringNoCaseEqual noCaseCompare;
    
    if (filespec.empty()) {
        cerr << "RcInitFile::parseFile: empty filespec" << endl;
        return false;
    }
    
    if (stat(filespec.c_str(), &stats) != 0)
    {
//      This is a normal case, since we check many places for an RC file.
//      So don't complain bitterly every time we run gnash!
//      cerr << _("RcInitFile: couldn't open file: ") << filespec << endl;
        return false;
    }

    in.open(filespec.c_str());
    
    if (!in) {
            cerr << _("RcInitFile: couldn't open file: ") << filespec << endl;
            return false;
    }

    //cout << _("RcInitFile: parsing ") << filespec << endl;
        
    // Read in each line and parse it
    size_t lineno = 0;
    while (std::getline(in, line)) {

        ++lineno;

        // Ignore comment and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        
        // Get the first token
        if ( ! (ss >> action) )
        {
            // Empty line 
            continue;
        }
        
        // 'action' should never be empty, or (ss >> action) 
        // above would have failed

        if ( action[0] == '#' ) continue; // discard comments

        // Get second token
        if (! (ss >> variable) )
        {
            // Do we need to warn here as well?
            continue;
        }

        if (noCaseCompare(action, "set") || noCaseCompare(action, "append") )
        {

             // The rest of the line is the value
            if (!std::getline (ss, value))
            {
                cerr << boost::format(_("Warning: missing value for "
                            "variable \"%s\" in rcfile %s, line %d"))
                    % variable % filespec % lineno << endl;
                continue;
            }
     
            // Erase leading spaces.
            // If there are nothing but spaces in the value,
            // e.g. "set writelog ", value should be an empty string,
            // so value.erase(0, string::npos) is correct.
            value.erase(0, value.find_first_not_of(' '));

            if (noCaseCompare(variable, "urlOpenerFormat")) {
                _urlOpenerFormat = value;
                continue;
            }

            if (noCaseCompare(variable, "flashVersionString")) {
                _flashVersionString = value;
                continue;
            }
            
            if(noCaseCompare(variable, "GSTAudioSink")) {
                _gstaudiosink = value;
                continue;
            }
            
            if (noCaseCompare(variable, "flashSystemOS")) {
                _flashSystemOS = value;
                continue;
            }

            if (noCaseCompare(variable, "flashSystemManufacturer")) {
                _flashSystemManufacturer = value;
                continue;
            }

            if (noCaseCompare(variable, "debuglog")) {
                expandPath (value);
                _log = value;
                continue;
            }

            if (noCaseCompare(variable, "mediaDir") ) {
                expandPath(value);
                _mediaCacheDir = value;
                continue;
            }
            
            if (noCaseCompare(variable, "documentroot") ) {
                _wwwroot = value;
                continue;
            }
            
            if (noCaseCompare(variable, "blacklist") ) {
                parseList(_blacklist, action, value);
                continue;
            }

            if (noCaseCompare(variable, "whitelist")) {
                parseList(_whitelist, action, value);
                continue;
            }

            if (noCaseCompare(variable, "localSandboxPath")) {
                parseList(_localSandboxPath, action, value);
                continue;
            }

            if (noCaseCompare(variable, "SOLSafeDir")) {
                expandPath (value);
                _solsandbox = value;
                continue;
            }

            if (noCaseCompare(variable, "CertDir") ) {
                expandPath(value);
                _certdir = value;
                continue;
            }
            
            if (noCaseCompare(variable, "CertFile") ) {
                expandPath(value);
                _certfile = value;
                continue;
            }
            
            if (noCaseCompare(variable, "RootCert") ) {
                _rootcert = value;
                continue;
            }
            
            if (noCaseCompare(action , "set") ) {
                 extractSetting(_splashScreen, "splashScreen", variable,
                           value)
            || 
                 extractSetting(_localhostOnly, "localhost", variable,
                           value)
            || 
                 extractSetting(_localdomainOnly, "localdomain", variable,
                           value)
            ||
                 extractSetting(_insecureSSL, "insecureSSL", variable,
                           value)
            ||
                 extractSetting(_debugger, "debugger", variable, value)
            ||
                 extractSetting(_actionDump, "actionDump", variable, value)
            ||
                 extractSetting(_parserDump, "parserDump", variable, value)
            ||
                 extractSetting(_writeLog, "writelog", variable, value)
            ||
                 extractSetting(_popups, "popupMessages", variable, value)
            ||
                 extractSetting(_sound, "sound", variable, value)
            ||
                 extractSetting(_pluginSound, "pluginsound", variable, value)
            ||
                 extractSetting(_useXv, "xvideo", variable, value)
            ||
                 extractSetting(_verboseASCodingErrors,
                           "ASCodingErrorsVerbosity", variable, value)
            ||
                 extractSetting(_verboseMalformedSWF, "MalformedSWFVerbosity",
                           variable, value)
            ||
                 extractSetting(_verboseMalformedAMF, "MalformedAMFVerbosity",
                           variable, value)
            ||
                 extractSetting(_extensionsEnabled, "EnableExtensions",
                           variable, value)
            ||
                 extractSetting(_startStopped, "StartStopped", variable, value)
            ||
                 extractSetting(_solreadonly, "SOLReadOnly", variable,
                           value)
            ||
                 extractSetting(_sollocaldomain, "solLocalDomain", variable,
                           value)
            ||
		 extractSetting(_lcdisabled, "LocalConnection", variable,
                           value)
            ||
		 extractSetting(_lctrace, "LCTrace", variable,
                           value)
            ||
                 extractNumber(_movieLibraryLimit, "movieLibraryLimit",
                         variable, value)
            ||
                 extractNumber(_delay, "delay", variable, value)
            ||
                 extractNumber(_verbosity, "verbosity", variable, value)
            ||
                 extractNumber(_lcshmkey, "LCShmkey", variable, value)
            ||
                 extractDouble(_streamsTimeout, "streamsTimeout", variable, 
                         value)
            ||
                 extractNumber(_quality, "quality", variable, value)
            ||
                 extractSetting(_saveLoadedMedia, "saveLoadedMedia",
                         variable, value)
            ||
                 extractSetting(_saveStreamingMedia, "saveStreamingMedia",
                         variable, value)
            || 
                 extractSetting(_ignoreFSCommand, "ignoreFsCommand", variable,
                         value)
            ||
				 extractNumber(_webcamDevice, "webcamDevice", variable, 
						 value)
            ||
                 extractNumber(_microphoneDevice, "microphoneDevice", variable, value)
			||
                 extractSetting(_ignoreShowMenu, "ignoreShowMenu", variable,
                           value)
            ||
                 cerr << boost::format(_("Warning: unrecognized directive "
                             "\"%s\" in rcfile %s line %d")) 
                             % variable % filespec % lineno << endl;
            }
        }
        else if (noCaseCompare(action, "include")) {
            //cout << "Include directive in " << filespec << endl; 
            // TODO: resolve relative paths ?
            // TODO: skip parsing if already parsed ?
            //       (would mess up user-requested parsing order, but
            //       would be safe in case of circular includes)
            //
            if (variable.empty()) {
                cerr << boost::format(_("Warning: empty include "
                            "specification in rcfile %s, line %d"))
                            % filespec % lineno << endl;
            }
            else {
                if ( variable[0] != '/' ) {
                    cerr << boost::format(_("Warning: include specification "
                                "must be an absolute path in rcfile %s, "
                                "line %d")) % filespec % lineno << endl;
                }
                else {
                    parseFile(variable);
                }
            }
        }
        else {
            cerr << boost::format(_("Warning: unrecognized action \"%s\" in "
                "rcfile %s, line %d")) % action % filespec % lineno << endl;
        }
    }

    if (in) {
        in.close();
    }

    return true;
}

bool
RcInitFile::updateFile()
{

    // We don't want to try writing to /etc/ et cetera, since
    // these are likely to be system-wide defaults set by administrators
    // or distros, useful for a model rcfile. They are likely to have
    // no write permission anyway unless you are running Gnash with
    // administrative privileges, and that is a bad idea.

    std::string writefile;

    // The file specified in GNASHRC environment variable
    // should be written first.
    // If you name an important file in GNASHRC and 'save' the
    // preferences, you'll obviously overwrite that file.
    char *gnashrc = std::getenv("GNASHRC");
    if (gnashrc) {
        std::string filelist(gnashrc);
        
        if (filelist.empty()) return false;
        
        std::string::size_type pos = filelist.find_last_of(':');
        
        if (pos == std::string::npos) {
            // no separator: just one file.
            writefile = filelist;
        } else {
            writefile = filelist.substr(pos + 1);
        }
    } else {
        // Check the users home directory
        const char *home = NULL;
#if defined(__amigaos4__)
        //on AmigaOS we have a GNASH: assign that point to program dir
        home = "/gnash";
#elif defined(HAIKU_HOST)
        BPath bp;
        if (B_OK != find_directory(B_USER_SETTINGS_DIRECTORY, &bp)) {
            log_error(_("Failed to find user settings directory"));
        } else {
            bp.Append("Gnash");
            home = bp.Path();
        }
#else
	//on AmigaOS we have a GNASH: assign that point to program dir
	home = std::getenv("HOME");
#endif
        if (home) {
            writefile = home;
#ifdef HAIKU_HOST
            writefile.append("/gnashrc");
#else
             writefile.append("/.gnashrc");
#endi
        }
    }
    
    if (writefile.empty()) return false;

    return updateFile(writefile);

}


// Write the changed settings to the config file
bool
RcInitFile::updateFile(const std::string& filespec)
{

    if (filespec.empty()) {
        return false;
    }

    std::ofstream out;
    
    out.open(filespec.c_str());
        
    if (!out) {
        cerr << boost::format(_("Couldn't open file %s for writing")) % filespec << endl;
        return false;
    }

    const std::string cmd = "set ";

    // Bools and numbers. We want boolean values written as words, hex values
    // prefixed with '0x'.
    out << std::boolalpha << std::showbase <<
    _("# Generated by Gnash. Manual changes to this file may be overridden.") 
    << endl <<
    cmd << "splashScreen " << _splashScreen << endl <<
    cmd << "localHost " << _localhostOnly << endl <<
    cmd << "localDomain " << _localdomainOnly << endl <<
    cmd << "insecureSSL " << _insecureSSL << endl <<
    cmd << "debugger " << _debugger << endl <<
    cmd << "actionDump " << _actionDump << endl <<
    cmd << "parserDump " << _parserDump << endl <<
    cmd << "writeLog " << _writeLog << endl <<
    cmd << "sound " << _sound << endl <<
    cmd << "popupMessages " << _popups << endl <<
    cmd << "pluginSound " << _pluginSound << endl <<
    cmd << "ASCodingErrorsVerbosity " << _verboseASCodingErrors << endl <<
    cmd << "malformedSWFVerbosity " << _verboseMalformedSWF << endl <<
    cmd << "malformedAMFVerbosity " << _verboseMalformedAMF << endl <<
    cmd << "enableExtensions " << _extensionsEnabled << endl <<
    cmd << "startStopped " << _startStopped << endl <<
    cmd << "streamsTimeout " << _streamsTimeout << endl <<
    cmd << "movieLibraryLimit " << _movieLibraryLimit << endl <<
    cmd << "quality " << _quality << endl <<    
    cmd << "delay " << _delay << endl <<
    cmd << "verbosity " << _verbosity << endl <<
    cmd << "solReadOnly " << _solreadonly << endl <<
    cmd << "solLocalDomain " << _sollocaldomain << endl <<
    cmd << "SOLSafeDir " << _solsandbox << endl <<
    cmd << "localConnection " << _lcdisabled << endl <<
    cmd << "LCTrace " << _lctrace << endl <<
    cmd << "LCShmkey " << std::hex << (boost::uint32_t) _lcshmkey << endl <<
    cmd << "ignoreFSCommand " << _ignoreFSCommand << endl <<    
    cmd << "saveStreamingMedia " << _saveStreamingMedia << endl <<    
    cmd << "saveLoadedMedia " << _saveLoadedMedia << endl <<    
    cmd << "XVideo " << _useXv << endl <<    
   
    // Strings.

    // If a string is empty, Gnash defaults to the values set in the
    // constructor.

    // This might be irritating for users who, for instance, set
    // debuglog to nothing, only to find it returns to "gnash-debug.log"
    // at the next run (even though that's not the way to use it...)

    cmd << "mediaDir " << _mediaCacheDir << endl <<    
    cmd << "debuglog " << _log << endl <<
    cmd << "documentroot " << _wwwroot << endl <<
    cmd << "flashSystemOS " << _flashSystemOS << endl <<
    cmd << "flashVersionString " << _flashVersionString << endl <<
    cmd << "urlOpenerFormat " << _urlOpenerFormat << endl <<
    cmd << "GSTAudioSink " << _gstaudiosink << endl;

    // Lists. These can't be handled very well at the moment. The main
    // inconvenience would be that disabling a list makes it an empty
    // array in the rc class, and writing that to a file would lose that
    // blacklist you'd spent ages collecting.
    //
    // For now we'll just make sure lists that gnashrc finds are written.
    // Commented out lists in gnashrc will be deleted!
    //
    // The difference between append and set also can't be used without
    // a bit of trickery. It would be possible, for instance, to pass a
    // special character (e.g. '+') as the first element of an array from
    // the gui. However, this is possibly not all that useful anyway,
    // as lists in other parsed rcfiles would be taken over and written
    // as part of the new file.
    //
    // It might be necessary to have a 'useWhitelist' flag as well
    // as a whitelist. This would allow rcfile to keep a whitelist and write
    // it to disk, but not have to use it.
    
    out << cmd << "whitelist ";
    writeList (_whitelist, out);

    out << cmd << "blacklist ";
    writeList (_blacklist, out);
    
    // This also adds the base URL of the current SWF, as that gets
    // added to the path automatically...
    //out << cmd << "localSandboxPath ";
    //writeList (_localSandboxPath, out);

    out.close();
    
    return true;
}

void
RcInitFile::useSplashScreen(bool value)
{
    _splashScreen = value;
}

void
RcInitFile::useLocalDomain(bool value)
{
    _localdomainOnly = value;
}

void
RcInitFile::useXv(bool value)
{
    _useXv = value;
}

void
RcInitFile::useLocalHost(bool value)
{
    _localhostOnly = value;
}

void
RcInitFile::useActionDump(bool value)
{
    _actionDump = value;
}

void
RcInitFile::showASCodingErrors(bool value)
{
    _verboseASCodingErrors = value;
}

void
RcInitFile::showMalformedSWFErrors(bool value)
{
    _verboseMalformedSWF = value;
}

void
RcInitFile::useParserDump(bool value)
{
    _parserDump = value;
}

void
RcInitFile::useWriteLog(bool value)
{
    _writeLog = value;
}

void
RcInitFile::dump()
{
    
    cerr << endl << "Dump RcInitFile:" << endl;
    cerr << "\tTimer interupt delay value: " << _delay << endl;
    cerr << "\tFlash debugger: "
         << ((_debugger)?"enabled":"disabled") << endl;
    cerr << "\tVerbosity Level: " << _verbosity << endl;
    cerr << "\tDump ActionScript processing: "
         << ((_actionDump)?"enabled":"disabled") << endl;
    cerr << "\tDump parser info: "
         << ((_parserDump)?"enabled":"disabled") << endl;
    cerr << "\tActionScript coding errors verbosity: "
         << ((_verboseASCodingErrors)?"enabled":"disabled") << endl;
    cerr << "\tMalformed SWF verbosity: "
         << ((_verboseASCodingErrors)?"enabled":"disabled") << endl;
    cerr << "\tUse Splash Screen: "
         << ((_splashScreen)?"enabled":"disabled") << endl;
    cerr << "\tUse Local Domain Only: "
         << ((_localdomainOnly)?"enabled":"disabled") << endl;
    cerr << "\tUse Localhost Only: "
         << ((_localhostOnly)?"enabled":"disabled") << endl;
    cerr << "\tWrite Debug Log To Disk: "
         << ((_writeLog)?"enabled":"disabled") << endl;
    cerr << "\tAllow insecure SSL connections: "
         << ((_insecureSSL)?"yes":"no") << endl;
    cerr << "\tEnable sound: "
         << ((_sound)?"enabled":"disabled") << endl;
    cerr << "\tEnable Plugin sound: "
         << ((_pluginSound)?"enabled":"disabled") << endl;
    cerr << "\tEnable Extensions: "
         << ((_extensionsEnabled)?"enabled":"disabled") << endl;

    if (!_log.empty()) {
        cerr << "\tDebug Log name is: " << _log << endl;
    }

    if (!_flashVersionString.empty()) {
        cerr << "\tFlash Version String is: " << _flashVersionString << endl;
    }
    
    if(!_gstaudiosink.empty()) {
        cerr << "\tGST Audio Sink is: " << _gstaudiosink << endl;
    }
    
    cerr << "\tWhitelist: ";
    writeList (_whitelist, cerr);
    
    cerr << "\tBlacklist: ";
    writeList (_blacklist, cerr);
    
    cerr << "\tSandbox: ";
    writeList (_localSandboxPath, cerr);
}


} // end of namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

