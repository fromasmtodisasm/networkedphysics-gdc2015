#include "Connection.h"
#include "TestMessages.h"
#include "TestPackets.h"
#include "NetworkSimulator.h"
#include "ReliableMessageChannel.h"

using namespace protocol;

class TestChannelStructure : public ChannelStructure
{
    TestMessageFactory m_messageFactory;
    ReliableMessageChannelConfig m_config;

public:

    TestChannelStructure()
    {
        m_config.maxMessagesPerPacket = 256;
        m_config.sendQueueSize = 2048;
        m_config.receiveQueueSize = 512;
        m_config.packetBudget = 4000;
        m_config.maxMessageSize = 1024;
        m_config.blockFragmentSize = 3900;
        m_config.maxLargeBlockSize = 32 * 1024 * 1024;
        m_config.messageFactory = &m_messageFactory;
        m_config.messageAllocator = &memory::default_allocator();
        m_config.smallBlockAllocator = &memory::default_allocator();
        m_config.largeBlockAllocator = &memory::default_allocator();

        AddChannel( "reliable message channel", 
                    [this] { return CreateReliableMessageChannel(); }, 
                    [this] { return CreateReliableMessageChannelData(); } );

        Lock();
    }

    ReliableMessageChannel * CreateReliableMessageChannel()
    {
        return new ReliableMessageChannel( m_config );
    }

    ReliableMessageChannelData * CreateReliableMessageChannelData()
    {
        return new ReliableMessageChannelData( m_config );
    }

    const ReliableMessageChannelConfig & GetConfig() const
    {
        return m_config;
    }
};

void soak_test()
{
#if PROFILE
    printf( "[profile]\n" );
#else
    printf( "[soak test]\n" );
#endif

    TestChannelStructure channelStructure;

    TestPacketFactory packetFactory( &channelStructure );

    const int MaxPacketSize = 4096;

#if !PROFILE
    NetworkSimulator simulator;
    simulator.AddState( { 0.0f, 0.0f, 0.0f } );
    simulator.AddState( { 0.0f, 0.0f, 0.0f } );
    simulator.AddState( { 0.1f, 0.01f, 1.0f } );
    simulator.AddState( { 0.1f, 0.01f, 1.0f } );
    simulator.AddState( { 0.2f, 0.10f, 5.0f } );
    simulator.AddState( { 0.3f, 0.20f, 10.0f } );
    simulator.AddState( { 0.5f, 0.25f, 25.0f } );
    simulator.AddState( { 1.0f, 0.50f, 50.0f } );
    simulator.AddState( { 1.0f, 1.00f, 100.0f } );
#endif

    Address address( "::1" );

    ConnectionConfig connectionConfig;
    connectionConfig.packetType = PACKET_CONNECTION;
    connectionConfig.maxPacketSize = MaxPacketSize;
    connectionConfig.packetFactory = &packetFactory;
    connectionConfig.slidingWindowSize = 1024;
    connectionConfig.channelStructure = &channelStructure;

    Connection connection( connectionConfig );

    auto messageChannel = static_cast<ReliableMessageChannel*>( connection.GetChannel( 0 ) );

    auto messageChannelConfig = channelStructure.GetConfig(); 
    
    uint16_t sendMessageId = 0;
    uint64_t numMessagesSent = 0;
    uint64_t numMessagesReceived = 0;

    TimeBase timeBase;
    timeBase.time = 0.0;
    timeBase.deltaTime = 0.01;

//    for ( int i = 0; i < 10000; ++i )
    while ( true )
    {
        const int maxMessagesToSend = 1 + rand() % 32;

        for ( int i = 0; i < maxMessagesToSend; ++i )
        {
            if ( !messageChannel->CanSendMessage() )
                break;

            int value = rand() % 10000;

            if ( value < 5000 )
            {
                // bitpacked message
                auto message = new TestMessage();
                assert( message );
//                printf( "allocate message %p (send)\n", message );
                message->sequence = sendMessageId;
                messageChannel->SendMessage( message );
            }
            else if ( value < 9999 )
            {
                // small block
                int index = sendMessageId % 32;
                Block block( memory::default_allocator(), index + 1 );
                uint8_t * data = block.GetData();
                for ( int i = 0; i < block.GetSize(); ++i )
                    data[i] = ( index + i ) % 256;
                messageChannel->SendBlock( block );
            }
            else
            {
                // large block
                int index = sendMessageId % 4;
                Block block( memory::default_allocator(), (index+1) * 1024 * 1000 + index );
                uint8_t * data = block.GetData();
                for ( int i = 0; i < block.GetSize(); ++i )
                    data[i] = ( index + i ) % 256;
                messageChannel->SendBlock( block );
            }
            
            sendMessageId++;
            numMessagesSent++;
        }

//        printf( "before write packet\n" );

        ConnectionPacket * writePacket = connection.WritePacket();

//        printf( "after write packet\n" );

        uint8_t buffer[MaxPacketSize];

        WriteStream writeStream( buffer, MaxPacketSize );
        writePacket->SerializeWrite( writeStream );
        writeStream.Flush();
        assert( !writeStream.IsOverflow() );

        delete writePacket;

//        printf( "deleted writePacket\n" );

//        printf( "before read packet\n" );

        ReadStream readStream( buffer, MaxPacketSize );
        auto readPacket = new ConnectionPacket( PACKET_CONNECTION, &channelStructure );
        readPacket->SerializeRead( readStream );
        assert( !readStream.IsOverflow() );

        connection.ReadPacket( static_cast<ConnectionPacket*>( readPacket ) );

//        printf( "after read packet\n" );

        delete readPacket;

//        printf( "deleted read packet\n" );

        connection.Update( timeBase );

        while ( true )
        {
            auto message = messageChannel->ReceiveMessage();

            if ( !message )
                break;

            assert( message->GetId() == numMessagesReceived % 65536 );
            assert( message->GetType() == MESSAGE_BLOCK || message->GetType() == MESSAGE_TEST );

            if ( message->GetType() == MESSAGE_TEST )
            {
#if !PROFILE
                printf( "%09.2f - received message %d - test message\n", timeBase.time, message->GetId() );
#endif
            }
            else
            {
                auto blockMessage = static_cast<BlockMessage*>( message );
                
                Block & block = blockMessage->GetBlock();                

                if ( block.GetSize() <= messageChannelConfig.maxSmallBlockSize )
                {
                    const int index = numMessagesReceived % 32;
                    assert( block.GetSize() == index + 1 );
                    const uint8_t * data = block.GetData();
                    for ( int i = 0; i < block.GetSize(); ++i )
                        assert( data[i] == ( index + i ) % 256 );
#if !PROFILE
                    printf( "%09.2f - received message %d - small block\n", timeBase.time, message->GetId() );
#endif
                }
                else
                {
                    const int index = numMessagesReceived % 4;
                    assert( block.GetSize() == ( index + 1 ) * 1024 * 1000 + index );
                    const uint8_t * data = block.GetData();
                    for ( int i = 0; i < block.GetSize(); ++i )
                        assert( data[i] == ( index + i ) % 256 );
#if !PROFILE
                    printf( "%09.2f - received message %d - large block\n", timeBase.time, message->GetId() );
#endif
                }
            }

            numMessagesReceived++;

//            printf( "deallocater message %p (received)\n", message );

            delete message;
        }

#if !PROFILE
        auto status = messageChannel->GetReceiveLargeBlockStatus();
        if ( status.receiving )
            printf( "%09.2f - receiving large block %d - %d/%d fragments\n",
                    timeBase.time, 
                    status.blockId, 
                    status.numReceivedFragments, 
                    status.numFragments );
#endif

        assert( messageChannel->GetCounter( RELIABLE_MESSAGE_CHANNEL_COUNTER_MESSAGES_SENT ) == numMessagesSent );
        assert( messageChannel->GetCounter( RELIABLE_MESSAGE_CHANNEL_COUNTER_MESSAGES_RECEIVED ) == numMessagesReceived );
        assert( messageChannel->GetCounter( RELIABLE_MESSAGE_CHANNEL_COUNTER_MESSAGES_EARLY ) == 0 );

        timeBase.time += timeBase.deltaTime;
    }
}

int main()
{
    srand( time( nullptr ) );

    memory::initialize();

    soak_test();

    memory::shutdown();

    return 0;
}
