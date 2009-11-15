#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "contrib/ips.h"

#include "callbacks.h"
#include "interface.h"
#include "support.h"

/*
 * Apply patch to the rom
 */
void *
process_patch (void *user_data)
{
	struct IPSPatch *patch;

	int8_t err;
	
	GtkWidget      *doneDialog;
	GtkWidget      *errorDialog;
	GtkWidget      *errorMessageView;
	GtkTextBuffer  *errorMessage;
	GtkFileChooser *fileChooser;
	
	char *romFileName, *patchFileName;
	
	gdk_threads_enter();
	
	err = IPS_ERROR;
	
	/* Get patch file name from file chooser widget */
	fileChooser = GTK_FILE_CHOOSER(g_object_get_data( G_OBJECT(user_data), "patchfilechooserbutton"));
	patchFileName = gtk_file_chooser_get_filename (fileChooser);

	/* Get rom file name from file chooser widget */
	fileChooser = GTK_FILE_CHOOSER(g_object_get_data( G_OBJECT(user_data), "romfilechooserbutton"));
	romFileName = gtk_file_chooser_get_filename (fileChooser);
	
	/* Retrieve patch structure */
	patch = (struct IPSPatch*)g_object_get_data( G_OBJECT(user_data) , "patch" );

	if(patch != NULL)
	{
		if((romFileName != NULL) && (patchFileName != NULL))
		{	
			/* Open files and make a backup of the rom */
			err = IPSOpen(patch,
			              patchFileName,
                          romFileName);

			if(err == IPS_OK)
			{
				/* Process patcj */
				while(err == IPS_OK)
				{
					err = IPSReadRecord(patch);
					if(err == IPS_OK)
					{
						err = IPSProcessRecord (patch);
					}
				}

				IPSClose(patch);

				/* If everything was ok open a greeting dialog */
				if(err == IPS_PATCH_END)
				{
					doneDialog =  create_doneDialog();	
					gtk_widget_show(doneDialog);
					
					gtk_widget_set_sensitive ( GTK_WIDGET(user_data), TRUE );
					gdk_threads_leave ();
					return NULL;
				}
			}
		}
	}
	
process_err:
	/* Error dialog */
	errorDialog = create_errorDialog ();
	errorMessageView = GTK_WIDGET(g_object_get_data( G_OBJECT(errorDialog), "errorMessageView"));
	
	errorMessage = gtk_text_view_get_buffer (GTK_TEXT_VIEW (errorMessageView));
	switch(err)
	{
		case IPS_ERROR_OFFSET:
			gtk_text_buffer_set_text( errorMessage, "Record offset is out of file bound.", -1);
		break;
		
		case IPS_ERROR_READ:
			gtk_text_buffer_set_text( errorMessage, "An unexpected error occured while reading sector.", -1);
		break;
		
		case IPS_ERROR_FILE_TYPE:
			gtk_text_buffer_set_text( errorMessage, "Unsupported patch type", -1);
		break;
		
		case IPS_ERROR_OPEN:
			gtk_text_buffer_set_text( errorMessage, "Unable to open files", -1);
		break;
		
		case IPS_ERROR:
			if(romFileName == NULL)
			{
				gtk_text_buffer_set_text( errorMessage, "Missing rom file", -1);
				if(patchFileName == NULL)
				{
					GtkTextIter iter;
					gtk_text_buffer_get_end_iter( errorMessage, &iter );
					gtk_text_buffer_insert( errorMessage, &iter, "\nMissing patch file", -1);
				}
			}
			else if(patchFileName == NULL)
			{
				gtk_text_buffer_set_text( errorMessage, "Missing patch file", -1);
			}
		break;
		
		default:
			gtk_text_buffer_set_text( errorMessage, "Unexpected error", -1);
		break;
	}

	gtk_text_view_set_buffer ( GTK_TEXT_VIEW (errorMessageView), errorMessage );
	gtk_widget_show (errorDialog);
	gtk_widget_set_sensitive ( GTK_WIDGET(user_data), TRUE );
		
	gdk_threads_leave ();
	
	return NULL;
}

/*
 * Apply button callback
 */
void
on_apply                               (gpointer         user_data,
                                        GtkButton       *button)
{
	pthread_t patchThread;
	struct PatchBlob *blob;
	/* Make the main window insensitive */
	gtk_widget_set_sensitive ( GTK_WIDGET(user_data) , FALSE );
	/* Create and launch the thread which will process the patch */
	pthread_create( &patchThread, NULL, process_patch, user_data);
}
