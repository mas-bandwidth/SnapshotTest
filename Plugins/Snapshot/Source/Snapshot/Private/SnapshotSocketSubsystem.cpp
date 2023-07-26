/*
    Network Next SDK. Copyright © 2017 - 2023 Network Next, Inc.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
    conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
       and the following disclaimer in the documentation and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
       products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SnapshotSocketSubsystem.h"
#include "SnapshotModule.h"
#include "SnapshotSocket.h"
#include "SnapshotSocketClient.h"
#include "SnapshotSocketServer.h"
#include "SocketSubsystemModule.h"

FName CreateSnapshotSocketSubsystem()
{
    FSnapshotSocketSubsystem* PlatformSubsystem = FSnapshotSocketSubsystem::Create();
    FString Error;
    if (PlatformSubsystem->Init(Error))
    {
        FSocketSubsystemModule & sockets = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
        sockets.RegisterSocketSubsystem(SNAPSHOT_SUBSYSTEM, PlatformSubsystem, true);
        return SNAPSHOT_SUBSYSTEM;
    }
    else
    {
        FSnapshotSocketSubsystem::Destroy();
        return NAME_None;
    }
}

void DestroySnapshotSocketSubsystem()
{
    FModuleManager& ModuleManager = FModuleManager::Get();
    if (ModuleManager.IsModuleLoaded("Sockets"))
    {
        FSocketSubsystemModule& socketSubsystem = FModuleManager::GetModuleChecked<FSocketSubsystemModule>("Sockets");
        socketSubsystem.UnregisterSocketSubsystem(SNAPSHOT_SUBSYSTEM);
    }
    FSnapshotSocketSubsystem::Destroy();
}

static FSnapshotSocketSubsystem* SocketSingleton;

FSnapshotSocketSubsystem* FSnapshotSocketSubsystem::Create()
{
    if (!SocketSingleton)
    {
        SocketSingleton = new FSnapshotSocketSubsystem();
    }

    return SocketSingleton;
}

void FSnapshotSocketSubsystem::Destroy()
{
    if (SocketSingleton)
    {
        SocketSingleton->Shutdown();
        delete SocketSingleton;
        SocketSingleton = NULL;
    }
}

bool FSnapshotSocketSubsystem::Init(FString& Error)
{
    UE_LOG(LogSnapshot, Display, TEXT("Snapshot socket subsystem initializing"));

    ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    UE_LOG(LogSnapshot, Display, TEXT("Platform default socket subsystem is %s"), PlatformSubsystem->GetSocketAPIName());

    return true;
}

void FSnapshotSocketSubsystem::Shutdown()
{
    UE_LOG(LogSnapshot, Display, TEXT("Snapshot socket subsystem shutdown"));

    for (auto Socket : this->SnapshotSockets)
    {
        Socket->Close();
    }

    SnapshotSockets.Empty();
}

FSocket* FSnapshotSocketSubsystem::CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolName)
{
    if (SocketType == FName("SnapshotSocketServer"))
    {
        // server socket
        UE_LOG(LogSnapshot, Display, TEXT("Create snapshot server socket"));
        FString ModifiedSocketDescription = SocketDescription;
        ModifiedSocketDescription.InsertAt(0, TEXT("SOCKET_TYPE_SNAPSHOT_SERVER_"));
        FSnapshotSocketServer* Socket = new FSnapshotSocketServer(ModifiedSocketDescription, ProtocolName);
        SnapshotSockets.Add(Socket);
        return Socket;
    }
    else if (SocketType == FName("SnapshotSocketClient"))
    {
        // client socket
        UE_LOG(LogSnapshot, Display, TEXT("Create snapshot client socket"));
        FString ModifiedSocketDescription = SocketDescription;
        ModifiedSocketDescription.InsertAt(0, TEXT("SOCKET_TYPE_SNAPSHOT_CLIENT_"));
        FSnapshotSocketClient* Socket = new FSnapshotSocketClient(ModifiedSocketDescription, ProtocolName);
        SnapshotSockets.Add(Socket);
        return Socket;
    }
    else
    {
        // platform socket
        UE_LOG(LogSnapshot, Display, TEXT("Create platform socket"));
        ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        return PlatformSubsystem->CreateSocket(SocketType, SocketDescription, ProtocolName);
    }
}

void FSnapshotSocketSubsystem::DestroySocket(FSocket* Socket)
{
    if (SnapshotSockets.RemoveSingleSwap((FSnapshotSocket*)Socket))
    {
        // snapshot socket
        UE_LOG(LogSnapshot, Display, TEXT("Destroy snapshot socket"));
        delete Socket;
    }
    else
    {
        // platform socket
        UE_LOG(LogSnapshot, Display, TEXT("Destroy platform socket"));
        ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        PlatformSubsystem->DestroySocket(Socket);
    }
}

FAddressInfoResult FSnapshotSocketSubsystem::GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName, EAddressInfoFlags QueryFlags, const FName ProtocolTypeName, ESocketType SocketType)
{
    ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    return PlatformSubsystem->GetAddressInfo(HostName, ServiceName, QueryFlags, ProtocolTypeName, SocketType);
}

TSharedPtr<FInternetAddr> FSnapshotSocketSubsystem::GetAddressFromString(const FString& InAddress)
{
    ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    return PlatformSubsystem->GetAddressFromString(InAddress);
}

bool FSnapshotSocketSubsystem::GetHostName(FString& HostName)
{
    ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    return PlatformSubsystem->GetHostName(HostName);
}

TSharedRef<FInternetAddr> FSnapshotSocketSubsystem::CreateInternetAddr()
{
    ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    return PlatformSubsystem->CreateInternetAddr();
}

bool FSnapshotSocketSubsystem::HasNetworkDevice()
{
    ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    return PlatformSubsystem->HasNetworkDevice();
}

const TCHAR* FSnapshotSocketSubsystem::GetSocketAPIName() const
{
    return TEXT("Snapshot");
}

ESocketErrors FSnapshotSocketSubsystem::GetLastErrorCode()
{
    return ESocketErrors::SE_NO_ERROR;
}

ESocketErrors FSnapshotSocketSubsystem::TranslateErrorCode(int32 Code)
{
    ISocketSubsystem* PlatformSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    return PlatformSubsystem->TranslateErrorCode(Code);
}

bool FSnapshotSocketSubsystem::Tick(float DeltaTime)
{
    for (auto Socket : SnapshotSockets)
    {
        Socket->Update();
    }
    return true;
}
