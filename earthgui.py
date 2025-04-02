import socket
import random
import struct
import threading
from tkinter import messagebox
import tkinter as tk
from tkinter import ttk

def compute_checksum(bitstring):
    checksum = 0
    for i in range(0, len(bitstring), 16):
        chunk = bitstring[i:i+16]
        if len(chunk) < 16:
            chunk = chunk.ljust(16, '0')
        checksum += int(chunk, 2)
        if checksum > 0xFFFF:
            checksum = (checksum & 0xFFFF) + 1
    return format(~checksum & 0xFFFF, '016b')

def create_data_packet(source_port, dest_port, seq_num, ack_num, payload_bits):
    src = format(source_port, '016b')
    dst = format(dest_port, '016b')
    seq = format(seq_num, '032b')
    ack = format(ack_num, '032b')
    data_offset = format(5, '04b')
    reserved = format(0, '03b')
    flags = format(0x08, '09b')  # PSH flag
    offset_flags = data_offset + reserved + flags
    window = format(0x24, '016b')
    checksum_placeholder = '0000000000000000'
    urgent = format(0, '016b')
    header = src + dst + seq + ack + offset_flags + window + checksum_placeholder + urgent
    full_packet = header + payload_bits
    checksum = compute_checksum(full_packet)
    final_packet = src + dst + seq + ack + offset_flags + window + checksum + urgent + payload_bits
    return final_packet

class MoonRoverTCPClient:
    def __init__(self, gui_app):
        self.gui = gui_app
        self.sock = None
        self.connected = False
        self.isn = random.randint(0, 2**32 - 1)
        self.ack_num = 0
        self.server_ip = "127.0.0.1"
        self.server_port = 8080
        self.receive_thread = None
        self.source_port = random.randint(49152, 65535)

    def _create_syn_packet(self):
        src = format(self.source_port, '016b')
        dst = format(self.server_port, '016b')
        seq = format(self.isn, '032b')
        ack = format(0, '032b')
        offset_flags = format(5, '04b') + format(0, '03b') + format(2, '09b')
        window = format(1024, '016b')
        checksum = '0000000000000000'
        urgent = format(0, '016b')
        header = src + dst + seq + ack + offset_flags + window + checksum + urgent
        computed_checksum = compute_checksum(header)
        return src + dst + seq + ack + offset_flags + window + computed_checksum + urgent

    def _create_ack_packet(self, ack_num):
        src = format(self.source_port, '016b')
        dst = format(self.server_port, '016b')
        seq = format(self.isn + 1, '032b')
        ack = format(ack_num, '032b')
        offset_flags = format(5, '04b') + format(0, '03b') + format(16, '09b')
        window = format(1024, '016b')
        checksum = '0000000000000000'
        urgent = format(0, '016b')
        header = src + dst + seq + ack + offset_flags + window + checksum + urgent
        computed_checksum = compute_checksum(header)
        return src + dst + seq + ack + offset_flags + window + computed_checksum + urgent

    def _send_data_command(self, command_code, data=""):
        payload_bits = format(command_code, '08b') + ''.join(format(ord(c), '08b') for c in data)
        packet = create_data_packet(self.source_port, self.server_port, self.isn + 1, self.ack_num, payload_bits)
        self.sock.send(packet.encode())
        self.gui._log(f"[Sent] Command {command_code} - Payload: {data}")

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.bind(('0.0.0.0', self.source_port))
            self.sock.connect((self.server_ip, self.server_port))
            syn_packet = self._create_syn_packet()
            self.sock.send(syn_packet.encode())
            self.gui._log(f"Sent SYN (seq={self.isn})")
            syn_ack = self.sock.recv(1024).decode().strip()
            self.gui._log(f"Received SYN-ACK: {syn_ack}")
            if len(syn_ack) != 160:
                raise ConnectionError(f"Invalid SYN-ACK length: {len(syn_ack)} bits")
            server_isn = int(syn_ack[32:64], 2)
            ack_packet = self._create_ack_packet(server_isn + 1)
            self.sock.send(ack_packet.encode())
            self.gui._log("Sent ACK")
            self.connected = True
            self.gui._log("TCP connection established")
            self.receive_thread = threading.Thread(target=self._receive_data, daemon=True)
            self.receive_thread.start()
            return True
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed: {str(e)}")
            self.disconnect()
            return False

    def disconnect(self):
        if self.connected:
            try:
                self._send_data_command(0)  # FIN command
            except:
                pass
            self.connected = False
            if self.sock:
                self.sock.close()
            self.gui._log("Disconnected from server")

    def send_command(self, command_code, value=""):
        if not self.connected:
            messagebox.showerror("Error", "Not connected!")
            return
        self._send_data_command(command_code, str(value))

    def _receive_data(self):
        while self.connected:
            try:
                data = self.sock.recv(1024).decode().strip()
                if not data:
                    break
                self.gui._log(f"[Received Raw]: {data}")
                if len(data) > 160:
                    payload = data[160:]
                    if payload.startswith("00000001"):  # MOVE
                        self.gui._log("→ MOVE Command Received")
                    elif payload.startswith("00000010"):  # STOP
                        self.gui._log("→ STOP Command Received")
                    elif payload.startswith("00000011"):  # SPEED
                        speed_bin = payload[8:16]
                        speed_val = int(speed_bin, 2)
                        self.gui._log(f"→ SPEED = {speed_val}")
                    elif payload.startswith("00000100"):  # SENSOR
                        moisture_bin = payload[12:28]
                        moisture = int(moisture_bin, 2)
                        self.gui._log(f"→ MOISTURE = {moisture}")
            except Exception as e:
                self.gui._log(f"Error: {str(e)}")
                break
        self.disconnect()

class EarthControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Moon Rover Control Panel")
        self.client = MoonRoverTCPClient(self)
        self._setup_ui()

    def _setup_ui(self):
        conn_frame = ttk.LabelFrame(self.root, text="Connection")
        conn_frame.grid(row=0, column=0, padx=10, pady=5, sticky="ew")
        ttk.Button(conn_frame, text="Connect", command=self._connect).grid(row=0, column=0, padx=5)
        ttk.Button(conn_frame, text="Disconnect", command=self._disconnect).grid(row=0, column=1, padx=5)

        cmd_frame = ttk.LabelFrame(self.root, text="Commands")
        cmd_frame.grid(row=1, column=0, padx=10, pady=5, sticky="ew")
        ttk.Button(cmd_frame, text="Go", command=lambda: self.client.send_command(1, "5")).grid(row=0, column=0, padx=5)
        ttk.Button(cmd_frame, text="Stop", command=lambda: self.client.send_command(2)).grid(row=0, column=1, padx=5)
        ttk.Button(cmd_frame, text="SENSOR DATA", command=lambda: self.client.send_command(4, "SensorData")).grid(row=0, column=2, padx=5)

        log_frame = ttk.LabelFrame(self.root, text="Communication Log")
        log_frame.grid(row=2, column=0, padx=10, pady=5, sticky="nsew")
        self.log = tk.Text(log_frame, height=12, wrap=tk.WORD)
        self.log.pack(fill="both", expand=True)

        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(2, weight=1)

    def _connect(self):
        self.client.connect()

    def _disconnect(self):
        self.client.disconnect()

    def _log(self, message):
        self.log.insert(tk.END, message + "\n")
        self.log.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = EarthControlGUI(root)
    root.mainloop()
