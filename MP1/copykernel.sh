sudo mount -o loop dev_kernel_grub.img /mnt/floppy
sudo cp kernel.bin /mnt/floppy/
sleep 1s
sudo MNT_DETACH /mnt/floppy/