#!/bin/bash

# System Monitor Verification Script
# Run this script while the monitor application is running to compare values

echo "=================================="
echo "SYSTEM MONITOR VERIFICATION SCRIPT"
echo "=================================="
echo "Run this while ./monitor is running to compare values"
echo ""

echo "=== SYSTEM INFORMATION ==="
echo "OS Name: $(uname -s)"
echo "Logged User: $(who | awk '{print $1}' | head -1)"
echo "Hostname: $(hostname)"
echo "Total Tasks: $(top -bn1 | grep 'Tasks:' | awk '{print $2}')"
echo "CPU Model: $(cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2 | xargs)"
echo ""

echo "=== MEMORY INFORMATION ==="
echo "RAM & SWAP Info:"
free -h
echo ""
echo "Disk Usage:"
df -h /
echo ""

echo "=== NETWORK INFORMATION ==="
echo "Network Interfaces:"
ifconfig | grep -E "^[a-z]|inet " | head -20
echo ""
echo "Network Statistics (first few lines):"
cat /proc/net/dev | head -5
echo ""

echo "=== THERMAL INFORMATION ==="
echo "Checking thermal sources..."
if [ -f /proc/acpi/ibm/thermal ]; then
    echo "IBM Thermal: $(cat /proc/acpi/ibm/thermal)"
elif [ -d /sys/class/thermal ]; then
    echo "Available thermal zones:"
    for zone in /sys/class/thermal/thermal_zone*/temp; do
        if [ -r "$zone" ]; then
            temp=$(cat "$zone")
            temp_c=$((temp / 1000))
            echo "  $(basename $(dirname $zone)): ${temp_c}°C"
        fi
    done
else
    echo "No thermal information available"
fi
echo ""

echo "=== PROCESS INFORMATION ==="
echo "Current monitor process:"
ps aux | grep monitor | grep -v grep
echo ""
echo "Top processes by CPU:"
top -bn1 | head -15 | tail -10
echo ""

echo "=================================="
echo "Compare these values with your running ./monitor application"
echo "=================================="
