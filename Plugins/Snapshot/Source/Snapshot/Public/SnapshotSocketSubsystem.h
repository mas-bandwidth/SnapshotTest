/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#pragma once

#include "CoreMinimal.h"
#include "SocketSubsystem.h"
#include "Containers/Ticker.h"

FName CreateSnapshotSocketSubsystem();

void DestroySnapshotSocketSubsystem();

class FSnapshotSocketSubsystem : public ISocketSubsystem, public FTSTickerObjectBase
{
private:

    TArray<class FSnapshotSocket*> SnapshotSockets;

public:

    static FSnapshotSocketSubsystem* Create();

    static void Destroy();

    FSnapshotSocketSubsystem() {}

    virtual bool Init(FString& Error) override;

    virtual void Shutdown() override;

    virtual FSocket* CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolName) override;

    virtual void DestroySocket(FSocket* Socket) override;

    virtual FAddressInfoResult GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName = NULL,
        EAddressInfoFlags QueryFlags = EAddressInfoFlags::Default,
        const FName ProtocolTypeName = NAME_None,
        ESocketType SocketType = ESocketType::SOCKTYPE_Unknown) override;

    virtual TSharedPtr<FInternetAddr> GetAddressFromString(const FString& InAddress) override;

    virtual bool RequiresChatDataBeSeparate() override {
        return false;
    }

    virtual bool RequiresEncryptedPackets() override {
        return false;
    }

    virtual bool GetHostName(FString& HostName) override;

    virtual TSharedRef<FInternetAddr> CreateInternetAddr() override;

    virtual bool HasNetworkDevice() override;

    virtual const TCHAR* GetSocketAPIName() const override;

    virtual ESocketErrors GetLastErrorCode() override;

    virtual ESocketErrors TranslateErrorCode(int32 Code) override;

    virtual bool IsSocketWaitSupported() const override { 
        return false; 
    }

    virtual bool Tick(float DeltaTime) override;
};
