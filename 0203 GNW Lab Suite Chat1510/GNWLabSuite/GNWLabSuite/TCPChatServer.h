#pragma once
#include "ChatLobby.h"//Interface With Game Functionality
#include "NetDefines.h"
#include <stdint.h>//specifided data length types included (uint16_t)
#include <map>
#include <vector>

class TCPChatServer
{
	ChatLobby& chat_interface;//For making calls to add/remove users and display text
	FD_SET masterSocket;
	//SOCKET m_serverSocket;
	SOCKET m_listenSocket;
	sockaddr_in m_sockAdd;
	tcpclient m_clients[4];
	bool stopit;
	int clientCount = 0;
public:
	TCPChatServer(ChatLobby& chat_int);
	~TCPChatServer(void);
	//Establishes a listening socket, Only Called once in Separate Server Network Thread, Returns true aslong as listening can and has started on the proper address and port, otherwise return false
	bool init(uint16_t port);
	//Recieves data from clients, parses it, responds accordingly, Only called in Separate Server Network Thread, Will be continuously called until return = false;
	bool run(void);
	//Notfies clients of close & closes down sockets, Can be called on multiple threads
	bool stop(void);
};

