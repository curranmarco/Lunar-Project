import tkinter as tk
from tkinter import ttk, messagebox

class EarthControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Earth Command Center")
        
        self.client = MoonRoverClient(self)
        
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
        
        ttk.Label(cmd_frame, text="Speed (0-100):").grid(row=2, column=0)
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
                
            self.client.send_move_command(degrees, meters, speed)
            
        except ValueError:
            messagebox.showerror("Error", "Invalid input! Enter numbers.")

    def _go_to_location(self):
        selected_location = self.location_combobox.get()
        if not selected_location:
            messagebox.showerror("Error", "Please select a destination!")
            return
        
        loc_id = [k for k, v in self.location_map.items() if v == selected_location][0]
        self.client.send_location_request(loc_id)
    
    def _update_location(self, loc_id):
        """Update the current location display"""
        location_name = self.location_map.get(loc_id, "Unknown Location")
        self.location_var.set(f"{loc_id}: {location_name}")
        self._log(f"Rover arrived at: {location_name}")
    
    def _log(self, message):
        """Add messages to the log"""
        self.log.insert(tk.END, message + "\n")
        self.log.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = EarthControlGUI(root)
    root.mainloop()