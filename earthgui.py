# earthgui.py (MODIFIED VERSION)
import socket
import random
import struct
import threading
from tkinter import messagebox
import tkinter as tk
from tkinter import ttk

# ------------------ Helper Functions ------------------

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

def decode_payload(payload_bits):
    return ''.join(chr(int(payload_bits[i:i+8], 2)) for i in range(0, len(payload_bits), 8))

# ------------------ TCP Client Class ------------------

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

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.bind(('0.0.0.0', self.source_port))
            self.sock.connect((self.server_ip, self.server_port))
            self.connected = True
            self.gui._log("Connected to the server")
            self.receive_thread = threading.Thread(target=self._receive_data, daemon=True)
            self.receive_thread.start()
            return True
        except Exception as e:
            messagebox.showerror("Connection Error", f"TCP connection failed: {str(e)}")
            self.disconnect()
            return False

    def disconnect(self):
        if self.connected:
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
            self.sock.send(cmd_str.encode())
            self.gui._log(f"Sent: {cmd_str}")
            return True
        except Exception as e:
            messagebox.showerror("Error", f"Failed to send command: {str(e)}")
            return False

    def _receive_data(self):
        while self.connected:
            try:
                data = self.sock.recv(2048).decode().strip()
                if not data:
                    break
                self.gui._log(f"Received: {data}")

                # Parse moisture reading
                if "MoistureSensor" in data:
                    try:
                        binary = ''.join(filter(lambda x: x in "01", data))
                        decoded = decode_payload(binary[160:])  # Skip TCP-like header
                        self.gui._update_moisture(decoded)
                    except Exception as e:
                        self.gui._log(f"[Decode Error] Moisture: {str(e)}")

                # Parse MOVE command response
                elif "MOVE" in data:
                    try:
                        parts = data.split()
                        speed = parts[-1]
                        self.gui._update_speed(speed)
                    except Exception as e:
                        self.gui._log(f"[Decode Error] Speed: {str(e)}")

            except Exception as e:
                self.gui._log(f"Receive error: {str(e)}")
                break
        self.disconnect()
        self.gui.root.after(0, lambda: messagebox.showwarning("Disconnected", "Lost connection to the server"))

# ------------------ GUI Class ------------------

class EarthControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Lunar Rover Control")

        self.client = MoonRoverTCPClient(self)
        self._setup_ui()

    def _setup_ui(self):
        # --- Connection Controls
        conn_frame = ttk.LabelFrame(self.root, text="Connection")
        conn_frame.grid(row=0, column=0, padx=10, pady=5, sticky="ew")
        ttk.Button(conn_frame, text="Connect", command=self._connect).grid(row=0, column=0, padx=5)
        ttk.Button(conn_frame, text="Disconnect", command=self._disconnect).grid(row=0, column=1, padx=5)

        # --- Command Buttons
        cmd_frame = ttk.LabelFrame(self.root, text="Commands")
        cmd_frame.grid(row=1, column=0, padx=10, pady=5, sticky="ew")
        ttk.Button(cmd_frame, text="Go", command=self._send_go_command).grid(row=0, column=0, padx=10, pady=5)
        ttk.Button(cmd_frame, text="Stop", command=self._send_stop_command).grid(row=0, column=1, padx=10, pady=5)

        # --- Data Displays
        data_frame = ttk.LabelFrame(self.root, text="Sensor Data")
        data_frame.grid(row=2, column=0, padx=10, pady=5, sticky="ew")
        self.speed_var = tk.StringVar(value="Speed: N/A")
        self.moisture_var = tk.StringVar(value="Moisture: N/A")
        ttk.Label(data_frame, textvariable=self.speed_var).grid(row=0, column=0, sticky="w", padx=5)
        ttk.Label(data_frame, textvariable=self.moisture_var).grid(row=1, column=0, sticky="w", padx=5)

        # --- Log
        log_frame = ttk.LabelFrame(self.root, text="Communication Log")
        log_frame.grid(row=3, column=0, padx=10, pady=5, sticky="nsew")
        self.log = tk.Text(log_frame, height=10, wrap=tk.WORD)
        self.log.pack(fill="both", expand=True)

        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(3, weight=1)

    def _connect(self):
        if self.client.connect():
            self._log("Connection established.")

    def _disconnect(self):
        self.client.disconnect()

    def _send_go_command(self):
        self.client.send_command("MOVE", 0, 5, 64)

    def _send_stop_command(self):
        self.client.send_command("MOVE", 0, 0, 0)

    def _log(self, message):
        self.log.insert(tk.END, message + "\n")
        self.log.see(tk.END)

    def _update_speed(self, speed):
        self.speed_var.set(f"Speed: {speed}")

    def _update_moisture(self, reading):
        self.moisture_var.set(f"Moisture: {reading}")

# ------------------ Run GUI ------------------

if __name__ == "__main__":
    root = tk.Tk()
    app = EarthControlGUI(root)
    root.mainloop()
