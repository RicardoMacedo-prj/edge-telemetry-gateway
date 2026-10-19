import subprocess
import threading
import signal

def read_process_output(process):
    for line in process.stdout:
        print(line, end="")

def main():
    # Compile binaries
    compileCar = subprocess.run(["g++", "-std=c++17", "car.cpp", "-o", "car"], capture_output=True, text=True)
    compileGateway = subprocess.run(["g++", "-std=c++17", "main.cpp", "storage.cpp", "decoder.cpp", 
                                    "mqtt_client.cpp", "-o", "gateway", "-lmosquitto"], capture_output=True, text=True)
    
    # Check compilation errors
    if compileCar.returncode != 0 or compileGateway.returncode != 0:
        print("Error: Compilation failed.")

        if compileCar.returncode != 0:
            print("[car compilation error]")
            print(compileCar.stderr)

        if compileGateway.returncode != 0:
            print("[gateway compilation error]")
            print(compileGateway.stderr)
        
        return

    # Start processes
    pCar = subprocess.Popen(["./car"], stdout=subprocess.DEVNULL)
    pGateway = subprocess.Popen(["./gateway"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)

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
