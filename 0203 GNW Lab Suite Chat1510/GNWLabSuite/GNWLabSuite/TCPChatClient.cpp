#include "TCPChatClient.h"
TCPChatClient::TCPChatClient(ChatLobby& chat_int) : chat_interface(chat_int)
{
	stopit = false;
}
TCPChatClient::~TCPChatClient(void)
{

}
bool TCPChatClient::init(std::string name, std::string ip_address, uint16_t port)
{
	//checking addr
	unsigned long cAddr = INADDR_NONE;
	cAddr = inet_addr(ip_address.c_str());
	if (cAddr == INADDR_ANY || cAddr == INADDR_NONE)
		return false;

	m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_clientSocket == INVALID_SOCKET)
		return false;

	sockaddr_in sockIn;
	sockIn.sin_family = AF_INET;
	sockIn.sin_port = htons(port);
	sockIn.sin_addr.S_un.S_addr = cAddr;

	int result = connect(m_clientSocket, (sockaddr*)&sockIn, sizeof(sockIn));

	if (result == SOCKET_ERROR)
	{
		return false;
	}

	char buffer[20];
	uint16_t length = 18;

	memcpy(buffer, &length, sizeof(uint16_t));
	buffer[2] = cl_reg;
	strcpy(&buffer[3], name.c_str());

	send(m_clientSocket, buffer, 20, 0);
	chat_interface.DisplayString("Client: I'm Registering!");
	return true;
}
bool TCPChatClient::run(void)
{
	char* message;
	short readBytes;
	char name[17];
	bool no_sv_cl_close = false;
	if (stopit)
	{
		return false;
	}

	int error_header = tcp_recv_whole(m_clientSocket, (char*)&readBytes, 2, 0);
	message = new char[readBytes];
	int error_message = tcp_recv_whole(m_clientSocket, message, readBytes, 0);

	NET_MESSAGE_TYPE msgType = (NET_MESSAGE_TYPE)message[0];
	if (msgType == sv_cnt)
	{
		memcpy(&cl_ID, &message[1], sizeof(uint8_t));
		char buffer[3];
		uint16_t Length = 1;
		memcpy(buffer, &Length, sizeof(uint16_t));
		buffer[2] = cl_get;
		send(m_clientSocket, buffer, 3, 0);
		chat_interface.DisplayString("Client: Trying to send request to the server to connect the new me! ");
		return true;
	}

	if (msgType == sv_full)
	{
		stop();
		no_sv_cl_close = true;
		chat_interface.DisplayString("Client: Disconnecting");
		return false;
	}

	if (msgType == sv_list)
	{
		uint8_t number = message[1];
		int index = 2;
		//memcpy(&number, &message[1], sizeof(uint8_t));
		uint8_t ID;
		for (int i = 0; i < number; i++)
		{
			ID = message[index];
			//memcpy(&ID, &message[index], sizeof(uint8_t));
			index++;
			strcpy(name, &message[index]);
			index += 17;
			//chat_interface.AddNameToUserList(name,(uint8_t)ID);
			chat_interface.AddNameToUserList(name,ID);
		}
		chat_interface.DisplayString("Client: Populating the list.");
		return true;
	}
	// once the client request the list from the server
	if (msgType == sv_add)
	{
		int index = 2;
		uint8_t ID = message[1];
		strcpy(name, &message[2]);
		chat_interface.AddNameToUserList(name, ID);
		chat_interface.DisplayString("Client: Adding the client.");
		return true;
	}

	if (msgType == sv_remove)
	{
		uint8_t ID = message[1];
		chat_interface.RemoveNameFromUserList(ID);
		chat_interface.DisplayString("Client: Removing the client.");
		return true;
	}

	if (msgType == sv_cl_msg)
	{
		uint8_t ID = message[1];
		chat_interface.AddChatMessage(&message[2], ID);
		chat_interface.DisplayString("Client: Displaying to the chatbox.");
		return true;
	}

	if (msgType == sv_cl_close)
	{
		stop();
		no_sv_cl_close = true;
		chat_interface.DisplayString("Client: Disconnecting.");
		return true;
	}

	return true;
}
bool TCPChatClient::send_message(std::string message)
{
	//int result = send(m_clientSocket, message.c_str(), sizeof(message), 0);
	

	char* buffer;
	int result;
	uint16_t length = message.size() + 3;
	buffer = new char[length + 2];
	memcpy(buffer, &length, sizeof(uint16_t));
	buffer[2] = sv_cl_msg;
	memcpy(&buffer[3], &cl_ID, sizeof(uint8_t));
	strcpy(&buffer[4], message.c_str());

	result = send(m_clientSocket, buffer, length + 2, 0);
	
	if (result == SOCKET_ERROR)
	{
	return false;
	}

	delete [] buffer;

	return true;
}
bool TCPChatClient::stop(void)
{
	stopit = true;
	char buffer[4];
	uint16_t Length = 2;
	memcpy(buffer, &Length, sizeof(uint16_t));
	buffer[2] = sv_cl_close;
	buffer[3] = cl_ID;
	send(m_clientSocket, buffer, 4, 0);

	shutdown(m_clientSocket, SD_BOTH);
	closesocket(m_clientSocket);
	return true;
}
