#include <chrono>
#include <thread>
#include <cstring>
#include <stdio.h>

#include "AudioSubsystem.h"
#include "AudioSubsystem_OpenAL.h"
#include "Decoders/iWaveDataProvider.h"
#include "Decoders/WAV/WAVDataProvider.h"
#include "Utils.h"
#include "Playlist.h"

const char* PORTAMP_VERSION = "0.99.9";

sConfig g_Config;
clPlaylist g_Playlist;

sConfig ReadConfigFromCommandLine( int argc, char* argv[] )
{
	sConfig Cfg;

	for ( int i = 1; i < argc; i++ )
	{
		if ( strstr( argv[i], "--loop" ) == argv[i] ) Cfg.m_Loop = true;
		else if ( strstr( argv[i], "--wav-modplug" ) == argv[i] ) Cfg.m_UseModPlugToDecodeWAV = true;
		else g_Playlist.EnqueueTrack( argv[i] );
	}

	return Cfg;
}

void PrintBanner()
{
	printf( "PortAMP version %s (%s)\n", PORTAMP_VERSION, __DATE__ " " __TIME__ " via " __COMPILER_VER__ " for " BUILD_OS );
	printf( "Copyright (C) 2015 Sergey Kosarevsky\n" );
	printf( "https://github.com/corporateshark/PortAMP\n" );
	printf( "\n" );
	printf( "portamp <filename> [--loop] [--wav-modplug]\n" );
	printf( "\n" );
}

int main( int argc, char* argv[] )
{
	if ( argc <= 1 )
	{
		PrintBanner();
		return 0;
	}

	g_Config = ReadConfigFromCommandLine( argc, argv );

	if ( g_Playlist.IsEmpty() ) g_Playlist.EnqueueTrack( "test.ogg" );

	auto AudioSubsystem = CreateAudioSubsystem_OpenAL();

	AudioSubsystem->Start();

	auto Source = AudioSubsystem->CreateAudioSource();

	bool RequestingExit = false;

	while ( !g_Playlist.IsEmpty() )
	{
		auto FileName = g_Playlist.GetAndPopNextTrack();
		auto DataBlob = ReadFileAsBlob( FileName.c_str() );
		auto Provider = CreateWaveDataProvider( FileName.c_str(), DataBlob );
		Source->BindDataProvider( Provider );
		Source->SetLooping( g_Config.m_Loop );
		Source->Play();

		while ( Source->IsPlaying() && !RequestingExit )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds(10) );

			if ( IsKeyPressed() ) RequestingExit = true;
		};

		Source->Stop();
	}

	Source = nullptr;

	AudioSubsystem->Stop();

	return 0;
};
