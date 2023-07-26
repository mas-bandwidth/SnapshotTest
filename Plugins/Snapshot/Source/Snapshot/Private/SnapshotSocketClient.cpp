/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#include "SnapshotSocketClient.h"
#include "snapshot_client.h"
#include "snapshot_connect_token.h"

FSnapshotSocketClient::FSnapshotSocketClient(const FString& InSocketDescription, const FName& InSocketProtocol)
    : FSnapshotSocket(ESnapshotSocketType::TYPE_Client, InSocketDescription, InSocketProtocol)
{
    UE_LOG(LogSnapshot, Display, TEXT("Client socket created"));
    SnapshotClient = NULL;
    ServerPort = 0;
    bConnected = false;
    bUpdatedThisFrame = false;
}

FSnapshotSocketClient::~FSnapshotSocketClient()
{
    Close();
    UE_LOG(LogSnapshot, Display, TEXT("Client socket destroyed"));
}

void FSnapshotSocketClient::Update()
{
    // ...
}

bool FSnapshotSocketClient::Close()
{
    if (SnapshotClient)
    {
        snapshot_client_destroy(SnapshotClient);
        SnapshotClient = NULL;
        ServerAddrAndPort = TEXT("");
        ServerAddr = TEXT("");
        ServerPort = 0;
        PacketQueue.Empty();
        UE_LOG(LogSnapshot, Display, TEXT("Client socket closed"));
    }
    return true;
}

bool FSnapshotSocketClient::Bind(const FInternetAddr& Addr)
{
    // We must ignore the local bind address and bind to 0.0.0.0 instead. XBoxOne binds to "::0" otherwise and breaks.
    int BindPort = Addr.GetPort();
    char BindAddress[256];
    TCString<char>::Sprintf(BindAddress, "0.0.0.0:%d", BindPort);

    UE_LOG(LogSnapshot, Display, TEXT("Bind Client Socket (%s)"), ANSI_TO_TCHAR(BindAddress));

    if (SnapshotClient)
    {
        Close();
    }

    struct snapshot_client_config_t client_config;
    snapshot_default_client_config(&client_config);
    client_config.context = this;
    client_config.process_passthrough_callback = ProcessPassthroughPacket;
    
    SnapshotClient = snapshot_client_create(BindAddress, &client_config, snapshot_platform_time());
    if (SnapshotClient == NULL)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Failed to create snapshot client"));
        return false;
    }

    UE_LOG(LogSnapshot, Display, TEXT("Created snapshot client"));

    const int ClientPort = snapshot_client_port(SnapshotClient);

    UE_LOG(LogSnapshot, Display, TEXT("Client port is %d"), ClientPort);

    return true;
}

static FString IntToIPv4String(uint32 ipAsInt)
{
    return FString::Printf(TEXT("%d.%d.%d.%d"), (ipAsInt >> 24), ((ipAsInt >> 16) & 0xff), ((ipAsInt >> 8) & 0xff), (ipAsInt & 0xff));
}

#define TEST_PROTOCOL_ID 0x1122334455667788
#define TEST_CONNECT_TOKEN_EXPIRY 30
#define TEST_CONNECT_TOKEN_TIMEOUT 5

static uint8_t test_client_private_key[SNAPSHOT_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea,
                                                               0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4,
                                                               0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                               0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };

bool FSnapshotSocketClient::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination)
{
#if PLATFORM_HAS_BSD_IPV6_SOCKETS

    /*
    *    When PLATFORM_HAS_BSD_IPV6_SOCKETS is set, Destination is of type FInternetAddrBSDIPv6.
    *    FInternetAddrBSDIPv6 converts IPv4 addresses to IPv6, so we convert them back to IPv4 here.
    */

    uint32 ipAddressAsInt = 0;
    Destination.GetIp(ipAddressAsInt);

    FString serverAddr = IntToIPv4String(ipAddressAsInt);
    int32 serverPort = Destination.GetPort();
    FString serverAddrAndPort = FString::Printf(TEXT("%s:%d"), *serverAddr, serverPort);
    
#else

    FString serverAddr = Destination.ToString(false);
    int32 serverPort = Destination.GetPort();
    FString serverAddrAndPort = Destination.ToString(true);

#endif

    if (!SnapshotClient)
        return false;

    // The first send indicates the server we want to connect to
    if (!bConnected)
    {
        UE_LOG(LogSnapshot, Display, TEXT("Client connecting to %s"), *serverAddrAndPort);

        ServerAddrAndPort = serverAddrAndPort;
        ServerAddr = serverAddr;
        ServerPort = serverPort;
        bConnected = true;

        // insecure connect (development only)

        const char* server_address = TCHAR_TO_ANSI(*ServerAddrAndPort);

        uint64_t client_id = 0;
        snapshot_crypto_random_bytes((uint8_t*)&client_id, 8);
        snapshot_printf(SNAPSHOT_LOG_LEVEL_INFO, "client id is %.16" PRIx64, client_id);

        uint8_t user_data[SNAPSHOT_USER_DATA_BYTES];
        snapshot_crypto_random_bytes(user_data, SNAPSHOT_USER_DATA_BYTES);

        uint8_t connect_token[SNAPSHOT_CONNECT_TOKEN_BYTES];

        if (snapshot_generate_connect_token(1, &server_address, TEST_CONNECT_TOKEN_EXPIRY, TEST_CONNECT_TOKEN_TIMEOUT, client_id, TEST_PROTOCOL_ID, test_client_private_key, user_data, connect_token) != SNAPSHOT_OK)
        {
            snapshot_printf(SNAPSHOT_LOG_LEVEL_ERROR, "failed to generate connect token");
            return 1;
        }

        snapshot_client_connect(SnapshotClient, connect_token);
    }

    if (!serverAddrAndPort.Equals(ServerAddrAndPort))
    {
        // The client socket can *only* send to the server address
        UE_LOG(LogSnapshot, Error, TEXT("Attempted to use client socket to send data to %s, but it can only send data to the server %s"), *serverAddrAndPort, *ServerAddrAndPort);
        return false;
    }

    snapshot_client_send_passthrough_packet(SnapshotClient, Data, Count);

    BytesSent = Count;

    return true;
}

void FSnapshotSocketClient::ProcessPassthroughPacket(void* context, const uint8_t* packet_data, int packet_bytes)
{
    // this callback is pumped on main thread from inside snapshot_client_update

    FSnapshotSocketClient* self = (FSnapshotSocketClient*)context;

    uint8_t* packet_data_copy = (uint8_t*)FMemory::Malloc(packet_bytes);
    
    memcpy(packet_data_copy, packet_data, packet_bytes);

    self->PacketQueue.Enqueue({
        packet_data_copy,
        packet_bytes,
        });
}

bool FSnapshotSocketClient::RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FInternetAddr& Source, ESocketReceiveFlags::Type Flags)
{
    if (!SnapshotClient)
        return false;            

    if (!bConnected)
        return false;

    if (Flags != ESocketReceiveFlags::None)
        return false;

    if (!bUpdatedThisFrame)
    {
        // make sure we update the client prior to receiving any packets this frame
        snapshot_client_update(SnapshotClient, snapshot_platform_time());
        bUpdatedThisFrame = true;
    }

    PacketData PassthroughPacket;
    if (!PacketQueue.Dequeue(PassthroughPacket))
    {
        // we have finished receiving passthrough packets for this frame
        bUpdatedThisFrame = false;
        return false;
    }

    // drop packet if it is too large to copy to the recieve buffer
    if (PassthroughPacket.packet_bytes > BufferSize)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Passthrough packet is too large to receive. Packet is %d bytes, but buffer is only %d bytes."), PassthroughPacket.packet_bytes, BufferSize);
        FMemory::Free(PassthroughPacket.packet_data);
        return false;
    }

    // Copy data from packet to buffer.
    memcpy(Data, PassthroughPacket.packet_data, PassthroughPacket.packet_bytes);
    BytesRead = PassthroughPacket.packet_bytes;
    FMemory::Free(PassthroughPacket.packet_data);

    // Packects *only* come from the server
    bool bIsValid;
    Source.SetPort(ServerPort);
    Source.SetIp(*ServerAddr, bIsValid);
    check(bIsValid);
    return true;
}

void FSnapshotSocketClient::GetAddress(FInternetAddr& OutAddr)
{
    // We *always* bind to "0.0.0.0"
    bool bIsValid;
    OutAddr.SetIp(TEXT("0.0.0.0"), bIsValid);
}

int32 FSnapshotSocketClient::GetPortNo()
{
    // Return the actual port number the socket is bound to. This may be a system assigned port if the bind port was 0.
    return SnapshotClient ? snapshot_client_port(SnapshotClient) : 0;
}
