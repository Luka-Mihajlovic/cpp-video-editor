#!/bin/bash

# Ensure bc is installed
if ! command -v bc &> /dev/null; then
    echo "Error: 'bc' is not installed. Install it using 'sudo apt install bc'."
    exit 1
fi

# Number of repetitions (default: 1000)
repetitions=${1:-100}

# Define the commands to benchmark
commands=(
    "./runme ./data/test.bin ./editedFile.bin reverse"
	"./runme ./data/test.bin ./editedFile.bin -M reverse"
	"./runme ./data/test.bin ./editedFile.bin -S reverse"

	"./runme ./data/test.bin ./editedFile.bin swap_channel 1,2"
	"./runme ./data/test.bin ./editedFile.bin -M swap_channel 1,2"
	"./runme ./data/test.bin ./editedFile.bin -S swap_channel 1,2"

	"./runme ./data/test.bin ./editedFile.bin clip_channel 1 [10,200]"
	"./runme ./data/test.bin ./editedFile.bin -M clip_channel 1 [10,200]"
	"./runme ./data/test.bin ./editedFile.bin -S clip_channel 1 [10,200]"

	"./runme ./data/test.bin ./editedFile.bin scale_channel 1 1.5"
	"./runme ./data/test.bin ./editedFile.bin -M scale_channel 1 1.5"
	"./runme ./data/test.bin ./editedFile.bin -S scale_channel 1 1.5"

	"./runme ./data/test.bin ./editedFile.bin sepia"
	"./runme ./data/test.bin ./editedFile.bin -M sepia"
	"./runme ./data/test.bin ./editedFile.bin -S sepia"
)

# Initialize results arrays
declare -A total_time
declare -A total_memory

echo "Running benchmarks for ${#commands[@]} commands, each $repetitions times..."

# Benchmark each command
for command in "${commands[@]}"; do
    # Initialize totals
    total_time["$command"]=0
    total_memory["$command"]=0
    
    echo "Benchmarking: $command"
    
    for i in $(seq 1 $repetitions); do
        echo "Run $i/$repetitions for: $command"
        
        # Measure time and memory using `/usr/bin/time -v`
        start_time=$(date +%s%N)
        if ! output=$(/usr/bin/time -v $command 2>&1); then
            echo "Error executing command: $command"
            continue
        fi
        end_time=$(date +%s%N)
        
        # Calculate elapsed time in seconds
        elapsed_time=$(echo "scale=9; ($end_time - $start_time) / 1000000000" | bc)
        
        # Extract maximum memory usage (in KB) from `/usr/bin/time -v` output
        max_memory=$(echo "$output" | grep "Maximum resident set size" | awk '{print $NF}')
        
        # Accumulate results
        total_time["$command"]=$(echo "${total_time["$command"]} + $elapsed_time" | bc -l)
        total_memory["$command"]=$(echo "${total_memory["$command"]} + $max_memory" | bc -l)
    done
done

# Print aggregated results
echo "=================================================="
echo "Results after $repetitions runs for each command:"
for command in "${commands[@]}"; do
    average_time=$(echo "${total_time["$command"]} / $repetitions" | bc -l)
    average_memory=$(echo "${total_memory["$command"]} / $repetitions" | bc -l)
    printf "Command: %s\n" "$command"
    printf "  Average Time: %.9f seconds\n" "$average_time"
    printf "  Average Memory: %.3f KB\n" "$average_memory"
    echo "--------------------------------------------------"
done
echo "=================================================="
