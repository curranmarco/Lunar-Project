import socket
import random
import struct
import threading
from tkinter import messagebox
import tkinter as tk
from tkinter import ttk

def compute_checksum(bitstring):
    """Compute 16-bit one's complement checksum (like in TCP/IP)."""
    checksum = 0
    for i in range(0, len(bitstring), 16):
        chunk = bitstring[i:i+16]
        if len(chunk) < 16:
            chunk = chunk.ljust(16, '0')  # pad with zeros if needed
        checksum += int(chunk, 2)
        if checksum > 0xFFFF:
            checksum = (checksum & 0xFFFF) + 1  # carry around

    return format(~checksum & 0xFFFF, '016b')  # one’s complement

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
    checksum = format(0, '016b')  # placeholder
    urgent = format(0, '016b')

    header = src + dst + seq + ack + offset_flags + window + checksum + urgent
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
        self.server_ip = "192.168.221.1"
        self.server_port = 8080
        self.receive_thread = None
        self.source_port = random.randint(49152, 65535)

    def _create_syn_packet(self):
            syn = ""
            syn += format(self.source_port, '016b')
            syn += format(self.server_port, '016b')
            syn += format(self.isn, '032b')
            syn += format(0, '032b')
            syn += format(5, '04b') + format(0, '03b') + format(2, '09b')  # SYN = 0x02
            syn += format(1024, '016b')
            syn += format(0, '016b')
            syn += format(0, '016b')
            return syn
    def _create_ack_packet(self, ack_num):
            ack = ""
            ack += format(self.source_port, '016b')
            ack += format(self.server_port, '016b')
            ack += format(self.isn + 1, '032b')  # ACKing SYN
            ack += format(ack_num, '032b')
            ack += format(5, '04b') + format(0, '03b') + format(16, '09b')  # ACK = 0x10
            ack += format(1024, '016b')
            ack += format(0, '016b')
            ack += format(0, '016b')
            return ack

    def _create_data_packet(self, data):
        payload_bits = "".join(format(ord(c), '08b') for c in data)
        return create_data_packet(self.source_port, self.server_port, self.isn + 1, self.ack_num, payload_bits)

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
                raise ConnectionError(f"Invalid SYN-ACK length: got {len(syn_ack)} bits, expected 160")

            server_isn = int(syn_ack[32:64], 2)

            ack_num = server_isn + 1

            ack_packet = self._create_ack_packet(ack_num)
            self.sock.send(ack_packet.encode())
            self.gui._log(f"Sent ACK (ack={ack_num})")

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
                fin_packet = self._create_syn_packet()  # Reuse SYN as mock FIN
                self.sock.send(fin_packet.encode())
                self.gui._log("Sent FIN")
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

            packet = self._create_data_packet(cmd_str)
            self.sock.send(packet.encode())
            self.gui._log(f"Sent command: {cmd_str}")
            return True
        except Exception as e:
            messagebox.showerror("Error", f"Failed to send command: {str(e)}")
            return False

    def _receive_data(self):
        while self.connected:
            try:
                data = self.sock.recv(1024).decode().strip()
                if not data:
                    break

                self.gui._log(f"Received raw data: {data}")

                if "LOCATION:" in data:
                    try:
                        parts = data.split()
                        if len(parts) >= 2:
                            loc_id = int(parts[1])
                            self.gui._update_location(loc_id)
                    except (IndexError, ValueError):
                        pass
                else:
                    self.gui._log(f"Received: {data}")

            except ConnectionResetError:
                break
            except Exception as e:
                self.gui._log(f"Receive error: {str(e)}")
                break

        self.disconnect()
        self.gui.root.after(0, lambda: messagebox.showwarning("Disconnected", "Connection to rover lost"))

class EarthControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Earth Command Center")

        self.client = MoonRoverTCPClient(self)

        self.location_map = {
            0: "Mare Imbrium",
            1: "Mare Serenitatis",
            2: "Mare Tranquillitatis",
            3: "Mare Crisium",
            4: "Mare Nectaris",
            5: "Tycho Crater",
            6: "Copernicus Crater",
            7: "Aristarchus Plateau",
            8: "South Pole-Aitken Basin",
            9: "Oceanus Procellarum",
            10: "Plato Crater",
            11: "Clavius Crater",
            12: "Kepler Crater",
            13: "Schrödinger Basin",
            14: "Hadley Rille",
            15: "Apollo 11 Landing Site",
            16: "Apollo 17 Taurus–Littrow",
            17: "Lacus Somniorum",
            18: "Mare Humboldtianum",
            19: "Marius Hills"
        }

        self._setup_ui()

    def _setup_ui(self):
        conn_frame = ttk.LabelFrame(self.root, text="Connection")
        conn_frame.grid(row=0, column=0, padx=10, pady=5, sticky="ew")

        ttk.Button(conn_frame, text="Connect to Moon", command=self._connect).grid(row=0, column=0, padx=5)
        ttk.Button(conn_frame, text="Disconnect", command=self._disconnect).grid(row=0, column=1, padx=5)

        cmd_frame = ttk.LabelFrame(self.root, text="Rover Movement Commands")
        cmd_frame.grid(row=1, column=0, padx=10, pady=5, sticky="ew")

        ttk.Label(cmd_frame, text="Degrees:").grid(row=0, column=0)
        self.degrees_entry = ttk.Entry(cmd_frame)
        self.degrees_entry.grid(row=0, column=1)

        ttk.Label(cmd_frame, text="Meters:").grid(row=1, column=0)
        self.meters_entry = ttk.Entry(cmd_frame)
        self.meters_entry.grid(row=1, column=1)

        ttk.Label(cmd_frame, text="Speed (0-128):").grid(row=2, column=0)
        self.speed_entry = ttk.Entry(cmd_frame)
        self.speed_entry.grid(row=2, column=1)

        ttk.Button(cmd_frame, text="Send Move Command", command=self._send_move).grid(row=3, columnspan=2, pady=5)

        loc_frame = ttk.LabelFrame(self.root, text="Location Commands")
        loc_frame.grid(row=2, column=0, padx=10, pady=5, sticky="ew")

        ttk.Label(loc_frame, text="Select Destination:").grid(row=0, column=0)
        self.location_combobox = ttk.Combobox(loc_frame, values=list(self.location_map.values()))
        self.location_combobox.grid(row=0, column=1, padx=5)
        ttk.Button(loc_frame, text="Go to Location", command=self._go_to_location).grid(row=0, column=2, padx=5)

        self.location_var = tk.StringVar(value="Not connected")
        ttk.Label(loc_frame, textvariable=self.location_var, font=('Arial', 10)).grid(row=1, columnspan=3, pady=5)

        log_frame = ttk.LabelFrame(self.root, text="Communication Log")
        log_frame.grid(row=3, column=0, padx=10, pady=5, sticky="nsew")

        self.log = tk.Text(log_frame, height=10, wrap=tk.WORD)
        self.log.pack(fill="both", expand=True)

        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(3, weight=1)

    def _connect(self):
        if self.client.connect():
            self.connected = True
            self._log("Connection established with lunar rover")

    def _disconnect(self):
        self.client.disconnect()
        self.connected = False

    def _send_move(self):
        if not self.client.connected:
            messagebox.showerror("Error", "Not connected to rover!")
            return

        try:
            degrees = float(self.degrees_entry.get())
            meters = float(self.meters_entry.get())
            speed = int(self.speed_entry.get())

            if not 0 <= speed <= 128:
                messagebox.showerror("Error", "Speed must be between 0 and 128!")
                return

            self.client.send_command("MOVE", degrees, meters, speed)

        except ValueError:
            messagebox.showerror("Error", "Invalid input! Enter numbers.")

    def _go_to_location(self):
        selected_location = self.location_combobox.get()
        if not selected_location:
            messagebox.showerror("Error", "Please select a destination!")
            return

        loc_id = [k for k, v in self.location_map.items() if v == selected_location][0]
        self.client.send_command("LOCATION", loc_id)

    def _update_location(self, loc_id):
        location_name = self.location_map.get(loc_id, "Unknown Location")
        self.location_var.set(f"{loc_id}: {location_name}")
        self._log(f"Rover arrived at: {location_name}")

    def _log(self, message):
        self.log.insert(tk.END, message + "\n")
        self.log.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = EarthControlGUI(root)
    root.mainloop()
