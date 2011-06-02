@ECHO OFF
IF NOT EXIST build mkdir build
bash.exe -c make
fat_imgen -c -F -f vm\kernel.img -i build\kernel.elf
fat_imgen -m -f vm\kernel.img -i vm\initfs.img
