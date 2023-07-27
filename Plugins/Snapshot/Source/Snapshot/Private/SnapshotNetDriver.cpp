/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#include "SnapshotNetDriver.h"
#include "SnapshotModule.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystem.h"
#include "SocketSubsystem.h"
#include "IpNetDriver.h"
#include "SnapshotSocketSubsystem.h"
#include "SnapshotSocketServer.h"
#include "SnapshotSocketClient.h"
#include "snapshot.h"
#include "snapshot_base64.h"

bool USnapshotNetDriver::IsAvailable() const
{
    ISocketSubsystem* SnapshotSockets = ISocketSubsystem::Get(SNAPSHOT_SUBSYSTEM);
    if (SnapshotSockets)
    {
        UE_LOG(LogSnapshot, Display, TEXT("Snapshot net driver is available"));
        return true;
    }

    UE_LOG(LogSnapshot, Display, TEXT("Snapshot net driver is NOT available"));

    return false;
}

ISocketSubsystem* USnapshotNetDriver::GetSocketSubsystem()
{
    return ISocketSubsystem::Get(SNAPSHOT_SUBSYSTEM);
}

bool USnapshotNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
    UE_LOG(LogSnapshot, Display, TEXT("USnapshotNetDriver::InitBase"));

    if (!UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
    {
        UE_LOG(LogSnapshot, Warning, TEXT("UIpNetDriver::InitBase failed"));
        return false;
    }
    
    ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
    if (SocketSubsystem == NULL)
    {
        UE_LOG(LogSnapshot, Warning, TEXT("Unable to find socket subsystem"));
        Error = TEXT("Unable to find socket subsystem");
        return false;
    }

    if (GetSocket() == NULL)
    {
        UE_LOG(LogSnapshot, Warning, TEXT("GetSocket() is NULL"));
        Error = FString::Printf(TEXT("socket failed (%i)"), (int32)SocketSubsystem->GetLastErrorCode());
        return false;
    }

    LocalAddr = SocketSubsystem->GetLocalBindAddr(*GLog);

    if (bInitAsClient)
    {
        // bind client to an ephemeral port
        LocalAddr->SetPort(0);
    }
    else
    {
        // bind server to the specified port
        LocalAddr->SetPort(URL.Port);
    }

    int32 BoundPort = SocketSubsystem->BindNextPort(GetSocket(), *LocalAddr, MaxPortCountToTry + 1, 1);

    UE_LOG(LogNet, Display, TEXT("%s bound to port %d"), *GetName(), BoundPort);

    return true;
}

bool USnapshotNetDriver::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
    UE_LOG(LogSnapshot, Display, TEXT("USnapshotNetDriver::InitConnect"));

    FSnapshotSocketSubsystem* SnapshotSockets = (FSnapshotSocketSubsystem*)ISocketSubsystem::Get(SNAPSHOT_SUBSYSTEM);
    if (!SnapshotSockets)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Could not find snapshot socket subsystem"));
        return false;
    }

    FUniqueSocket NewSocket = SnapshotSockets->CreateUniqueSocket(FName(TEXT("SnapshotSocketClient")), TEXT("Unreal client (Snapshot)"), FName(TEXT("Snapshot")));

    TSharedPtr<FSocket> SharedSocket(NewSocket.Release(), FSocketDeleter(NewSocket.GetDeleter()));

    SetSocketAndLocalAddress(SharedSocket);

    bool result = Super::InitConnect(InNotify, ConnectURL, Error);

    bIsClient = true;
    bIsServer = false;

    FSnapshotSocketClient* ClientSocket = GetClientSocket();
    if (ClientSocket == NULL)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Client socket is NULL"));
        return false;
    }

    // IMPORTANT: Must be done *after* Super::InitConnect because client socket bind happens there
    if (ConnectURL.Host.StartsWith("snapshot."))
    {
        uint8_t connect_token[SNAPSHOT_CONNECT_TOKEN_BYTES];
        if (snapshot_base64_decode_data(TCHAR_TO_ANSI(*ConnectURL.Host + 9), connect_token, SNAPSHOT_CONNECT_TOKEN_BYTES) == SNAPSHOT_CONNECT_TOKEN_BYTES)
        {
            ClientSocket->SnapshotSecureConnect(connect_token);
        }
        else
        {
            UE_LOG(LogSnapshot, Display, TEXT("Invalid snapshot connect token: '%s'"), *ConnectURL.Host);
        }
    }
    else
    {
        ClientSocket->SnapshotInsecureConnect(ConnectURL.Host, ConnectURL.Port);
    }

    return result;
}

bool USnapshotNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
    UE_LOG(LogSnapshot, Display, TEXT("USnapshotNetDriver::InitListen"));

    FSnapshotSocketSubsystem* SnapshotSockets = (FSnapshotSocketSubsystem*)ISocketSubsystem::Get(SNAPSHOT_SUBSYSTEM);
    if (!SnapshotSockets)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Could not find snapshot socket subsystem"));
        return false;
    }

    FUniqueSocket NewSocket = SnapshotSockets->CreateUniqueSocket(FName(TEXT("SnapshotSocketServer")), TEXT("Unreal server (Snapshot)"), FName(TEXT("Snapshot")));

    TSharedPtr<FSocket> SharedSocket(NewSocket.Release(), FSocketDeleter(NewSocket.GetDeleter()));

    SetSocketAndLocalAddress(SharedSocket);

    bIsClient = false;
    bIsServer = true;

    FSnapshotSocketServer* ServerSocket = GetServerSocket();
    if (ServerSocket == NULL)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Server socket is NULL"));
        return false;
    }

    bool result = Super::InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error);

    return result;
}

void USnapshotNetDriver::Shutdown()
{
    UE_LOG(LogSnapshot, Display, TEXT("FSnapshotNetDriver::Shutdown"));

    Super::Shutdown();
}

bool USnapshotNetDriver::IsNetResourceValid()
{
    return true;
}

bool USnapshotNetDriver::IsClient() const
{
    return bIsClient;
}

bool USnapshotNetDriver::IsServer() const
{
    return bIsServer;
}

FSnapshotSocketClient* USnapshotNetDriver::GetClientSocket()
{
    return IsClient() ? (FSnapshotSocketClient*)GetSocket(): NULL;
}

FSnapshotSocketServer* USnapshotNetDriver::GetServerSocket()
{
    return IsServer() ? (FSnapshotSocketServer*)GetSocket() : NULL;
}

