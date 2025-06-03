#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "Data_Live.h" // Include the new header file

// variable globale C (définie dans main.c)
extern "C" float vitesse;

extern "C" {
    extern Data_Live_t Data_Live; // Declare Data_Live as extern
}

Model::Model() : modelListener(0)
{
}

void Model::tick()
{
    // Rien ici pour le moment — utile si on veut faire des callbacks vers la View
    // via modelListener plus tard
}

float Model::getVitesse()
{
    return Data_Live.vitesse; // Use Data_Live structure to get the updated vitesse
}
