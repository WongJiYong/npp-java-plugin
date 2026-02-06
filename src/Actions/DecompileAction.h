#pragma once

#include <memory>

#include "PluginInfrastructure/NppPlugin.h"
#include "Components/ConsoleLogger.h"
#include "Command/LoggingCommand.h"
#include "Components/NppGateway.h"
#include "Configuration/ConfigurationProvider.h"
#include "Action.h"

namespace NppJavaPlugin {

	class DecompileAction
	{
	private:
		HANDLE hModule;
		std::shared_ptr<ConfigurationProvider> configurationProvider;
		std::shared_ptr<ConsoleLogger> consoleLogger;
		std::shared_ptr<NppGateway> nppGateway;
		std::shared_ptr<LoggingCommandExecutor> commandExecutor;

		std::wstring BuildDecompileCommand(DocumentInfo currentDocumentInfo, std::wstring outputDir);
		std::wstring GetCfrJarPath();
		bool Validate(DocumentInfo currentDocumentInfo);
	public:
		DecompileAction(
			HANDLE hModule,
			std::shared_ptr<ConfigurationProvider> configurationProvider,
			std::shared_ptr<ConsoleLogger> consoleLogger,
			std::shared_ptr<NppGateway> nppGateway,
			std::shared_ptr<LoggingCommandExecutor> commandExecutor)
			: hModule(hModule),
			configurationProvider(configurationProvider),
			consoleLogger(consoleLogger),
			nppGateway(nppGateway),
			commandExecutor(commandExecutor) {}

		void Initialize();

		void Execute();
	};

	class DecompileCommandCompletionHandler : public TimeoutableCommandCompletionHandler {
	private:
		std::shared_ptr<ConsoleLogger> consoleLogger;
		DocumentInfo currentDocumentInfo;
		std::wstring outputDir;
		std::wstring baseFileName;
		NppData nppData;
	public:
		DecompileCommandCompletionHandler(
			std::shared_ptr<ConsoleLogger> consoleLogger,
			DocumentInfo currentDocumentInfo,
			std::wstring outputDir,
			std::wstring baseFileName,
			NppData nppData)
			: consoleLogger(consoleLogger),
			currentDocumentInfo(currentDocumentInfo),
			outputDir(outputDir),
			baseFileName(baseFileName),
			nppData(nppData) {}

		void onSuccess();
		void onFailure(int exitCode);
		void onTimeout();
	};
}
