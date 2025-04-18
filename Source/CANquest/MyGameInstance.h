#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "MyGameInstance.generated.h"

/**
 *
 */

UCLASS()
class CANQUEST_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "TCP")
	void ConnectToServer(const FString& playerIpInput);

	UFUNCTION(BlueprintCallable, Category = "TCP")
	void DisconnectFromServer();

	UFUNCTION(BlueprintCallable, Category = "TCP")
	void SendMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "TCP")
	void OnConnected();

	UFUNCTION(BlueprintCallable, Category = "TCP")
	void OnConnectionError(const FString& Error);

	//UFUNCTION(BlueprintCallable, Category = "TCP")
	//void OnConnectionClosed();

	//UFUNCTION(BlueprintCallable, Category = "TCP")
	//void OnMessageReceived(const FString& MessageString); 

	UPROPERTY(BlueprintReadWrite, Category = "TCP")
	FString LatestMessage;

	UPROPERTY(BlueprintReadWrite, Category = "TCP")
	bool bIsConnected = false;              // Flag to determine if the socket is connected

	UPROPERTY(BlueprintReadWrite, Category = "Level")
    FString LevelSelection = "Level_Tutorial"; // Default level, can be modified in Blueprints

private:

	FSocket* Socket;

	//bool bMessageSent = false;  // Flag to ensure the message is only sent once

    bool bIsConnecting = false;             // Flag to determine if the socket is connecting

	// Reconnection-related variables
	bool bShouldReconnect = true;           // Determines if we should keep reconnecting
	float ReconnectInterval = 5.0f;         // Interval in seconds for reconnect attempts
	FTimerHandle ReconnectTimerHandle;      // Handle for managing reconnection timer

    FTimerHandle ReceiveTimerHandle;        // Handle for managing the receive timer

	// TCP configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TCP", meta = (AllowPrivateAccess = "true"))
	FString ServerIP = "0.0.0.0";      // Default IP, can be modified in Blueprints

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TCP", meta = (AllowPrivateAccess = "true"))
	int32 ServerPort = 8080;             // Default port, can be modified in Blueprints

	void CreateSocket();                 // Helper function to create the TCP socket instance with the specified IP and Port
	void ReceiveData();                  // Function to handle receiving data from the server

    void CheckConnectionStatus();        // Function to check the connection status
    FTimerHandle ConnectionCheckTimerHandle; // Handle for managing the connection check timer

	void HandleDisconnection(); // Function to handle server disconnect

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    int32 GraphicsIdx = 0;			// Index of the graphics settings

};