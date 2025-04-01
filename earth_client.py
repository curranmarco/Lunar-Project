import socket
import random
import struct
import threading
from tkinter import messagebox

class MoonRoverTCPClient:
    def __init__(self, gui_app):
        self.gui = gui_app
        self.sock = None
        self.connected = False
        self.isn = random.randint(0, 2**32 - 1)
        self.ack_num = 0
        self.server_ip = "192.168.1.100"  
        self.server_port = 8080
        self.receive_thread = None
        self.source_port = random.randint(49152, 65535)

    def _calculate_checksum(self, data):
        """Calculate 16-bit one's complement checksum """
        total = 0
        for i in range(0, len(data), 2):
            if i + 1 < len(data):
                word = (data[i] << 8) + data[i + 1]
            else:
                word = (data[i] << 8)  # Pad with zero if odd length
            total += word
            total = (total & 0xffff) + (total >> 16)  
        return ~total & 0xffff  

    def _create_pseudo_header(self, tcp_length):
        """Create TCP pseudo-header for checksum calculation"""
        return struct.pack(
            '!4s4sHH',
            socket.inet_aton('0.0.0.0'), 
            socket.inet_aton(self.server_ip),  
            socket.IPPROTO_TCP,  
            tcp_length  
        )

    def _create_tcp_header(self, flags, seq_num, ack_num=0, data=b''):
        """Create TCP header with specified flags and sequence numbers"""
        
        header = struct.pack(
            '!HHIIBBHHH',
            self.source_port,  
            self.server_port, 
            seq_num,          
            ack_num,       
            (5 << 4),       
            flags,           
            1024,            
            0,               
            0                
        ) + data

        
        pseudo_header = self._create_pseudo_header(len(header))
        checksum = self._calculate_checksum(pseudo_header + header)

        
        return struct.pack(
            '!HHIIBBHHH',
            self.source_port,
            self.server_port,
            seq_num,
            ack_num,
            (5 << 4),
            flags,
            1024,
            checksum,
            0
        ) + data

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            
           
            self.sock.bind(('0.0.0.0', self.source_port))
            
            
            self.sock.connect((self.server_ip, self.server_port))
            
            
            syn_packet = self._create_tcp_header(0b00000010, self.isn)
            self.sock.send(syn_packet)
            self.gui._log(f"Sent SYN (seq={self.isn})")

           
            syn_ack = self.sock.recv(1024)
            if len(syn_ack) < 20:
                raise ConnectionError("Invalid SYN-ACK response length")
            
            
            syn_ack_flags = struct.unpack_from('!B', syn_ack, 13)[0]
            if (syn_ack_flags & 0b00010010) != 0b00010010:  # Check SYN and ACK flags
                raise ConnectionError("Invalid SYN-ACK flags")
            
           
            server_isn = struct.unpack_from('!I', syn_ack, 4)[0]
            self.gui._log(f"Received SYN-ACK (seq={server_isn})")

          
            self.ack_num = server_isn + 1
            ack_packet = self._create_tcp_header(0b00010000, self.isn + 1, self.ack_num)
            self.sock.send(ack_packet)
            self.gui._log(f"Sent ACK (ack={self.ack_num})")

            self.connected = True
            self.gui._log("TCP connection established")
            
            
            self.receive_thread = threading.Thread(target=self._receive_data, daemon=True)
            self.receive_thread.start()
            
            return True
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"TCP connection failed: {str(e)}")
            self.disconnect()
            return False

    def disconnect(self):
        if self.connected:
            try:
               
                fin_packet = self._create_tcp_header(0b00000001, self.isn + 1)
                self.sock.send(fin_packet)
                self.gui._log("Sent FIN")
                
               
                fin_ack = self.sock.recv(1024)
                if len(fin_ack) >= 20:
                    fin_ack_flags = struct.unpack_from('!B', fin_ack, 13)[0]
                    if (fin_ack_flags & 0b00010000):  # Check ACK flag
                        self.gui._log("Received FIN-ACK")
            except:
                pass
            
            self.connected = False
            if self.sock:
                self.sock.close()
            self.gui._log("TCP connection closed")

    def send_command(self, command_type, *args):
        if not self.connected:
            messagebox.showerror("Error", "Not connected to server!")
            return False
        
        try:
            
            if command_type == "MOVE":
                cmd_str = f"MOVE {args[0]} {args[1]} {args[2]}"
            elif command_type == "LOCATION":
                cmd_str = f"LOCATION {args[0]}"
            else:
                raise ValueError("Invalid command type")
            
           
            packet = self._create_tcp_header(
                flags=0b00001000,  # PSH flag
                seq_num=self.isn + 1,
                ack_num=self.ack_num,
                data=cmd_str.encode()
            )
            
            self.sock.send(packet)
            self.gui._log(f"Sent command: {cmd_str}")
            return True
        except Exception as e:
            messagebox.showerror("Error", f"Failed to send command: {str(e)}")
            return False

    def _receive_data(self):
        while self.connected:
            try:
                data = self.sock.recv(1024)
                if not data:
                    break
                   
                if len(data) >= 20:  
                    header = data[:20]
                    (src_port, dst_port, seq_num, ack_num, 
                     offset_reserved_flags, window_size, 
                     checksum, urgent_ptr) = struct.unpack('!HHIIBBHHH', header)
                    flags = offset_reserved_flags & 0b00111111
                    payload = data[20:]
                    if payload:
                        message = payload.decode().strip()
                        self.gui._log(f"Received: {message}")
                        
                       
                        if message.startswith("LOCATION:"):
                            try:
                                loc_id = int(message.split()[1])
                                self.gui._update_location(loc_id)
                            except (IndexError, ValueError):
                                pass
                    if flags & 0b00010000:  # ACK flag
                        self.ack_num = ack_num
                    
            except ConnectionResetError:
                break
            except Exception as e:
                self.gui._log(f"Receive error: {str(e)}")
                break
        
        self.disconnect()
        self.gui.root.after(0, lambda: messagebox.showwarning("Disconnected", "Connection to rover lost"))