#!/bin/bash

# Define the paths to the two binaries
binary1="./ft_ping"
binary2="./inet_ping"

# Run the first binary in the background and capture its PID
$binary1 localhost &
pid1=$!

# Run the second binary in the background and capture its PID
$binary2 localhost &
pid2=$!

# Sleep for 5 seconds
sleep 5

# Kill both processes
kill $pid1
kill $pid2

# Store the output of both binaries in separate files
output1=$(mktemp)
output2=$(mktemp)

# Redirect the output of the binaries to their respective files
$binary1 > $output1 2>&1
$binary2 > $output2 2>&1

# Compare the output of the two binaries
echo "Diff output:"
diff $output1 $output2

# Clean up temporary files
rm $output1 $output2
