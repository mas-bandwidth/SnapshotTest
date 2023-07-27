/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#include "SnapshotSocketServer.h"
#include "snapshot_server.h"
#include "snapshot_address.h"
#include "snapshot_platform.h"
#include "snapshot_base64.h"

FSnapshotSocketServer::FSnapshotSocketServer(const FString& InSocketDescription, const FName& InSocketProtocol)
    : FSnapshotSocket(ESnapshotSocketType::TYPE_Server, InSocketDescription, InSocketProtocol)
{
    UE_LOG(LogSnapshot, Display, TEXT("Server socket created"));
    SnapshotServer = NULL;
    bUpdatedThisFrame = false;
}

FSnapshotSocketServer::~FSnapshotSocketServer()
{
    Close();
    UE_LOG(LogSnapshot, Display, TEXT("Server socket destroyed"));
}

void FSnapshotSocketServer::Update()
{
    // ...
}

bool FSnapshotSocketServer::Close()
{
    if (SnapshotServer)
    {
        snapshot_server_destroy(SnapshotServer);
        SnapshotServer = NULL;
    }

    PacketData PassthroughPacket;
    while (PacketQueue.Dequeue(PassthroughPacket))
    {
        FMemory::Free(PassthroughPacket.packet_data);
    }

    UE_LOG(LogSnapshot, Display, TEXT("Server socket closed"));

    return true;
}

#define TEST_PROTOCOL_ID 0x1122334455667788

static uint8_t test_server_private_key[SNAPSHOT_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea,
                                                               0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4,
                                                               0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                               0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };

bool FSnapshotSocketServer::Bind(const FInternetAddr& Addr)
{
    Close();

    FString ServerAddress = Addr.ToString(true);

    FString ServerAddressOverride;
    if (FParse::Value(FCommandLine::Get(), TEXT("-serveraddress="), ServerAddressOverride))
    {
        ServerAddress = ServerAddressOverride;
    }

    UE_LOG(LogSnapshot, Display, TEXT("Server address is '%s'"), *ServerAddress);

    struct snapshot_server_config_t server_config;
    snapshot_default_server_config(&server_config);
    server_config.context = this;
    server_config.protocol_id = TEST_PROTOCOL_ID;                                                           // todo: get protocol id from somewhere meaningful in the engine, eg. hash of code + content?
    server_config.process_passthrough_callback = ProcessPassthroughPacket;
    memcpy(&server_config.private_key, test_server_private_key, SNAPSHOT_KEY_BYTES);

    FString PrivateKeyOverride;
    if (FParse::Value(FCommandLine::Get(), TEXT("-privatekey="), PrivateKeyOverride))
    {
        uint8_t private_key_override[SNAPSHOT_KEY_BYTES];
        if (snapshot_base64_decode_data(TCHAR_TO_ANSI(*PrivateKeyOverride), private_key_override, SNAPSHOT_KEY_BYTES) == SNAPSHOT_KEY_BYTES)
        {
            UE_LOG(LogSnapshot, Display, TEXT("Found valid private key override on command line"), *PrivateKeyOverride);
            memcpy(&server_config.private_key, private_key_override, SNAPSHOT_KEY_BYTES);
        }
        else
        {
            UE_LOG(LogSnapshot, Display, TEXT("Invalid private key override '%s', falling back to test private key"), *PrivateKeyOverride);
        }
    }

    SnapshotServer = snapshot_server_create(TCHAR_TO_ANSI(*ServerAddress), &server_config, snapshot_platform_time());
    if (!SnapshotServer)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Failed to create snapshot server"));
        return false;
    }

    UE_LOG(LogSnapshot, Display, TEXT("Created snapshot server"));

    return true;
}

bool FSnapshotSocketServer::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination)
{
    if (!SnapshotServer)
        return false;
    
    snapshot_address_t dest;
    if (snapshot_address_parse(&dest, TCHAR_TO_ANSI(*(Destination.ToString(true)))) != SNAPSHOT_OK)
    {
        UE_LOG(LogSnapshot, Warning, TEXT("Invalid address passed to FSnapshotSocketServer::SendTo (%s)"), *Destination.ToString(true));
        return false;
    }

    int client_index = snapshot_server_find_client_index_by_address(SnapshotServer, &dest);
    if (client_index < 0)
    {
        UE_LOG(LogSnapshot, Warning, TEXT("No connected client with address %s in FSnapshotSocketServer::SendTo"), *Destination.ToString(true));
        return false;
    }

    snapshot_server_send_passthrough_packet(SnapshotServer, client_index, Data, Count);
    
    BytesSent = Count;

    return true;
}

void FSnapshotSocketServer::ProcessPassthroughPacket(void* context, const snapshot_address_t* client_address, int client_index, const uint8_t* packet_data, int packet_bytes)
{
    // IMPORTANT: This is called from main thread inside snapshot_server_update

    FSnapshotSocketServer* self = (FSnapshotSocketServer*)context;

    uint8_t* packet_data_copy = (uint8_t*)FMemory::Malloc(packet_bytes);

    memcpy(packet_data_copy, packet_data, packet_bytes);

    self->PacketQueue.Enqueue({*client_address, packet_data_copy, packet_bytes});
}

bool FSnapshotSocketServer::RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FInternetAddr& Source, ESocketReceiveFlags::Type Flags)
{
    if (!SnapshotServer)
        return false;

    if (Flags != ESocketReceiveFlags::None)
        return false;

    if (!bUpdatedThisFrame)
    {
        // make sure we update the server prior to receiving any packets this frame
        snapshot_server_update(SnapshotServer, snapshot_platform_time());
        bUpdatedThisFrame = true;
    }

    PacketData PassthroughPacket;
    if (!PacketQueue.Dequeue(PassthroughPacket))
    {
        // we have finished receiving packets for this frame
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

    // Convert snapshot address to string.
    char snapshot_address_string[SNAPSHOT_MAX_ADDRESS_STRING_LENGTH];
    snapshot_address_to_string(&PassthroughPacket.from, snapshot_address_string);
    FString UnrealAddressString = FString(ANSI_TO_TCHAR(snapshot_address_string));
    bool bIsValid;
    Source.SetIp(*UnrealAddressString, bIsValid);
    Source.SetPort(PassthroughPacket.from.port);
    return bIsValid;
}

void FSnapshotSocketServer::GetAddress(FInternetAddr& OutAddr)
{
    // We *always* bind to 0.0.0.0
    bool IsValid = false;
    OutAddr.SetIp(TEXT("0.0.0.0"), IsValid);
}

int32 FSnapshotSocketServer::GetPortNo()
{
    // Return the port number that the server socket is listening on
    return SnapshotServer ? snapshot_server_port(SnapshotServer) : 0;
}
