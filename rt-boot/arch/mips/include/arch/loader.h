#ifndef __ARCH_LOADER_H__
#define __ARCH_LOADER_H__

void arch_linux_loader(image_header_t *hdr, char *cmdline);
void arch_linux_jump(rt_uint32_t linux_extra);

#endif
