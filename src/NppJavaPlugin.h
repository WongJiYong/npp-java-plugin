#pragma once

#include <memory>

#include "PluginInfrastructure/NppPlugin.h"
#include "Components/JavaConsole.h"
#include "Actions/CompileAction.h"
#include "Actions/RunAction.h"
#include "Actions/CompileAndRunAction.h"
#include "Command/LoggingCommand.h"
#include "Components/NppGateway.h"
#include "Configuration/Configuration.h"
#include "Actions/ClearConsoleAction.h"
#include "Actions/ShowHideJavaConsoleAction.h"
#include "Actions/ShowAboutDialogAction.h"
#include "Actions/DecompileAction.h"
#include "Actions/OpenSettingsAction.h"

namespace NppJavaPlugin {

	class NppJavaPlugin : public NppPlugin
	{
	private:
		HANDLE hModule;

		std::shared_ptr<ConfigurationProvider> configurationProvider;
		std::shared_ptr<NppGateway> nppGateway;
		std::shared_ptr<JavaConsole> console;
		std::shared_ptr<ConfigurationValidator> configurationValidator;
		std::shared_ptr<LoggingCommandExecutor> commandExecutor;
		std::shared_ptr<CommandPrompt> commandPrompt;
		std::shared_ptr<CompileAction> compileAction;
		std::shared_ptr<RunAction> runAction;
		std::shared_ptr<CompileAndRunAction> compileAndRunAction;
		std::shared_ptr<ClearConsoleAction> clearConsoleAction;
		std::shared_ptr<ShowHideJavaConsoleAction> showHideJavaConsoleAction;
		std::shared_ptr<AboutDialog> aboutDialog;
		std::shared_ptr<ShowAboutDialogAction> showAboutDialogAction;
		std::shared_ptr<DecompileAction> decompileAction;
		std::shared_ptr<OpenSettingsAction> openSettingsAction;

		void InitializeLibraries();
		void InitializeDependencies(NppData pNppData);
		void InitializeCommands();		

	public:
		NppJavaPlugin(HANDLE hModule) :
			hModule(hModule),
			configurationProvider(std::make_shared<ConfigurationProvider>(hModule)),
			nppGateway(std::make_shared<NppGateway>()),
			console(std::make_shared<JavaConsole>(hModule, nppGateway)),
			configurationValidator(std::make_shared<ConfigurationValidator>(console)),
			commandExecutor(std::make_shared<LoggingCommandExecutor>(console)),
			compileAction(std::make_shared<CompileAction>(configurationProvider, configurationValidator, console, nppGateway, commandExecutor)),
			runAction(std::make_shared<RunAction>(configurationProvider, configurationValidator, console, nppGateway, commandPrompt)),
			compileAndRunAction(std::make_shared<CompileAndRunAction>(compileAction, runAction)),
			clearConsoleAction(std::make_shared<ClearConsoleAction>(console)),
			showHideJavaConsoleAction(std::make_shared<ShowHideJavaConsoleAction>(console)),
			aboutDialog(std::make_shared<AboutDialog>(hModule)),
			showAboutDialogAction(std::make_shared<ShowAboutDialogAction>(aboutDialog)),
			decompileAction(std::make_shared<DecompileAction>(hModule, configurationProvider, console, nppGateway, commandExecutor)),
			openSettingsAction(std::make_shared<OpenSettingsAction>(hModule, console)) {}

		const TCHAR* getName();
		const size_t getNumberOfActions();

		void Initialize(NppData nppData);

		void compileActionExecutor();
		void runActionExecutor();
		void compileAndRunActionExecutor();

		void clearConsoleActionExecutor();
		void showHideJavaConsoleActionExecutor();
		void showAboutDialogActionExecutor();
		void decompileActionExecutor();
		void openSettingsActionExecutor();
	};

	void compileActionAdapter();
	void runActionAdapter();
	void compileAndRunActionAdapter();

	void clearConsoleActionExecutorAdapter();
	void showHideJavaConsoleActionExecutorAdapter();
	void showAboutDialogActionExecutorAdapter();
	void decompileActionAdapter();
	void openSettingsActionAdapter();
}
