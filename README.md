# SoftWare projet Kartrack

Ressources
	FatFS – Système de fichiers pour cartes SD :
		http://elm-chan.org/fsw/ff/00index_e.html



Structure de la carte SD
	/settings
    	Contient les paramètres de l'interface utilisateur (UI).

	/tracks
    	/tracks/images
        	Idée : fichiers JPG des circuits, affichés en direct avec les secteurs mis en surbrillance à l’écran.
        
    	/tracks/gate
        	Contient les positions GPS de chaque gate (point de passage) pour le calcul des temps.

	/records
    	Contient l’historique des sessions de roulage précédentes.

	/logs
    	Journaux système généraux (logs globaux).
