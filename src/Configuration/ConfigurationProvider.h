#pragma once

#include <memory>
#include <windows.h>

#include "Configuration.h"

namespace NppJavaPlugin {

	class ConfigurationProvider {
	private:
		std::shared_ptr<Configuration> configuration;
	public:
		ConfigurationProvider(HANDLE hModule) : configuration(std::make_shared<Configuration>(hModule)) {}

		std::shared_ptr<Configuration> getConfiguration();
	};
}
