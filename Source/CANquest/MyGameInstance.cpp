// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "WebSocketsModule.h"

void UMyGameInstance::Init() {
	Super::Init(); // Allows for base logic to get executed

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets")) //If Web Socket Module not Loaded
	{
        FModuleManager::Get().LoadModule("WebSockets"); //Load
	}

	//WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://localhost:8080"); //Local IP and 8080 Port; can use specific IP

	// Create WebSocket using IP and Port
    CreateWebSocket(); // Needed before "WebSocket->" is used or game crashes

	WebSocket->OnConnected().AddUObject(this, &UMyGameInstance::OnWebSocketConnected);
	WebSocket->OnConnectionError().AddUObject(this, &UMyGameInstance::OnWebSocketConnectionError);
	WebSocket->OnClosed().AddUObject(this, &UMyGameInstance::OnWebSocketClosed);
	WebSocket->OnMessage().AddUObject(this, &UMyGameInstance::OnWebSocketMessageReceived);
	WebSocket->OnMessageSent().AddUObject(this, &UMyGameInstance::OnWebSocketMessageSent);



	// Initial attempt to connect
	ConnectToWebSocket();

    UE_LOG(LogTemp, Warning, TEXT("Game Instance Initialized"));
}

void UMyGameInstance::Shutdown()
{
	// Stop reconnect attempts on shutdown
	bShouldReconnect = false;
	GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);

	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();
	}

    UE_LOG(LogTemp, Warning, TEXT("Game Instance Shutdown"));
    Super::Shutdown(); // Allows for base logic to get executed after custom logic
}

void UMyGameInstance::CreateWebSocket()
{
	// Format the WebSocket URL with the IP and Port
	FString WebSocketURL = FString::Printf(TEXT("ws://%s:%d"), *WebSocketIP, WebSocketPort);
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(WebSocketURL);

    UE_LOG(LogTemp, Warning, TEXT("CreateWebSocket: WebSocket created with URL: %s"), *WebSocketURL);
}

void UMyGameInstance::ConnectToWebSocket()
{
	if (WebSocket.IsValid() && !WebSocket->IsConnected())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Attempting to connect....");
		WebSocket->Connect();
	}

    UE_LOG(LogTemp, Warning, TEXT("ConnectToWebSocket: Attempting to connect to WebSocket"));
}

void UMyGameInstance::OnWebSocketConnected()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Successfully connected");

	// Stop the reconnect attempts once connected
	bShouldReconnect = false;
	GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);
	//if (!bMessageSent) {
	//	SendMessage("This is a message from the UE5 Client!");
	//	bMessageSent = true;
	//}
    SendMessage("This is a message from the UE5 Client!");
	
    UE_LOG(LogTemp, Warning, TEXT("OnWebSocketConnected: Successfully connected to WebSocket"));
}

void UMyGameInstance::OnWebSocketConnectionError(const FString& Error)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Connection error: " + Error);

	// Start the reconnect attempts if there was an error connecting
	if (bShouldReconnect)
	{
		GetWorld()->GetTimerManager().SetTimer(ReconnectTimerHandle, this, &UMyGameInstance::ConnectToWebSocket, ReconnectInterval, false);
	}

    UE_LOG(LogTemp, Warning, TEXT("OnWebSocketConnectionError: Connection error: %s"), *Error);
}

void UMyGameInstance::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, bWasClean ? FColor::Green : FColor::Red, "Connection closed: " + Reason); //Green if clean, red if not clean
	
	bShouldReconnect = true; // Attempt to reconnect if disconnected

	// Attempt to reconnect if the connection closes unexpectedly
	if (bShouldReconnect)
	{
		GetWorld()->GetTimerManager().SetTimer(ReconnectTimerHandle, this, &UMyGameInstance::ConnectToWebSocket, ReconnectInterval, false);
	}
    UE_LOG(LogTemp, Warning, TEXT("OnWebSocketClosed: Connection closed with status code %d and reason: %s"), StatusCode, *Reason);
}

//void UMyGameInstance::AttemptReconnect()
//{
//	// Check if we should continue trying to reconnect
//	if (bShouldReconnect)
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Attempting to reconnect....");
//		ConnectToWebSocket();
//	}
//
//    UE_LOG(LogTemp, Warning, TEXT("AttemptReconnect: Attempting to reconnect to WebSocket"));
//}

void UMyGameInstance::DisconnectFromWebSocket()
{
	bShouldReconnect = false;  // Stop trying to reconnect if we're disconnecting intentionally
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
	GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);

    UE_LOG(LogTemp, Warning, TEXT("DisconnectFromWebSocket: Attempting to disconnect from WebSocket"));
}

void UMyGameInstance::SendMessage(const FString& Message)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Send(Message);
	}

    UE_LOG(LogTemp, Warning, TEXT("SendMessage: Attempting to send message: %s"), *Message);
}

void UMyGameInstance::OnWebSocketMessageReceived(const FString& MessageString)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Received message: " + MessageString);

    UE_LOG(LogTemp, Warning, TEXT("OnWebSocketMessageReceived: Received message: %s"), *MessageString);
}

void UMyGameInstance::OnWebSocketMessageSent(const FString& MessageString)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Sent message: " + MessageString);

    UE_LOG(LogTemp, Warning, TEXT("OnWebSocketMessageSent: Sent message: %s"), *MessageString);
}