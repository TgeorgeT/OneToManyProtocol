import subprocess

clients = ["onetomanyprotocol-client1-1", "onetomanyprotocol-client2-1", "onetomanyprotocol-client3-1","onetomanyprotocol-client4-1"]

command = "/protocol/udp_client > output"

docker_command = ["docker", "exec", clients[0], "/bin/sh", "-c", command]
processes = []
i = 0
for client in clients:
    i+=1
    try:
        new_command = command+ str(i)
        docker_command[2]=client
        docker_command[-1]=new_command
        process = subprocess.Popen(docker_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        processes.append(process)
    except subprocess.CalledProcessError as e:
        print(f"Error: {e.stderr.decode()}")