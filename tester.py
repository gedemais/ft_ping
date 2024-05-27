import subprocess
import random
import time

# Define a timeout value for how long each ping command should run
TIMEOUT = 5  # seconds

# Define the commands to be tested
commands = [
    "127.0.0.1",  # Good IP
    "999.999.999.999",  # Bad IP
    "localhost",  # Good hostname
    "nonexistent.hostname",  # Bad hostname
]

# Define the base options without values
options = [
    "-f",  # Flood
    "-w",  # Timeout (will add a random number)
    "-c",  # Count (will add a random number)
    "-i",  # Interval (will add a random float)
    "--ttl"  # TTL (will add a random number)
]

def get_random_option(option):
    """Generate a random value for the given option."""
    if option in ["-w", "-c", "--ttl"]:
        return f"{option} {random.randint(1, 100)}"
    elif option == "-i":
        return f"{option} {random.uniform(0.1, 2.0):.1f}"
    else:
        return option

def run_command(command, binary):
    """Runs a command with a given binary and captures its output."""
    full_command = f"./{binary} {command}"
    try:
        result = subprocess.run(full_command.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=TIMEOUT)
        return result.stdout.decode('utf-8'), result.stderr.decode('utf-8')
    except subprocess.TimeoutExpired:
        return "Timeout expired", ""

def generate_random_combination():
    """Generate a random combination of base commands and options."""
    cmd = random.choice(commands)
    selected_options = random.sample(options, random.randint(0, len(options)))
    random_opts = [get_random_option(opt) for opt in selected_options]
    return f"{cmd} {' '.join(random_opts)}"

def run_tests():
    """Runs the tests with various combinations and compares outputs."""
    for _ in range(10):  # Number of random tests
        full_command = generate_random_combination()
        print(f"Testing command: {full_command}")
        
        # Run ft_ping
        ft_ping_out, ft_ping_err = run_command(full_command, "ft_ping")
        print(f"ft_ping output:\n{ft_ping_out}\nft_ping error:\n{ft_ping_err}")
        
        # Run ping
        ping_out, ping_err = run_command(full_command, "ping")
        print(f"ping output:\n{ping_out}\nping error:\n{ping_err}")

        # Compare the outputs (simple comparison, can be enhanced)
        if ft_ping_out != ping_out or ft_ping_err != ping_err:
            print("Difference found!")
        else:
            print("Outputs are identical.")

        print("="*80)

if __name__ == "__main__":
    run_tests()
