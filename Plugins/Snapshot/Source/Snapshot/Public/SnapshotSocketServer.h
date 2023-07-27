/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#pragma once

#include "SnapshotSocket.h"

struct snapshot_server_t;

#ifndef SNAPSHOT_ADDRESS_ALREADY_DEFINED
struct snapshot_address_t
{
    union { uint8_t ipv4[4]; uint16_t ipv6[8]; } data;
    uint16_t port;
    uint8_t type;
};
#define SNAPSHOT_ADDRESS_ALREADY_DEFINED
#endif // #ifndef SNAPSHOT_ADDRESS_ALREADY_DEFINED

class FSnapshotSocketServer : public FSnapshotSocket
{
private:

    snapshot_server_t* SnapshotServer;

    bool bUpdatedThisFrame;

    struct PacketData {
        snapshot_address_t from;
        uint8_t* packet_data;
        int packet_bytes;
    };

    TQueue<PacketData> PacketQueue;

public:

    FSnapshotSocketServer(const FString& InSocketDescription, const FName& InSocketProtocol);

    virtual ~FSnapshotSocketServer();

    virtual void Update() override;

    /**
     * Closes the socket
     *
     * @param true if it closes without errors, false otherwise
     */
    virtual bool Close() override;

    /**
     * Binds a socket to a network byte ordered address
     *
     * @param Addr the address to bind to
     *
     * @return true if successful, false otherwise
     */
    virtual bool Bind(const FInternetAddr& Addr) override;

    /**
     * Sends a buffer to a network byte ordered address
     *
     * @param Data the buffer to send
     * @param Count the size of the data to send
     * @param BytesSent out param indicating how much was sent
     * @param Destination the network byte ordered address to send to
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
     * Gets the address the socket is bound to
     *
     * @param OutAddr address the socket is bound to
     */
    virtual void GetAddress(FInternetAddr& OutAddr) override;

    /**
     * Gets the port number this socket is bound to
     */
    virtual int32 GetPortNo() override;

protected:

    // Callback for passthrough packets sent from clients to the server

    static void ProcessPassthroughPacket(void* context, const snapshot_address_t* client_address, int client_index, const uint8_t* packet_data, int packet_bytes);

    // Callback when clients connect and disconnect

    static void ClientConnectDisconnect(void* context, int client_index, int connect);
};
