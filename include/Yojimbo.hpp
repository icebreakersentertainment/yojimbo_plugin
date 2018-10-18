#ifndef YOJIMBO_H_
#define YOJIMBO_H_

#include <string>

#include <yojimbo.h>

#include "networking/INetworkingEngine.hpp"

#include "handles/HandleVector.hpp"
#include "utilities/Properties.hpp"
#include "fs/IFileSystem.hpp"
#include "logger/ILogger.hpp"

namespace ice_engine
{
namespace networking
{
namespace yojimbo
{

namespace yojimbo = ::yojimbo;

inline int GetNumBitsForMessage( uint16_t sequence )
{
    static int messageBitsArray[] = { 1, 320, 120, 4, 256, 45, 11, 13, 101, 100, 84, 95, 203, 2, 3, 8, 512, 5, 3, 7, 50 };
    const int modulus = sizeof( messageBitsArray ) / sizeof( int );
    const int index = sequence % modulus;
    return messageBitsArray[index];
}

struct TestMessage : public ::yojimbo::Message
{
    uint16_t sequence;

    TestMessage()
    {
        sequence = 0;
    }

    template <typename Stream>
    bool Serialize( Stream & stream )
    {        
        serialize_bits( stream, sequence, 16 );

        int numBits = GetNumBitsForMessage( sequence );
        int numWords = numBits / 32;
        uint32_t dummy = 0;
        for ( int i = 0; i < numWords; ++i )
            serialize_bits( stream, dummy, 32 );
        int numRemainderBits = numBits - numWords * 32;
        if ( numRemainderBits > 0 )
            serialize_bits( stream, dummy, numRemainderBits );

        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct DefaultBlockMessage : public ::yojimbo::BlockMessage
{
    uint16_t sequence;

    DefaultBlockMessage()
    {
        sequence = 0;
    }

    template <typename Stream>
    bool Serialize( Stream & stream )
    {        
        serialize_bits( stream, sequence, 16 );
        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

enum MessageType
{
    DEFAULT_BLOCK_MESSAGE,
    TEST_MESSAGE,
    NUM_MESSAGE_TYPES
};

YOJIMBO_MESSAGE_FACTORY_START(NetworkingMessageFactory, NUM_MESSAGE_TYPES);
	YOJIMBO_DECLARE_MESSAGE_TYPE( DEFAULT_BLOCK_MESSAGE, DefaultBlockMessage );
	YOJIMBO_DECLARE_MESSAGE_TYPE( TEST_MESSAGE,  TestMessage );
YOJIMBO_MESSAGE_FACTORY_FINISH();

class NetworkingAdapter : public ::yojimbo::Adapter
{
public:

    ::yojimbo::MessageFactory* CreateMessageFactory(::yojimbo::Allocator& allocator)
    {
        return YOJIMBO_NEW(allocator, NetworkingMessageFactory, allocator);
    }
};

struct Server
{
	template <typename ... Args>
	Server(Args&& ... args) : server(std::forward<Args>(args) ...) {}
	
	::yojimbo::Server server;
	handles::HandleVector<std::pair<uint64, int32>, RemoteConnectionHandle> remoteConnections_;
};

struct Client
{
	template <typename ... Args>
	Client(Args&& ... args) : client(std::forward<Args>(args) ...) {}
	
	::yojimbo::Client client;
	bool connected = false;
	bool connectFailed = false;
	handles::HandleVector<std::pair<uint64, int32>, RemoteConnectionHandle> remoteConnections_;
};

class Yojimbo : public INetworkingEngine
{
public:
	Yojimbo(utilities::Properties* properties, fs::IFileSystem* fileSystem, logger::ILogger* logger);
	virtual ~Yojimbo() override;

	Yojimbo(const Yojimbo& other) = delete;
	
	virtual ServerHandle createServer() override;
	virtual ClientHandle createClient() override;
	
	virtual void destroyServer(const ServerHandle& serverHandle) override;
	virtual void destroyClient(const ClientHandle& clientHandle) override;
	
	virtual void tick(const float32 delta) override;
	
	virtual void send(const ServerHandle& serverHandle, const std::vector<uint8>& data) override;
	virtual void send(const ServerHandle& serverHandle, const RemoteConnectionHandle& remoteConnectionHandle, const std::vector<uint8>& data) override;
	
	virtual void send(const ClientHandle& clientHandle, const std::vector<uint8>& data) override;
	
	virtual void processEvents() override;
	virtual void addEventListener(IEventListener* eventListener) override;
	virtual void removeEventListener(IEventListener* eventListener) override;

private:
	std::vector<IEventListener*> eventListeners_;
	handles::HandleVector<Server, ServerHandle> servers_;
	handles::HandleVector<Client, ClientHandle> clients_;
	
	NetworkingAdapter adapter_;
	
	utilities::Properties* properties_;
	fs::IFileSystem* fileSystem_;
	logger::ILogger* logger_;
};

}
}
}

#endif /* YOJIMBO_H_ */
