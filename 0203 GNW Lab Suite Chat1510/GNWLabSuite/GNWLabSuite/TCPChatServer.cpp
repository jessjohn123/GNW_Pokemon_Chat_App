#include "TCPChatServer.h"

TCPChatServer::TCPChatServer(ChatLobby& chat_int) : chat_interface(chat_int)
{
	stopit = false;
}
TCPChatServer::~TCPChatServer(void)
{

}
bool TCPChatServer::init(uint16_t port)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_clients[i].m_socket = 0;
		m_clients[i].ID = 99;
	}
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET)
		return false;

	m_sockAdd.sin_family = AF_INET;
	m_sockAdd.sin_port = htons(port);
	m_sockAdd.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (bind(m_listenSocket, (sockaddr*)&m_sockAdd, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		return false;
	}

	if (listen(m_listenSocket, 1) == SOCKET_ERROR)
	{
		return false;
	}

	//m_serverSocket = accept(m_listenSocket, NULL, NULL); Do it inside 4 loop 
	// in the run

	FD_ZERO(&masterSocket);
	FD_SET(m_listenSocket, &masterSocket);
	
	
	return true;
}
bool TCPChatServer::run(void)
{
	if (stopit)
	{
		return false;
	}
	char clBuffer[MAX_CLIENTS];
	int msgLen[MAX_CLIENTS];
	int numReady;
	SOCKET tempSocket;
	FD_SET readSet;
	FD_ZERO(&readSet);
	readSet = masterSocket;
	timeval timeout;
	int numClients = 0;
	

	numReady = select(0, &readSet, NULL, NULL,NULL);

	//if the listen socket is ready...
	if (clientCount >= 4)
		clientCount = 0;
	if (FD_ISSET(m_listenSocket, &readSet))
	{
		tempSocket = accept(m_listenSocket, NULL, NULL);
		if (tempSocket == INVALID_SOCKET)
			return false;
		FD_SET(tempSocket, &masterSocket);
		m_clients[clientCount].m_socket = tempSocket;
		return true;
	}

	//Process each ready client
	for (size_t i = 0; i < MAX_CLIENTS ; i++)
	{
		if (FD_ISSET(m_clients[i].m_socket, &readSet))
		{
			char* message;
			uint16_t readBytes;
			char name[17];
			bool no_sv_cl_close = false;

			int error_header = tcp_recv_whole(m_clients[i].m_socket, (char*)&readBytes, 2, 0);
			message = new char[readBytes];
			int error_message = tcp_recv_whole(m_clients[i].m_socket, message, readBytes, 0);

			NET_MESSAGE_TYPE msgType = (NET_MESSAGE_TYPE)message[0];

			if (msgType == cl_reg)
			{
				char username[MAX_NAME_LENGTH];
				strcpy(username, &message[1]);

				if (clientCount > 3)
				{
					char buffer[3];
					uint16_t Length = 1;
					memcpy(buffer, &Length, sizeof(uint16_t));
					buffer[2] = sv_full;
					send(m_clients[i].m_socket, buffer, 3, 0);
				}
				else
				{
					char buffer[4];
					uint16_t Length = 2;
					memcpy(buffer, &Length, sizeof(uint16_t));
					buffer[2] = sv_cnt;
					buffer[3] = i;
					send(m_clients[i].m_socket, buffer, 4, 0);
					clientCount++;

					m_clients[i].ID = i;
					strcpy(m_clients[i].name, username);

					for (int j = 0; j < MAX_CLIENTS; j++)
					{
						if (m_clients[j].ID == 99 || m_clients[j].ID == i)
						{
							continue;
						}

						char buffer[21];
						uint16_t length = 19;
						memcpy(buffer, &length, sizeof(uint16_t));
						buffer[2] = sv_add;
						buffer[3] = i;
						strcpy(&buffer[4], m_clients[i].name);
						send(m_clients[j].m_socket, buffer, 21, 0);
					}
				}
				return true;
			}

			if (msgType == cl_get)
			{
			
					char *buffer = new char[(18 * clientCount) + 4];
					uint16_t Length = (18 * clientCount) + 2;
					memcpy(buffer, &Length, sizeof(uint16_t));
					buffer[2] = sv_list;
					buffer[3] = clientCount;
					int index = 4;
					uint8_t ID;
					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						if (m_clients[i].m_socket == 0)
							continue;
						buffer[index] = m_clients[i].ID;
						index++;
						strcpy(&buffer[index], m_clients[i].name);
						index += 17;
					}
					send(m_clients[i].m_socket, buffer, ((18 * clientCount) + 4), 0);
			}

			if (msgType == sv_cl_msg)
			{
				char *buffer = new char[(readBytes + 2)];
				uint16_t Length = readBytes;
				char *terminatedChatMsg;
				memcpy(buffer, &Length, sizeof(uint16_t));
				buffer[2] = sv_cl_msg;
				buffer[3] = i;
				int index = 4;
				strcpy(&buffer[index], &message[2]);
				index++;

				for (int j = 0; j < MAX_CLIENTS; j++)
				{
					if (m_clients[j].m_socket == 0)
						continue;
					send(m_clients[j].m_socket, buffer, (readBytes + 2), 0);
				}
				
			}

			if (msgType == sv_cl_close)
			{
				//char *buffer = new char[(readBytes + 2)];
				char buffer[4];
				uint16_t length = 2;
				memcpy(buffer, &length, sizeof(uint16_t));
				buffer[2] = sv_remove;
				buffer[3] = message[1];
				//unsigned int id = (unsigned int)message[1];
				//int index = 4;
				//strcpy(&buffer[index], m_clients[i].name);
				//index++;
				for (int j = 0; j < MAX_CLIENTS; j++)
				{
					if (m_clients[j].m_socket == 0)
						continue;
					//if (id == j)
					if(m_clients[j].ID = message[1])
					{
						FD_CLR(m_clients[j].m_socket, &masterSocket);
						shutdown(m_clients[j].m_socket, SD_BOTH);
						closesocket(m_clients[j].m_socket);
						m_clients[j].m_socket = 0;
					}
					else
					{
						send(m_clients[j].m_socket, buffer, 4, 0);
					}
					
				}
				clientCount--;
			}
		}
	}

	return true;
}
bool TCPChatServer::stop(void)
{
	stopit = true;
	char buffer[4];
	uint16_t Length = 2;
	memcpy(buffer, &Length, sizeof(uint16_t));
	buffer[2] = sv_cl_close;
	buffer[3] = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_clients[i].m_socket == 0)
			continue;
		send(m_clients[i].m_socket, buffer, 4, 0);
		FD_CLR(m_clients[i].m_socket, &masterSocket);
		shutdown(m_clients[i].m_socket, SD_BOTH);
		closesocket(m_clients[i].m_socket);
		m_clients[i].m_socket = 0;
	}

	FD_ZERO(&masterSocket);
	shutdown(m_listenSocket, SD_BOTH);
	closesocket(m_listenSocket);
	m_listenSocket = 0;
	
	return true;
}