# GNW_Pokemon_Chat_App:
This tech application is intended to introduce network related changes into an environment that is graphically running in real time.
Process:
In this demo, I started by building the behavior of client and server module, where the server creates a socket, binds it, sets up the listening queue and waits  for invitation being accepted by client. The client on the other side creates a socket, and connects with the server. Once both of them are connected, I go on to develop the functionality of receiving and sending message on chat GUI. And finally when one of them decided to leave I make sure that the socket is being closed, and all active methods are terminated gracefully on both ends.
Instructions:
Open two windows, one for the client and the other for the server.
Host the server, and then connect the client.
Once both connected, enjoy the experience of chatting.
