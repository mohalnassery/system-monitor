#!/usr/bin/env python3
"""
System Monitor Audit Test Suite
Comprehensive testing framework for the system monitor application
"""

import subprocess
import time
import os
import re
import json
from datetime import datetime
from typing import Dict, List, Tuple, Any

class AuditTester:
    def __init__(self):
        self.results = []
        self.log_file = "audit_test_log.txt"
        self.report_file = "audit_test_report.md"
        
    def log(self, message: str):
        """Log message to both console and file"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"[{timestamp}] {message}"
        print(log_entry)
        with open(self.log_file, "a") as f:
            f.write(log_entry + "\n")
    
    def run_command(self, command: str) -> Tuple[str, int]:
        """Run shell command and return output and return code"""
        try:
            result = subprocess.run(command, shell=True, capture_output=True, text=True, timeout=10)
            return result.stdout.strip(), result.returncode
        except subprocess.TimeoutExpired:
            return "Command timed out", 1
        except Exception as e:
            return f"Error: {str(e)}", 1
    
    def test_system_info(self) -> Dict[str, Any]:
        """Test system information accuracy"""
        self.log("=== Testing System Information ===")
        tests = {}
        
        # Test 1: OS Name (should be linux)
        self.log("Testing OS name...")
        os_output, _ = self.run_command("uname -s")
        expected_os = "Linux"
        tests["os_name"] = {
            "expected": expected_os,
            "actual": os_output,
            "passed": os_output.lower() == expected_os.lower(),
            "description": "Operating system name should be Linux"
        }
        
        # Test 2: Logged user
        self.log("Testing logged user...")
        who_output, _ = self.run_command("who | awk '{print $1}' | head -1")
        tests["logged_user"] = {
            "expected": who_output,
            "actual": "Need to check in application",
            "passed": None,  # Will be determined by manual inspection
            "description": "User should match 'who' command output"
        }
        
        # Test 3: Hostname
        self.log("Testing hostname...")
        hostname_output, _ = self.run_command("hostname")
        tests["hostname"] = {
            "expected": hostname_output,
            "actual": "Need to check in application",
            "passed": None,
            "description": "Hostname should match 'hostname' command output"
        }
        
        # Test 4: Task count
        self.log("Testing task count...")
        top_output, _ = self.run_command("top -bn1 | grep 'Tasks:' | awk '{print $2}'")
        tests["task_count"] = {
            "expected": top_output,
            "actual": "Need to check in application",
            "passed": None,
            "description": "Total tasks should match 'top' command output"
        }
        
        # Test 5: CPU type
        self.log("Testing CPU type...")
        cpu_output, _ = self.run_command("cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2 | xargs")
        tests["cpu_type"] = {
            "expected": cpu_output,
            "actual": "Need to check in application",
            "passed": None,
            "description": "CPU type should match /proc/cpuinfo model name"
        }
        
        return tests
    
    def test_memory_info(self) -> Dict[str, Any]:
        """Test memory information accuracy"""
        self.log("=== Testing Memory Information ===")
        tests = {}
        
        # Test RAM info
        self.log("Testing RAM information...")
        free_output, _ = self.run_command("free -h")
        tests["ram_info"] = {
            "expected": free_output,
            "actual": "Need to check in application",
            "passed": None,
            "description": "RAM usage should match 'free -h' output"
        }
        
        # Test SWAP info
        self.log("Testing SWAP information...")
        tests["swap_info"] = {
            "expected": free_output,
            "actual": "Need to check in application", 
            "passed": None,
            "description": "SWAP usage should match 'free -h' output"
        }
        
        # Test Disk info
        self.log("Testing disk information...")
        df_output, _ = self.run_command("df -h /")
        tests["disk_info"] = {
            "expected": df_output,
            "actual": "Need to check in application",
            "passed": None,
            "description": "Disk usage should match 'df -h /' output"
        }
        
        return tests
    
    def test_network_info(self) -> Dict[str, Any]:
        """Test network information accuracy"""
        self.log("=== Testing Network Information ===")
        tests = {}
        
        # Test network interfaces
        self.log("Testing network interfaces...")
        ifconfig_output, _ = self.run_command("ifconfig")
        tests["network_interfaces"] = {
            "expected": ifconfig_output,
            "actual": "Need to check in application",
            "passed": None,
            "description": "Network interfaces should match 'ifconfig' output"
        }
        
        # Test network statistics
        self.log("Testing network statistics...")
        netdev_output, _ = self.run_command("cat /proc/net/dev")
        tests["network_stats"] = {
            "expected": netdev_output,
            "actual": "Need to check in application",
            "passed": None,
            "description": "Network RX/TX stats should match /proc/net/dev"
        }
        
        return tests
    
    def test_thermal_info(self) -> Dict[str, Any]:
        """Test thermal information"""
        self.log("=== Testing Thermal Information ===")
        tests = {}
        
        # Check if thermal info is available
        thermal_output, ret_code = self.run_command("cat /proc/acpi/ibm/thermal 2>/dev/null || echo 'Not available'")
        tests["thermal_info"] = {
            "expected": thermal_output if ret_code == 0 else "Alternative thermal source needed",
            "actual": "Need to check in application",
            "passed": None,
            "description": "Temperature should match system thermal readings"
        }
        
        return tests

    def run_application_test(self) -> bool:
        """Test if application runs without crashing"""
        self.log("=== Testing Application Launch ===")

        # Build the application first
        self.log("Building application...")
        build_output, build_ret = self.run_command("make clean && make")
        if build_ret != 0:
            self.log(f"Build failed: {build_output}")
            return False

        self.log("Application built successfully")

        # Test if application starts (run for 3 seconds then kill)
        self.log("Testing application launch...")
        try:
            process = subprocess.Popen(["./monitor"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            time.sleep(3)  # Let it run for 3 seconds
            process.terminate()
            process.wait(timeout=5)
            self.log("Application launched and terminated successfully")
            return True
        except Exception as e:
            self.log(f"Application launch failed: {str(e)}")
            return False

    def generate_manual_test_checklist(self) -> List[Dict[str, str]]:
        """Generate checklist for manual testing"""
        return [
            {
                "category": "System Monitor UI",
                "test": "Check for tabbed section with CPU, Fan, Thermal tabs",
                "instructions": "Run ./monitor and verify tabs exist in system window"
            },
            {
                "category": "CPU Tab",
                "test": "Performance graph with CPU percentage overlay",
                "instructions": "Open CPU tab, verify graph shows and has percentage text"
            },
            {
                "category": "CPU Tab",
                "test": "FPS and Y-scale slider controls",
                "instructions": "Verify sliders exist and affect graph behavior"
            },
            {
                "category": "CPU Tab",
                "test": "Animation stop/start control",
                "instructions": "Verify button/checkbox to pause graph animation"
            },
            {
                "category": "Thermal Tab",
                "test": "Performance graph with temperature overlay",
                "instructions": "Open Thermal tab, verify graph and temperature display"
            },
            {
                "category": "Thermal Tab",
                "test": "FPS and Y-scale slider controls",
                "instructions": "Verify sliders exist and work properly"
            },
            {
                "category": "Thermal Tab",
                "test": "Animation control",
                "instructions": "Verify animation can be stopped/started"
            },
            {
                "category": "Fan Tab",
                "test": "Performance graph with fan information",
                "instructions": "Open Fan tab, verify graph and fan status/speed/level"
            },
            {
                "category": "Fan Tab",
                "test": "FPS and Y-scale controls",
                "instructions": "Verify slider controls work"
            }
        ]

    def run_all_tests(self):
        """Run all automated tests"""
        self.log("Starting comprehensive audit test suite...")

        # Clear previous log
        if os.path.exists(self.log_file):
            os.remove(self.log_file)

        # Test application launch
        app_launch_success = self.run_application_test()

        # Run system tests
        system_tests = self.test_system_info()
        memory_tests = self.test_memory_info()
        network_tests = self.test_network_info()
        thermal_tests = self.test_thermal_info()

        # Compile all results
        all_tests = {
            "application_launch": {"passed": app_launch_success},
            "system_info": system_tests,
            "memory_info": memory_tests,
            "network_info": network_tests,
            "thermal_info": thermal_tests
        }

        # Generate manual test checklist
        manual_tests = self.generate_manual_test_checklist()

        # Generate report
        self.generate_report(all_tests, manual_tests)

        self.log("Test suite completed. Check audit_test_report.md for detailed results.")

    def generate_report(self, automated_tests: Dict, manual_tests: List[Dict]):
        """Generate comprehensive markdown report"""
        with open(self.report_file, "w") as f:
            f.write("# System Monitor Audit Test Report\n\n")
            f.write(f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")

            # Executive Summary
            f.write("## Executive Summary\n\n")
            f.write("This report contains the results of comprehensive testing against the audit requirements.\n\n")

            # Automated Tests Results
            f.write("## Automated Test Results\n\n")

            for category, tests in automated_tests.items():
                if category == "application_launch":
                    status = "✅ PASS" if tests["passed"] else "❌ FAIL"
                    f.write(f"### Application Launch: {status}\n\n")
                    continue

                f.write(f"### {category.replace('_', ' ').title()}\n\n")
                f.write("| Test | Expected | Status | Description |\n")
                f.write("|------|----------|--------|-------------|\n")

                for test_name, test_data in tests.items():
                    status = "✅ PASS" if test_data["passed"] is True else "❓ MANUAL" if test_data["passed"] is None else "❌ FAIL"
                    expected = test_data["expected"][:50] + "..." if len(test_data["expected"]) > 50 else test_data["expected"]
                    f.write(f"| {test_name} | {expected} | {status} | {test_data['description']} |\n")
                f.write("\n")

            # Manual Test Checklist
            f.write("## Manual Test Checklist\n\n")
            f.write("The following tests require manual verification by running the application:\n\n")

            current_category = ""
            for test in manual_tests:
                if test["category"] != current_category:
                    current_category = test["category"]
                    f.write(f"### {current_category}\n\n")

                f.write(f"**Test:** {test['test']}\n")
                f.write(f"**Instructions:** {test['instructions']}\n")
                f.write("**Result:** [ ] PASS [ ] FAIL\n\n")


if __name__ == "__main__":
    tester = AuditTester()
    tester.run_all_tests()
