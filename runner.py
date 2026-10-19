import subprocess
import threading
import signal

def read_process_output(process):
    for line in process.stdout:
        print(line, end="")

def main():
    # Configure and build binaries with CMake
    config = subprocess.run(["cmake", "-B", "build"], capture_output=True, text=True)
    if config.returncode != 0:
        print("Error: CMake configuration failed.")
        print(config.stderr)
        return

    build = subprocess.run(["cmake", "--build", "build"], capture_output=True, text=True)
    if build.returncode != 0:
        print("Error: CMake build failed.")
        print(build.stderr)
        return

    # Start processes
    pCar = subprocess.Popen(["./build/car"], stdout=subprocess.DEVNULL)
    pGateway = subprocess.Popen(["./build/gateway"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)

    # Read gateway output in a thread
    tGateway = threading.Thread(target=read_process_output, args=(pGateway,), daemon=True)
    tGateway.start()

    # Wait for gateway or handle Ctrl+C
    try:
        pGateway.wait()
    except KeyboardInterrupt:
        print("\n[Runner] Shutting down...")
        
        # Stop processes if still running
        if pCar.poll() is None:
            pCar.send_signal(signal.SIGINT)
        if pGateway.poll() is None:
            pGateway.send_signal(signal.SIGINT)

        pCar.wait()
        pGateway.wait()
        print("[Runner] Done.")

if __name__ == "__main__":
    main()
