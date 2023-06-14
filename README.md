# FTP-Client

Program was written using C++ on Microsoft Visual Studio 2022

## How to run/test the Client

1. Compile the Client.cpp file (or run the executable file instead). It will start the Client Socket and open a prompt window. Start the server.py provided in the Server folder.

2. The program will ask for IP Address (in dotted format). If you enter the wrong input, an error will be displayed in the window and you will be asked to enter the IP Address again. Otherwise, a TCP connection between the client and the server will be set-up.

3. After that, the program will ask for an instruction. Two words are supposed to be entered for every instruction except "exit". The first one is the command in block letters (UPLOAD, DOWNLOAD or RETRIEVE) and the second one is the file name (for e.g. abc.text). For "exit" instruction, just type "exit" (one word with small letters) and enter. If the wrong instruction is entered, the window will display the relevant message and will ask you to input the instruction again.

4. Each instruction will behave in a different way:

	* UPLOAD: The file to be uploaded should be in the same folder as the 56584862_56275217.cpp. Once the correct instruction is given, the window will display each line of the file that is sent. This file will be saved on the server. If the file is already on the server, the window will display a message explaining the situation.

	* DOWNLOAD: If the file mentioned in the correct instruction is not saved on the server, the window will display an error message. Otherwise the file will be downloaded and saved in the same folder as 56584862_56275217.cpp file. Each line of the file will be displayed on the window.

	* RETRIEVE: If the file mentioned in the correct instruction is not saved on the server, the window will display "NO". Otherwise, it will display "YES."
    
	* exit: Connection will be ended, socket will be closed and the program will be terminated.

The program will keep asking for instructions until the "exit" instruction is given.