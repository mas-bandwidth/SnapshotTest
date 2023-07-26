/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#include "SnapshotSocket.h"

FSnapshotSocket::FSnapshotSocket(ESnapshotSocketType InSocketType, const FString& InSocketDescription, FName InSocketProtocol)
    : FSocket(SOCKTYPE_Datagram, InSocketDescription, InSocketProtocol)
{
    this->SnapshotSocketType = InSocketType;
}

bool FSnapshotSocket::Shutdown(ESocketShutdownMode Mode)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Shutdown is not supported"))
    return false;
}

bool FSnapshotSocket::Connect(const FInternetAddr& Addr)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Connect is not supported"))
    return false;
}

bool FSnapshotSocket::Listen(int32 MaxBacklog)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Listen is not supported"))
    return false;
}

bool FSnapshotSocket::WaitForPendingConnection(bool& bHasPendingConnection, const FTimespan& WaitTime)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::WaitForPendingConnection is not supported"))
    return false;
}

bool FSnapshotSocket::HasPendingData(uint32& PendingDataSize)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::HasPendingData is not supported"))
    return false;
}


FSocket* FSnapshotSocket::Accept(const FString& InSocketDescription)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Accept is not supported"))
    return NULL;
}

FSocket* FSnapshotSocket::Accept(FInternetAddr& OutAddr, const FString& InSocketDescription)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Accept is not supported"))
    return NULL;
}

bool FSnapshotSocket::Send(const uint8* Data, int32 Count, int32& BytesSent)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Send is not supported"))
    BytesSent = 0;
    return false;
}

bool FSnapshotSocket::Recv(uint8* Data, int32 BufferSize, int32& BytesRead, ESocketReceiveFlags::Type Flags)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Recv is not supported"))
    BytesRead = 0;
    return false;
}

bool FSnapshotSocket::Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::Wait is not supported"))
    return false;
}

ESocketConnectionState FSnapshotSocket::GetConnectionState()
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::GetConnectionState is not supported"))
    return SCS_NotConnected;
}

bool FSnapshotSocket::GetPeerAddress(FInternetAddr& OutAddr)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::GetPeerAddress is not supported"))
    return false;
}

bool FSnapshotSocket::SetNonBlocking(bool bIsNonBlocking)
{
    // *** IGNORED ***
    return true;
}

bool FSnapshotSocket::SetBroadcast(bool bAllowBroadcast)
{
    // *** IGNORED ***
    return true;
}

bool FSnapshotSocket::JoinMulticastGroup(const FInternetAddr& GroupAddress)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::JoinMulticastGroup is not supported"))
    return false;
}

bool FSnapshotSocket::LeaveMulticastGroup(const FInternetAddr& GroupAddress)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::LeaveMulticastGroup is not supported"))
    return false;
}

bool FSnapshotSocket::JoinMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::JoinMulticastGroup is not supported"))
    return false;
}

bool FSnapshotSocket::LeaveMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::LeaveMulticastGroup is not supported"))
    return false;
}

bool FSnapshotSocket::SetMulticastLoopback(bool bLoopback)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::SetMulticastLoopback is not supported"))
    return false;
}

bool FSnapshotSocket::SetMulticastTtl(uint8 TimeToLive)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::SetMulticastTtl is not supported"))
    return false;
}

bool FSnapshotSocket::SetMulticastInterface(const FInternetAddr& InterfaceAddress)
{
    // *** NOT SUPPORTED ***
    UE_LOG(LogSnapshot, Warning, TEXT("FSnapshotSocket::SetMulticastInterface is not supported"))
    return false;
}

bool FSnapshotSocket::SetReuseAddr(bool bAllowReuse)
{
    // *** IGNORED ***
    return true;
}

bool FSnapshotSocket::SetLinger(bool bShouldLinger, int32 Timeout)
{
    // *** IGNORED ***
    return true;
}

bool FSnapshotSocket::SetRecvErr(bool bUseErrorQueue)
{
    // *** IGNORED ***
    return true;
}

bool FSnapshotSocket::SetSendBufferSize(int32 Size, int32& NewSize)
{
    // *** IGNORED ***
    return true;
}

bool FSnapshotSocket::SetReceiveBufferSize(int32 Size, int32& NewSize)
{
    // *** IGNORED ***
    return true;
}

bool FSnapshotSocket::SetNoDelay(bool flag)
{
    // *** IGNORED ***
    return false;
}
