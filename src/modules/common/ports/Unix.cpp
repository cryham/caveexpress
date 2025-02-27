#include "Unix.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <SDL_assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <dlfcn.h>
#include <locale.h>
#include <signal.h>
#include <errno.h>
#include <algorithm>
#include <iterator>
#include <SDL.h>
#include "common/Config.h"
#include "common/ConfigManager.h"
#include "common/String.h"
#include "common/System.h"
#include "common/Application.h"
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#include <signal.h>
#include <cxxabi.h>
#endif
#include <signal.h>

Unix::Unix() :
		ISystem()
{
	void (*handler)(int);
	handler = signal(SIGPIPE, SIG_IGN);
	if (handler != SIG_DFL) {
		signal(SIGPIPE, handler);
	}
	struct passwd *p;

	if ((p = getpwuid(getuid())) == nullptr)
		_user = "";
	else
		_user = p->pw_name;
#ifdef HAVE_EXECINFO_H
	signal(SIGFPE, globalSignalHandler);
	signal(SIGSEGV, globalSignalHandler);
#endif
}

Unix::~Unix ()
{
}

std::string Unix::getCurrentUser ()
{
	return _user;
}

std::string Unix::normalizePath (const std::string& path)
{
	return path;
}

bool Unix::mkdir (const std::string& directory)
{
	if (directory.empty())
		return false;
	std::string s = directory;
	if (s[s.size() - 1] != '/') {
		// force trailing / so we can handle everything in loop
		s += '/';
	}

	size_t pre = 0, pos;
	while ((pos = s.find_first_of('/', pre)) != std::string::npos) {
		const std::string dir = s.substr(0, pos++);
		pre = pos;
		if (dir.empty())
			continue; // if leading / first time is 0 length
		const char *dirc = dir.c_str();
		const int retVal = ::mkdir(dirc, 0777);
		if (retVal == -1 && errno != EEXIST) {
			return false;
		}
	}
	return true;
}

DirectoryEntries Unix::listDirectory (const std::string& basedir, const std::string& subdir)
{
	DirectoryEntries entities;
	std::string search(basedir + "/");
	DIR *directory;
	struct dirent *d;
	struct stat st;

	if (!subdir.empty())
		search.append(subdir).append("/");

	if ((directory = ::opendir(search.c_str())) == nullptr)
		return entities;

	while ((d = ::readdir(directory)) != nullptr) {
		std::string name(search);
		name.append("/").append(d->d_name);
		if (::stat(name.c_str(), &st) == -1)
			continue;
		if (st.st_mode & S_IFDIR) {
			if (strcmp(d->d_name, ".") && strcmp(d->d_name, "..")) {
				std::string newsubdirs;
				if (!subdir.empty())
					newsubdirs.append(subdir).append("/");
				newsubdirs.append(d->d_name);
				const DirectoryEntries& subDirEnts = listDirectory(basedir, newsubdirs);
				entities.insert(entities.end(), subDirEnts.begin(), subDirEnts.end());
			}
		} else {
			std::string filename(subdir);
			if (!filename.empty())
				filename.append("/");
			filename.append(d->d_name);
			entities.push_back(filename);
		}
	}

	::closedir(directory);

	return entities;
}

void Unix::exit (const std::string& reason, int errorCode)
{
	if (errorCode != 0) {
		Log::error(LOG_COMMON, "%s", reason.c_str());
		backtrace(reason.c_str());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", reason.c_str(), nullptr);
	} else {
		Log::info(LOG_COMMON, "%s", reason.c_str());
	}

#ifdef DEBUG
	SDL_TriggerBreakpoint();
#endif
	::exit(errorCode);
}

int Unix::openURL (const std::string& url, bool) const
{
	const std::string cmd = "xdg-open \"" + url + "\"";
	return system(cmd.c_str());
}

int Unix::exec (const std::string& command, std::vector<std::string>& arguments) const
{
	const pid_t childPid = ::fork();
	if (childPid < 0) {
		Log::error(LOG_COMMON, "fork failed: %s", strerror(errno));
		return -1;
	}

	if (childPid == 0) {
		const char* argv[64];
		int argc = 0;
		if (arguments.size() > sizeof(argv))
			arguments.resize(sizeof(argv));
		for (std::vector<std::string>::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
			argv[argc++] = i->c_str();
		}
		// we are the child
		::execv(command.c_str(), const_cast<char* const*>(argv));

		// this should never get called
		Log::error(LOG_COMMON, "failed to run '%s' with %i parameters: %s (%i)", command.c_str(), (int)arguments.size(), strerror(errno), errno);
		::exit(10);
	}

	// we are the parent and are blocking until the child stopped
	int status;
	const pid_t pid = ::wait(&status);
#ifdef DEBUG
	SDL_assert(pid == childPid);
#else
	(void)pid;
#endif

	// check for success
	if (!WIFEXITED(status)) {
		Log::info(LOG_COMMON, "child process exists with error");
		return -1;
	}

	Log::info(LOG_COMMON, "child process returned with code %d", WEXITSTATUS(status));
	if (WEXITSTATUS(status) >= 5)
		return -1;

	// success
	return 0;
}

#ifdef HAVE_EXECINFO_H
namespace {
char* appendMessage(char* buf, const char* end, const char* fmt, ...) {
	if (buf >= (end - 2))
		return buf;

	va_list argList;
	va_start(argList, fmt);
	const int maxBytes = end - buf;
	const int num = SDL_vsnprintf(buf, maxBytes, fmt, argList);
	va_end(argList);
	if (num >= maxBytes)
		// output would have been truncated
		return buf;

	buf += num;
	return buf;
}
}
#endif

void Unix::backtrace (const char *errorMessage)
{
#ifdef HAVE_EXECINFO_H
	const int bufSize = 1 << 14;
	const int frameSize = 20;
	void *array[frameSize];

	std::unique_ptr<char[]> strBufStart(new char[bufSize]);
	char* strBuf = strBufStart.get();
	const char* strBufEnd = strBuf + bufSize;

	// get backtrace addresses
	const int size = ::backtrace((void**)array, frameSize);
	// demangle addresses to string thru symbols
	char **symbollist = ::backtrace_symbols(array, size);
	if (errorMessage) {
		strBuf = appendMessage(strBuf, strBufEnd, errorMessage);
		strBuf = appendMessage(strBuf, strBufEnd, "\n\n");
	}
	strBuf = appendMessage(strBuf, strBufEnd, "---START BACKTRACE CALLSTACK---\n");
	strBuf = appendMessage(strBuf, strBufEnd, "Obtained %zd stack frames.\n", size);
	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	for (int i = 1; i < size; ++i) {
		char *beginName = nullptr;
		char *beginOffset = nullptr;
		char *endOffset = nullptr;

		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for (char *p = symbollist[i]; *p; ++p) {
			if (*p == '(')
				beginName = p;
			else if (*p == '+')
				beginOffset = p;
			else if (*p == ')' && beginOffset) {
				endOffset = p;
				break;
			}
		}

		if (beginName && beginOffset && endOffset && beginName < beginOffset) {
			*beginName++ = '\0';
			*beginOffset++ = '\0';
			*endOffset = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status;
			char* funcname = abi::__cxa_demangle(beginName, nullptr, nullptr, &status);
			if (status == 0) {
				strBuf = appendMessage(strBuf, strBufEnd, "  %s : %s +%s\n",
						symbollist[i], funcname, beginOffset);
				free(funcname);
			} else {
				// demangling failed. Output function name as a C function with
				// no arguments.
				strBuf = appendMessage(strBuf, strBufEnd, "  %s : %s() +%s\n",
						symbollist[i], beginName, beginOffset);
			}
		} else {
			// couldn't parse the line? print the whole line.
			strBuf = appendMessage(strBuf, strBufEnd, "  %s\n", symbollist[i]);
		}
	}
	free(symbollist);
	appendMessage(strBuf, strBufEnd, "----END BACKTRACE CALLSTACK----\n");
	Log::error(LOG_COMMON, "%s", strBufStart.get());
#else
	Log::error(LOG_COMMON, "%s", errorMessage);
#endif
}

#ifdef DEBUG
int Unix::getScreenPadding ()
{
	return Config.getConfigVar("testscreenpadding", "0", true)->getIntValue();
}
#endif

std::string Unix::getRateURL (const std::string& packageName) const
{
	return "";
}
