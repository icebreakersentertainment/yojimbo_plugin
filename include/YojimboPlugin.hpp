#ifndef YOJIMBOPLUGIN_H_
#define YOJIMBOPLUGIN_H_

#include <memory>

#include "INetworkingPlugin.hpp"

namespace ice_engine
{

class YojimboPlugin : public INetworkingPlugin
{
public:
	YojimboPlugin();
	virtual ~YojimboPlugin();

	virtual std::string getName() const override;

	virtual std::unique_ptr<networking::INetworkingEngineFactory> createFactory() const override;

};

}

#endif /* YOJIMBOPLUGIN_H_ */
