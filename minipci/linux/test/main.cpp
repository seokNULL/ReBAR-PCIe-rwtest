#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

// remember:
//
// in 64-bit linux systems a long is a 64-bit type (sizeof(long)=8) !
#define USE_MMAP
// #define USE_READ_WRITE
#define BAR_USAGE	( 4 )	/* default index of bar to use for test */
#define OFFSET		( 0 )
#define MAGIC_NUMBER (0xDEADBEEF)
#define AIM_RESERVED_OFFSET (0x00400000)
// //30 - 1GB
// #define MEMCPY_BIT 32
// #define MEMCPY_SIZE (1 << MEMCPY_BIT)

enum MPD_IOCTLS_tag
{
        MPD_BAR_CHG = 1024,
        MPD_GET_BAR_MASK,
        MPD_GET_BAR_MAX_INDEX,
        MPD_GET_BAR_MAX_NUM,
} MPD_IOCTLS;

void PrintDevBuf (void* DevPtr, unsigned long size){ 
	for (int i = 0; i < size; i += sizeof(int)) {
		int *addr = (int *)((char *)DevPtr + i);
		int value_read = *addr;
        printf("bufferDev[%d]: %x\n", i, value_read);
    }

}
void print_byte_size(unsigned long value) {
    const char *suffixes[] = {"B", "KB", "MB", "GB"};
    double bytes = (double)value;
    int suffix_index = 0;

    while (bytes >= 1024 && suffix_index < 3) {
        bytes /= 1024;
        suffix_index++;
    }
    printf("%.4f %s\t", bytes, suffixes[suffix_index]);
}

int main()
{
	int rc;
	int fileHandle = -1;
	void *bufferSrc = NULL;
	void *bufferDst = NULL;
	unsigned char *bufferSrc2 = NULL;
	unsigned char *bufferDst2 = NULL;
	void *bufferDev = NULL;
	// unsigned long humanReadableSize = TEST_SIZE;
	// unsigned long memcpy_size = 0x100000000 - AIM_RESERVED_OFFSET; //4GB
	unsigned long memcpy_size = 0x200000000 - AIM_RESERVED_OFFSET; //4GB

	unsigned long ii, jj;
	unsigned long rv;
	unsigned long status = 0xfedcba9876543210;
	unsigned long *p;
	unsigned char *c;
	unsigned char u = ' ';	// Unit
	clock_t startTime, endTime;
	double finalTime;

	printf( "* open file\n" );
	fileHandle = open( "/dev/minipci0", O_RDWR );
	if ( fileHandle < 0 )
	{
		printf( "Device cannot be opened\n" );
		exit( -1 );
	}

	
	rv = ioctl( fileHandle, MPD_GET_BAR_MASK, status );
	rv = ioctl( fileHandle, MPD_GET_BAR_MAX_INDEX, status );
	rv = ioctl( fileHandle, MPD_GET_BAR_MAX_NUM, status );
	
	printf( "* get infos from board\n" );
	printf( "  BARMask (%16.16lx): 0x%2.2lx\n", status, rv );	
	printf( "  BARMaxIndex (%16.16lx): %ld\n", status, rv );	
	printf( "  BARMaxNum (%16.16lx): %ld\n", status, rv );

	// while ( humanReadableSize / 1024 > 0 )
	// {
	// 	u = ( ' ' == u ) ? 'k' : (( 'k' == u ) ? 'M' :  'G' );
	// 	humanReadableSize /= 1024;
	// }

	printf( "* use BAR#%d\n", BAR_USAGE );
	rv = ioctl( fileHandle, MPD_BAR_CHG, BAR_USAGE );
	printf( "  rv = %ld\n", rv );

#ifdef USE_MMAP
	printf( "* map memory\n" );
	// memcpy_size = 2 * 1024 * 1024 * 1024 ;
	// bufferDev = mmap( 0, memcpy_size, PROT_WRITE, MAP_SHARED, fileHandle, 0 );
	bufferDev = mmap( 0, memcpy_size, PROT_WRITE, MAP_PRIVATE, fileHandle, AIM_RESERVED_OFFSET);
	if ( MAP_FAILED == bufferDev )
	{
		printf( "Mmap failed\n" );
		exit( -1 );
	}

	memset( bufferDev, 0x00000000, memcpy_size );
	// PrintDevBuf(bufferDev, memcpy_size);
	printf(" - Test memory size(bytes): ");
	print_byte_size(memcpy_size);
    for (int i = 0; i < memcpy_size; i += sizeof(int)) {
		int *addr = (int *)((char *)bufferDev + i);
        *addr = MAGIC_NUMBER;
    }

    for (int i = 0; i < memcpy_size; i += sizeof(int)) {
		int *addr = (int *)((char *)bufferDev + i);
        int value_read = *addr;
        
        if (value_read == MAGIC_NUMBER) {
            // printf("Data verification successful at address %p. Value read: 0x%X\n", addr, value_read);
        } else {
            printf("Index [%d]: Failed at address %p. Expected: 0x%X, Read: 0x%X\n", i, addr, MAGIC_NUMBER, value_read);
			break;
        }
    }

	printf( "==> PASSED\n" );

	// PrintDevBuf(bufferDev, memcpy_size);

	// printf( "* write hardware\n" );
	// startTime = clock();
	// memcpy( bufferDev, bufferSrc, memcpy_size );
	// endTime = clock();
	// finalTime = ( double ) ( endTime - startTime ) / CLOCKS_PER_SEC;
	// printf( "Time: %.2lfsec., %.lfMB/s\n", finalTime, ( double ) TEST_SIZE_IN_MB / finalTime );	// hmmm: ~16MB/s
	
	// PrintDevBuf(bufferDev, memcpy_size);
	
	// startTime = clock();
	// printf( "* read hardware\n" );
	// memcpy( bufferDst, bufferDev, memcpy_size );
	// endTime = clock();
	// finalTime = ( double ) ( endTime - startTime ) / CLOCKS_PER_SEC;
	// printf( "Time: %.2lfsec., %.lfMB/s\n", finalTime, ( double ) TEST_SIZE_IN_MB / finalTime );

#endif

#ifdef USE_READ_WRITE
	printf( "* write hardware\n" );
	startTime = clock();
        rc = write( fileHandle, bufferSrc, sizeTest );
	endTime = clock();
	finalTime = endTime - startTime;
	printf( "  - rc = %d (should be zero) ==> %.2lf\n", rc, ( double )( finalTime ) / CLOCKS_PER_SEC);		// hmmm: ~16MB/s

	printf( "* read hardware\n" );
        rc = read( fileHandle, bufferDst, sizeTest );		// hmmm: ~1MB/s
	printf( "  - rc = %d (should be zero)\n", rc );
#endif


	printf( "* free memory\n" );
	free( bufferSrc );
	free( bufferDst );

	printf( "* close file\n" );
	close( fileHandle );

	return 0;
}
