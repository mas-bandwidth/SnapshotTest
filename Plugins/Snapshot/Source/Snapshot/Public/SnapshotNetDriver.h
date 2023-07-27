/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "IpNetDriver.h"
#include "SnapshotNetDriver.generated.h"

class FNetworkNotify;

UCLASS(transient, config = Engine)
class USnapshotNetDriver : public UIpNetDriver
{
    GENERATED_BODY()

    virtual class ISocketSubsystem* GetSocketSubsystem() override;
    virtual bool IsAvailable() const override;
    virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
    virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
    virtual bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) override;
    virtual void Shutdown() override;
    virtual bool IsNetResourceValid() override;

    bool IsClient() const;
    bool IsServer() const;

protected:

    class FSnapshotSocketClient* GetClientSocket();
    class FSnapshotSocketServer* GetServerSocket();

private:

    bool bIsClient;
    bool bIsServer;
};
