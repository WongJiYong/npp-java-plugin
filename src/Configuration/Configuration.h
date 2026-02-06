#pragma once

#include <string>
#include <tchar.h>
#include <windows.h>

namespace NppJavaPlugin {

	constexpr const wchar_t* JAVA_HOME = _T("JAVA_HOME");
	constexpr const wchar_t* JAVAC_EXECUTABLE_NAME = _T("javac.exe");
	constexpr const wchar_t* JAVA_EXECUTABLE_NAME = _T("java.exe");
	constexpr const wchar_t* CONFIG_SECTION = _T("JavaPlugin");
	constexpr const wchar_t* CONFIG_JAVA_HOME = _T("JavaHome");
	constexpr const wchar_t* CONFIG_CFR_JAR_PATH = _T("CfrJarPath");

	class Configuration
	{
	private:
		HANDLE hModule;
		std::wstring configFilePath;
		std::wstring GetConfigFilePath();
		std::wstring ReadConfigValue(const wchar_t* key, const wchar_t* defaultValue = _T(""));
	public:
		Configuration(HANDLE hModule) : hModule(hModule) {}
		std::wstring getJavaHome();
		std::wstring getJavacPath();
		std::wstring getJavaPath();
		std::wstring getCfrJarPath();
	};
}
