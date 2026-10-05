import tkinter as tk
from tkinter import ttk
import os
import re

LOG_FILE = "supervisor.log"
NUM_WORKERS = 3


class SupervisorGUI:

    def __init__(self, root):
        self.root = root

        self.root.title("Fault-Tolerant Process Supervisor")
        self.root.geometry("800x600")
        self.root.resizable(False, False)

        self.workers = {}

        for i in range(1, NUM_WORKERS + 1):
            self.workers[i] = {
                "pid": "-",
                "status": "NOT STARTED",
                "restarts": 0
            }

        self.total_failures = 0
        self.total_restarts = 0

        self.create_gui()
        self.update_dashboard()

    def create_gui(self):

        title = tk.Label(
            self.root,
            text="FAULT-TOLERANT PROCESS SUPERVISOR",
            font=("Arial", 20, "bold")
        )
        title.pack(pady=20)

        status_frame = tk.Frame(self.root)
        status_frame.pack(pady=5)

        tk.Label(
            status_frame,
            text="SUPERVISOR STATUS:",
            font=("Arial", 12, "bold")
        ).pack(side="left")

        self.supervisor_status = tk.Label(
            status_frame,
            text="● RUNNING",
            font=("Arial", 12, "bold")
        )
        self.supervisor_status.pack(side="left", padx=10)

        table_frame = tk.Frame(self.root)
        table_frame.pack(pady=20)

        columns = ("worker", "pid", "status", "restarts")

        self.tree = ttk.Treeview(
            table_frame,
            columns=columns,
            show="headings",
            height=5
        )

        self.tree.heading("worker", text="Worker")
        self.tree.heading("pid", text="PID")
        self.tree.heading("status", text="Status")
        self.tree.heading("restarts", text="Restarts")

        self.tree.column("worker", width=120, anchor="center")
        self.tree.column("pid", width=180, anchor="center")
        self.tree.column("status", width=180, anchor="center")
        self.tree.column("restarts", width=120, anchor="center")

        self.tree.pack()

        for i in range(1, NUM_WORKERS + 1):
            self.tree.insert(
                "",
                "end",
                iid=str(i),
                values=(
                    f"Worker {i}",
                    "-",
                    "NOT STARTED",
                    "0"
                )
            )

        stats_frame = tk.Frame(self.root)
        stats_frame.pack(pady=20)

        self.failure_label = tk.Label(
            stats_frame,
            text="Total Failures: 0",
            font=("Arial", 12, "bold")
        )
        self.failure_label.pack(side="left", padx=30)

        self.restart_label = tk.Label(
            stats_frame,
            text="Total Restarts: 0",
            font=("Arial", 12, "bold")
        )
        self.restart_label.pack(side="left", padx=30)

        tk.Label(
            self.root,
            text="RECENT EVENTS",
            font=("Arial", 13, "bold")
        ).pack(pady=(10, 5))

        self.event_box = tk.Text(
            self.root,
            width=85,
            height=10,
            state="disabled"
        )
        self.event_box.pack(pady=5)

    def read_log(self):

        if not os.path.exists(LOG_FILE):
            return

        try:
            with open(LOG_FILE, "r") as file:
                lines = file.readlines()
        except Exception:
            return

        for i in range(1, NUM_WORKERS + 1):
            self.workers[i]["pid"] = "-"
            self.workers[i]["status"] = "NOT STARTED"
            self.workers[i]["restarts"] = 0

        self.total_failures = 0
        self.total_restarts = 0

        events = []

        for line in lines:

            line = line.strip()

            if not line:
                continue

            match = re.match(
                r"(STARTED|TERMINATED|RESTARTED) \| Worker (\d+) \| PID (\d+)",
                line
            )

            if not match:
                continue

            event_type = match.group(1)
            worker_id = int(match.group(2))
            pid = match.group(3)

            if worker_id not in self.workers:
                continue

            if event_type == "STARTED":

                self.workers[worker_id]["pid"] = pid
                self.workers[worker_id]["status"] = "RUNNING"

            elif event_type == "TERMINATED":

                self.total_failures += 1

                self.workers[worker_id]["status"] = "FAILED"
                self.workers[worker_id]["pid"] = pid

                events.append(
                    f"Worker {worker_id} terminated | PID {pid}"
                )

            elif event_type == "RESTARTED":

                self.total_restarts += 1
                self.workers[worker_id]["restarts"] += 1

                self.workers[worker_id]["status"] = "RUNNING"
                self.workers[worker_id]["pid"] = pid

                events.append(
                    f"Worker {worker_id} restarted | New PID {pid}"
                )

        self.update_table()
        self.update_statistics()
        self.update_events(events)

    def update_table(self):

        for i in range(1, NUM_WORKERS + 1):

            worker = self.workers[i]

            self.tree.item(
                str(i),
                values=(
                    f"Worker {i}",
                    worker["pid"],
                    worker["status"],
                    worker["restarts"]
                )
            )

    def update_statistics(self):

        self.failure_label.config(
            text=f"Total Failures: {self.total_failures}"
        )

        self.restart_label.config(
            text=f"Total Restarts: {self.total_restarts}"
        )

    def update_events(self, events):

        self.event_box.config(state="normal")
        self.event_box.delete("1.0", tk.END)

        for event in reversed(events[-10:]):
            self.event_box.insert(tk.END, event + "\n")

        self.event_box.config(state="disabled")

    def update_dashboard(self):

        self.read_log()
        self.root.after(1000, self.update_dashboard)


root = tk.Tk()
app = SupervisorGUI(root)
root.mainloop()