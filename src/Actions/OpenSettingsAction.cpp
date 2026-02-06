#include "OpenSettingsAction.h"
#include "Components/Utils.h"
#include <tchar.h>
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

namespace NppJavaPlugin {

	void OpenSettingsAction::Execute()
	{
		// Get plugin directory
		std::wstring pluginDir = getPluginDirectory(hModule);
		if (pluginDir.empty()) {
			consoleLogger->Error(_T("Failed to get plugin directory."));
			return;
		}

		// Build config file path
		std::wstring configPath = pluginDir + L"\\NppJavaPlugin.ini";

		// Create default config if not exists
		if (!fileExists(configPath)) {
			// Create default configuration file
			std::wstring defaultConfig =
				L"[JavaPlugin]\r\n"
				L"JavaHome=\r\n"
				L"CfrJarPath=\r\n"
				L"\r\n";

			HANDLE hFile = CreateFile(configPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				DWORD bytesWritten;
				std::string configUtf8 = wstringAsString(defaultConfig);
				WriteFile(hFile, configUtf8.c_str(), (DWORD)configUtf8.length(), &bytesWritten, NULL);
				CloseHandle(hFile);
			}
		}

		// Open config file with default text editor
		ShellExecute(NULL, L"open", configPath.c_str(), NULL, NULL, SW_SHOW);
		consoleLogger->Info(std::wstring().append(_T("Opening configuration file: \"")).append(configPath).append(_T("\"")).c_str());
	}
}
