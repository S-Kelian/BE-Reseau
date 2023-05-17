#include <mictcp.h>
#include <api/mictcp_core.h>
#define TO 100
#define maxReq 10
#define acceptedLossRate 10

mic_tcp_sock globSocket;
mic_tcp_sock_addr addrDest;
unsigned int seq_num_glob=0;
unsigned int ack_num_glob=0;

// Nombres de paquets réellement envoyés
unsigned int nbEnvois=0;
// Nombre de paquets potentiellement envoyés
unsigned int nbTentativesEnvoi=0;


/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(1);
   return result;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   globSocket.addr=addr;
   return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");

    mic_tcp_pdu pdu;
    init_PDU(&pdu);

    // attente de la reception d'un socket SYN
    while (globSocket.state!=SYN_RECEIVED){
        
    }
    printf("SYN recu\n");
    pdu.header.source_port = globSocket.addr.port;
    pdu.header.dest_port = addr->port;
    pdu.header.syn = 1;
    pdu.header.ack = 1;
    if (IP_send(pdu,*addr)==-1){
        return -1;
    } 
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    addrDest=addr;
    printf("Creation des pdu\n");
    mic_tcp_pdu pdu, pduRcv;
    init_PDU(&pdu);
    init_PDU(&pduRcv);
    pdu.header.dest_port=addr.port;
    pdu.header.syn=1;
    pdu.header.ack=0;
    printf("Envoi du pdu syn : %d, ack : %d\n", pdu.header.syn, pdu.header.ack);
    while (globSocket.state!=SYN_SENT){
        if (IP_send(pdu,addrDest)!=-1){
            globSocket.state=SYN_SENT;
        }
    }
    printf("SYN envoye\n");
    for(int nbConReq=1; nbConReq<maxReq; nbConReq++) {   
        printf("Attente du SYNACK\n");   
        if( IP_recv(&pduRcv, &addr, TO)!=-1){
            printf("pdu recu\n");
            if (pduRcv.header.ack==1 && pduRcv.header.syn==1){
                printf("SYNACK recu\n");
                pdu.header.syn=0;
                pdu.header.ack=1;
                if (IP_send(pdu,addrDest)==-1){
                    printf("Erreur envoi ACK\n");
                    return -1;
                }
                printf("ACK envoye\n");
                globSocket.state=ESTABLISHED;
                return 0;
            }
        } else  {
            printf("Erreur reception paquet\n");
            if (IP_send(pdu,addrDest)==-1){
                return -1;
            }
        }
    }
    printf("Nombre de requetes de connexion max atteint\n");
    return -1;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    int sentSize=-1;
    mic_tcp_pdu pdu, pduRCV;
    init_PDU(&pdu);
    init_PDU(&pduRCV);
    // pdu.header.source_port=globSocket.addr.port;
    // pdu.header.dest_port=addrDest.port;
    // pdu.header.seq_num=seq_num_glob;
    // pdu.header.ack_num=0;
    // pdu.header.ack=0;
    // pdu.header.syn=0;
    // pdu.header.fin=0;
    // la fonction init_PDU() initialise les champs à la bonne valeur pour l'envoi
    pdu.payload.data=mesg;
    pdu.payload.size=mesg_size;
    seq_num_glob = (seq_num_glob+1)%2;
    sentSize=IP_send(pdu,addrDest);
    nbTentativesEnvoi++;
    int success =-1;
    //attente ack
    while (success ==-1){
        if ((IP_recv(&pduRCV, &addrDest, 1)==-1) || (seq_num_glob != pduRCV.header.ack_num) || (pduRCV.header.ack != 1)){
            printf("%d < %d",nbEnvois*100, (100-acceptedLossRate)*nbTentativesEnvoi);
            if (nbEnvois*100<(100-acceptedLossRate)*nbTentativesEnvoi){
                printf("Erreur non acceptée -> paquet renvoyé\n");
                sentSize=IP_send(pdu,addrDest);
            } else {
                printf("Erreur acceptée -> paquet suivant\n");
                success = 0;
            }          
        } else {
            success = 0;
            nbEnvois++;
        }
    }    
    return sentSize ;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    int deliveredSize =-1;
    mic_tcp_payload pld;
    pld.data=mesg;
    pld.size=max_mesg_size;
    deliveredSize= app_buffer_get(pld);
    return deliveredSize;
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_pdu pduAck;
    init_PDU(&pduAck);
    if (pdu.header.seq_num == ack_num_glob){
        ack_num_glob = (ack_num_glob+1)%2;
        app_buffer_put(pdu.payload);
    }
    // si le paquet recu n'est pas une demande de connexion
    if (pdu.header.syn!=1 && pdu.header.ack==0){
        pduAck.header.ack=1;
        pduAck.header.ack_num=ack_num_glob;
        IP_send(pduAck,addr);
    }
}

void init_PDU(mic_tcp_pdu * pdu){
    pdu->header.source_port=globSocket.addr.port;
    pdu->header.dest_port=addrDest.port;
    pdu->header.seq_num=seq_num_glob;
    pdu->header.ack_num=ack_num_glob;
    pdu->header.ack=0;
    pdu->header.syn=0;
    pdu->header.fin=0;
    pdu->payload.data=NULL;
    pdu->payload.size=0;
}
