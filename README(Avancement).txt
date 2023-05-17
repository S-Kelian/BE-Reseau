Lors de ce BE nous avons fini la V3, avec une fiabilité partielle. Tout fonctionne correctement, autant pour le texte que pour la vidéo.
Vous trouverez le code correspondant à cette version via le tag "V3" sur notre git (https://github.com/S-Kelian/BE-Reseau/releases/tag/v3).

De plus, nous avons commencé la V4 mais des problèmes persistent, et ne permettent pas d'établir la connexion. Cependant nous estimons qu'il
ne manque pas grand chose pour que celle-ci fonctionne. Vous trouverez le code correspondant dans la branche à jour de notre dépôt.

Pour faire fonctionner notre V3, il suffit d'utiliser les fonctions implémentées dans le dépôt initial, à savoir ./tsock_texte [-p|-s] ou ./tsock_video [-p|-s] [-t mictcp].
Pour choisir le lossrate souhaité, il faut modifier sa valeur dans le fichier mictcp.c à la ligne 27 et recompiler le programme à l'aide de la commande "make".



Explication des choix:
  - Nous avons pris l'initiative de créer la fonction init_PDU durant la création de la V4 afin de simplifier la création des paquets spécifiques à la connexion en créant
  un paquet standard qui n'est alors que peu modifié en fonction de son utilisation (connect/accept/send...)
  - Le mécanisme de fiabilité partielle : Deux variables globales permettent de compter les paquets effectivement envoyés et ceux que l'on a essayé d'envoyer,
  incluant ceux qui ont été perdus dans la couche IP. Dès lors, il suffit de vérifier que le nombre de pertes acceptées n'est alors pas trop élevé.
  Il faut passer l'équation suivante : nbEnvois/nbTentativesEnvoi<(100-acceptedLossRate)/100 que nous avons simplifié en nbEnvois*100<(100-acceptedLossRate)*nbTentativesEnvoi
  afin d'éviter les divisions et ainsi simplifier le calcul pour la machine. En fonction du résultat, on renvoie ou non le paquet perdu. (Ligne 147 de mictcp.c)
  - La valeur de TO (timeout) est de 100, ce qui est élevé mais permet de rendre le terminal plus fluide lors des tentatives de debug.

Le reste de notre travail suit une vision plutôt classique du problème demandé et ne relève donc pas d'une prise d'initiative ou d'un choix à expliquer spécifiquement.

Bonne lecture, Kelian et Théophile.