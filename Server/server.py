import time
from socketserver import BaseRequestHandler, TCPServer
from socket import IPPROTO_TCP, TCP_NODELAY


SENDING_COOLDOWN = 0.1
BUFFER_SIZE = 4096


def timestamp():
    return '[{}]'.format(time.strftime('%d-%m-%Y %H:%M:%S',time.localtime(int(round(time.time()*1000))/1000)))


def _extract_filename(filepath):
    if '/' in filepath:
        return filepath.split('/')[-1]
    elif '\\\\' in filepath:
        return filepath.rsplit('\\\\')[-1]
    return filepath


class EchoHandler(BaseRequestHandler):
    uploading = False
    exiting = False
    filename = None
    line_buffer = []
    saved_files = []

    def handle(self):
        print(timestamp(), 'Got connection from', self.client_address)
        while True:
            try:
                msg = self.request.recv(BUFFER_SIZE)
                if not msg: break                
                msg_str = str(msg, encoding='utf-8')
                if self.uploading:
                    if msg_str != '#\n':
                        print(timestamp(), 'Line received:', msg_str.rstrip())
                        self.line_buffer.append(msg_str)
                    else:
                        print(timestamp(), 'EOF received:', msg_str.rstrip())

                        # save the received file
                        f = open(self.filename, 'w')
                        f.writelines(self.line_buffer)
                        print(timestamp(), 'Saving to {}'.format(self.filename))
                        self.saved_files.append(self.filename)
                        f.close()

                        # reset file buffer
                        self.filename = None
                        self.uploading = False
                        self.line_buffer = []
                    self.request.send(b'ACK')
                    print(timestamp(), 'Sending ACK')
                    continue

                msg_pieces = msg_str.split()
                command = msg_pieces[0]
                if command in ['UPLOAD', 'DOWNLOAD', 'RETRIEVE']:
                    if len(msg_pieces) < 2:
                        print(timestamp(), 'ERROR: please provide file name')
                        self.request.send(b'ERROR: please provide file name'); time.sleep(SENDING_COOLDOWN)
                    elif len(msg_pieces) > 2:
                        print(timestamp(), 'ERROR: to many arguments')
                        self.request.send(b'ERROR: to many arguments'); time.sleep(SENDING_COOLDOWN)
                    elif command == 'UPLOAD':
                        self.filename = _extract_filename(msg_pieces[1])
                        if self.filename in self.saved_files:
                            print(timestamp(), 'ERROR: the file is there')
                            self.request.send(b'ERROR: the file is there'); time.sleep(SENDING_COOLDOWN)
                        else:
                            self.request.send(b'SUCCESS'); time.sleep(SENDING_COOLDOWN)
                            self.uploading = True
                    elif command == 'DOWNLOAD':
                        self.filename = _extract_filename(msg_pieces[1])
                        if self.filename not in self.saved_files:
                            print(timestamp(), 'ERROR: the file does not exist')
                            self.request.send(b'ERROR: the file does not exist'); time.sleep(SENDING_COOLDOWN)
                        else:
                            self.request.send(b'SUCCESS'); time.sleep(SENDING_COOLDOWN)
                            # send the required file to client
                            f = open(self.filename, 'r')
                            content = f.readlines()
                            for line in content:
                                self.request.send(bytes(line, encoding='utf-8'))
                                print(timestamp(), 'Sending line:', line.rstrip())
                                ack = self.request.recv(BUFFER_SIZE)
                                ack_str = str(ack, encoding='utf-8')
                                print(timestamp(), 'Receive ACK for line: {}'.format(line.rstrip()))
                            self.request.send(b'#')
                            print(timestamp(), 'Sending EOF')
                            ack = self.request.recv(BUFFER_SIZE)
                            ack_str = str(ack, encoding='utf-8')
                            print(timestamp(), 'Receive ACK for EOF')
                    elif command == 'RETRIEVE':
                        self.filename = _extract_filename(msg_pieces[1])
                        if self.filename in self.saved_files:
                            self.request.send(b'YES'); time.sleep(SENDING_COOLDOWN)
                        else:
                            self.request.send(b'NO'); time.sleep(SENDING_COOLDOWN)
                elif command == 'exit':
                    exiting = True
                    print(timestamp(), 'TCP connection closed')
                    self.request.send(b'TCP connection closed'); time.sleep(SENDING_COOLDOWN)
                else:
                    print(timestamp(), 'ERROR: unknown command')
                    self.request.send(b'ERROR: unknown command'); time.sleep(SENDING_COOLDOWN)
            except Exception:
                pass


if __name__ == '__main__':
    serv = TCPServer(('', 16000), EchoHandler)
    serv.serve_forever()
