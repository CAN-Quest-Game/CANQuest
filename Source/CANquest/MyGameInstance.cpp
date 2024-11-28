// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Networking.h"

void UMyGameInstance::Init()
{
	Super::Init();

	// Create and connect the TCP socket
	CreateSocket();
	ConnectToServer();
	// Start a timer to regularly call ReceiveData
	if (Socket && Socket->GetConnectionState() == SCS_Connected)
	{
		GetWorld()->GetTimerManager().SetTimer(ReceiveTimerHandle, this, &UMyGameInstance::ReceiveData, 0.01f, true);
	}

	UE_LOG(LogTemp, Warning, TEXT("Game Instance Initialized"));
}

void UMyGameInstance::Shutdown()
{
	// Stop reconnect attempts on shutdown
	bShouldReconnect = false;
	GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);

	// Close the socket if it's valid
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}

	UE_LOG(LogTemp, Warning, TEXT("Game Instance Shutdown"));
	Super::Shutdown();
}

void UMyGameInstance::CreateSocket()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Creating Socket...");
	// Create the socket
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

}

void UMyGameInstance::ConnectToServer()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Connecting to Server...");
	// Set up the address
	FIPv4Address IP;
	FIPv4Address::Parse(ServerIP, IP);
	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Addr->SetIp(IP.Value);
	Addr->SetPort(ServerPort);

	// Connect to the server
	bool bIsConnected = Socket->Connect(*Addr);
	if (bIsConnected)
	{
		OnConnected();
	}
	else
	{
		OnConnectionError(TEXT("Failed to connect to server"));
	}
}

void UMyGameInstance::DisconnectFromServer()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Disconnecting from Server...");
	bShouldReconnect = false;  // Stop trying to reconnect if we're disconnecting intentionally
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
	GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("DisconnectFromServer: Attempting to disconnect from server"));
}

void UMyGameInstance::SendMessage(const FString& Message)
{
	// Check if the socket is valid and connected
	if (Socket && Socket->GetConnectionState() == SCS_Connected)
	{
		// Convert the TCHAR string to UTF8
		FTCHARToUTF8 Converted(*Message);
		int32 Size = Converted.Length();
		int32 Sent = 0;

		// Send the message over the socket
		bool bIsSent = Socket->Send((uint8*)Converted.Get(), Size, Sent);

		// Check if the message was sent successfully
		if (bIsSent)
		{
			UE_LOG(LogTemp, Log, TEXT("SendMessage: Successfully sent message: %s"), *Message);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "SendMessage: Successfully sent message: " + Message);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SendMessage: Failed to send message: %s"), *Message);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "SendMessage: Failed to send message: " + Message);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SendMessage: Socket is not connected."));
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "SendMessage: Socket is not connected.");
	}
}

void UMyGameInstance::OnConnected()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Successfully connected");

	// Stop the reconnect attempts once connected
	bShouldReconnect = false;
	GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);

    int val = 1;
    if (val == 1)
    {
        SendMessage("Hello from UE4!");
        val = 0;

    }

	UE_LOG(LogTemp, Warning, TEXT("OnConnected: Successfully connected to server"));
}

void UMyGameInstance::OnConnectionError(const FString& Error)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Connection error: " + Error);

    bShouldReconnect = true; // Attempt to reconnect if there was an error connecting

	// Start the reconnect attempts if there was an error connecting
	if (bShouldReconnect)
	{
		GetWorld()->GetTimerManager().SetTimer(ReconnectTimerHandle, this, &UMyGameInstance::ConnectToServer, ReconnectInterval, false);
	}

	UE_LOG(LogTemp, Warning, TEXT("OnConnectionError: Connection error: %s"), *Error);
}

void UMyGameInstance::OnConnectionClosed()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Connection closed");

	bShouldReconnect = true; // Attempt to reconnect if disconnected

	// Attempt to reconnect if the connection closes unexpectedly
	if (bShouldReconnect)
	{
		GetWorld()->GetTimerManager().SetTimer(ReconnectTimerHandle, this, &UMyGameInstance::ConnectToServer, ReconnectInterval, false);
	}

	UE_LOG(LogTemp, Warning, TEXT("OnConnectionClosed: Connection closed"));
}

void UMyGameInstance::OnMessageReceived(const FString& MessageString)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Received message: " + MessageString);

	UE_LOG(LogTemp, Warning, TEXT("OnMessageReceived: Received message: %s"), *MessageString);
}

void UMyGameInstance::ReceiveData()
{
	if (Socket && Socket->GetConnectionState() == SCS_Connected)
	{
		TArray<uint8> ReceivedData;
		uint32 Size;

		// Check if there is pending data
		if (Socket->HasPendingData(Size))
		{
			// Limiting the amount of data to avoid overflow
			uint32 DataSize = FMath::Min(Size, 65507u);

			ReceivedData.SetNumUninitialized(DataSize);
			int32 Read = 0;

			// Receiving the data
			if (Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read))
			{
				// Ensure null-termination
				ReceivedData.Add(0);

				FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));
				if (!ReceivedString.IsEmpty())
				{
					OnMessageReceived(ReceivedString);
				}
			}
		}
	}
}

