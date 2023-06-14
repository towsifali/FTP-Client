#include<iostream>
#include<string>
#include<WS2tcpip.h>
#include<fstream>
#include<sstream>
#include<typeinfo>
#pragma comment(lib, "WS2_32.lib")
#define	fileSize 500
#define bufferSize 4096

using namespace std;

struct sockaddr_in server_address;

string ChartoString(char* c_arr, int n) { //Function for converting Char Array to String
	string str = "";
	for (int i = 0; i < n; i++)
		str += c_arr[i];
	return str;
}

int main() {

	string ipAddress;
	string str_port;
	int port = 16000;

	//Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);

	if (wsResult != 0) {
		cerr << "Error, failed to start WSADATA" << wsResult << endl;
		return -1;
	}
	 
	//Task 1: Socket initialization
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		cerr << "Socket was creation was failed. Error #" << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	//Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	
	//Task 2: Initiate the connection request
	//Getting inputs for ip address and port number
	bool check = true;
	do {
		cout << "Enter IP Adress: ";
		getline(cin, ipAddress);

		if (inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr) <= 0) {
			cout << "Invalid address/not supported" << endl;
			cout << "Enter again:" << endl;
			check = true;
			continue;
		}
		hint.sin_port = htons(port);
		cout << "IP Address: " << ipAddress << "    " << "Port Number: " << port << endl;
		int connRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
		if (connRes == SOCKET_ERROR) {
			cerr << "Connect status: FAIL" << endl;
			check = true;
		}
		else {
			cout << "Connect status: SUCCESS" << endl;
			check = false;
		}
	} while (check);

	//declaration for handling instructions
	char buf[4096];
	string userInput;
	bool exit = false;

	//Task 3: Send request to the file server, and receive content from the file server and save the file
	do {
		//getting instructions
		cout << "Enter instruction: ";
		getline(cin, userInput);
		string servermsg = ""; //Handling Server Messages


		//Separating the Command and the File Name
		string command = "";
		string fileName = "";
		bool isSpace = false;
		for (auto x : userInput) {
			if (x == ' ') {
				isSpace = true;
				continue;
			}
			if (!isSpace) command += x;
			else fileName += x;
		}

		//sending message to server
		send(sock, userInput.c_str(), userInput.size(), 0);
		
		//Receiving the message from the server
		char rcvText[bufferSize];
		memset(rcvText, 0, bufferSize);
		int len;
		len = recv(sock, rcvText, bufferSize, 0);	//rcvText gets message from server, len gets the number of bytes of the message
		servermsg = ChartoString(rcvText, strlen(rcvText)); //servermsg gets message from server as string 
		servermsg = ChartoString(rcvText, strlen(rcvText)); //servermsg gets message from server as string 
		
		//Extracting the first word from the server message
		string firstword = "";
		isSpace = false;
		for (auto x : servermsg) {
			if (x == ' ') {
				isSpace = true;
				continue;
			}
			if (!isSpace) firstword += x;
		}

		//handling errors in input instruction
		if (firstword.compare("ERROR:") == 0) {
			if (servermsg.compare("ERROR: the file does not exist") == 0) cout << "FAIL--the file does not exist" << endl;
			else if (servermsg.compare("ERROR: the file is there") == 0) cout << "SUCCESS --the file is there" << endl;
			else if (servermsg.compare("ERROR: to many arguments") == 0) cout << "ERROR: too many arguments" << endl;
			else cout << servermsg << endl;
		}
		else {
			if (command.compare("DOWNLOAD") == 0) {//Handling DOWNLOAD instruction
				ofstream fout;
				fout.open(fileName.c_str());
				if (fout.is_open()) {
					cout << "FILE OPEN: SUCCESS" << endl;
				}
				else {
					cout << "FILE OPEN: FAIL" << endl;
					continue;
				}

				string temp;
				if (len > 0) {					
					memset(rcvText, 0, bufferSize);
					recv(sock, rcvText, bufferSize, 0);
					send(sock, "ACK", 3, 0);
					while (ChartoString(rcvText, strlen(rcvText)) != "#") {
						fout << rcvText;
						cout << "Received text: " << rcvText;
						memset(rcvText, 0, bufferSize);
						recv(sock, rcvText, bufferSize, 0);
						send(sock, "ACK", 3, 0);
					}
					cout<< "EOF Received: " << rcvText << endl;
					cout << "Save text status: SUCCESS"<<endl;
				}
				else {
					cout << "Did not receive any text" << endl;
				}
				servermsg.clear();
				//closing text file
				fout.close();
			}
			else if (command.compare("UPLOAD") == 0) { //Handling UPLOAD instruction
				ifstream fin;
				fin.open(fileName.c_str());
				if (fin.is_open()) {
					cout << "FILE OPEN: SUCCESS" << endl;
					string Line;
					getline(fin, Line);
					Line += '\n';
					send(sock, Line.c_str(), Line.size(), 0);
					cout << "Sent: " << Line;
					Line.clear();
					string text;
					char recT[bufferSize];
					for (string Line; getline(fin, Line);) {
						memset(recT, 0, bufferSize);
						recv(sock, recT, bufferSize, 0);
						stringstream buffer(recT);
						getline(buffer, text);
						if (text.compare("ACK") == 0) {
							Line += '\n';
							send(sock, Line.c_str(), Line.size(), 0);
							cout << "Sent: " << Line;
						}
					}
					send(sock, "#\n", 2, 0);
					memset(recT, 0, bufferSize);
					recv(sock, recT, bufferSize, 0);
					stringstream buffer(recT);
					getline(buffer, text, '\n');
					if (text.compare("ACK") == 0) {
						cout << "Sent: EOF" << endl;
					}
					
					recv(sock, recT, bufferSize, 0);//ACK for file saved
				}
				else {
					cout << "FILE OPEN: FAIL" << endl;
				}
				servermsg.clear();
				fin.close(); //Closing the file
			}
			else if (command.compare("RETRIEVE") == 0) {//Handling the RETRIEVE Command
				cout << servermsg << endl;
				servermsg.clear();
			}
			else if (userInput.compare("exit") == 0) { //Handling the exit Command
				cout << servermsg;
				exit = true;
			}
		}
	} while (!exit);

	//Task 4: Close socket
	closesocket(sock);
	WSACleanup();

	return 0;
}