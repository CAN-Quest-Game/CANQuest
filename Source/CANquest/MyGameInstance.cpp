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
	//CreateSocket();
	//ConnectToServer();
	// Start a timer to regularly call ReceiveData
	if (Socket && Socket->GetConnectionState() == SCS_Connected)
	{
		GetWorld()->GetTimerManager().SetTimer(ReceiveTimerHandle, this, &UMyGameInstance::ReceiveData, 0.01f, true);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Receive Data should have been called");
	}

	// Start monitoring connection status
    GetWorld()->GetTimerManager().SetTimer(ConnectionCheckTimerHandle, this, &UMyGameInstance::CheckConnectionStatus, 1.0f, true);

	UE_LOG(LogTemp, Warning, TEXT("Game Instance Initialized"));
}

void UMyGameInstance::CheckConnectionStatus()
{
	if (Socket)
	{
		ESocketConnectionState ConnectionState = Socket->GetConnectionState();

		if (ConnectionState != SCS_Connected)
		{
			if (bIsConnected) // Only log when the status changes
			{
				UE_LOG(LogTemp, Warning, TEXT("Lost connection to the server!"));
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Lost connection to the server!");

				bIsConnected = false;

				// Clean up the socket if disconnected
				HandleDisconnection();
			}
		}
		//else
		//{
		//	if (!bIsConnected) // Only log when the status changes
		//	{
		//		UE_LOG(LogTemp, Warning, TEXT("Reconnected to the server!"));
		//		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Reconnected to the server!");

		//		bIsConnected = true;
		//	}
		//}
	}
}

void UMyGameInstance::HandleDisconnection()
{
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}

	// Optionally, notify other parts of the game
	UE_LOG(LogTemp, Warning, TEXT("Socket cleaned up after disconnection."));
}



void UMyGameInstance::Shutdown()
{
	// Stop reconnect attempts on shutdown
	bShouldReconnect = false;
	//GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);

	GetWorld()->GetTimerManager().ClearTimer(ConnectionCheckTimerHandle); // Clear connection check timer

	bIsConnected = false; // Update the connection status

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

void UMyGameInstance::ConnectToServer(const FString& playerIpInput)
{
	// Prevent re-entrance if a connection attempt is already in progress
	if (bIsConnected || bIsConnecting)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Already connected or connecting...");
		return;
	}

	bIsConnecting = true; // Mark that we are trying to connect

    // Check if the IP address is empty

	if (playerIpInput.IsEmpty()) {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "No IP Address entered");
        bIsConnecting = false;
        return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Connecting to Server...");

	// Ensure proper socket handling
	if (Socket)
	{
		if (Socket->GetConnectionState() == SCS_Connected)
		{
			Socket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		}
		Socket = nullptr;
	}

    CreateSocket();

	if (!Socket)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Failed to create socket");
		bIsConnecting = false;
		return;
	}

	// Set up the address
	FIPv4Address IP;
	if (!FIPv4Address::Parse(playerIpInput, IP))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Invalid IP Address");
		bIsConnecting = false;
		return;
	}

	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Addr->SetIp(IP.Value);
	Addr->SetPort(ServerPort);

	// Attempt connection
	bIsConnected = Socket->Connect(*Addr);
	bIsConnecting = false; // Reset connecting flag

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
	bShouldReconnect = false;  // Stop trying to reconnect if we're disconnecting intentionally

	bIsConnected = false; // Update the connection status

    bIsConnecting = false; // Reset the connecting flag

	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
        Socket = nullptr;

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Disconnected from Server");
		UE_LOG(LogTemp, Warning, TEXT("DisconnectFromServer: Disconnected from server"));
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Not connected to a server");
	}
	//GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);
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
	//GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);

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

    bIsConnected = false; // Update the connection status

	// Start the reconnect attempts if there was an error connecting
	/*if (bShouldReconnect)
	{
		GetWorld()->GetTimerManager().SetTimer(ReconnectTimerHandle, this, &UMyGameInstance::ConnectToServer, ReconnectInterval, false);
	}*/

	UE_LOG(LogTemp, Warning, TEXT("OnConnectionError: Connection error: %s"), *Error);
}

//void UMyGameInstance::OnConnectionClosed()
//{
//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Connection closed");
//
//	bShouldReconnect = true; // Attempt to reconnect if disconnected
//
//    bIsConnected = false; // Update the connection status
//
//	// Attempt to reconnect if the connection closes unexpectedly
//	/*if (bShouldReconnect)
//	{
//		GetWorld()->GetTimerManager().SetTimer(ReconnectTimerHandle, this, &UMyGameInstance::ConnectToServer, ReconnectInterval, false);
//	}*/
//
//	UE_LOG(LogTemp, Warning, TEXT("OnConnectionClosed: Connection closed"));
//}


void UMyGameInstance::ReceiveData()
{
	if (Socket && Socket->GetConnectionState() == SCS_Connected)
	{
		TArray<uint8> ReceivedData;
		uint32 Size;

		// Check if there is pending data
		if (Socket->HasPendingData(Size))
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Receive Data has pending Data");
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
					// Update the latest message
					LatestMessage = ReceivedString;
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Latest Message: " + LatestMessage);

				}
			}
		}
	}
}

