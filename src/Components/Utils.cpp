#include <shlwapi.h>
#include <tchar.h>
#include "Utils.h"

namespace NppJavaPlugin {

	constexpr int PER_INCH_VALUE = 1440;
	constexpr int DPI_VALUE = 96;
	constexpr char FILE_NAME_EXTENSION_SEPARATOR = '.';

	std::wstring GetLastErrorAsString()
	{
		DWORD errorCode = GetLastError();

		if (errorCode == 0)
			return std::wstring();
		else {
			LPWSTR messageBuffer = nullptr;
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, nullptr);

			std::wstring errorMsg = messageBuffer;

			LocalFree(messageBuffer);

			return errorMsg;
		}
	}

	int PixelsToTwips(int pixels)
	{
		// Assumes hard-coded DPI value to allow Windows Text Auto-Scaling for High-DPI devices
		return abs(MulDiv(pixels, PER_INCH_VALUE, DPI_VALUE));
	}

	TCHAR* wstringAsTChar(std::wstring str) {
		size_t length = str.length();
		TCHAR* buff = new TCHAR[length + 1];
		lstrcpyn(buff, str.c_str(), (int)length + 1);
		return buff;
	}

	std::string wstringAsString(std::wstring str) {
		std::wstring variableValueWString(str);
		std::string variableValueString;
		variableValueString.reserve(variableValueWString.size());
		for (wchar_t wc : variableValueWString) {
			variableValueString.push_back(static_cast<char>(wc));
		}
		return variableValueString;
	}

	std::wstring getEnvVariableValue(std::wstring variableName) {
		std::string variableNameString = wstringAsString(variableName);

		char* variableValue = std::getenv(variableNameString.c_str());
		if (variableValue == NULL)
			return std::wstring();

		std::string variableValueString(variableValue);
		std::wstring variableValueWString(variableValueString.begin(), variableValueString.end());
		return variableValueWString;
	}

	std::wstring replaceExtension(std::wstring fileName, std::wstring targetExtension) {
		std::wstring fileNameWithErasedExtension = removeExtension(fileName);
		fileNameWithErasedExtension.append(targetExtension);
		return fileNameWithErasedExtension;
	}

	std::wstring removeExtension(std::wstring fileName) {
		size_t indexOfExtensionSeparator = fileName.find_last_of(FILE_NAME_EXTENSION_SEPARATOR, std::wstring::npos);

		if (indexOfExtensionSeparator != std::wstring::npos) {
			std::wstring fileNameWithErasedExtension = fileName;
			fileNameWithErasedExtension.erase(indexOfExtensionSeparator);
			return fileNameWithErasedExtension;
		}
		else
			return fileName;
	}

	bool hasFileExtension(std::wstring fileName, std::wstring expectedExtension) {
		std::wstring extenion = getFileExtension(fileName);

		return expectedExtension == extenion;
	}

	std::wstring getFileExtension(std::wstring fileName) {
		size_t indexOfExtensionSeparator = fileName.find_last_of(FILE_NAME_EXTENSION_SEPARATOR, std::wstring::npos);

		if (indexOfExtensionSeparator != std::wstring::npos) {
			return fileName.substr(indexOfExtensionSeparator);
		}
		else
			return std::wstring();
	}

	bool fileExists(std::wstring filePath)
	{
		return PathFileExists(filePath.c_str());
	}

	std::wstring getPluginVersion(HANDLE hModule) {
		TCHAR moduleFileName[MAX_PATH + 1];
		if (GetModuleFileName(static_cast<HMODULE>(hModule), moduleFileName, MAX_PATH) == 0)
			return std::wstring();

		DWORD fileVersionInfoSize = GetFileVersionInfoSize(moduleFileName, NULL);
		if (fileVersionInfoSize == 0)
			return std::wstring();

		BYTE* fileVersionInfoData = new BYTE[fileVersionInfoSize];
		if (!GetFileVersionInfo(moduleFileName, NULL, fileVersionInfoSize, fileVersionInfoData)) {
			delete[] fileVersionInfoData;
			return std::wstring();
		}

		UINT fileVersionInfoUILength = 0;
		VS_FIXEDFILEINFO* fileVersionInfo = nullptr;

		if (VerQueryValue(fileVersionInfoData, _T("\\"), reinterpret_cast<void**>(&fileVersionInfo), &fileVersionInfoUILength) == 0) {
			delete[] fileVersionInfoData;
			return std::wstring();
		}

		std::wstring versionString = std::wstring()
			.append(std::to_wstring(HIWORD(fileVersionInfo->dwProductVersionMS)))
			.append(_T("."))
			.append(std::to_wstring(LOWORD(fileVersionInfo->dwProductVersionMS)))
			.append(_T("."))
			.append(std::to_wstring(HIWORD(fileVersionInfo->dwProductVersionLS)));

		delete[] fileVersionInfoData;

		return versionString;
	}

	std::wstring getPluginDirectory(HANDLE hModule) {
		TCHAR moduleFileName[MAX_PATH + 1];
		if (GetModuleFileName(static_cast<HMODULE>(hModule), moduleFileName, MAX_PATH) == 0)
			return std::wstring();

		std::wstring modulePath(moduleFileName);
		size_t lastSlash = modulePath.find_last_of(L"\\/");
		if (lastSlash != std::wstring::npos) {
			return modulePath.substr(0, lastSlash);
		}
		return std::wstring();
	}

	std::wstring findCfrJarFile(std::wstring directory) {
		std::wstring searchPattern = directory + L"\\cfr*.jar";
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFile(searchPattern.c_str(), &findData);

		if (hFind == INVALID_HANDLE_VALUE) {
			return std::wstring();
		}

		std::wstring result;
		do {
			std::wstring fileName = findData.cFileName;
			
			// Skip directories, only look for files
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				result = directory + L"\\" + fileName;
				break;
			}
		} while (FindNextFile(hFind, &findData) != 0);

		FindClose(hFind);
		return result;
	}

	std::wstring findJavaFileInDirectory(std::wstring directory, std::wstring baseFileName) {
		std::wstring searchPattern = directory + L"\\*";
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFile(searchPattern.c_str(), &findData);

		if (hFind == INVALID_HANDLE_VALUE) {
			return std::wstring();
		}

		std::wstring targetFileName = baseFileName + L".java";
		std::wstring result;

		do {
			std::wstring fileName = findData.cFileName;
			
			// Skip . and ..
			if (fileName == L"." || fileName == L"..") {
				continue;
			}

			std::wstring fullPath = directory + L"\\" + fileName;

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Recursively search subdirectories
				result = findJavaFileInDirectory(fullPath, baseFileName);
				if (!result.empty()) {
					break;
				}
			}
			else if (fileName == targetFileName) {
				// Found the target file
				result = fullPath;
				break;
			}
		} while (FindNextFile(hFind, &findData) != 0);

		FindClose(hFind);
		return result;
	}}