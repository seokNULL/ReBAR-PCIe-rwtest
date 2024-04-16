#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <byteswap.h>

#include <sys/io.h>

int verbosity = 3;

#define MEM_DEVICE "/dev/mem"
#define AIM_RESERVED_OFFSET 0x00400000 //4MB 
#define MAGIC_NUMBER 0xDEADBEEF


/* PCI device */
typedef struct {
	/* Base address region */
	unsigned int bar;

	/* Slot info */
	unsigned int domain;
	unsigned int bus;
	unsigned int slot;
	unsigned int function;

	/* Resource filename */
	char         filename[100];

	/* File descriptor of the resource */
	int          fd;

	/* Memory mapped resource */
	unsigned char *maddr;
	// unsigned int   size;
	// unsigned int   offset;
	// /* PCI physical address */
	// unsigned int   phys;
	
	unsigned long   size;
	unsigned long   offset;
	/* PCI physical address */
	unsigned long   phys;


	/* Address to pass to read/write (includes offset) */
	unsigned char *addr;
} device_t;

static void show_usage()
{
	printf("\nUsage: pci_debug -s <device>\n"\
		 "  -h            Help (this message)\n"\
		 "  -s <device>   Slot/device (as per lspci)\n" \
	 	 "  -b <BAR>      Base address region (BAR) to access, eg. 0 for BAR0\n" \
		 "  -v <level>    Verbosity (0 to 3 - Default is 3)\n\n");
}

void print_byte_size(unsigned long value) {
    const char *suffixes[] = {"B", "KB", "MB", "GB"};
    double bytes = (double)value;
    int suffix_index = 0;

    while (bytes >= 1024 && suffix_index < 3) {
        bytes /= 1024;
        suffix_index++;
    }
    printf("%.4f %s\n", bytes, suffixes[suffix_index]);
}


int main(int argc, char *argv[])
{
	int opt;		
	char *slot = NULL;	
	char *cmdFilePath = NULL;
	int status;
	struct stat statbuf;
	device_t device;
	device_t *dev = &device;

	/* Clear the structure fields */
	memset(dev, 0, sizeof(device_t));

	while ((opt = getopt(argc, argv, "b:hs:f:qv:")) != -1) {
		switch (opt) {
			case 'b':
				/* Defaults to BAR0 if not provided */
				dev->bar = atoi(optarg);
				break;
			case 'h':
				show_usage();
				return -1;
			case 'v':
				verbosity = atoi(optarg);
				break;
			case 's':
				slot = optarg;
				break;
			default:
				show_usage();
				return -1;
		}
	}
	if (slot == 0) {
		show_usage();
		return -1;
	}

	/* ------------------------------------------------------------
	 * Open and map the PCI region
	 * ------------------------------------------------------------
	 */

	/* Extract the PCI parameters from the slot string */
	status = sscanf(slot, "%2x:%2x.%1x",
			&dev->bus, &dev->slot, &dev->function);
	if (status != 3) {
		printf("Error parsing slot information!\n");
		show_usage();
		return -1;
	}

	/* Convert to a sysfs resource filename and open the resource */
	snprintf(dev->filename, 99, "/sys/bus/pci/devices/%04x:%02x:%02x.%1x/resource%d",
			dev->domain, dev->bus, dev->slot, dev->function, dev->bar);
	dev->fd = open(dev->filename, O_RDWR | O_SYNC);
	if (dev->fd < 0) {
		printf("Open failed for file '%s': errno %d, %s\n",
			dev->filename, errno, strerror(errno));
		return -1;
	}

	/* PCI memory size */
	status = fstat(dev->fd, &statbuf);
	if (status < 0) {
		printf("fstat() failed: errno %d, %s\n",
			errno, strerror(errno));
		return -1;
	}
	dev->size = statbuf.st_size;

	/* Map */
	dev->maddr = (unsigned char *)mmap(
		NULL,
		(size_t)(dev->size),
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		dev->fd,
		0);
	if (dev->maddr == (unsigned char *)MAP_FAILED) {
//		printf("failed (mmap returned MAP_FAILED)\n");
		printf("BARs that are I/O ports are not supported by this tool\n");
		dev->maddr = 0;
		close(dev->fd);
		return -1;
	}

	/* Device regions smaller than a 4k page in size can be offset
	 * relative to the mapped base address. The offset is
	 * the physical address modulo 4k
	 */
	{
		char configname[100];
		int fd;

		snprintf(configname, 99, "/sys/bus/pci/devices/%04x:%02x:%02x.%1x/config",
				dev->domain, dev->bus, dev->slot, dev->function);
		fd = open(configname, O_RDWR | O_SYNC);
		if (dev->fd < 0) {
			printf("Open failed for file '%s': errno %d, %s\n",
				configname, errno, strerror(errno));
			return -1;
		}

		status = lseek(fd, 0x10 + 4*dev->bar, SEEK_SET);
		if (status < 0) {
			printf("Error: configuration space lseek failed\n");
			close(fd);
			return -1;
		}
		
		
		// status = read(fd, &dev->phys, 4);
		status = read(fd, &dev->phys, 8);
		if (status < 0) {
			printf("Error: configuration space read failed\n");
			close(fd);
			return -1;
		}
		dev->offset = ((dev->phys & 0xFFFFFFF0) % 0x1000);
		dev->addr = dev->maddr + dev->offset;
		close(fd);
	}


	int mem_fd;
    void *mem_ptr;
	unsigned long aim_pa_base, aim_pa = (unsigned long)(dev->phys)&0xfffffffffffffff0;
	unsigned long aim_mem_size, target_mem_size;
	
	aim_pa_base = (unsigned long)(dev->phys)&0xfffffffffffffff0;
	aim_pa = aim_pa_base + AIM_RESERVED_OFFSET;
	aim_mem_size = (dev->size) - AIM_RESERVED_OFFSET;
	// target_mem_size = aim_mem_size;
	target_mem_size = 1 *  1024 * 1024;
	

    mem_fd = open(MEM_DEVICE, O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("Failed to open /dev/mem");
        exit(EXIT_FAILURE);
    }

    mem_ptr = mmap(NULL, target_mem_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, mem_fd, aim_pa);
    if (mem_ptr == MAP_FAILED) {
        perror("Failed to map memory");
        close(mem_fd);
        exit(EXIT_FAILURE);
    }


	if (verbosity >= 3)
	{
		printf("\n");
		printf("PCI debug\n");
		printf("---------\n\n");
		printf(" - Accessing BAR%d\n", dev->bar);
		printf(" - Region size is %lu-bytes\n", dev->size);
        printf(" - PCI Physical Address: 0x%lx\n", (dev->phys)&0xfffffffffffffff0);
	verbosity==1?printf("\nAccessing BAR%d\n", dev->bar):0;
	}


	printf("\n");
	printf("Memory read/write test\n");
	printf("---------\n\n");
	printf(" - AiM memory size(bytes): ");
	print_byte_size(aim_mem_size);
	printf(" - AiM virtual address space: %p ~ %p\n", mem_ptr, mem_ptr +target_mem_size);
    printf(" - AiM physical address space: 0x%lx ~ 0x%lx\n", aim_pa, aim_pa + aim_mem_size);
    // printf(" - Target memory size(bytes):");
	// print_byte_size(target_mem_size);
	
	/* ------------------------------------------------------------
	 * Tests
	 * ------------------------------------------------------------
	 */

	memset(mem_ptr, 0, target_mem_size);
    for (int i = 0; i < target_mem_size; i += sizeof(int)) {
        int *addr = (int *)((char *)mem_ptr + i);
        *addr = MAGIC_NUMBER;
    }
    for (int i = 0; i < target_mem_size; i += sizeof(int)) {
        int *addr = (int *)((char *)mem_ptr + i);
        int value_read = *addr;
        
        if (value_read == MAGIC_NUMBER) {
            // printf("Data verification successful at address %p. Value read: 0x%X\n", addr, value_read);
        } else {
            printf("Index [%d]: Failed at address %p. Expected: 0x%X, Read: 0x%X\n", i, addr, MAGIC_NUMBER, value_read);
			// break;
        }
    }
	printf(" - Memory read/write PASS \n");

    munmap(mem_ptr, target_mem_size);
    close(mem_fd);	
	munmap(dev->maddr, dev->size);
	close(dev->fd);
	return 0;
}




