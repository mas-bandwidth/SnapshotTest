/*
    Snapshot Copyright © 2023 Mas Bandwidth LLC. This source code is licensed under GPL version 3 or any later version.
    Commercial licensing under different terms is available. Please email licensing@mas-bandwidth.com for details.
*/

using UnrealBuildTool;

public class Snapshot : ModuleRules
{
    public Snapshot(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDefinitions.Add("SNAPSHOT_UNREAL_ENGINE=1");

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "OnlineSubsystemUtils",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Projects",
                "CoreUObject",
                "Engine",
                "Sockets",
                "OnlineSubsystem",
                "PacketHandler",
            }
        );

        PrivateIncludePaths.AddRange(new string[] { "Snapshot/Private/include", "Snapshot/Private/sodium" });

        bEnableUndefinedIdentifierWarnings = false;
        
        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicFrameworks.Add("SystemConfiguration");
        } 
        else if (Target.Platform.ToString() == "PS4")
        {
            string SDKDir = System.Environment.GetEnvironmentVariable("SCE_ORBIS_SDK_DIR");
            string LibDir = System.IO.Path.Combine(SDKDir, "target", "lib");
            PublicAdditionalLibraries.Add(System.IO.Path.Combine(LibDir, "libSceSecure.a"));
        }
    }
}
