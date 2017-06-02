import sys, threading, importlib

# Now monkeypatch Thread.start() method to daemonize plumbing threads.
importlib.reload(threading)
def start_asdaemon(self):
    self.daemon = True
    self.start_nodaemon()    
threading.Thread.start_nodaemon = threading.Thread.start
threading.Thread.start = start_asdaemon

# Then load plumbing
from framework.plumbing import *

# Create an `FmkPlumbing` object for our end-user.
fmk = FmkPlumbing()
print("\n > Interact with the `fmk` object:")
print("   > Type `fmk.show_projects()` to list available projects.")
print("   > Type `exit()` or Ctrl-D (i.e. EOF) to exit gracefully.\n")

# Handler to terminate fuddly properly
def exit_gracefully():
    print("Terminating fuddly properly...", file=sys.stderr)
    fmk.exit_fmk()

    for thread in threading.enumerate():
        try:
            print(" > Waiting for {}...".format(thread))
            thread.join(5)
        except:
            pass

# Handle exiting with ^D (EOF) by plugging `exit_gracefully` as handler.
atexit.register(exit_gracefully)

