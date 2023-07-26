/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#include "SnapshotModule.h"
#include "SnapshotNetDriver.h"
#include "SnapshotSocketSubsystem.h"
#include "Core.h"
#include "snapshot.h"

DEFINE_LOG_CATEGORY(LogSnapshot);

IMPLEMENT_MODULE(FSnapshotModule, Snapshot)

void FSnapshotModule::StartupModule()
{
    UE_LOG(LogSnapshot, Display, TEXT("Snapshot Plugin loaded"));

    m_initialized_sdk = false;

    if (snapshot_init() != SNAPSHOT_OK)
    {
        UE_LOG(LogSnapshot, Error, TEXT("Snapshot SDK failed to initalize!"));
        return;
    }

    UE_LOG(LogSnapshot, Display, TEXT("Snapshot SDK initialized"));

    m_initialized_sdk = true;

    // Perform snapshot allocations through the unreal allocator instead of default malloc/free
    snapshot_allocator(&FSnapshotModule::Malloc, &FSnapshotModule::Free);

    // Setup logging to go to the "Snapshot" log category
    snapshot_log_function(&FSnapshotModule::Log);

    CreateSnapshotSocketSubsystem();
}

void FSnapshotModule::ShutdownModule()
{
    UE_LOG(LogSnapshot, Display, TEXT("Snapshot Plugin shutting down"));

    DestroySnapshotSocketSubsystem();

    snapshot_term();

    m_initialized_sdk = false;

    UE_LOG(LogSnapshot, Display, TEXT("Snapshot Plugin unloaded"));
}

void FSnapshotModule::Log(int level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    FString Message = FString(buffer);

    switch (level)
    {
    case SNAPSHOT_LOG_LEVEL_ERROR:
        UE_LOG(LogSnapshot, Error, TEXT("%s"), *Message);
        break;
    case SNAPSHOT_LOG_LEVEL_WARN:
        UE_LOG(LogSnapshot, Warning, TEXT("%s"), *Message);
        break;
    case SNAPSHOT_LOG_LEVEL_INFO:
        UE_LOG(LogSnapshot, Display, TEXT("%s"), *Message);
        break;
    default:
        UE_LOG(LogSnapshot, Display, TEXT("%s"), *Message);
        break;
    }
}

void* FSnapshotModule::Malloc(void* context, size_t size)
{
    return FMemory::Malloc(size);
}

void FSnapshotModule::Free(void* context, void* src)
{
    return FMemory::Free(src);
}
