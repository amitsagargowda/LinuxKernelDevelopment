cmd_/media/sf_shared/git/LinuxKernelDevelopment/IRQ/Module.symvers := sed 's/ko$$/o/' /media/sf_shared/git/LinuxKernelDevelopment/IRQ/modules.order | scripts/mod/modpost -m -a   -o /media/sf_shared/git/LinuxKernelDevelopment/IRQ/Module.symvers -e -i Module.symvers   -T -