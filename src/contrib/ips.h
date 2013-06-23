#ifndef _IPS_H_
#define _IPS_H_

#include <stdio.h>
#include <stdint.h>

/**
 * IPS result values.
 */ 
typedef enum
{
	IPS_ERROR_OFFSET    = -8,
	IPS_ERROR_PROCESS   = -7,
	IPS_ERROR_READ      = -6,
    IPS_ERROR_WRITE     = -5,
	IPS_ERROR_SAVE      = -4,
	IPS_ERROR_OPEN      = -3,
	IPS_ERROR_FILE_TYPE = -2,
	IPS_ERROR           = -1,
	IPS_PATCH_END       =  0,
	IPS_OK              =  1
} IPSResult;

#define IPS_RECORD_OFFSET(record) ((record).offset & 0x00ffffff)
#define IPS_RECORD_INFO(record) (((record).offset >> 24) & 0xff)
#define IPS_RECORD_RLE 1

/**
 * IPS record.
 */
struct IPSRecord
{
	uint32_t offset;  /**< Destination offset */
	uint16_t size;    /**< Size of the data to be replaced */
	uint8_t  rleData; /**< RLE encoded byte */
};

/**
 * IPS patch helper.
 */
struct IPSPatch
{
	FILE             *rom;     /**< File to be patched */
	size_t            romSize; /**< Input file size */
	FILE             *patch;   /**< IPS file */
	struct IPSRecord  record;  /**< Current IPS record */
};

/* 
 * Reset ips patch structure
 */
void IPSReset(struct IPSPatch *ips);

/*
 * Open rom and patch
 */
IPSResult IPSOpen(struct IPSPatch *ips,
                  const char *patchName,
                  const char *romName);
   
/*
 * Close patch and rom files 
 */           
void IPSClose(struct IPSPatch *ips);

/*
 * Read IPS record from file
 */
IPSResult IPSReadRecord(struct IPSPatch *ips);

/*
 * Process current record
 */
IPSResult IPSProcessRecord (struct IPSPatch *ips);

/*
 * Open the file and write IPS header in i
 */
IPSResult IPSWriteBegin(FILE** out, char* filename);

/*
 * Append a new record to IPS file
 */
IPSResult IPSWriteRecord (FILE* out, uint32_t offset, uint16_t size, uint8_t *data);

/*
 * Write IPS footer and close file.
 */
IPSResult IPSWriteEnd(FILE** out);

#endif /* _IPS_H_ */
