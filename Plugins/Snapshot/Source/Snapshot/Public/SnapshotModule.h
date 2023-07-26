/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSnapshot, Display, Display);

#ifndef SNAPSHOT_SUBSYSTEM
#define SNAPSHOT_SUBSYSTEM FName(TEXT("SNAPSHOT"))
#endif

class FSnapshotModule : public IModuleInterface
{
public:

    /** Called right after the module DLL has been loaded and the module object has been created */
    virtual void StartupModule() override;
    
    /** Called before the module is unloaded, right before the module object is destroyed */
    virtual void ShutdownModule() override;

    /** This plugin does not support dynamic reloading */
    virtual bool SupportsDynamicReloading() override {
        return false;
    };

private:

    /** The static handler for logs from the Snapshot SDK */
    static void Log(int level, const char* format, ...);

    /** Static handlers for memory allocations from the Snapshot SDK */
    static void* Malloc(void* context, size_t size);
    static void Free(void* context, void* src);

    /** Have we initialized the Snapshot SDK? */
    bool m_initialized_sdk = false;
};
