// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CANQUEST_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

//public: 
//
//	virtual void Init() override; // When Game opens
//	virtual void Shutdown() override;	//When Game closes
//
//private: 
//	TSharedPtr<IWebSocket> WebSocket; //Variable UE uses to communicate with WS server
//	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void ConnectToWebSocket();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void DisconnectFromWebSocket();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void OnWebSocketConnected();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void OnWebSocketConnectionError(const FString& Error);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void OnWebSocketMessageReceived(const FString& MessageString);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void OnWebSocketMessageSent(const FString& MessageString);

private:
	TSharedPtr<IWebSocket> WebSocket;

	bool bMessageSent = false;  // Flag to ensure the message is only sent once

	// Reconnection-related variables
	bool bShouldReconnect = true;           // Determines if we should keep reconnecting
	float ReconnectInterval = 5.0f;         // Interval in seconds for reconnect attempts
	FTimerHandle ReconnectTimerHandle;      // Handle for managing reconnection timer

	// WebSocket configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket", meta = (AllowPrivateAccess = "true"))
	FString WebSocketIP = "192.168.56.101";      // Default IP, can be modified in Blueprints

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket", meta = (AllowPrivateAccess = "true"))
	int32 WebSocketPort = 8080;             // Default port, can be modified in Blueprints

	//void AttemptReconnect();                // Function to handle the reconnect logic
	void CreateWebSocket();                 // Helper function to create the WebSocket instance with the specified IP and Port
};
