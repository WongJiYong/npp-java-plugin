#pragma once

#include <memory>
#include <windows.h>

#include "Components/ConsoleLogger.h"

namespace NppJavaPlugin {

	class OpenSettingsAction
	{
	private:
		HANDLE hModule;
		std::shared_ptr<ConsoleLogger> consoleLogger;
	public:
		OpenSettingsAction(
			HANDLE hModule,
			std::shared_ptr<ConsoleLogger> consoleLogger)
			: hModule(hModule),
			consoleLogger(consoleLogger) {}

		void Execute();
	};
}
