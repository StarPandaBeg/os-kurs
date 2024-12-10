@echo off
tostool fs format -f --isectors 1024 -s 1024 disk.tos && tostool fs shell -c "upload ../client /" disk.tos