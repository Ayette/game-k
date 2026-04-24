#include "option.h"
#include "puzzle.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    Option o;
    puzel p;
    int etat;
    int ancien_etat;

    srand(time(NULL));

    if (!initialiser_option(&o))
    {
        printf("Erreur initialisation option\n");
        return 1;
    }

    if (!initialiser_puzzle(&p, o.renderer))
    {
        printf("Erreur initialisation puzzle\n");
        nettoyer_option(&o);
        return 1;
    }

    etat = 0;
    ancien_etat = -1;

    while (o.running && p.running)
    {
        if (etat != ancien_etat)
        {
            if (etat == 2)
            {
                generer_puzzle(&p, o.renderer);
            }
            ancien_etat = etat;
        }

        if (etat == 0)
        {
            gerer_evenements_option(&o, &etat);
            afficher_option(&o);
        }
        else if (etat == 2)
        {
            gerer_evenements_puzzle(&p, o.renderer, &etat);
            afficher_puzzle(&p, o.renderer);
        }
    }

    nettoyer_puzzle(&p);
    nettoyer_option(&o);

    return 0;
}
