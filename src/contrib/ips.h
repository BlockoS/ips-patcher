#ifndef _IPS_H_
#define _IPS_H_

/**
 * IPS result values.
 */ 
enum IPSResult
{
	IPS_ERROR_OFFSET    = -7,
	IPS_ERROR_PROCESS   = -6,
	IPS_ERROR_READ      = -5,
	IPS_ERROR_SAVE      = -4,
	IPS_ERROR_OPEN      = -3,
	IPS_ERROR_FILE_TYPE = -2,
	IPS_ERROR           = -1,
	IPS_PATCH_END       =  0,
	IPS_OK              =  1
};

#define IPS_RECORD_OFFSET(record) ((record).offset & 0x00ffffff)
#define IPS_RECORD_INFO(record) (((record).offset >> 24) & 0xff)
#define IPS_RECORD_RLE 1

/*
 * Structures
 */

struct IPSRecord
{
	uint32_t offset;
	uint16_t size;
	uint8_t  rleData;
};

struct IPSPatch
{
	FILE             *rom;
	uint32_t         romSize;
	FILE             *patch;
	struct IPSRecord record;
};

/* 
 * Reset ips patch structure
 */
IPSResult IPSReset(struct IPSPatch *ips);

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

#endif /* _IPS_H_ */
