#include <tchar.h>

#include "Configuration.h"
#include "Components/Utils.h"

namespace NppJavaPlugin {

	std::wstring Configuration::GetConfigFilePath() {
		if (configFilePath.empty()) {
			std::wstring pluginDir = getPluginDirectory(hModule);
			if (!pluginDir.empty()) {
				configFilePath = pluginDir + L"\\NppJavaPlugin.ini";
			}
		}
		return configFilePath;
	}

	std::wstring Configuration::ReadConfigValue(const wchar_t* key, const wchar_t* defaultValue) {
		std::wstring configFile = GetConfigFilePath();
		if (configFile.empty()) {
			return std::wstring(defaultValue);
		}

		TCHAR buffer[1024];
		DWORD result = GetPrivateProfileString(CONFIG_SECTION, key, defaultValue, buffer, 1024, configFile.c_str());
		
		if (result > 0) {
			std::wstring value(buffer);
			// Trim whitespace
			size_t start = value.find_first_not_of(L" \t\r\n");
			size_t end = value.find_last_not_of(L" \t\r\n");
			if (start != std::wstring::npos && end != std::wstring::npos) {
				return value.substr(start, end - start + 1);
			}
		}
		
		return std::wstring(defaultValue);
	}


	std::wstring Configuration::getJavaHome() {
		// Try to read from config file first
		std::wstring javaHome = ReadConfigValue(CONFIG_JAVA_HOME);
		if (!javaHome.empty()) {
			return javaHome;
		}
		// Fall back to environment variable
		return getEnvVariableValue(std::wstring(JAVA_HOME));
	}

	std::wstring Configuration::getJavacPath() {
		return getJavaHome().append(_T("\\bin\\")).append(JAVAC_EXECUTABLE_NAME);
	}

	std::wstring Configuration::getJavaPath() {
		return getJavaHome().append(_T("\\bin\\")).append(JAVA_EXECUTABLE_NAME);
	}

	std::wstring Configuration::getCfrJarPath() {
		// Try to read from config file first
		std::wstring cfrJarPath = ReadConfigValue(CONFIG_CFR_JAR_PATH);
		if (!cfrJarPath.empty()) {
			bool isAbsolute = (cfrJarPath.find(L":") != std::wstring::npos) ||
				(!cfrJarPath.empty() && (cfrJarPath[0] == L'\\' || cfrJarPath[0] == L'/'));
			if (!isAbsolute) {
				std::wstring pluginDir = getPluginDirectory(hModule);
				if (!pluginDir.empty()) {
					return pluginDir + L"\\" + cfrJarPath;
				}
			}
			return cfrJarPath;
		}
		// Default: search for cfr*.jar in plugin directory
		std::wstring pluginDir = getPluginDirectory(hModule);
		if (!pluginDir.empty()) {
			return findCfrJarFile(pluginDir);
		}
		return std::wstring();
	}

}
