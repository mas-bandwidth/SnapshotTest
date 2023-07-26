/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#pragma once

#include "SnapshotSocket.h"

struct snapshot_client_t;

#ifndef SNAPSHOT_ADDRESS_ALREADY_DEFINED
struct snapshot_address_t
{
    union { uint8_t ipv4[4]; uint16_t ipv6[8]; } data;
    uint16_t port;
    uint8_t type;
};
#define SNAPSHOT_ADDRESS_ALREADY_DEFINED
#endif // #ifndef SNAPSHOT_ADDRESS_ALREADY_DEFINED

class FSnapshotSocketClient : public FSnapshotSocket
{
private:

    snapshot_client_t* SnapshotClient;

    FString ServerAddrAndPort;
    FString ServerAddr;
    int32 ServerPort;

    bool bConnected;
    bool bUpdatedThisFrame;

    struct PacketData {
        uint8_t* packet_data;
        int packet_bytes;
    };

    TQueue<PacketData> PacketQueue;

public:

    FSnapshotSocketClient(const FString& InSocketDescription, const FName& InSocketProtocol);

    virtual ~FSnapshotSocketClient();

    virtual void Update() override;

    /**
     * Closes the socket
     *
     * @param true if it closes without errors, false otherwise
     */
    virtual bool Close() override;

    /**
     * Binds the socket
     *
     * @param Addr the address to bind to
     *
     * @return true if successful, false otherwise
     */
    virtual bool Bind(const FInternetAddr& Addr) override;

    /**
     * Sends a packet to the server.
     *
     * @param Data the buffer to send
     * @param Count the size of the data to send
     * @param BytesSent out param indicating how much was sent
     * @param Destination this parameter is ignored. the packet is always sent to the server
     */
    virtual bool SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination) override;

    /**
     * Reads a packet from the socket.
     *
     * @param Data the buffer to read into
     * @param BufferSize the max size of the buffer
     * @param BytesRead out param indicating how many bytes were read from the socket
     * @param Source out param receiving the address of the sender of the data
     * @param Flags the receive flags (must be ESocketReceiveFlags::None)
     */
    virtual bool RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FInternetAddr& Source, ESocketReceiveFlags::Type Flags = ESocketReceiveFlags::None) override;

    /**
     * Gets the address the socket is bound to. Is always "0.0.0.0"
     *
     * @param OutAddr address the socket is bound to
     */
    virtual void GetAddress(FInternetAddr& OutAddr) override;

    /**
     * Gets the port number the socket is bound to.
     */
    virtual int32 GetPortNo() override;

    // Callback for passthrough packets sent from the server

    static void ProcessPassthroughPacket(void* context, const uint8_t* packet_data, int packet_bytes);
};
