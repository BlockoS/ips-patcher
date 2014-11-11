#include <cstring>
#include <gtk/gtk.h>
#include "log.h"
#include "ips.h"
#include "io.h"
#include "utils.h"

#include "gui.inl"

class GUILogOutput : public Log::Output
{
    public:
        GUILogOutput();
        virtual ~GUILogOutput();
        virtual void out(Log::Type type, const char* format, va_list args);
        void attach(GtkScrolledWindow *scrolledWindow, GtkBox *box);
    private:
        GtkBox *_box;
        GtkScrolledWindow *_scrolledWindow;
};

GUILogOutput::GUILogOutput()
    : Log::Output()
    , _box(nullptr)
    , _scrolledWindow(nullptr)
{}
GUILogOutput::~GUILogOutput()
{}
void GUILogOutput::attach(GtkScrolledWindow *scrolledWindow, GtkBox *box)
{
    _scrolledWindow = scrolledWindow;
    _box = box;
}
void GUILogOutput::out(Log::Type type, const char* format, va_list args)
{
    if(nullptr == _box)
    {
        return;
    }
    
    GtkWidget *entry;
    char const* iconName = "gtk-apply";
    
    switch(type)
    {
        case Log::Type::Error:
            iconName = "gtk-dialog-error";
            break;
        case Log::Type::Warning:
            iconName = "gtk-dialog-warning";
            break;
        case Log::Type::Info:
            iconName = "gtk-dialog-info";
            break;
    }
    
    char *buffer = (char*)malloc(256);
    vsnprintf(buffer, 256, format, args);
    
    GtkWidget *image = gtk_image_new_from_stock(iconName, GTK_ICON_SIZE_MENU);
    GtkWidget *label = gtk_label_new ( buffer );
    
    entry = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_widget_set_visible( entry, TRUE );
    gtk_widget_set_can_focus( entry, FALSE );

    gtk_box_pack_start ( GTK_BOX( entry ), image, FALSE, TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX( entry ), label, FALSE, TRUE, 2 );
                         
    gtk_widget_show(image);
    gtk_widget_show(label);
    gtk_widget_show(entry);
    
    gtk_box_pack_end ( _box, entry, FALSE, TRUE, 0 );
    
    if(nullptr != _scrolledWindow)
    {
        gtk_scrolled_window_set_placement( _scrolledWindow,  GTK_CORNER_BOTTOM_LEFT );
    }
}

struct AppConfig
{
    GtkFileChooser *inputFileChooser;
    GtkFileChooser *patchFileChooser;
    GtkBox         *outputFileBox;
    GtkFileChooser *outputFileChooser;
    GtkEntry       *outputFileEntry;
};

static void onApplyButtonClicked (GtkWidget*, gpointer userData)
{
    AppConfig *app = (AppConfig*)userData;
    bool ok = true;
    
    gchar* inputFilename  = gtk_file_chooser_get_filename (app->inputFileChooser);
    gchar* patchFilename  = gtk_file_chooser_get_filename (app->patchFileChooser);
    const gchar* outputFilename = gtk_entry_get_text (app->outputFileEntry);
    
    if(nullptr == inputFilename)
    {
        Error("Missing input rom filename.");
        ok = false;
    }
    if((nullptr == outputFilename) || (0 == strlen(outputFilename)))
    {
        Error("Missing output rom filename.");
        ok = false;
    }
    if(nullptr == patchFilename)
    {
        Error("Missing IPS patch filename.");
        ok = false;
    }
    if(false == ok)
    {
        return;
    }
    
    IPS::Patch  patch;
    IPS::IO     io;
    bool ret;
    
    ret = io.read(patchFilename, patch);
    if(true == ret)
    {
        IPS::apply(inputFilename, outputFilename, patch, true);
    }
    
    g_free(patchFilename);
    g_free(inputFilename);
    
    return;
}

static void onOutputButtonClicked (GtkWidget*, gpointer userData)
{
    AppConfig *app = (AppConfig*)userData;
    
    if (gtk_dialog_run (GTK_DIALOG(app->outputFileChooser)) == GTK_RESPONSE_ACCEPT)
     {
        gtk_entry_set_text(app->outputFileEntry, gtk_file_chooser_get_filename (app->outputFileChooser) );
    }
    gtk_widget_hide(GTK_WIDGET(app->outputFileChooser));
}

int main(int argc, char **argv)
{
    gtk_init ( &argc, &argv );
   
    GtkBuilder *builder;
    GtkWidget  *window;
    GError     *error = NULL;

    AppConfig appConfig;

    gtk_init( &argc, &argv );

    builder = gtk_builder_new();
    if( ! gtk_builder_add_from_string( builder, g_resources, strlen(g_resources), &error) )
    {
        Error("%s", error->message);
        g_free( error );
        return( 1 );
    }

    window = GTK_WIDGET( gtk_builder_get_object( builder, "IPS Patcher" ) );
    g_signal_connect ( GTK_WIDGET(window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
    
    GObject *quitButton = gtk_builder_get_object (builder, "quitButton");
    g_signal_connect (quitButton, "clicked", G_CALLBACK (gtk_main_quit), &appConfig);

    GObject *applyButton = gtk_builder_get_object (builder, "applyButton");
    g_signal_connect (applyButton, "clicked", G_CALLBACK (onApplyButtonClicked), &appConfig);

    GObject *outputFileButton = gtk_builder_get_object (builder, "outputFileButton");
    g_signal_connect (outputFileButton, "clicked", G_CALLBACK (onOutputButtonClicked), &appConfig);

    appConfig.inputFileChooser  = GTK_FILE_CHOOSER( gtk_builder_get_object (builder, "inputFileChooser" ) );
    appConfig.patchFileChooser  = GTK_FILE_CHOOSER( gtk_builder_get_object (builder, "patchFileChooser" ) );
    
    appConfig.outputFileBox     = GTK_BOX   ( gtk_builder_get_object (builder, "outputFileBox" ) );
    appConfig.outputFileEntry   = GTK_ENTRY ( gtk_builder_get_object (builder, "outputFileEntry" ) );
    
    appConfig.outputFileChooser = GTK_FILE_CHOOSER( gtk_file_chooser_dialog_new ("Save File",
                                      nullptr,
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                      NULL ) );
    gtk_file_chooser_set_do_overwrite_confirmation (appConfig.outputFileChooser, TRUE);
    
    GtkWidget *scrolledWindow = GTK_WIDGET( gtk_builder_get_object (builder, "logWindow") );
    GtkWidget *logBox = GTK_WIDGET( gtk_builder_get_object (builder, "logBox") );

    Log::Logger& logger = Log::Logger::instance();
    GUILogOutput  output;
    output.attach ( GTK_SCROLLED_WINDOW(scrolledWindow), GTK_BOX( logBox ) );
    logger.begin ( &output );

    gtk_builder_connect_signals( builder, NULL );

    gtk_widget_show( window );

    gtk_main();

    logger.end();
    return 0;
}
