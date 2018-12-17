#ifndef __BOARD_LOADER_H__
#define __BOARD_LOADER_H__

int board_linux_loader(image_header_t *hdr);
void board_autoboot_linux(void);

#endif
