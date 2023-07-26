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
    
    FSocket* NewSocket = SnapshotSockets->CreateSocket(FName(TEXT("SnapshotSocketClient")), TEXT("Unreal client (Snapshot)"), FName(TEXT("Snapshot")));
    if (!NewSocket)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Could not create snapshot client socket"));
        return false;
    }

    SetSocketAndLocalAddress(NewSocket);

    ClientSocket = (FSnapshotSocketClient*)NewSocket;
    ServerSocket = NULL;

    return Super::InitConnect(InNotify, ConnectURL, Error);
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

    FSocket* NewSocket = SnapshotSockets->CreateSocket(FName(TEXT("SnapshotSocketServer")), TEXT("Unreal server (Snapshot)"), FName(TEXT("Snapshot")));
    if (!NewSocket)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Could not create snapshot server socket"));
        return false;
    }

    SetSocketAndLocalAddress(NewSocket);

    ServerSocket = (FSnapshotSocketServer*)NewSocket;
    ClientSocket = NULL;

    return Super::InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void USnapshotNetDriver::Shutdown()
{
    UE_LOG(LogSnapshot, Display, TEXT("FSnapshotNetDriver::Shutdown"));

    ClientSocket = NULL;
    ServerSocket = NULL;

    Super::Shutdown();
}

bool USnapshotNetDriver::IsNetResourceValid()
{
    return true;
}
