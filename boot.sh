#!/bin/bash
set -e

export BZIMAGE=arch/x86/boot/bzImage
export INITRAMFS=initramfs-7.0.0-wasp.img
export DISK=/home/julian/vms/Fedora-Cloud-Base-Generic-43-1.6.x86_64.qcow2

sudo qemu-system-x86_64 \
  -enable-kvm \
  -m 8G \
  -smp 8,sockets=8,cores=1,threads=1 \
  -object memory-backend-ram,id=m0,size=1G \
  -object memory-backend-ram,id=m1,size=1G \
  -object memory-backend-ram,id=m2,size=1G \
  -object memory-backend-ram,id=m3,size=1G \
  -object memory-backend-ram,id=m4,size=1G \
  -object memory-backend-ram,id=m5,size=1G \
  -object memory-backend-ram,id=m6,size=1G \
  -object memory-backend-ram,id=m7,size=1G \
  -numa node,nodeid=0,cpus=0,memdev=m0 \
  -numa node,nodeid=1,cpus=1,memdev=m1 \
  -numa node,nodeid=2,cpus=2,memdev=m2 \
  -numa node,nodeid=3,cpus=3,memdev=m3 \
  -numa node,nodeid=4,cpus=4,memdev=m4 \
  -numa node,nodeid=5,cpus=5,memdev=m5 \
  -numa node,nodeid=6,cpus=6,memdev=m6 \
  -numa node,nodeid=7,cpus=7,memdev=m7 \
  -kernel "$BZIMAGE" \
  -initrd "$INITRAMFS" \
  -append "root=/dev/vda4 rootflags=subvol=root rootfstype=btrfs ro console=ttyS0 plymouth.enable=0 selinux=0 linux" \
  -drive file="$DISK",if=virtio,format=qcow2 \
  -netdev user,id=n1,hostfwd=tcp::2222-:22 \
  -device virtio-net-pci,netdev=n1 \
  -virtfs local,path="$PWD/shared",mount_tag=shared,security_model=passthrough,id=shared \
  -nographic
