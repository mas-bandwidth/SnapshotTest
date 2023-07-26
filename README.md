# Snapshot Test

This is a test project for the Snapshot Unreal Engine Plugin.

To use the plugin in your own projects, copy the entire "Plugins/Snapshot" directory into your own project under "Plugins", then modify DefaultEngine.ini adding this at the bottom:

```
  [/Script/Engine.Engine]
  !NetDriverDefinitions=ClearArray
  +NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/Snapshot.SnapshotNetDriver",DriverClassNameFallback="/Script/Snapshot.SnapshotNetDriver")
  
  [/Script/Snapshot.SnapshotNetDriver]
  NetConnectionClassName="/Script/Snapshot.SnapshotConnection"
```
