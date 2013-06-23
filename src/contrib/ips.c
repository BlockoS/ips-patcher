#include <string.h>
#include "ips.h"

/* 
 * Reset ips patch structure
 */
void IPSReset(struct IPSPatch *ips)
{
	ips->patch = NULL;	
	
	ips->rom      = NULL;
	ips->romSize  = 0;
	
	ips->record.offset  = 0;
	ips->record.size    = 0;
	ips->record.rleData = 0;
}

/*
 * Open and check patch file
 */
IPSResult IPSOpenPatch(struct IPSPatch *ips, const char *fileName)
{
	char   header[5];
	size_t nRead;
	
	ips->patch = fopen(fileName, "rb");
	if(ips->patch == NULL)
	{
		return IPS_ERROR;
	}
	
	fseek(ips->patch, 0, SEEK_SET);
	
	/* Read header and check it */
	nRead = fread(header, 1, 5, ips->patch);
	if(nRead != 5)
	{
		return IPS_ERROR_FILE_TYPE;
	}
	
	if((header[0] != 'P') ||
	   (header[1] != 'A') ||
	   (header[2] != 'T') ||
	   (header[3] != 'C') ||
	   (header[4] != 'H'))
	{
		return IPS_ERROR_FILE_TYPE;
	}
	
	return IPS_OK;
}

/*
 * Open rom and make a backup of it 
 */
IPSResult IPSOpenInOUT(struct IPSPatch *ips,
                       const char *romName)
{
	char      backupFileName[64];
	uint32_t  backupFileExtOffset;
	FILE      *backup;
	long      nTotal;
	size_t    nRead;
	char      buffer[256];

	if(romName == NULL)
	{
		return IPS_ERROR;
	}

	/* Create backup file name */
	strncpy(backupFileName, romName, 52);
		
	backupFileExtOffset = strlen(romName);
	if(backupFileExtOffset > 52)
	{
		backupFileExtOffset = 52;
	}
		
	backupFileName[backupFileExtOffset++] = '.';
	backupFileName[backupFileExtOffset++] = 's';
	backupFileName[backupFileExtOffset++] = 'a';
	backupFileName[backupFileExtOffset++] = 'v';
	backupFileName[backupFileExtOffset++] = '\0';
		
	/* Open rom */
	ips->rom = fopen(romName, "rb+");
	if(ips->rom == NULL)
	{
		return IPS_ERROR;
	}
	
	/* Get its size */
	fseek(ips->rom, 0, SEEK_END);
	nTotal  = ftell(ips->rom);
	fseek(ips->rom, 0, SEEK_SET);
	nTotal -= ftell(ips->rom);
	
	ips->romSize = (uint32_t)nTotal;
	if(nTotal <= 0)
	{
		return IPS_ERROR;
	}

	/* Open backup */
	backup = fopen(backupFileName, "wb");
	if(backup == NULL)
	{
		return IPS_ERROR;
	}
	
	/* A copy rom data to it */
	fseek(backup, 0, SEEK_SET);
	for(nTotal=0; nTotal<ips->romSize; nTotal+=nRead)
	{
		nRead = fread(buffer, 1, 256, ips->rom);
		fwrite(buffer, 1, nRead, backup);
	}		
	fclose(backup);

	/* Reset rom file pointer */
	fseek(ips->rom, 0, SEEK_SET);
	
	return IPS_OK;
}

/*
 * Open rom and patch
 */
IPSResult IPSOpen(struct IPSPatch *ips,
                  const char *patchName,
                  const char *romName)
{
	IPSResult res;
	
	ips->patch = NULL;
	
	ips->rom     = NULL;
	ips->romSize = 0;
		
	ips->record.offset = 0;
	ips->record.size   = 0;
	
	res = IPSOpenPatch(ips, patchName);
	if(res == IPS_OK)
	{
		res = IPSOpenInOUT(ips, romName);
	}
	
	if(res != IPS_OK)
	{
		IPSClose(ips);
	}
	
	return res;
}

/*
 * Close patch and rom files 
 */
void IPSClose(struct IPSPatch *ips)
{
	if(ips->rom!= NULL)
	{
		fclose(ips->rom);
		ips->rom = NULL;
	}
	
	if(ips->patch != NULL)
	{
		fclose(ips->patch);
		ips->patch = NULL;
	}
}

/*
 * Read IPS record from file
 */
IPSResult IPSReadRecord(struct IPSPatch *ips)
{
	uint8_t buffer[5];
	size_t  nRead;
	
	nRead = fread(buffer, 1, 3, ips->patch);
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
	ips->record.offset = (buffer[0] << 16) | 
	                     (buffer[1] <<  8) |
	                     (buffer[2]      );
	
	/* Data size */
	nRead = fread(buffer, 1, 2, ips->patch);
	if(nRead < 2)
	{
		return IPS_ERROR_READ;
	}
	ips->record.size = (buffer[0] << 8) | 
	                   (buffer[1]     );

	/* We have a RLE section if data size is zero */
	if(ips->record.size == 0)
	{
		ips->record.offset |= IPS_RECORD_RLE << 24;
		
		/* Read RLE size */
		nRead = fread(buffer, 1, 2, ips->patch);
		if(nRead < 2)
		{
			return IPS_ERROR_READ;
		}
		ips->record.size = (buffer[0] << 8) | 
		                   (buffer[1]     );
		
		/* Read byte data to copy */
		nRead = fread(&(ips->record.rleData), 1, 1, ips->patch);
		if(nRead < 1)
		{
			return IPS_ERROR_READ;
		}
	}
	
	return IPS_OK;
}

/*
 * Process current record
 */
IPSResult IPSProcessRecord (struct IPSPatch *ips)
{
	uint32_t i;
	uint8_t  byte;
	uint32_t offset;

	offset = IPS_RECORD_OFFSET(ips->record);
	if(offset > ips->romSize)
	{
		return IPS_ERROR_OFFSET;
	}

	fseek(ips->rom, offset, SEEK_SET);
	
	if(IPS_RECORD_INFO(ips->record) == IPS_RECORD_RLE)
	{
		for(i=0; i<ips->record.size; ++i)
		{
			fwrite(&(ips->record.rleData), 1, 1, ips->rom);
		}
	}
	else
	{	
		for(i=0; i<ips->record.size; ++i)
		{
			fread (&byte, 1, 1, ips->patch);
			fwrite(&byte, 1, 1, ips->rom);
		}
	}

	/* Update rom size if needed */
	if((offset + ips->record.size) > ips->romSize)
	{
		ips->romSize += ips->record.size;
	}
	
	return IPS_OK;
}

/*
 * Open the file and write IPS header in it.
 */
IPSResult IPSWriteBegin( FILE** out, char* filename )
{
	const char buffer[5] = { 'P', 'A', 'T', 'C', 'H' };
	size_t nWritten;
	
	*out = fopen( filename, "wb" );
	if( *out == NULL )
	{
		return IPS_ERROR_OPEN;
	}
	
	fseek( *out, 0, SEEK_SET );
	
	nWritten = fwrite( buffer, 1, 5, *out );
	if( nWritten != 5 )
	{
		return IPS_ERROR_WRITE;
	}
	
	return IPS_OK;
}

/*
 * Append a new record to IPS file
 */
IPSResult IPSWriteRecord ( FILE* out, uint32_t offset, uint16_t size, uint8_t *data )
{
	uint8_t buffer[5];
	size_t  nWritten;
	
	if( out == NULL )
	{
		return IPS_ERROR;
	}
	
	/* serialize offset */
	buffer[0] = (offset >> 16) & 0xff;
	buffer[1] = (offset >>  8) & 0xff;
	buffer[2] = (offset      ) & 0xff;
	
	/* ... and data size */
	buffer[3] = (size >> 8);
	buffer[4] = (size     ) & 0xff;
	
	nWritten = fwrite( buffer, 1, 5, out );
	if( nWritten != 5 )
	{
		return IPS_ERROR_WRITE;
	}

	/* write data */
	nWritten = fwrite( data, 1, size, out );
	if( nWritten != size )
	{
		return IPS_ERROR_WRITE;
	}
	
	return IPS_OK;
}

/*
 * Write IPS footer and close file.
 */
IPSResult IPSWriteEnd( FILE** out )
{
	const char endStr[3] = { 'E', 'O', 'F' };
	size_t nWritten;
	
	if( *out == NULL )
	{
		fclose( *out );
		*out = NULL;
		return IPS_ERROR;
	}
	
	nWritten = fwrite( endStr, 1, 3, *out );
	if( nWritten != 3 )
	{
		return IPS_ERROR_WRITE;
	}
	
	fclose( *out );
	*out = NULL;
	
	return IPS_OK;
}
