#include "Yojimbo.hpp"

namespace ice_engine
{
namespace networking
{
namespace yojimbo
{

const int MaxPacketSize = 16 * 1024;
const int MaxSnapshotSize = 8 * 1024;
const int MaxBlockSize = 64 * 1024;

static const int UNRELIABLE_UNORDERED_CHANNEL = 0;
static const int RELIABLE_ORDERED_CHANNEL = 1;

double time_ = 100.0;
Yojimbo::Yojimbo(utilities::Properties* properties, fs::IFileSystem* fileSystem, logger::ILogger* logger)
	:
	properties_(properties),
	fileSystem_(fileSystem),
	logger_(logger)
{
	if (!InitializeYojimbo())
	{
		auto message = std::string("Unable to initialize Yojimbo");
		throw std::runtime_error(message);
	}
	
	yojimbo_log_level( YOJIMBO_LOG_LEVEL_INFO );
}

Yojimbo::~Yojimbo()
{
	for (auto& client : clients_)
	{
		client.client.Disconnect();
	}
	for (auto& server : servers_)
	{
		server.server.Stop();
	}
	
	ShutdownYojimbo();
}

ServerHandle Yojimbo::createServer()
{
	::yojimbo::ClientServerConfig config;
    config.maxPacketSize = MaxPacketSize;
    config.clientMemory = 10 * 1024 * 1024;
    config.serverGlobalMemory = 10 * 1024 * 1024;
    config.serverPerClientMemory = 10 * 1024 * 1024;
    config.numChannels = 2;
    config.channel[UNRELIABLE_UNORDERED_CHANNEL].type = ::yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
    config.channel[UNRELIABLE_UNORDERED_CHANNEL].maxBlockSize = MaxSnapshotSize;
    config.channel[RELIABLE_ORDERED_CHANNEL].type = ::yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
    config.channel[RELIABLE_ORDERED_CHANNEL].maxBlockSize = MaxBlockSize;
	config.channel[RELIABLE_ORDERED_CHANNEL].blockFragmentSize = 1024;

    uint8 privateKey[::yojimbo::KeyBytes];
    memset(privateKey, 0, ::yojimbo::KeyBytes);
	
	auto handle = servers_.create(::yojimbo::GetDefaultAllocator(), privateKey, ::yojimbo::Address("127.0.0.1", 40000), config, adapter_, time_);
	auto& server = servers_[handle];
	
    server.server.Start(8);

    char addressString[::yojimbo::MaxAddressLength];
    server.server.GetAddress().ToString(addressString, sizeof(addressString));
	LOG_INFO(logger_, "Server address is " + std::string(addressString));
	
	return ServerHandle();
}

ClientHandle Yojimbo::createClient()
{
	uint64_t clientId = 0;
    ::yojimbo::random_bytes((uint8*) &clientId, 8);
    LOG_INFO(logger_, "Client id is " + std::to_string(clientId));

    ::yojimbo::ClientServerConfig config;
    config.maxPacketSize = MaxPacketSize;
    config.clientMemory = 10 * 1024 * 1024;
    config.serverGlobalMemory = 10 * 1024 * 1024;
    config.serverPerClientMemory = 10 * 1024 * 1024;
    config.numChannels = 2;
    config.channel[UNRELIABLE_UNORDERED_CHANNEL].type = ::yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
    config.channel[UNRELIABLE_UNORDERED_CHANNEL].maxBlockSize = MaxSnapshotSize;
    config.channel[RELIABLE_ORDERED_CHANNEL].type = ::yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
    config.channel[RELIABLE_ORDERED_CHANNEL].maxBlockSize = MaxBlockSize;
	config.channel[RELIABLE_ORDERED_CHANNEL].blockFragmentSize = 1024;

    auto handle = clients_.create(::yojimbo::GetDefaultAllocator(), ::yojimbo::Address("0.0.0.0"), config, adapter_, time_);
	auto& client = clients_[handle];

    ::yojimbo::Address serverAddress("127.0.0.1", 40000);
    
    uint8 privateKey[::yojimbo::KeyBytes];
    memset(privateKey, 0, ::yojimbo::KeyBytes);

    client.client.InsecureConnect(privateKey, clientId, serverAddress);

    char addressString[256];
    client.client.GetAddress().ToString(addressString, sizeof(addressString));
	LOG_INFO(logger_, "Client address is " + std::string(addressString));
	
	return handle;
}

void Yojimbo::destroyServer(const ServerHandle& serverHandle)
{
	
}

void Yojimbo::destroyClient(const ClientHandle& clientHandle)
{
	
}

uint16 numMessagesSentToServer = 0;
void send(::yojimbo::Client& client, const std::vector<uint8>& data)
{
	if (!client.IsConnected())
	{
		auto message = std::string("Client is not connected");
		throw std::runtime_error(message);
	}

	if (!client.CanSendMessage(RELIABLE_ORDERED_CHANNEL))
	{
		auto message = std::string("Client cannot send");
		throw std::runtime_error(message);
	}
	
	DefaultBlockMessage* message = static_cast<DefaultBlockMessage*>( client.CreateMessage(DEFAULT_BLOCK_MESSAGE) );
	std::cout << "SENDING 1" << std::endl;
	if (message)
	{
		message->sequence = (uint16_t) numMessagesSentToServer;
		//const int blockSize = 1 + ( int( numMessagesSentToServer ) * 33 ) % MaxBlockSize;
		std::cout << "SENDING 2: " << message->sequence << ", " << data.size() << std::endl;
		uint8* blockData = client.AllocateBlock(data.size());
		std::cout << "SENDING 3" << std::endl;
		if (blockData)
		{
			std::copy(data.begin(), data.end(), blockData);
			
			client.AttachBlockToMessage(message, blockData, data.size());
			std::cout << "SENDING" << std::endl;
			client.SendMessage(RELIABLE_ORDERED_CHANNEL, message);
			numMessagesSentToServer++;
		}
		else
		{
			client.ReleaseMessage(message);
			auto message = std::string("Could not create block");
			throw std::runtime_error(message);
		}
	}
	else
	{
		auto message = std::string("Could not create message");
		throw std::runtime_error(message);
	}
}

uint16 numMessagesSentToClient = 0;
void send(::yojimbo::Server& server, const std::vector<uint8>& data)
{
	if (!server.IsRunning())
	{
		auto message = std::string("Server is not running");
		throw std::runtime_error(message);
	}

	for (int i=0; i < server.GetMaxClients(); ++i)
	{
		if (server.IsClientConnected(i))
		{
			if (!server.CanSendMessage(i, RELIABLE_ORDERED_CHANNEL)) break;
			
			DefaultBlockMessage* message = static_cast<DefaultBlockMessage*>( server.CreateMessage(i, DEFAULT_BLOCK_MESSAGE) );
			if (message)
			{
				message->sequence = (uint16_t) numMessagesSentToClient;
				uint8* blockData = server.AllocateBlock(i, data.size());
				if (blockData)
				{
					std::copy(data.begin(), data.end(), blockData);
					
					server.AttachBlockToMessage(i, message, blockData, data.size());
					server.SendMessage(i, RELIABLE_ORDERED_CHANNEL, message);
					numMessagesSentToClient++;
				}
				else
				{
					server.ReleaseMessage(i, message);
					auto message = std::string("Could not create block");
					throw std::runtime_error(message);
				}
			}
			else
			{
				auto message = std::string("Could not create message");
				throw std::runtime_error(message);
			}
		}
	}
}

std::vector<std::pair<int32, std::vector<uint8>>> receive(::yojimbo::Server& server)
{
	std::vector<std::pair<int32, std::vector<uint8>>> data;
	
	if (!server.IsRunning())
	{
		auto message = std::string("Server is not running");
		throw std::runtime_error(message);
	}

	for (int i=0; i < server.GetMaxClients(); ++i)
	{
		if (server.IsClientConnected(i))
		{
			while (true)
			{
				::yojimbo::Message* message = server.ReceiveMessage(i, RELIABLE_ORDERED_CHANNEL);
				if (!message) break;
				
				switch (message->GetType())
				{
					case DEFAULT_BLOCK_MESSAGE:
						{
							std::pair<int32, std::vector<uint8>> d;
							d.first = i;
							
							DefaultBlockMessage* blockMessage = static_cast<DefaultBlockMessage*>(message);
							
							const int blockSize = blockMessage->GetBlockSize();
							const uint8* blockData = blockMessage->GetBlockData();
							
							d.second = std::vector<uint8>(&blockData[0], &blockData[blockSize]);
							
							server.ReleaseMessage(i, message);
							
							data.push_back(std::move(d));
						}
						break;
					
					default:
						printf("server received message but type was not known: %d\n", message->GetType());
						break;
				}
			}
		}
	}
	
	return data;
}

std::vector<std::vector<uint8>> receive(::yojimbo::Client& client)
{
	std::vector<std::vector<uint8>> data;
	
	if (client.IsDisconnected())
	{
		auto message = std::string("Client is disconnected");
		throw std::runtime_error(message);
	}
	
	while (true)
	{
		
		::yojimbo::Message* message = client.ReceiveMessage(RELIABLE_ORDERED_CHANNEL);
		if (!message) break;
		
		switch (message->GetType())
		{
			case DEFAULT_BLOCK_MESSAGE:
				{
					std::vector<uint8> d;
					
					DefaultBlockMessage* blockMessage = static_cast<DefaultBlockMessage*>(message);
					
					const int blockSize = blockMessage->GetBlockSize();
					const uint8* blockData = blockMessage->GetBlockData();
					
					d = std::vector<uint8>(&blockData[0], &blockData[blockSize]);
					
					client.ReleaseMessage(message);
					
					data.push_back(std::move(d));
				}
				break;
			
			default:
				printf("client received message but type was not known: %d\n", message->GetType());
				break;
		}
	}
	
	return data;
}

void Yojimbo::send(const ServerHandle& serverHandle, const std::vector<uint8>& data)
{
	auto& server = servers_[serverHandle];
	
	networking::yojimbo::send(server.server, data);
}

void Yojimbo::send(const ServerHandle& serverHandle, const RemoteConnectionHandle& remoteConnectionHandle, const std::vector<uint8>& data)
{
	
}

void Yojimbo::send(const ClientHandle& clientHandle, const std::vector<uint8>& data)
{
	auto& client = clients_[clientHandle];
	
	networking::yojimbo::send(client.client, data);
}

void Yojimbo::tick(const float32 delta)
{
	for (auto& server : servers_)
	{
		server.server.SendPackets();
	}
	for (auto& client : clients_)
	{
		client.client.SendPackets();
	}
	
	for (auto& server : servers_)
	{
		server.server.ReceivePackets();
	}
	for (auto& client : clients_)
	{
		client.client.ReceivePackets();
	}

	time_ += 0.01;//delta;
	
	for (auto& client : clients_)
	{
		client.client.AdvanceTime(time_);
	}
	for (auto& server : servers_)
	{
		server.server.AdvanceTime(time_);
	}
}

std::vector<RemoteConnectionHandle> findDisconnectedRemoteConnections(const Server& server)
{
	std::vector<RemoteConnectionHandle> disconnected;
	
	for (auto it = server.remoteConnections_.begin(); it != server.remoteConnections_.end(); it++)
	{
		if (!server.server.IsClientConnected((*it).second))
		{
			disconnected.push_back(it.handle());
		}
		else if (server.server.GetClientId((*it).second) != (*it).first)
		{
			disconnected.push_back(it.handle());
		}
	}
	
	return disconnected;
}

std::vector<int32> findNewRemoteConnections(const Server& server)
{
	std::vector<int32> connected;
	
	for (int32 i=0; i < server.server.GetMaxClients(); ++i)
	{
		if (server.server.IsClientConnected(i))
		{
			bool found = false;
			for (auto it = server.remoteConnections_.begin(); it != server.remoteConnections_.end(); it++)
			{
				if ((*it).second == i)
				{
					found = true;
					break;
				}
			}
			
			if (!found)
			{
				connected.push_back(i);
			}
		}
	}
	
	return connected;
}

void Yojimbo::processEvents()
{
	for (auto it = servers_.begin(); it != servers_.end(); it++)
	{
		auto& server = *it;
		std::vector<RemoteConnectionHandle> disconnected = findDisconnectedRemoteConnections(server);
		
		for (auto& h : disconnected)
		{
			server.remoteConnections_.destroy(h);
			
			DisconnectEvent event;
			event.type = EventType::CLIENTDISCONNECT;
			//event.timestamp = ???;
			event.serverHandle = it.handle();
			//event.clientHandle = ;
			event.remoteConnectionHandle = h;
			
			for ( auto it : eventListeners_ )
			{
				it->processEvent(event);
			}
		}
		
		std::vector<int32> connected = findNewRemoteConnections(server);
		for (uint32 index : connected)
		{
			auto handle = server.remoteConnections_.create(server.server.GetClientId(index), index);
			
			ConnectEvent event;
			event.type = EventType::CLIENTCONNECT;
			//event.timestamp = ???;
			event.serverHandle = it.handle();
			//event.clientHandle = ;
			event.remoteConnectionHandle = handle;
			
			for ( auto it : eventListeners_ )
			{
				it->processEvent(event);
			}
		}
		
		auto data = receive(server.server);
		for (auto& d : data)
		{
			RemoteConnectionHandle remoteConnectionHandle;
			for (auto it = server.remoteConnections_.begin(); it != server.remoteConnections_.end(); it++)
			{
				if ((*it).second == d.first)
				{
					remoteConnectionHandle = it.handle();
					break;
				}
			}
						
			MessageEvent event;
			event.type = EventType::CLIENTMESSAGE;
			//event.timestamp = ???;
			event.serverHandle = it.handle();
			//event.clientHandle = ;
			event.remoteConnectionHandle = remoteConnectionHandle;
			event.message = std::move(d.second);
			
			for ( auto it : eventListeners_ )
			{
				it->processEvent(event);
			}
		}
	}
	
	for (auto it = clients_.begin(); it != clients_.end(); it++)
	{
		auto& client = *it;
		
		if (client.connected && client.client.IsDisconnected())
		{
			client.connected = false;
			client.connectFailed = false;
			
			DisconnectEvent event;
			event.type = EventType::SERVERDISCONNECT;
			//event.timestamp = ???;
			//event.serverHandle = ;
			event.clientHandle = it.handle();
			//auto h = client.remoteConnections_[0]; // ???
			//event.remoteConnectionHandle = h;
			
			for ( auto it : eventListeners_ )
			{
				it->processEvent(event);
			}
		}
		
		if (!client.connected && client.client.IsConnected())
		{
			client.connected = true;
			client.connectFailed = false;
			
			//auto handle = client.remoteConnections_.create(client.client.GetClientId(index), index);
			
			ConnectEvent event;
			event.type = EventType::SERVERCONNECT;
			//event.timestamp = ???;
			//event.serverHandle = ;
			event.clientHandle = it.handle();
			//event.remoteConnectionHandle = handle;
			
			for ( auto it : eventListeners_ )
			{
				it->processEvent(event);
			}
		}
		
		if (!client.connectFailed && client.client.ConnectionFailed())
		{ 
			client.connected = false;
			client.connectFailed = true;
			
			//auto handle = client.remoteConnections_.create(client.client.GetClientId(index), index);
			
			ConnectEvent event;
			event.type = EventType::SERVERCONNECTFAILED;
			//event.timestamp = ???;
			//event.serverHandle = ;
			event.clientHandle = it.handle();
			//event.remoteConnectionHandle = handle;
			
			for ( auto it : eventListeners_ )
			{
				it->processEvent(event);
			}
		}
		
		if (!client.client.IsDisconnected())
		{
			auto data = receive(client.client);
			for (auto& d : data)
			{
				//RemoteConnectionHandle remoteConnectionHandle;
				
				MessageEvent event;
				event.type = EventType::SERVERMESSAGE;
				//event.timestamp = ???;
				//event.serverHandle = ;
				event.clientHandle = it.handle();
				//event.remoteConnectionHandle = remoteConnectionHandle;
				event.message = std::move(d);
				
				for ( auto it : eventListeners_ )
				{
					it->processEvent(event);
				}
			}
		}
	}
}

void Yojimbo::addEventListener(IEventListener* eventListener)
{
	if (eventListener == nullptr)
	{
		throw std::invalid_argument("IEventListener cannot be null.");
	}
	
	eventListeners_.push_back(eventListener);
}

void Yojimbo::removeEventListener(IEventListener* eventListener)
{
	auto it = std::find(eventListeners_.begin(), eventListeners_.end(), eventListener);
	
	if (it != eventListeners_.end())
	{
		eventListeners_.erase( it );
	}
}

}
}
}
