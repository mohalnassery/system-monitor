#!/bin/bash

# Remove all object files
rm -f *.o

# Remove object files from ImGui directory
rm -f imgui/lib/*.o
rm -f imgui/lib/backend/*.o
rm -f imgui/lib/gl3w/GL/*.o

# Remove the executable
rm -f monitor

echo "Cleanup complete!" 