#include "Network.h"
#include "BSDSocket.h"
#include "TestPackets.h"

using namespace protocol;

void test_bsd_socket_send_and_receive_ipv4()
{
    printf( "test_bsd_socket_send_and_receive_ipv4\n" );

    BSDSocketConfig config;

    TestPacketFactory packetFactory;

    config.port = 10000;
    config.family = AF_INET;
    config.maxPacketSize = 1024;
    config.packetFactory = &packetFactory;

    BSDSocket interface( config );

    Address address( "127.0.0.1" );
    address.SetPort( config.port );

    TimeBase timeBase;
    timeBase.deltaTime = 0.01f;

    std::chrono::milliseconds ms( (int) ( timeBase.deltaTime * 1000 ) );

    bool receivedConnectPacket = false;
    bool receivedUpdatePacket = false;
    bool receivedDisconnectPacket = false;

    int iterations = 0;

    ConnectPacket connectPacketTemplate;
    UpdatePacket updatePacketTemplate;
    DisconnectPacket disconnectPacketTemplate;

    connectPacketTemplate.a = 2;
    connectPacketTemplate.b = 6;
    connectPacketTemplate.c = -1;

    updatePacketTemplate.timestamp = 500;

    disconnectPacketTemplate.x = -100;

    while ( true )
    {
        assert ( iterations++ < 10 );

        auto connectPacket = new ConnectPacket();
        auto updatePacket = new UpdatePacket();
        auto disconnectPacket = new DisconnectPacket();

        *connectPacket = connectPacketTemplate;
        *updatePacket = updatePacketTemplate;
        *disconnectPacket = disconnectPacketTemplate;

        interface.SendPacket( address, connectPacket );
        interface.SendPacket( address, updatePacket );
        interface.SendPacket( address, disconnectPacket );

        interface.Update( timeBase );

        std::this_thread::sleep_for( ms );

        while ( true )
        {
            auto packet = interface.ReceivePacket();
            if ( !packet )
                break;

            assert( packet->GetAddress() == address );

            switch ( packet->GetType() )
            {
                case PACKET_CONNECT:
                {
                    auto recv_connectPacket = static_cast<ConnectPacket*>( packet );
                    assert( *recv_connectPacket == connectPacketTemplate );
                    receivedConnectPacket = true;
                }
                break;

                case PACKET_UPDATE:
                {
                    auto recv_updatePacket = static_cast<UpdatePacket*>( packet );
                    assert( *recv_updatePacket == updatePacketTemplate );
                    receivedUpdatePacket = true;
                }
                break;

                case PACKET_DISCONNECT:
                {
                    auto recv_disconnectPacket = static_cast<DisconnectPacket*>( packet );
                    assert( *recv_disconnectPacket == disconnectPacketTemplate );
                    receivedDisconnectPacket = true;
                }
                break;
            }

            delete packet;
        }

        if ( receivedConnectPacket && receivedUpdatePacket && receivedDisconnectPacket )
            break;

        timeBase.time += timeBase.deltaTime;
    }
}

void test_bsd_socket_send_and_receive_ipv6()
{
    printf( "test_bsd_socket_send_and_receive_ipv6\n" );

    BSDSocketConfig config;

    TestPacketFactory packetFactory;

    config.port = 10000;
    config.family = AF_INET6;
    config.maxPacketSize = 1024;
    config.packetFactory = &packetFactory;

    BSDSocket interface( config );

    Address address( "::1" );
    address.SetPort( config.port );

    TimeBase timeBase;
    timeBase.deltaTime = 0.01f;

    std::chrono::milliseconds ms( (int) ( timeBase.deltaTime * 1000 ) );

    bool receivedConnectPacket = false;
    bool receivedUpdatePacket = false;
    bool receivedDisconnectPacket = false;

    ConnectPacket connectPacketTemplate;
    UpdatePacket updatePacketTemplate;
    DisconnectPacket disconnectPacketTemplate;

    connectPacketTemplate.a = 2;
    connectPacketTemplate.b = 6;
    connectPacketTemplate.c = -1;

    updatePacketTemplate.timestamp = 500;

    disconnectPacketTemplate.x = -100;

    int iterations = 0;

    while ( true )
    {
        assert ( iterations++ < 10 );

        auto connectPacket = new ConnectPacket();
        auto updatePacket = new UpdatePacket();
        auto disconnectPacket = new DisconnectPacket();

        *connectPacket = connectPacketTemplate;
        *updatePacket = updatePacketTemplate;
        *disconnectPacket = disconnectPacketTemplate;

        interface.SendPacket( address, connectPacket );
        interface.SendPacket( address, updatePacket );
        interface.SendPacket( address, disconnectPacket );

        interface.Update( timeBase );

        std::this_thread::sleep_for( ms );

        while ( true )
        {
            auto packet = interface.ReceivePacket();
            if ( !packet )
                break;

            assert( packet->GetAddress() == address );

            switch ( packet->GetType() )
            {
                case PACKET_CONNECT:
                {
                    auto recv_connectPacket = static_cast<ConnectPacket*>( packet );
                    assert( *recv_connectPacket == connectPacketTemplate );
                    receivedConnectPacket = true;
                }
                break;

                case PACKET_UPDATE:
                {
                    auto recv_updatePacket = static_cast<UpdatePacket*>( packet );
                    assert( *recv_updatePacket == updatePacketTemplate );
                    receivedUpdatePacket = true;
                }
                break;

                case PACKET_DISCONNECT:
                {
                    auto recv_disconnectPacket = static_cast<DisconnectPacket*>( packet );
                    assert( *recv_disconnectPacket == disconnectPacketTemplate );
                    receivedDisconnectPacket = true;
                }
                break;
            }

            delete packet;
        }

        if ( receivedConnectPacket && receivedUpdatePacket && receivedDisconnectPacket )
            break;

        timeBase.time += timeBase.deltaTime;
    }
}

void test_bsd_socket_send_and_receive_multiple_ipv4()
{
    printf( "test_bsd_socket_send_and_receive_multiple_ipv4\n" );

    TestPacketFactory packetFactory;

    BSDSocketConfig sender_config;
    sender_config.port = 10000;
    sender_config.family = AF_INET;
    sender_config.maxPacketSize = 1024;
    sender_config.packetFactory = &packetFactory;

    BSDSocket interface_sender( sender_config );
    
    BSDSocketConfig receiver_config;
    receiver_config.port = 10001;
    receiver_config.family = AF_INET;
    receiver_config.maxPacketSize = 1024;
    receiver_config.packetFactory = &packetFactory;

    BSDSocket interface_receiver( receiver_config );

    Address sender_address( "[127.0.0.1]:10000" );
    Address receiver_address( "[127.0.0.1]:10001" );

    TimeBase timeBase;
    timeBase.deltaTime = 0.01f;

    std::chrono::milliseconds ms( (int) ( timeBase.deltaTime * 1000 ) );

    bool receivedConnectPacket = false;
    bool receivedUpdatePacket = false;
    bool receivedDisconnectPacket = false;

    ConnectPacket connectPacketTemplate;
    UpdatePacket updatePacketTemplate;
    DisconnectPacket disconnectPacketTemplate;

    connectPacketTemplate.a = 2;
    connectPacketTemplate.b = 6;
    connectPacketTemplate.c = -1;

    updatePacketTemplate.timestamp = 500;

    disconnectPacketTemplate.x = -100;

    int iterations = 0;

    while ( true )
    {
        assert ( iterations++ < 4 );

        auto connectPacket = new ConnectPacket();
        auto updatePacket = new UpdatePacket();
        auto disconnectPacket = new DisconnectPacket();

        *connectPacket = connectPacketTemplate;
        *updatePacket = updatePacketTemplate;
        *disconnectPacket = disconnectPacketTemplate;

        interface_sender.SendPacket( receiver_address, connectPacket );
        interface_sender.SendPacket( receiver_address, updatePacket );
        interface_sender.SendPacket( receiver_address, disconnectPacket );

        interface_sender.Update( timeBase );
        interface_receiver.Update( timeBase );

        std::this_thread::sleep_for( ms );

        while ( true )
        {
            auto packet = interface_receiver.ReceivePacket();
            if ( !packet )
                break;

            assert( packet->GetAddress() == sender_address );

            switch ( packet->GetType() )
            {
                case PACKET_CONNECT:
                {
                    auto recv_connectPacket = static_cast<ConnectPacket*>( packet );
                    assert( *recv_connectPacket == connectPacketTemplate );
                    receivedConnectPacket = true;
                }
                break;

                case PACKET_UPDATE:
                {
                    auto recv_updatePacket = static_cast<UpdatePacket*>( packet );
                    assert( *recv_updatePacket == updatePacketTemplate );
                    receivedUpdatePacket = true;
                }
                break;

                case PACKET_DISCONNECT:
                {
                    auto recv_disconnectPacket = static_cast<DisconnectPacket*>( packet );
                    assert( *recv_disconnectPacket == disconnectPacketTemplate );
                    receivedDisconnectPacket = true;
                }
                break;
            }

            delete packet;
        }

        if ( receivedConnectPacket && receivedUpdatePacket && receivedDisconnectPacket )
            break;

        timeBase.time += timeBase.deltaTime;
    }
}

void test_bsd_socket_send_and_receive_multiple_ipv6()
{
    printf( "test_bsd_socket_send_and_receive_multiple_ipv6\n" );

    TestPacketFactory packetFactory;

    BSDSocketConfig sender_config;
    sender_config.port = 10000;
    sender_config.family = AF_INET6;
    sender_config.maxPacketSize = 1024;
    sender_config.packetFactory = &packetFactory;

    BSDSocket interface_sender( sender_config );
    
    BSDSocketConfig receiver_config;
    receiver_config.port = 10001;
    receiver_config.family = AF_INET6;
    receiver_config.maxPacketSize = 1024;
    receiver_config.packetFactory = &packetFactory;

    BSDSocket interface_receiver( receiver_config );

    Address sender_address( "[::1]:10000" );
    Address receiver_address( "[::1]:10001" );

    TimeBase timeBase;
    timeBase.deltaTime = 0.01f;

    std::chrono::milliseconds ms( (int) ( timeBase.deltaTime * 1000 ) );

    bool receivedConnectPacket = false;
    bool receivedUpdatePacket = false;
    bool receivedDisconnectPacket = false;

    ConnectPacket connectPacketTemplate;
    UpdatePacket updatePacketTemplate;
    DisconnectPacket disconnectPacketTemplate;

    connectPacketTemplate.a = 2;
    connectPacketTemplate.b = 6;
    connectPacketTemplate.c = -1;

    updatePacketTemplate.timestamp = 500;

    disconnectPacketTemplate.x = -100;

    int iterations = 0;

    while ( true )
    {
        assert ( iterations++ < 4 );

        auto connectPacket = new ConnectPacket();
        auto updatePacket = new UpdatePacket();
        auto disconnectPacket = new DisconnectPacket();

        *connectPacket = connectPacketTemplate;
        *updatePacket = updatePacketTemplate;
        *disconnectPacket = disconnectPacketTemplate;

        interface_sender.SendPacket( receiver_address, connectPacket );
        interface_sender.SendPacket( receiver_address, updatePacket );
        interface_sender.SendPacket( receiver_address, disconnectPacket );

        interface_sender.Update( timeBase );
        interface_receiver.Update( timeBase );

        std::this_thread::sleep_for( ms );

        while ( true )
        {
            auto packet = interface_receiver.ReceivePacket();
            if ( !packet )
                break;

            assert( packet->GetAddress() == sender_address );

            switch ( packet->GetType() )
            {
                case PACKET_CONNECT:
                {
                    auto recv_connectPacket = static_cast<ConnectPacket*>( packet );
                    assert( *recv_connectPacket == connectPacketTemplate );
                    receivedConnectPacket = true;
                }
                break;

                case PACKET_UPDATE:
                {
                    auto recv_updatePacket = static_cast<UpdatePacket*>( packet );
                    assert( *recv_updatePacket == updatePacketTemplate );
                    receivedUpdatePacket = true;
                }
                break;

                case PACKET_DISCONNECT:
                {
                    auto recv_disconnectPacket = static_cast<DisconnectPacket*>( packet );
                    assert( *recv_disconnectPacket == disconnectPacketTemplate );
                    receivedDisconnectPacket = true;
                }
                break;
            }

            delete packet;
        }

        if ( receivedConnectPacket && receivedUpdatePacket && receivedDisconnectPacket )
            break;

        timeBase.time += timeBase.deltaTime;
    }
}