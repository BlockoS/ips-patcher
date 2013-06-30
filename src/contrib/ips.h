#ifndef _IPS_H_
#define _IPS_H_

#include <stdio.h>
#include <stdint.h>

/**
 * IPS result values.
 */ 
typedef enum
{
    IPS_ERROR_INVALID   = -9,
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

/**
 * IPS starting point for positioning functions.
 */
typedef enum
{
    IPS_SET = 0,
    IPS_CUR,
    IPS_END
} IPSWhence;

struct IPSPatch;

/**
 * IPS record.
 */
struct IPSRecord
{
    uint32_t data;   /**< Offset of the patch data in IPS file. */
	uint32_t offset; /**< Destination offset. */
	uint16_t size;   /**< Size of the data to be replaced. */
};

/**
 * Apply IPS patch to target file.
 */
IPSResult IPSApply(const char *patchFilename, const char *targetFilename);

/**
 * Open IPS patch.
 */
IPSResult IPSOpen(struct IPSPatch *patch, const char *filename);

/**
 * Close IPS patch.
 */
IPSResult IPSClose(struct IPSPatch *patch);

/**
 * Get the number of records stored in the patch.
 */
IPSResult IPSGetRecordCount(struct IPSPatch *patch, int *count);

/**
 * Jump to a given record.
 */
IPSResult IPSSeekTo(struct IPSPatch *patch, off_t record, IPSWhence whence);

/**
 * Read record infos.
 * The next call will fetch the next record.
 */
IPSResult IPSReadInfos(struct IPSPatch *patch, struct IPSRecord *record);

/**
 * Read record data.
 */
IPSResult IPSReadData(struct IPSPatch *patch, struct IPSRecord *record, uint8_t *data);

/**
 * Add a record to ips file.
 */
IPSResult IPSAddRecord(struct IPSPatch *patch, uint32_t offset, uint16_t size, const uint8_t* data);

#endif /* _IPS_H_ */
