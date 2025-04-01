import tkinter as tk
from tkinter import ttk, messagebox
import socket
import threading

class EarthControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Earth Command Center")
        
        self.connected = False
        self.sock = None
        
        self._setup_ui()
    
    def _setup_ui(self):
        conn_frame = ttk.LabelFrame(self.root, text="Connection")
        conn_frame.grid(row=0, column=0, padx=10, pady=5, sticky="ew")
        
        ttk.Button(conn_frame, text="Connect to Moon", command=self._connect).grid(row=0, column=0, padx=5)
        ttk.Button(conn_frame, text="Disconnect", command=self._disconnect).grid(row=0, column=1, padx=5)
        
        cmd_frame = ttk.LabelFrame(self.root, text="Rover Commands")
        cmd_frame.grid(row=1, column=0, padx=10, pady=5, sticky="ew")
        
        ttk.Label(cmd_frame, text="Degrees:").grid(row=0, column=0)
        self.degrees_entry = ttk.Entry(cmd_frame)
        self.degrees_entry.grid(row=0, column=1)
        
        ttk.Label(cmd_frame, text="Meters:").grid(row=1, column=0)
        self.meters_entry = ttk.Entry(cmd_frame)
        self.meters_entry.grid(row=1, column=1)
        
        ttk.Button(cmd_frame, text="Send Move Command", command=self._send_move).grid(row=2, columnspan=2, pady=5)
        
        log_frame = ttk.LabelFrame(self.root, text="Communication Log")
        log_frame.grid(row=2, column=0, padx=10, pady=5, sticky="nsew")
        
        self.log = tk.Text(log_frame, height=10, wrap=tk.WORD)
        self.log.pack(fill="both", expand=True)
        
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(2, weight=1)
    
    def _connect(self):
        """Simulate connecting to the Moon rover"""
        self.connected = True
        self._log("Connected to lunar rover (simulated)")
    
    def _disconnect(self):
        """Simulate disconnecting"""
        self.connected = False
        self._log("Disconnected from rover (simulated)")
    
    def _send_move(self):
        """Simulate sending a move command"""
        if not self.connected:
            messagebox.showerror("Error", "Not connected to rover!")
            return
        
        try:
            degrees = float(self.degrees_entry.get())
            meters = float(self.meters_entry.get())
            self._log(f"📡 Sent command: MOVE {degrees}°, {meters}m (simulated)")
            # Simulate rover response after 1.28s (Moon-Earth latency)
            self.root.after(1280, lambda: self._log(f"Rover response: Moving {degrees}°, {meters}m"))
        except ValueError:
            messagebox.showerror("Error", "Invalid input! Enter numbers.")
    
    def _log(self, message):
        """Add messages to the log"""
        self.log.insert(tk.END, message + "\n")
        self.log.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = EarthControlGUI(root)
    root.mainloop()