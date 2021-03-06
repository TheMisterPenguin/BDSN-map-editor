#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

static SDL_Window *sdl_window;
static SDL_Renderer *sdl_renderer;
static SDL_Surface *sdl_surface;
static GtkWindow *gtk_window;
static GtkWidget *gtk_da;
static void *gdk_window;
static void *window_id;

void erreur(const char* message){
    GtkWidget *fenDialogue = NULL;

    fenDialogue = gtk_dialog_new_with_buttons("Erreur",
                                              gtk_window,
                                              GTK_DIALOG_MODAL,
                                              "OK",
                                              GTK_RESPONSE_OK,
                                              NULL);

    GtkWidget *contenu = gtk_dialog_get_content_area(GTK_DIALOG(fenDialogue));
    GtkWidget *texte = gtk_label_new(message);

    gtk_widget_set_size_request(fenDialogue, 200, 150);

    g_signal_connect_swapped (fenDialogue,
                          "response",
                          G_CALLBACK (gtk_widget_destroy),
                          fenDialogue);

    gtk_container_add(GTK_CONTAINER(contenu), texte);

    gtk_widget_show_all(fenDialogue);

}

FILE *ouvrir_fichier(GtkApplication *app, gpointer fenetre){
    GtkWidget *fenDialogue = NULL;
    gint res;

    fenDialogue = gtk_file_chooser_dialog_new("Ouvrir une map",
                                              fenetre,
                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                              "Annuler",
                                              GTK_RESPONSE_CANCEL,
                                              "Ouvrir",
                                              GTK_RESPONSE_ACCEPT,
                                              NULL);

    res = gtk_dialog_run(GTK_DIALOG(fenDialogue));
    if(res == GTK_RESPONSE_ACCEPT){
        char *nom_fichier = NULL;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(fenDialogue);
        nom_fichier = gtk_file_chooser_get_filename(chooser);
        FILE *f = fopen(nom_fichier, "w");

    switch(errno){
        case 0 : break;
        case EACCES : erreur("Impossible d'ouvrir le fichier : acc??s refus?? !"); break;
        case ENFILE :
        case EMFILE : erreur("Impossible d'ouvrir le fichier : le nombre maximal de fichier ouvert ?? ??t?? atteint !"); break; 
        case EROFS : erreur("Impossible d'ouvrir le fichier : le fichier est en lecture seule !"); break;
        default : erreur("Impossible d'ouvrir le fichier : erreur inconnue !");
    }
    fclose(f);
    g_free(nom_fichier);
    }
    gtk_widget_destroy(fenDialogue);
    return NULL;
}

static gboolean idle(void *ud) {
    (void)ud;
    if (!sdl_window) {
        printf("creating SDL window for window id %p\n", window_id);
        sdl_window = SDL_CreateWindowFrom(window_id);
        printf("sdl_window=%p\n", sdl_window);
        if (!sdl_window) {
            printf("%s\n", SDL_GetError());
            }
        sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
        printf("sdl_renderer=%p\n", sdl_renderer);
        if (!sdl_renderer) {
            printf("%s\n", SDL_GetError());
            }
        }
    else {
        SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
        SDL_RenderClear(sdl_renderer);
        SDL_RenderPresent(sdl_renderer);
        }
    return true;

    }



GtkWidget *create_menu_bar(GtkAccelGroup *accel_group) {
    GtkWidget *barre = gtk_box_new(FALSE, 0);

    GtkWidget *menubar = gtk_menu_bar_new();

    /* Cr??ation des cat??gories de menu */
    GtkWidget *menuFichier = gtk_menu_new();
    GtkWidget *menuEdition = gtk_menu_new();

    /*Cr??ation des boutons de menu */
    GtkWidget *fichierMi = gtk_menu_item_new_with_label("Fichier");
    GtkWidget *ouvrirMi = gtk_menu_item_new_with_label("Ouvrir une map");
    GtkWidget *quitterMi = gtk_menu_item_new_with_label("Quitter");

    GtkWidget *edition = gtk_menu_item_new_with_label("??dition");
    GtkWidget *undo = gtk_menu_item_new_with_label("Annuler");
    GtkWidget *redo = gtk_menu_item_new_with_label("R??tablir");

    GtkWidget *sep = gtk_separator_menu_item_new();

    /* Liaison des parents */
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fichierMi), menuFichier);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edition), menuEdition);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fichierMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edition);

    gtk_menu_shell_append(GTK_MENU_SHELL(menuFichier), ouvrirMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menuFichier), sep);
    gtk_menu_shell_append(GTK_MENU_SHELL(menuFichier), quitterMi);

    gtk_menu_shell_append(GTK_MENU_SHELL(menuEdition), undo);
    gtk_menu_shell_append(GTK_MENU_SHELL(menuEdition), redo);

    /* Ajout des raccourcis claviers */
    gtk_menu_set_accel_group(GTK_MENU(menuFichier), accel_group);

    gtk_widget_add_accelerator(ouvrirMi, "activate", accel_group, GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(quitterMi, "activate", accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    gtk_widget_add_accelerator(undo, "activate", accel_group, GDK_KEY_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(redo, "activate", accel_group, GDK_KEY_y, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    /* Ajout des fonction ex??cut??s lors de l'activation de boutons */
    g_signal_connect(G_OBJECT(quitterMi), "activate", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(ouvrirMi), "activate", G_CALLBACK(ouvrir_fichier), gtk_window);

    gtk_box_pack_start(GTK_BOX(barre), menubar, FALSE, FALSE, 0);

    return (barre);
}

int main(int argc, char **argv) {
    GtkWidget *menubar = NULL;
    GtkWidget *menuFichier = NULL;
    GtkWidget *fichierMi = NULL;
    GtkWidget *ouvrirMi = NULL;
    GtkWidget *quitterMi = NULL;

    GtkWidget *drawingArea = NULL;
    GtkAccelGroup *accel_group = NULL;

    /* On initialise les librairies */
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Impossible de d??mmarer le programme : %s\n", SDL_GetError());
        exit(1);
    }

    gtk_init(&argc, &argv);

    /* On cr??er la fen??tre */
    gtk_window = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(gtk_window, 300, 200);
    gtk_window_set_title(gtk_window, "Bloody Sanada Map Editor");

    /* On cr??er le groupe accel pour les racourcis claviers */
    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(gtk_window, accel_group);

    /* On cr??e la boite principale */
    GtkWidget *hbox = gtk_box_new(TRUE, 0);
    gtk_container_add(GTK_CONTAINER(gtk_window), hbox);

    /* On cr??er la barre de navigation */
    gtk_box_pack_start(GTK_BOX(hbox), create_menu_bar(accel_group), FALSE, FALSE, 0);

    /* On cr??er la zone de dessin (SDL) */
    drawingArea = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(hbox), drawingArea, TRUE, TRUE, 0);

    /* On affiche tout les widgets */
    gtk_widget_show_all(GTK_WIDGET(gtk_window));

    /* On r??cup??re la fen??tre de dessin pour ??crire dessus avec la SDL */
    gdk_window = gtk_widget_get_window(GTK_WIDGET(drawingArea));
    window_id = (void *)(intptr_t)GDK_WINDOW_XID(gdk_window);

    g_idle_add(&idle, 0);

    gtk_main();
    return 0;
    }