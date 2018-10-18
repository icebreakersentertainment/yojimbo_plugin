#include <boost/config.hpp> // for BOOST_SYMBOL_EXPORT

#include "YojimboPlugin.hpp"

#include "YojimboFactory.hpp"

namespace ice_engine
{

YojimboPlugin::YojimboPlugin()
{
}

YojimboPlugin::~YojimboPlugin()
{
}

std::string YojimboPlugin::getName() const
{
	return std::string("yojimbo");
}

std::unique_ptr<networking::INetworkingEngineFactory> YojimboPlugin::createFactory() const
{
	std::unique_ptr<networking::INetworkingEngineFactory> ptr = std::make_unique< networking::yojimbo::YojimboFactory >();
	
	return std::move( ptr );
}

// Exporting `my_namespace::plugin` variable with alias name `plugin`
// (Has the same effect as `BOOST_DLL_ALIAS(my_namespace::plugin, plugin)`)
extern "C" BOOST_SYMBOL_EXPORT YojimboPlugin plugin;
YojimboPlugin plugin;

}
