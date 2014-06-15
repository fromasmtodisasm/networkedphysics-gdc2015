/*
    Network Protocol Library
    Copyright (c) 2013-2014 Glenn Fiedler <glenn.fiedler@gmail.com>
*/

#ifndef PROTOCOL_CHANNEL_H
#define PROTOCOL_CHANNEL_H

#include "Common.h"

namespace protocol
{
    /*
        Channel data is passed to the serializer per-packet
        and is how a channel specifies what data should be
        serialized in the connection packet.
    */

    class ChannelData : public Object
    {
    public:
        virtual ~ChannelData() {}
    };

    /*
        Implement this interface to integrate a custom channel
        inside a connection, see ReliableMessageChannel for a
        working example.
    */

    class Channel
    {
    public:

        virtual ~Channel()
        {
            // ...
        }

        virtual void Reset() = 0;

        virtual int GetError() const = 0;

        virtual ChannelData * GetData( uint16_t sequence ) = 0;

        virtual bool ProcessData( uint16_t sequence, ChannelData * data ) = 0;

        virtual void ProcessAck( uint16_t ack ) = 0;

        virtual void Update( const TimeBase & timeBase ) = 0;
    };

    /*  
        Many test cases don't need full functionality of a channel.
        Define a channel adapter here that stubs out all functionality
        so these tests are less verbose.
    */

    class ChannelAdapter : public Channel
    {
    public:

        void Reset() {}

        int GetError() const { return 0; }

        ChannelData * GetData( uint16_t sequence ) { return nullptr; }

        bool ProcessData( uint16_t sequence, ChannelData * data ) { return true; }

        void ProcessAck( uint16_t ack ) {}

        void Update( const TimeBase & timeBase ) {}
    };

    /*
        In order for two connections to be able to communicate with each
        other they must share the same channel structure precisely.

        This class describes that structure. Prior to being passed in
        to a connection, the channel structure must be locked, as the
        channel structure cannot change once a channel is using it.

        See ClientServerPackets.h or TestConnection.cpp for examples of use.
    */

    class ChannelStructure
    {
        typedef std::function<Channel*()> CreateChannelFunction;
        typedef std::function<ChannelData*()> CreateChannelDataFunction;

        struct ChannelEntry
        {
            std::string name;
            CreateChannelFunction createChannel;
            CreateChannelDataFunction createChannelData;
        };

        bool m_locked = false;
        int m_numChannels = 0;
        ChannelEntry m_channelEntries[MaxChannels];

    public:

        void AddChannel( const std::string & name,
                         CreateChannelFunction createChannel,
                         CreateChannelDataFunction createChannelData )
        {
            assert( !m_locked );
            if ( m_locked )
                return;

            assert( m_numChannels < MaxChannels - 1 );

            ChannelEntry & entry = m_channelEntries[m_numChannels];

            entry.name = name;
            entry.createChannel = createChannel;
            entry.createChannelData = createChannelData;

            m_numChannels++;
        }

        void Lock()
        {
            m_locked = true;
        }

        bool IsLocked() const
        {
            return m_locked;
        }

        int GetNumChannels()
        {
            return m_numChannels;
        }

        Channel * CreateChannel( int channelIndex )
        {
            assert( m_locked );
            assert( channelIndex >= 0 );
            assert( channelIndex < m_numChannels );
            return m_channelEntries[channelIndex].createChannel();
        }

        ChannelData * CreateChannelData( int channelIndex )
        {
            assert( m_locked );
            assert( channelIndex >= 0 );
            assert( channelIndex < m_numChannels );
            return m_channelEntries[channelIndex].createChannelData();
        }
    };
}

#endif
