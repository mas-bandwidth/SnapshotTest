# Snapshot Test

This is a test project for the Snapshot Unreal Engine Plugin.

To use the plugin in your own project, copy the entire "Plugins/Snapshot" directory into your own project, then add this to the bottom of DefaultEngine.ini:

```
  [/Script/Engine.Engine]
  !NetDriverDefinitions=ClearArray
  +NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/Snapshot.SnapshotNetDriver",DriverClassNameFallback="/Script/Snapshot.SnapshotNetDriver")
  
  [/Script/Snapshot.SnapshotNetDriver]
  NetConnectionClassName="/Script/Snapshot.SnapshotConnection"
```

The snapshot net driver is now active and will take over sending and receiving UDP packets between the client and server.

You can verify snapshot is active by looking at the log category "LogSnapshot":

<img width="2560" alt="screenshot" src="https://github.com/mas-bandwidth/SnapshotTest/assets/696656/548d7f60-31f8-4823-b90a-770b470b57f7">
