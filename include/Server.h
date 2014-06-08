/*
    Network Protocol Library
    Copyright (c) 2013-2014 Glenn Fiedler <glenn.fiedler@gmail.com>
*/

#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include "Connection.h"
#include "NetworkInterface.h"
#include "ClientServerPackets.h"

namespace protocol
{
    enum ConnectionRequestDenyReason
    {
        CONNECTION_REQUEST_DENIED_ServerClosed,                 // server is closed. all connection requests are denied.
        CONNECTION_REQUEST_DENIED_ServerFull,                   // server is full. no free slots for a connecting client.
        CONNECTION_REQUEST_DENIED_AlreadyConnected              // client is already connected to the server by address but with different guids.
    };

    enum ServerClientState
    {
        SERVER_CLIENT_Disconnected,                             // client is disconnected. default state.
        SERVER_CLIENT_SendingChallenge,                         // responding with connection challenge waiting for challenge response
        SERVER_CLIENT_SendingServerData,                        // sending server data to client
        SERVER_CLIENT_RequestingClientData,                     // requesting client data
        SERVER_CLIENT_ReceivingClientData,                      // receiving client data
        SERVER_CLIENT_Connected                                 // client is fully connected. connection packets are now exchanged.
    };

    struct ServerClientData
    {
        Address address;                                        // the client address that started this connection.
        double accumulator;                                     // accumulator used to determine when to send next packet.
        double lastPacketTime;                                  // time at which the last valid packet was received from the client. used for timeouts.
        uint64_t clientGuid;                                    // the client guid generated by the client and sent to us via connect request.
        uint64_t serverGuid;                                    // the server guid generated randomly on connection request unique to this client.
        ServerClientState state;                                // the current state of this client slot.
        shared_ptr<Connection> connection;                      // connection object, once in SERVER_CLIENT_Connected state this becomes active.
    };

    struct ServerConfig
    {
        uint64_t protocolId = 42;                               // the protocol id. must be the same for client and server to talk.
        
        int maxClients = 16;                                    // max number of clients supported by this server.

        float connectingSendRate = 10;                          // packets to send per-second while a client slot is connecting.
        float connectedSendRate = 30;                           // packets to send per-second once a client is connected.

        float connectingTimeOut = 5.0f;                         // timeout in seconds while a client is connecting
        float connectedTimeOut = 10.0f;                         // timeout in seconds once a client is connected

        shared_ptr<NetworkInterface> networkInterface;          // network interface used to send and receive packets
        shared_ptr<ChannelStructure> channelStructure;          // defines the connection channel structure
        
        shared_ptr<Block> block;                                // data block sent to clients on connect. must be constant, eg. protocol is a function of this
    };

    class Server
    {
        const ServerConfig m_config;

        TimeBase m_timeBase;

        bool m_open = true;

        vector<ServerClientData> m_clients;

    public:

        Server( const ServerConfig & config )
            : m_config( config )
        {
            assert( m_config.networkInterface );
            assert( m_config.channelStructure );
            assert( m_config.maxClients >= 1 );

//            cout << "creating server with " << m_config.maxClients << " client slots" << endl;

            m_clients.resize( m_config.maxClients );

            ConnectionConfig connectionConfig;
            connectionConfig.packetType = PACKET_Connection;
            connectionConfig.maxPacketSize = m_config.networkInterface->GetMaxPacketSize();
            connectionConfig.channelStructure = m_config.channelStructure;
            connectionConfig.packetFactory = make_shared<ClientServerPacketFactory>( m_config.channelStructure );

            for ( int i = 0; i < m_clients.size(); ++i )
                m_clients[i].connection = make_shared<Connection>( connectionConfig );
        }

        void Open()
        {
            m_open = true;
        }

        void Close()
        {
            m_open = false;
        }

        bool IsOpen() const
        {
            return m_open;
        }

        void Update( const TimeBase & timeBase )
        {
            m_timeBase = timeBase;

            UpdateClients();

            UpdateNetworkInterface();

            UpdateReceivePackets();
        }

        void DisconnectClient( int clientIndex )
        {
            assert( clientIndex >= 0 );
            assert( clientIndex < m_config.maxClients );

            auto & client = m_clients[clientIndex];

            if ( client.state == SERVER_CLIENT_Disconnected )
                return;

//            cout << "sent disconnected packet to client" << endl;

            auto packet = make_shared<DisconnectedPacket>();

            packet->protocolId = m_config.protocolId;
            packet->clientGuid = client.clientGuid;
            packet->serverGuid = client.serverGuid;

            m_config.networkInterface->SendPacket( client.address, packet );

            ResetClientSlot( clientIndex );
        }

        ServerClientState GetClientState( int clientIndex ) const
        {
            assert( clientIndex >= 0 );
            assert( clientIndex < m_config.maxClients );
            return m_clients[clientIndex].state;
        }

        shared_ptr<Connection> GetClientConnection( int clientIndex )
        {
            assert( clientIndex >= 0 );
            assert( clientIndex < m_config.maxClients );
            return m_clients[clientIndex].connection;
        }

    protected:

        void UpdateClients()
        {
            for ( int i = 0; i < m_clients.size(); ++i )
            {
                switch ( m_clients[i].state )
                {
                    case SERVER_CLIENT_SendingChallenge: 
                        UpdateSendingChallenge( i );
                        break;

                    case SERVER_CLIENT_SendingServerData:
                        UpdateSendingServerData( i );
                        break;

                    case SERVER_CLIENT_RequestingClientData:
                        UpdateRequestingClientData( i );
                        break;

                    case SERVER_CLIENT_ReceivingClientData:
                        UpdateReceivingClientData( i );
                        break;

                    case SERVER_CLIENT_Connected:
                        UpdateConnected( i );
                        break;

                    default:
                        break;
                }

                UpdateTimeouts( i );
            }
        }

        void UpdateSendingChallenge( int clientIndex )
        {
            ServerClientData & client = m_clients[clientIndex];

            assert( client.state == SERVER_CLIENT_SendingChallenge );

            if ( client.accumulator > 1.0 / m_config.connectingSendRate )
            {
                auto packet = make_shared<ConnectionChallengePacket>();

                packet->protocolId = m_config.protocolId;
                packet->clientGuid = client.clientGuid;
                packet->serverGuid = client.serverGuid;

                m_config.networkInterface->SendPacket( client.address, packet );

                client.accumulator = 0.0;
            }
        }

        void UpdateSendingServerData( int clientIndex )
        {
            ServerClientData & client = m_clients[clientIndex];

            assert( client.state == SERVER_CLIENT_SendingServerData );

            // todo: not implemented yet
            assert( false );
        }

        void UpdateRequestingClientData( int clientIndex )
        {
            ServerClientData & client = m_clients[clientIndex];

            assert( client.state == SERVER_CLIENT_RequestingClientData );

            if ( client.accumulator > 1.0 / m_config.connectingSendRate )
            {
//                cout << "sent request client data packet" << endl;

                auto packet = make_shared<RequestClientDataPacket>();

                packet->protocolId = m_config.protocolId;
                packet->clientGuid = client.clientGuid;
                packet->serverGuid = client.serverGuid;

                m_config.networkInterface->SendPacket( client.address, packet );

                client.accumulator = 0.0;
            }
        }

        void UpdateReceivingClientData( int clientIndex )
        {
            ServerClientData & client = m_clients[clientIndex];

            assert( client.state == SERVER_CLIENT_ReceivingClientData );

            // todo: not implemented yet
            assert( false );
        }

        void UpdateConnected( int clientIndex )
        {
            ServerClientData & client = m_clients[clientIndex];

            assert( client.state == SERVER_CLIENT_Connected );

            assert( client.connection );

            client.connection->Update( m_timeBase );

            if ( client.accumulator > 1.0 / m_config.connectedSendRate )
            {
                auto packet = client.connection->WritePacket();

//                cout << "server sent connection packet" << endl;

                m_config.networkInterface->SendPacket( client.address, packet );

                client.accumulator = 0.0;
            }
        }

        void UpdateTimeouts( int clientIndex )
        {
            ServerClientData & client = m_clients[clientIndex];

            if ( client.state == SERVER_CLIENT_Disconnected )
                return;

            client.accumulator += m_timeBase.deltaTime;

            const float timeout = client.state == SERVER_CLIENT_Connected ? m_config.connectedTimeOut : m_config.connectingTimeOut;

            if ( client.lastPacketTime + timeout < m_timeBase.time )
            {
//                cout << "client " << clientIndex << " timed out" << endl;

                ResetClientSlot( clientIndex );
            }
        }

        void UpdateNetworkInterface()
        {
            assert( m_config.networkInterface );

            m_config.networkInterface->Update( m_timeBase );
        }

        void UpdateReceivePackets()
        {
            while ( true )
            {
                auto packet = m_config.networkInterface->ReceivePacket();
                if ( !packet )
                    break;

//                cout << "server received packet" << endl;

                switch ( packet->GetType() )
                {
                    case PACKET_ConnectionRequest:
                        ProcessConnectionRequest( static_pointer_cast<ConnectionRequestPacket>( packet ) );
                        break;

                    case PACKET_ChallengeResponse:
                        ProcessChallengeResponse( static_pointer_cast<ChallengeResponsePacket>( packet ) );
                        break;

                    case PACKET_ReadyForConnection:
                        ProcessReadyForConnection( static_pointer_cast<ReadyForConnectionPacket>( packet ) );
                        break;

                    case PACKET_Connection:
                        ProcessConnection( static_pointer_cast<ConnectionPacket>( packet ) );
                        break;

                    default:
                        break;
                }
            }
        }

        void ProcessConnectionRequest( shared_ptr<ConnectionRequestPacket> packet )
        {
//            cout << "server received connection request packet" << endl;

            if ( !m_open )
            {
//                cout << "server is closed. denying connection request" << endl;

                auto connectionDeniedPacket = make_shared<ConnectionDeniedPacket>();
                connectionDeniedPacket->protocolId = m_config.protocolId;
                connectionDeniedPacket->clientGuid = packet->clientGuid;
                connectionDeniedPacket->reason = CONNECTION_REQUEST_DENIED_ServerClosed;

                m_config.networkInterface->SendPacket( packet->GetAddress(), connectionDeniedPacket );

                return;
            }

            auto address = packet->GetAddress();

            if ( FindClientIndex( address, packet->clientGuid ) != -1 )
            {
//                cout << "ignoring connection request. client already has a slot" << endl;
                return;
            }

            int clientIndex = FindClientIndex( address );
            if ( clientIndex != -1 && m_clients[clientIndex].clientGuid != packet->clientGuid )
            {
//                cout << "client is already connected. denying connection request" << endl;
                auto connectionDeniedPacket = make_shared<ConnectionDeniedPacket>();
                connectionDeniedPacket->protocolId = m_config.protocolId;
                connectionDeniedPacket->clientGuid = packet->clientGuid;
                connectionDeniedPacket->reason = CONNECTION_REQUEST_DENIED_AlreadyConnected;
                m_config.networkInterface->SendPacket( address, connectionDeniedPacket );
            }

            clientIndex = FindFreeClientSlot();
            if ( clientIndex == -1 )
            {
//              cout << "server is full. denying connection request" << endl;
                auto connectionDeniedPacket = make_shared<ConnectionDeniedPacket>();
                connectionDeniedPacket->protocolId = m_config.protocolId;
                connectionDeniedPacket->clientGuid = packet->clientGuid;
                connectionDeniedPacket->reason = CONNECTION_REQUEST_DENIED_ServerFull;
                m_config.networkInterface->SendPacket( address, connectionDeniedPacket );
            }

//            cout << "new client connection at index " << clientIndex << endl;

            ServerClientData & client = m_clients[clientIndex];

            client.address = address;
            client.clientGuid = packet->clientGuid;
            client.serverGuid = GenerateGuid();
            client.lastPacketTime = m_timeBase.time;
            client.state = SERVER_CLIENT_SendingChallenge;
        }

        void ProcessChallengeResponse( shared_ptr<ChallengeResponsePacket> packet )
        {
//            cout << "server received challenge response packet" << endl;

            const int clientIndex = FindClientIndex( packet->GetAddress(), packet->clientGuid );
            if ( clientIndex == -1 )
            {
//                cout << "found no matching client" << endl;
                return;
            }

            ServerClientData & client = m_clients[clientIndex];

            if ( client.serverGuid != packet->serverGuid )
            {
//                cout << "client server guid does not match" << endl;
                return;
            }

            if ( client.state != SERVER_CLIENT_SendingChallenge )
            {
//                cout << "ignoring because client slot is not in sending challenge state" << endl;
                return;
            }

            client.accumulator = 0.0;
            client.lastPacketTime = m_timeBase.time;
            client.state = m_config.block ? SERVER_CLIENT_SendingServerData : SERVER_CLIENT_RequestingClientData;
        }

        void ProcessReadyForConnection( shared_ptr<ReadyForConnectionPacket> packet )
        {
//            cout << "server received ready for connection packet" << endl;

            const int clientIndex = FindClientIndex( packet->GetAddress(), packet->clientGuid );
            if ( clientIndex == -1 )
            {
//                cout << "found no matching client" << endl;
                return;
            }

            ServerClientData & client = m_clients[clientIndex];

            if ( client.serverGuid != packet->serverGuid )
            {
//                cout << "client server guid does not match" << endl;
                return;
            }

            // todo: should also transition here from received client data state

            if ( client.state != SERVER_CLIENT_RequestingClientData )
            {
//                cout << "ignoring because client slot is not in correct state" << endl;
                return;
            }

            client.accumulator = 0.0;
            client.lastPacketTime = m_timeBase.time;
            client.state = SERVER_CLIENT_Connected;
        }

        void ProcessConnection( shared_ptr<ConnectionPacket> packet )
        {
//            cout << "server received connection packet" << endl;

            const int clientIndex = FindClientIndex( packet->GetAddress() );
            if ( clientIndex == -1 )
            {
//                cout << "found no matching client" << endl;
                return;
            }

            ServerClientData & client = m_clients[clientIndex];
            if ( client.state != SERVER_CLIENT_Connected )
            {
//                cout << "ignoring because client slot is not in correct state" << endl;
                return;
            }

            client.connection->ReadPacket( packet );

            client.lastPacketTime = m_timeBase.time;
        }

        int FindClientIndex( const Address & address ) const
        {
            for ( int i = 0; i < m_clients.size(); ++i )
            {
                if ( m_clients[i].state == SERVER_CLIENT_Disconnected )
                    continue;
                
                if ( m_clients[i].address == address )
                {
                    assert( m_clients[i].state != SERVER_CLIENT_Disconnected );
                    return i;
                }
            }
            return -1;
        }

        int FindClientIndex( const Address & address, uint64_t clientGuid ) const
        {
            for ( int i = 0; i < m_clients.size(); ++i )
            {
                if ( m_clients[i].state == SERVER_CLIENT_Disconnected )
                    continue;
                
                if ( m_clients[i].address == address && m_clients[i].clientGuid == clientGuid )
                {
                    assert( m_clients[i].state != SERVER_CLIENT_Disconnected );
                    return i;
                }
            }
            return -1;
        }

        int FindFreeClientSlot() const
        {
            for ( int i = 0; i < m_clients.size(); ++i )
            {
                if ( m_clients[i].state == SERVER_CLIENT_Disconnected )
                    return i;
            }
            return -1;
        }

        void ResetClientSlot( int clientIndex )
        {
//          cout << "reset client slot " << clientIndex << endl;

            ServerClientData & client = m_clients[clientIndex];

            client.state = SERVER_CLIENT_Disconnected;
            client.address = Address();
            client.accumulator = 0.0;
            client.lastPacketTime = 0.0;
            client.clientGuid = 0;
            client.serverGuid = 0;

            assert( client.connection );
            client.connection->Reset();
        }
    };
}

#endif
