#include <string.h>
#include "ips.h"

#define IPS_HEADER_LEN 5

#define IPS_RECORD_OFFSET(record) ((record).offset & 0x00ffffff)
#define IPS_RECORD_INFO(record) (((record).offset >> 24) & 0xff)
#define IPS_RECORD_RLE 1

/**
 * IPS patch helper.
 */
struct IPSPatch
{
	FILE   *source; /**< IPS file */
    size_t size;    /**< IPS file size */
    int    status;  /**< IPS patch status (0=read, 1=write) */
};

void IPSCleanup(struct IPSPatch *patch)
{
    patch->size = 0;
    patch->status = 0;
    if(patch->source != NULL)
    {
        fclose(patch->source);
        patch->source = NULL;
    }
}

IPSResult IPSWriteHeader(struct IPSPatch *patch)
{
    size_t nWritten;
    const char header[IPS_HEADER_LEN] = { 'P', 'A', 'T', 'C', 'H' };
    nWritten = fwrite(header, 1, IPS_HEADER_LEN, patch->source);
    if(nWritten != IPS_HEADER_LEN)
    {
        return IPS_ERROR_WRITE;
    }
    return IPS_OK;
}

IPSResult IPSReadRLE(struct IPSPatch *patch, struct IPSRecord *record, uint8_t *data)
{
    uint8_t buffer[2];
    size_t nRead;
    
    /* Read RLE size */
    nRead = fread(buffer, 1, 2, patch->source);
    if(nRead < 2)
    {
        return IPS_ERROR_READ;
    }
    record->size = (buffer[0] << 8) | 
                   (buffer[1]     );
    
    /* Read byte data to copy */
    nRead = fread(data, 1, 1, patch->source);
    if(nRead < 1)
    {
        return IPS_ERROR_READ;
    }
    return IPS_OK;
}

/**
 * Open IPS patch.
 */
IPSResult IPSOpen(struct IPSPatch *patch, const char *filename)
{
    char buffer[IPS_HEADER_LEN];
    size_t nRead;
    IPSResult res = IPS_OK;
    
    patch->source = fopen(filename, "rb+");
    if(patch->source == NULL)
    {
        res = IPS_ERROR_OPEN;
        /* Try to create file if it doesn't exist. */
        patch->source = fopen(filename, "wb");
        if(patch->source != NULL)
        {
            patch->status = 1;
            
            /* Write header. */
            res = IPSWriteHeader(patch);
        }
        return res;
    }

    patch->status = 0;
    
    fseek(patch->source, 0, SEEK_END);
    patch->size  = ftell(patch->source);
    fseek(patch->source, 0, SEEK_SET);
    patch->size -= ftell(patch->source);
    
    /* Read header and check it */
	nRead = fread(buffer, 1, IPS_HEADER_LEN, patch->source);
	if(nRead != IPS_HEADER_LEN)
	{
    
		res = IPS_ERROR_FILE_TYPE;
	}
	
    if(res == IPS_OK)
    {
        if((buffer[0] != 'P') ||
           (buffer[1] != 'A') ||
           (buffer[2] != 'T') ||
           (buffer[3] != 'C') ||
           (buffer[4] != 'H'))
        {
            res = IPS_ERROR_FILE_TYPE;
        }
    }
    
    if(res != IPS_OK)
    {
        IPSCleanup(patch);
    }
	return res;
}

/**
 * Close IPS patch.
 */
IPSResult IPSClose(struct IPSPatch *patch)
{
    IPSResult res = IPS_OK;
    if(patch->status)
    {
        char buffer[3] = { 'E', 'O', 'F' };
        size_t nWritten;
        
        nWritten = fwrite(buffer, 1, 3, patch->source);
        if(nWritten != 3)
        {
            res = IPS_ERROR_WRITE;
        }
    }
    IPSCleanup(patch);
    return res;
}

/**
 * Get the number of records stored in the patch.
 */
IPSResult IPSGetRecordCount(struct IPSPatch *patch, int *count)
{
    struct IPSRecord record;
    IPSResult res = IPS_OK;
    off_t backup = ftell(patch->source);
    
    *count = 0;
    
    /* Jump after IPS header. */
    fseek(patch->source, IPS_HEADER_LEN, SEEK_SET);
    
    while( (res = IPSReadInfos(patch, &record)) == IPS_OK )
    {
        *count += 1;
    }
    
    if(res == IPS_PATCH_END)
    {
        res = IPS_OK;
    }
    
    /* Put file pointer back. */
    fseek(patch->source, backup, SEEK_SET);
    return res;
}

/**
 * Read record infos.
 * The next call will fetch the next record.
 */
IPSResult IPSReadInfos(struct IPSPatch *patch, struct IPSRecord *record)
{
    uint8_t buffer[5];
	size_t  nRead;
    off_t   offset;
	
	nRead = fread(buffer, 1, 3, patch->source);
	if(nRead < 3)
	{
		return IPS_ERROR_READ;
	}
	/* Is it the end of the patch ? */
	if((buffer[0] == 'E') &&
	   (buffer[1] == 'O') &&
	   (buffer[2] == 'F'))
	{
		return IPS_PATCH_END;
	}
	
	/* Retrieve rom offset */
	record->offset = (buffer[0] << 16) | 
	                 (buffer[1] <<  8) |
	                 (buffer[2]      );
	
	/* Data size */
	nRead = fread(buffer, 1, 2, patch->source);
	if(nRead < 2)
	{
		return IPS_ERROR_READ;
	}
	record->size = (buffer[0] << 8) | 
	               (buffer[1]     );

    record->data = ftell(patch->source);
                   
	/* We have a RLE section if data size is zero */
	if(record->size == 0)
	{
        record->offset |= IPS_RECORD_RLE << 24;
        offset = 3; 
	}
    else
    {
        offset = record->size;
    }
    
    /* Move to next record. */
    fseek(patch->source, offset, SEEK_CUR);
    
	return IPS_OK;
}

/**
 * Read record data.
 */
IPSResult IPSReadData(struct IPSPatch *patch, struct IPSRecord *record, uint8_t *data)
{
    IPSResult res = IPS_OK;
    
    fseek(patch->source, record->data, SEEK_SET);
    if(IPS_RECORD_INFO(*record) == IPS_RECORD_RLE)
    {
        res = IPSReadRLE(patch, record, data);
    }
    else
    {
        size_t nRead;
        nRead = fread(data, 1, record->size, patch->source);
        if(nRead != record->size)
        {
            res = IPS_ERROR_READ;
        }
    }
    
    return res;
}

/**
 * Add a record to ips file.
 */
IPSResult IPSAddRecord(struct IPSPatch *patch, uint32_t offset, uint16_t size, const uint8_t* data)
{
    uint8_t buffer[5];
    size_t  nWritten;
  
    if(patch->status == 0)
    {
        IPSResult res;
        struct IPSRecord record;
        
        /* Search for last record. */
        fseek(patch->source, IPS_HEADER_LEN, SEEK_SET);
        while( (res = IPSReadInfos(patch, &record)) == IPS_OK )
        {}
        if(res != IPS_PATCH_END)
        {
            return res;
        }
        /* Rewind in order to overwrite the EOF record. */
        fseek(patch->source, -3, SEEK_CUR);
    }

    patch->status = 1;
    
    /* Serialize offset */
	buffer[0] = (offset >> 16) & 0xff;
	buffer[1] = (offset >>  8) & 0xff;
	buffer[2] = (offset      ) & 0xff;
	
	/* ... and data size */
	buffer[3] = (size >> 8);
	buffer[4] = (size     ) & 0xff;
	
	nWritten = fwrite( buffer, 1, 5, patch->source );
	if( nWritten != 5 )
	{
		return IPS_ERROR_WRITE;
	}

	/* Write data. */
	nWritten = fwrite( data, 1, size, patch->source );
	if( nWritten != size )
	{
		return IPS_ERROR_WRITE;
	}	
    
    return IPS_OK;
}

/**
 * Jump to a given record.
 */
IPSResult IPSSeekTo(struct IPSPatch *patch, unsigned int recordNum)
{
    off_t backup;
    int i;
    IPSResult res;
    struct IPSRecord record;
    
    backup = ftell(patch->source);
    
    fseek(patch->source, IPS_HEADER_LEN, SEEK_SET);
    for(i=0; (i<recordNum) && (res == IPS_OK); i++)
    {
        res = IPSReadInfos(patch, &record);
    }
    
    fseek(patch->source, backup, SEEK_SET);
    
    return res;
}

/**
 * Apply current record.
 */
IPSResult IPSApplyRecord(struct IPSPatch *patch, struct IPSRecord *record, FILE* out)
{
    // [todo] expand rom if needed
    unsigned int i;
    IPSResult res = IPS_OK;
    size_t offset = IPS_RECORD_OFFSET(*record);
    size_t nWritten;
    
    fseek(patch->source, record->data, SEEK_SET);
    fseek(out, offset, SEEK_SET);
    if(IPS_RECORD_INFO(*record) == IPS_RECORD_RLE)
    {
        uint8_t data;
        
        res = IPSReadRLE(patch, record, &data);
        for(i=0; (i<record->size) && (res == IPS_OK); i++)
        {
            nWritten = fwrite(&data, 1, 1, out);
            if(nWritten != 1)
            {
                res = IPS_ERROR_WRITE;
            }
        }
    }
    else
    {
        size_t nRead, nElmnt;
        uint8_t buffer[64];
        
        for(i=record->size; (i>0) && (res == IPS_OK); i-=nRead)
        {
            nElmnt = (i < 64) ? i : 64;
            nRead = fread (buffer, 1, nElmnt, patch->source);
            if(nRead != nElmnt)
            {
                res = IPS_ERROR_READ;
            }
            else
            {
                nWritten = fwrite(buffer, 1, nRead, out);
                if(nWritten != nRead)
                {
                    res = IPS_ERROR_WRITE;
                }
            }
        }
    }

    return res;
}

/**
 * Apply IPS patch to target file.
 */
IPSResult IPSApply(const char *patchFilename, const char *targetFilename)
{
    struct IPSPatch patch;
    IPSResult res;
    FILE *output;
    
    output = fopen(targetFilename, "rb+");
    if(output != NULL)
    {
        return IPS_ERROR_OPEN;
    }
    
    res = IPSOpen(&patch, patchFilename);
    if(res != IPS_OK)
    {
        return res;
    }
    
    // [todo] apply records
    
    return res;
}
