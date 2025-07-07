# System Monitor Audit Test Report

**Generated:** 2025-07-07 23:07:57

## Executive Summary

This report contains the results of comprehensive testing against the audit requirements.

## Automated Test Results

### Application Launch: ❌ FAIL

### System Info

| Test | Expected | Status | Description |
|------|----------|--------|-------------|
| os_name | Linux | ✅ PASS | Operating system name should be Linux |
| logged_user | mohamed | ❓ MANUAL | User should match 'who' command output |
| hostname | mohamed-RP-15 | ❓ MANUAL | Hostname should match 'hostname' command output |
| task_count | 521 | ❓ MANUAL | Total tasks should match 'top' command output |
| cpu_type | AMD Ryzen 7 4800H with Radeon Graphics | ❓ MANUAL | CPU type should match /proc/cpuinfo model name |

### Memory Info

| Test | Expected | Status | Description |
|------|----------|--------|-------------|
| ram_info | total        used        free      shared  buff/ca... | ❓ MANUAL | RAM usage should match 'free -h' output |
| swap_info | total        used        free      shared  buff/ca... | ❓ MANUAL | SWAP usage should match 'free -h' output |
| disk_info | Filesystem      Size  Used Avail Use% Mounted on
/... | ❓ MANUAL | Disk usage should match 'df -h /' output |

### Network Info

| Test | Expected | Status | Description |
|------|----------|--------|-------------|
| network_interfaces | br-a0ee4b57c86b: flags=4099<UP,BROADCAST,MULTICAST... | ❓ MANUAL | Network interfaces should match 'ifconfig' output |
| network_stats | Inter-|   Receive                                 ... | ❓ MANUAL | Network RX/TX stats should match /proc/net/dev |

### Thermal Info

| Test | Expected | Status | Description |
|------|----------|--------|-------------|
| thermal_info | Not available | ❓ MANUAL | Temperature should match system thermal readings |

## Manual Test Checklist

The following tests require manual verification by running the application:

### System Monitor UI

**Test:** Check for tabbed section with CPU, Fan, Thermal tabs
**Instructions:** Run ./monitor and verify tabs exist in system window
**Result:** [ ] PASS [ ] FAIL

### CPU Tab

**Test:** Performance graph with CPU percentage overlay
**Instructions:** Open CPU tab, verify graph shows and has percentage text
**Result:** [ ] PASS [ ] FAIL

**Test:** FPS and Y-scale slider controls
**Instructions:** Verify sliders exist and affect graph behavior
**Result:** [ ] PASS [ ] FAIL

**Test:** Animation stop/start control
**Instructions:** Verify button/checkbox to pause graph animation
**Result:** [ ] PASS [ ] FAIL

### Thermal Tab

**Test:** Performance graph with temperature overlay
**Instructions:** Open Thermal tab, verify graph and temperature display
**Result:** [ ] PASS [ ] FAIL

**Test:** FPS and Y-scale slider controls
**Instructions:** Verify sliders exist and work properly
**Result:** [ ] PASS [ ] FAIL

**Test:** Animation control
**Instructions:** Verify animation can be stopped/started
**Result:** [ ] PASS [ ] FAIL

### Fan Tab

**Test:** Performance graph with fan information
**Instructions:** Open Fan tab, verify graph and fan status/speed/level
**Result:** [ ] PASS [ ] FAIL

**Test:** FPS and Y-scale controls
**Instructions:** Verify slider controls work
**Result:** [ ] PASS [ ] FAIL

