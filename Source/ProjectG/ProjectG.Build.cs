// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class ProjectG : ModuleRules
{
	public ProjectG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);

        PrivateIncludePaths.Add(EnginePath + "Source/Runtime/Online/Voice/Private");
        PrivateIncludePaths.Add(EnginePath + "Source/Runtime/Online/Voice/Private/Windows");

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
                                                            "GameplayAbilities", "GameplayTags", "GameplayTasks",
                                                            "UMG", "Slate", "SlateCore",
                                                            "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemSteam",
                                                            "AIModule", "NavigationSystem", "NetCore", "Niagara",
                                                            "Voice", "AudioMixer", "AudioCapture", "AudioCaptureCore",
                                                            "ChaosCaching", "GeometryCollectionEngine", "PhysicsCore",
                                                            "LevelSequence", "MovieScene", "MovieSceneTracks", "MediaAssets" });


        PrivateDependencyModuleNames.AddRange(new string[] { "Networking", "Steamworks" });

		PublicIncludePaths.AddRange(new string[] { "ProjectG" });

//        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
