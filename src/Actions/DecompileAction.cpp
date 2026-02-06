#include "DecompileAction.h"
#include "Components/Utils.h"
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

namespace NppJavaPlugin {

	constexpr int DECOMPILE_ACTION_TIMEOUT_IN_MILLIS = 60 * 1000;

	void DecompileAction::Initialize()
	{
	}

	std::wstring DecompileAction::GetCfrJarPath() {
		return configurationProvider->getConfiguration()->getCfrJarPath();
	}

	void DecompileAction::Execute()
	{
		DocumentInfo currentDocumentInfo = nppGateway->GetCurrentDocumentInfo();

		consoleLogger->Info(std::wstring().append(_T("Decompiling File \"")).append(currentDocumentInfo.fileName).append(_T("\"...")).c_str());

		if (!Validate(currentDocumentInfo))
			return;

		// Create temp directory for output
		TCHAR tempPath[MAX_PATH];
		GetTempPath(MAX_PATH, tempPath);
		
		// Remove trailing backslash if present
		std::wstring tempDir(tempPath);
		if (!tempDir.empty() && tempDir.back() == L'\\') {
			tempDir.pop_back();
		}
		
		// Get base file name (without extension)
		std::wstring fileNameWithoutExt = removeExtension(currentDocumentInfo.fileName);

		// Build and execute decompile command
		CommandInfo commandInfo(BuildDecompileCommand(currentDocumentInfo, tempDir), currentDocumentInfo.currentDirectory);
		
		// Get nppData from nppGateway
		NppData nppDataCopy = nppGateway->GetNppData();
		
		commandExecutor->Execute(commandInfo, DECOMPILE_ACTION_TIMEOUT_IN_MILLIS, 
			std::make_shared<DecompileCommandCompletionHandler>(consoleLogger, currentDocumentInfo, tempDir, fileNameWithoutExt, nppDataCopy));
	}

	bool DecompileAction::Validate(DocumentInfo currentDocumentInfo) {
		if (currentDocumentInfo.currentDirectory.empty()) {
			consoleLogger->Error(std::wstring().append(_T("Current File \"")).append(currentDocumentInfo.fileName).append(_T("\" is not saved to disk, decompilation will not be performed.")).c_str());
			return false;
		}

		if (!hasFileExtension(currentDocumentInfo.currentPath, _T(".class"))) {
			consoleLogger->Error(std::wstring().append(_T("Current active file \"")).append(currentDocumentInfo.fileName)
				.append(_T("\" does not have \".class\" extension, only \".class\" files are supported, decompilation will not be performed.")).c_str());
			return false;
		}

		// Check if CFR jar exists
		std::wstring cfrJarPath = GetCfrJarPath();
		if (cfrJarPath.empty() || !fileExists(cfrJarPath)) {
			consoleLogger->Error(std::wstring().append(_T("CFR decompiler not found at: ")).append(cfrJarPath).c_str());
			return false;
		}

		return true;
	}

	std::wstring DecompileAction::BuildDecompileCommand(DocumentInfo currentDocumentInfo, std::wstring outputDir) {
		std::wstring decompileCommand;
		std::wstring cfrJarPath = GetCfrJarPath();
		std::wstring javaPath = configurationProvider->getConfiguration()->getJavaPath();

		// Build command: java -jar cfr.jar input.class --outputdir tempdir
		decompileCommand.append(_T("\""));
		decompileCommand.append(javaPath);
		decompileCommand.append(_T("\" -jar \""));
		decompileCommand.append(cfrJarPath);
		decompileCommand.append(_T("\" \""));
		decompileCommand.append(currentDocumentInfo.currentPath);
		decompileCommand.append(_T("\" --outputdir \""));
		decompileCommand.append(outputDir);
		decompileCommand.append(_T("\""));

		return decompileCommand;
	}

	void DecompileCommandCompletionHandler::onSuccess()
	{
		consoleLogger->Success(std::wstring().append(_T("File \"")).append(currentDocumentInfo.fileName).append(_T("\" decompiled successfully.")).c_str());

		// Find the decompiled file (CFR creates it based on package structure)
		std::wstring outputFilePath = findJavaFileInDirectory(outputDir, baseFileName);

		if (!outputFilePath.empty() && fileExists(outputFilePath)) {
			SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)outputFilePath.c_str());
			consoleLogger->Info(std::wstring().append(_T("Opened decompiled file: \"")).append(outputFilePath).append(_T("\"")).c_str());
		}
		else {
			consoleLogger->Error(std::wstring().append(_T("Decompiled file not found in: \"")).append(outputDir).append(_T("\"")).c_str());
		}
	}

	void DecompileCommandCompletionHandler::onFailure(int exitCode)
	{
		consoleLogger->Error(std::wstring().append(_T("Failed to decompile file \"")).append(currentDocumentInfo.fileName)
			.append(_T("\", exit code = ")).append(std::to_wstring(exitCode)).c_str());
	}

	void DecompileCommandCompletionHandler::onTimeout()
	{
		consoleLogger->Error(std::wstring().append(_T("Failed to decompile file \"")).append(currentDocumentInfo.fileName)
			.append(_T("\", decompilation was not finished within timeout = ")).append(std::to_wstring(DECOMPILE_ACTION_TIMEOUT_IN_MILLIS)).c_str());
	}
}
