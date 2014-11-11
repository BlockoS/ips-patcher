#include <gtk/gtk.h>
#include "log.h"

struct AppConfig
{
    GtkFileChooser *inputFileChooser;
    GtkFileChooser *patchFileChooser;
    GtkFileChooser *outputFileChooser;
    GtkLabel       *outputFileLabel;
};

static void onApplyButtonClicked (GtkWidget* button, gpointer userData)
{
    AppConfig *app = (AppConfig*)userData;
    
    gchar* inputFilename  = gtk_file_chooser_get_filename (app->inputFileChooser);
    gchar* outputFilename = gtk_file_chooser_get_filename (app->outputFileChooser);
    gchar* patchFilename  = gtk_file_chooser_get_filename (app->patchFileChooser);
    
    g_print("input: %s\npatch: %s\noutput: %s\n", inputFilename, patchFilename, outputFilename);
    // [todo] read patch, apply patch
}

static void onOutputButtonClicked (GtkWidget* button, gpointer userData)
{
    AppConfig *app = (AppConfig*)userData;
    
    if (gtk_dialog_run (GTK_DIALOG(app->outputFileChooser)) == GTK_RESPONSE_ACCEPT)
    {
        gtk_label_set_text(app->outputFileLabel, gtk_file_chooser_get_filename (app->outputFileChooser));
    }
    gtk_widget_hide(GTK_WIDGET(app->outputFileChooser));
}

static void onQuitButtonClicked (GtkWidget* button, gpointer userData)
{
    // [todo]
    gtk_main_quit();
}

int main(int argc, char **argv)
{
    GtkBuilder *builder;
    GtkWidget  *window;
    GError     *error = NULL;

    AppConfig appConfig;

    gtk_init( &argc, &argv );

    builder = gtk_builder_new();
    if( ! gtk_builder_add_from_file( builder, "resources/ips-patcher.glade", &error ) )
    {
        Error("%s", error->message);
        g_free( error );
        return( 1 );
    }

    window     = GTK_WIDGET( gtk_builder_get_object( builder, "IPS Patcher" ) );
    
    GObject *quitButton = gtk_builder_get_object (builder, "quitButton");
    g_signal_connect (quitButton, "clicked", G_CALLBACK (onQuitButtonClicked), &appConfig);

    GObject *applyButton = gtk_builder_get_object (builder, "applyButton");
    g_signal_connect (applyButton, "clicked", G_CALLBACK (onApplyButtonClicked), &appConfig);

    GObject *outputFileButton = gtk_builder_get_object (builder, "outputFileButton");
    g_signal_connect (outputFileButton, "clicked", G_CALLBACK (onOutputButtonClicked), &appConfig);

    appConfig.inputFileChooser  = GTK_FILE_CHOOSER( gtk_builder_get_object (builder, "inputFileChooser" ) );
    appConfig.patchFileChooser  = GTK_FILE_CHOOSER( gtk_builder_get_object (builder, "patchFileChooser" ) );
    appConfig.outputFileLabel   = GTK_LABEL       ( gtk_builder_get_object (builder, "outputFileLabel" ) );
    
    appConfig.outputFileChooser = GTK_FILE_CHOOSER( gtk_file_chooser_dialog_new ("Save File",
                                      GTK_WINDOW(window),
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                      NULL ) );
    gtk_file_chooser_set_do_overwrite_confirmation (appConfig.outputFileChooser, TRUE);
    
    gtk_builder_connect_signals( builder, NULL );

    gtk_widget_show( window );

    gtk_main();

    return 0;
}
