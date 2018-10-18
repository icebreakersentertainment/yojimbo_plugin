#ifndef YOJIMBOFACTORY_H_
#define YOJIMBOFACTORY_H_

#include <memory>

#include "networking/INetworkingEngine.hpp"
#include "networking/INetworkingEngineFactory.hpp"

namespace ice_engine
{
namespace networking
{
namespace yojimbo
{

class YojimboFactory : public INetworkingEngineFactory
{
public:
	YojimboFactory();
	virtual ~YojimboFactory();

	virtual std::unique_ptr<INetworkingEngine> create(
		utilities::Properties* properties,
		fs::IFileSystem* fileSystem,
		logger::ILogger* logger
	) override;

};

}
}
}

#endif /* YOJIMBOFACTORY_H_ */
