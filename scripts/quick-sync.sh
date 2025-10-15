#!/bin/bash
# Quick sync - copies all source files to Windows

rsync -av --delete \
    --exclude='.git/' --exclude='.vs/' --exclude='x64/' \
    --exclude='Debug/' --exclude='Release/' --exclude='*.obj' \
    --exclude='*.pch' --exclude='*.pdb' --exclude='*.ilk' \
    --exclude='.claude/' \
    /root/projects/TaskAutomationCOMServer/ \
    /mnt/c/Users/andre/Desktop/Projects/TaskAutomationCOMServer/

echo "✓ Synced to C:\\Users\\andre\\Desktop\\Projects\\TaskAutomationCOMServer"
