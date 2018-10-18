#include "YojimboFactory.hpp"

#include "Yojimbo.hpp"

namespace ice_engine
{
namespace networking
{
namespace yojimbo
{

YojimboFactory::YojimboFactory()
{
}

YojimboFactory::~YojimboFactory()
{
}

std::unique_ptr<INetworkingEngine> YojimboFactory::create(
	utilities::Properties* properties,
	fs::IFileSystem* fileSystem,
	logger::ILogger* logger
)
{
	std::unique_ptr<INetworkingEngine> ptr = std::make_unique< Yojimbo >( properties, fileSystem, logger );
	
	return std::move( ptr );
}

}
}
}
