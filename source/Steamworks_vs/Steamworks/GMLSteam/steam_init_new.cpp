
#include "pch.h"
#include "steam_api.h"
#include "Extension_Interface.h"
#include "YYRValue.h"
#include "steam_common.h"

#include <filesystem>
#include <string>
#include <vector>
#include <sstream>

#include "DesktopExtensionTools.h"

#if OS_MacOs
#include <dlfcn.h>
#endif

int requestInd = 0;
int getAsyncRequestInd()
{
	requestInd++;
	return requestInd;
}

YYEXPORT void steam_update(RValue& Result, CInstance* selfinst, CInstance* otherinst, int argc, RValue* arg)
{
	if (!steam_is_initialised)
	{
		Result.kind = VALUE_REAL;
		Result.val = 0;
		return;
	}

	SteamAPI_RunCallbacks();
	Steam_UserStats_Process();

	Result.kind = VALUE_REAL;
	Result.val = 1;
}

bool steam_is_initialised = false;

CSteamAPIContext* g_SteamContext;
YYEXPORT void steam_init(RValue& Result, CInstance* selfinst, CInstance* otherinst, int argc, RValue* arg)//(int appid)
{
	if (!steam_is_initialised)
	{
		Result.kind = VALUE_REAL;
		Result.val = 0;
		return;
	}

	Result.kind = VALUE_REAL;
	Result.val = 1;
}

#include <unordered_map>

using IniString = std::string;
using IniSection = std::unordered_map<IniString, IniString>;
using IniFile = std::unordered_map<IniString, IniSection>;

#ifdef _WIN32
HINSTANCE hDLLInstance(nullptr);
LPWSTR DllDirectory(nullptr);
VOID ObtainDllDirectory(VOID) {
	WCHAR pathbuff[8100] = { L'\0' };
	DWORD wlen = GetModuleFileNameW(hDLLInstance, pathbuff, (sizeof(pathbuff) / sizeof(pathbuff[0])) - 1);
	size_t idx = 0;
	if (wlen > 0) {
		for (idx = static_cast<size_t>(wlen) - 1; idx != 0; --idx) {
			if (pathbuff[idx] == L'\\') {
				pathbuff[idx + 1] = L'\0';
				break;
			}
		}
		DllDirectory = _wcsdup(pathbuff);
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		hDLLInstance = hinstDLL;
		ObtainDllDirectory();
	}

	return TRUE;
}
#endif

std::string getOptionsIni() {
	std::string optionsini;
#ifdef _WIN32
	WCHAR wcPathBuff[8100] = { L'\0' };
	wcscpy_s(wcPathBuff, DllDirectory);
	wcscat_s(wcPathBuff, L"options.ini");
	HANDLE hFile = CreateFileW(wcPathBuff, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile) {
		LARGE_INTEGER liFileSize = { 0 };
		if (GetFileSizeEx(hFile, &liFileSize)) {
			DWORD dwRead = 0;
			optionsini.resize(static_cast<size_t>(liFileSize.QuadPart));
			if (!ReadFile(hFile, const_cast<char*>(optionsini.data()), static_cast<DWORD>(liFileSize.QuadPart), &dwRead, nullptr)) {
				tracef("Failed to read options.ini!\n");
			}
		}
		CloseHandle(hFile);
	}
#else
	Dl_info dlinf = { 0 };
    /* this also seems to work on macOS... */
	if (dladdr(reinterpret_cast<void*>(&getOptionsIni), &dlinf)) {
		tracef(".so path is %s\n", dlinf.dli_fname);

		std::string s(dlinf.dli_fname);
		if (s.length()) {
			for (size_t idx = s.length() - 1; idx != 0; --idx) {
				if (s[idx] == '/') {
					s = s.substr(0, idx + 1);
					break;
				}
			}

			s += "options.ini";
			tracef("options.ini path is %s\n", s.c_str());
			FILE* f = fopen(s.c_str(), "rb");
			if (f) {
				fseek(f, 0, SEEK_END);
				size_t fsiz = static_cast<size_t>(ftell(f));
				fseek(f, 0, SEEK_SET);
				optionsini.resize(fsiz);
				fread(const_cast<char*>(optionsini.data()), 1, fsiz, f);
				fclose(f);
				tracef("options.ini read OK\n");
			}
		}
	}
#endif
	return optionsini;
}

std::string trimString(const std::string& inp) {
	std::string out(inp);

	if (out.empty())
		return out;

	for (std::string::iterator i = out.begin(); i != out.end();) {
		char c(*i);
		if (c == ' ' || c == '\t' || c == '\r') {
			i = out.erase(i);
		}
		else {
			break;
		}
	}

	for (std::string::reverse_iterator i = out.rbegin(); i != out.rend();) {
		char c(*i);
		if (c == ' ' || c == '\t' || c == '\r') {
			i = std::string::reverse_iterator(out.erase(std::next(i).base()));
		}
		else {
			break;
		}
	}

	return out;
}

IniFile parseIniFromString() {
	std::stringstream sstr(getOptionsIni());

	IniFile ini;
	IniString inisectionname;
	IniString line;
	//while (!IsDebuggerPresent());
	while (std::getline(sstr, line)) {
		if (line.length() <= 1) {
			continue;
		}

		if (line[0] == ';' || line[0] == '#') {
			continue;
		}

		if (line.length() > 2) {
			line = trimString(line);
			if (line[0] == '[' && line[line.length() - 1] == ']') {
				inisectionname = line.substr(1, line.length() - 2);
				//tracef("ini [%s]\n", inisectionname.c_str());
				continue;
			}
		}

		if (inisectionname.empty()) {
			continue;
		}

		size_t sizeidx = line.find('=');
		if (sizeidx == IniString::npos) {
			continue;
		}

		IniString key = trimString(line.substr(0, sizeidx));
		IniString value = trimString(line.substr(sizeidx + 1));
		if (value.length() > 1) {
			if (value[0] == '"' && value[value.length() - 1] == '"') {
				value = value.substr(1, value.length() - 2);
			}
		}

		//tracef("ini '%s'='%s'\n", key.c_str(), value.c_str());
		ini[inisectionname][key] = value;
	}

	return ini;
}

void OldPreGraphicsInitialisation()
{
	IniFile inifile = parseIniFromString();
	uint32 AppID = static_cast<uint32>(std::stoul(inifile["Steamworks"]["AppID"]));

    // a game cannot have an invalid appid
    if (AppID == k_uAppIdInvalid)
    {
        tracef("Invalid AppID, check extension settings in IDE, check file permissions.");
        return;
    }

	bool debug = (inifile["Steamworks"]["Debug"] == "Enabled")? true : (inifile["SteamworksIDE"]["IsRunningFromIDE"] == "True");
    
#ifndef DEBUG
    tracef("Library is built in release mode.\n");
#endif
    
    if (debug)
    {
        std::filesystem::path steamAppIdTxtPath = DesktopExtensionTools_getPathToExe();
        steamAppIdTxtPath /= "steam_appid.txt";
        std::ofstream steamAppIdTxt(steamAppIdTxtPath.string());
        std::string pathasstring = steamAppIdTxtPath.string();
        tracef("Debug: Writing AppID %u to file %s", static_cast<unsigned int>(AppID), pathasstring.c_str());
        if (steamAppIdTxt && (steamAppIdTxt << AppID))
        {
            tracef("Debug: Wrote AppID without errors.");
        }
        else
        {
            tracef("Debug: Unable to open the file or write the AppID, check file permissions?");
            // do not return; from here as macOS doesn't really allow you to write to your own .app?
            // SteamAPI_Init() will fail if it really can't guess the app id and we should rely on that instead.
        }
		steamAppIdTxt.close();
    }
    else
    {
        // https://partner.steamgames.com/doc/sdk/api#initialization_and_shutdown
        if (SteamAPI_RestartAppIfNecessary(AppID))
        {
            tracef("RestartAppIfNecessary check failed, the game is not allowed to continue");
            exit(0);
            return;
        }
    }

    // will also check if it can determine the app id
    if (!SteamAPI_Init())
    {
        tracef("SteamAPI_Init had failed, please check your Steamworks SDK path and that Steam is running! See Output above for possible errors.");
        return;
    }

    tracef("SteamAPI_Init had succeeded without errors, debug flag = %d", debug ? 1 : 0);

    steam_is_initialised = true;
}

YYEXPORT void steam_initialised(RValue& Result, CInstance* selfinst, CInstance* otherinst, int argc, RValue* arg)
{
	Result.kind = VALUE_BOOL;
	Result.val = steam_is_initialised;
}

extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char* pchDebugText)
{
	DebugConsoleOutput(pchDebugText);
	DebugConsoleOutput("\n");
}

YYEXPORT void steam_set_warning_message_hook(RValue& Result, CInstance* selfinst, CInstance* otherinst, int argc, RValue* arg)
{
	if (!steam_is_initialised)
	{
		Result.kind = VALUE_REAL;
		Result.val = 0;
		return;
	}

	SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);
}

YYEXPORT void steam_is_subscribed(RValue& Result, CInstance* selfinst, CInstance* otherinst, int argc, RValue* arg)
{

	if (!steam_is_initialised)
	{
		Result.kind = VALUE_REAL;
		Result.val = 0;
		return;
	}

	/*
	g_SteamContext = new CSteamAPIContext();
	g_SteamContext->Init();
	*/

	//Check player has rights to the game.
	//DebugConsoleOutput("SteamApps()->BIsSubscribed() failed\n");
	//DebugConsoleOutput("Steam must be running to play this game (user not subscribed)");
	//!need to shut down steamAPI since we have called SteamAPI_Init()

	Result.kind = VALUE_BOOL;
	Result.val = SteamApps()->BIsSubscribed();
}


//YYEXPORT void ext_json_test(RValue& Result, CInstance* selfinst, CInstance* otherinst, int argc, RValue* arg)
//{
//	DebugConsoleOutput("ext_json_test\n");
//
//	const char* arg1 = YYGetString(arg, 0);
//	json_object* jobj = json_tokener_parse(arg1);
//
//	uint32 AppID = json_object_get_int(json_object_object_get(jobj, "ProductId"));
//	bool debug = json_object_get_boolean(json_object_object_get(jobj, "Debug"));
//
//	DebugConsoleOutput("ext_json_test %i\n", AppID);
//	DebugConsoleOutput("ext_json_test %i\n", debug);
//}


YYEXPORT void steam_shutdown(RValue& Result, CInstance* selfinst, CInstance* otherinst, int argc, RValue* arg)
{
	if (!steam_is_initialised)
	{
		Result.kind = VALUE_REAL;
		Result.val = 0;
		return;
	}

	SteamAPI_Shutdown();
}

//dllg void Steam_Json_Test() {
//	nlohmann::json j = {
//		  {"pi", 3.141},
//		  {"happy", true},
//		  {"name", "Niels"},
//		  {"nothing", nullptr},
//		  {"answer", {
//			{"everything", 42}
//		  }},
//		  {"list", {1, 0, 2}},
//		{"strlist", {"1", "0", "2"}},
//		{"strlist2", {"1", "0"}},
//		{"strlist3", {"1"}},
//		  {"object", {
//			{"currency", "USD"},
//			{"value", 42.99}
//		  }}
//	};
//	std::string s = j.dump();
//	trace("json? %s", s.c_str());
//}
