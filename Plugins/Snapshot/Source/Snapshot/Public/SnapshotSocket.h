/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemNames.h"
#include "Sockets.h"
#include "SnapshotModule.h"

/*
    There are two types of sockets in the Snapshot Unreal Plugin:

        1. Client Sockets
        2. Server Sockets

    Each socket implements *only* UDP datagram support. All TCP style socket support is stubbed out.

    The client socket is used by client when it connects to a server.

    The server socket is used by a listen server.

    All other socket use is directed towards the regular platform socket interface and socket types. 
*/

enum class ESnapshotSocketType : uint8
{
    TYPE_None,
    TYPE_Client,
    TYPE_Server,
};

class FSnapshotSocket : public FSocket
{
private:

    ESnapshotSocketType SnapshotSocketType;

public:

    FSnapshotSocket(ESnapshotSocketType InSocketType, const FString& InSocketDescription, FName InProtocol);

    virtual void Update() = 0;

    // IMPORTANT: All methods below are stubbed out. Please don't use them :)

    virtual bool Shutdown(ESocketShutdownMode Mode) override;

    virtual bool Connect(const FInternetAddr& Addr) override;

    virtual bool Listen(int32 MaxBacklog) override;

    virtual bool WaitForPendingConnection(bool& bHasPendingConnection, const FTimespan& WaitTime) override;

    virtual bool HasPendingData(uint32& PendingDataSize) override;

    virtual class FSocket* Accept(const FString& SocketDescription) override;

    virtual class FSocket* Accept(FInternetAddr& OutAddr, const FString& SocketDescription) override;

    virtual bool Send(const uint8* Data, int32 Count, int32& BytesSent) override;

    virtual bool Recv(uint8* Data, int32 BufferSize, int32& BytesRead, ESocketReceiveFlags::Type Flags = ESocketReceiveFlags::None) override;

    virtual bool Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime) override;

    virtual ESocketConnectionState GetConnectionState() override;

    virtual bool GetPeerAddress(FInternetAddr& OutAddr) override;

    virtual bool SetNonBlocking(bool bIsNonBlocking = true) override;

    virtual bool SetBroadcast(bool bAllowBroadcast = true) override;

    virtual bool JoinMulticastGroup(const FInternetAddr& GroupAddress) override;

    virtual bool LeaveMulticastGroup(const FInternetAddr& GroupAddress) override;

    virtual bool JoinMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress) override;

    virtual bool LeaveMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress) override;

    virtual bool SetMulticastLoopback(bool bLoopback) override;

    virtual bool SetMulticastTtl(uint8 TimeToLive) override;

    virtual bool SetMulticastInterface(const FInternetAddr& InterfaceAddress) override;

    virtual bool SetReuseAddr(bool bAllowReuse = true) override;

    virtual bool SetLinger(bool bShouldLinger = true, int32 Timeout = 0) override;

    virtual bool SetRecvErr(bool bUseErrorQueue = true) override;

    virtual bool SetSendBufferSize(int32 Size,int32& NewSize) override;

    virtual bool SetReceiveBufferSize(int32 Size,int32& NewSize) override;

    virtual bool SetNoDelay(bool flag) override;
};
