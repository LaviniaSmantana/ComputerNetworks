#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 2024
#define MAX_BUFFER_SIZE 5000

extern int errno;	

/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));	
  /* portul utilizat de client */
  bzero (port, 7);
  sprintf (port, ":%d", ntohs (address.sin_port));	
  strcat (str, port);
  return (str);
}


int conectare(int client)
{
	char username[50];
	char parola[50];
	char membri[10] = "conturi";  // membri comunitatii
	char linie[100];
	FILE *fp;
	char cont_gasit[2];
	char status[50] = "";  // statusul logarii

	if((fp = fopen (membri, "r")) == NULL)
	{
		perror("Eroare la deschiderea fisierului de conturi.\n");
		return errno;
	}

	memset(username, '\0', 50);
	memset(parola, '\0', 50);

	/* citim username-ul si parola */
	if(read(client, username, 50) < 0)
	{
		perror("[server] Eroare la citirea username-ului.\n");
		return 0;
	}
	username[strlen(username)-1] = '\0';

	if(read(client, parola, 50) < 0)
	{
		perror("[server] Eroare la citirea parolei.\n");
		return 0;
	}
	parola[strlen(parola)] = '\0';

	// punem parola langa username, de forma: "nume parola" si cautam daca exista contul in fisier
	strcat(username, " ");
	strcat(username, parola);

	
	//Verificam daca exista contul si parola
	while(fgets(linie, 100, fp) != NULL)
	{
		if(strstr(linie, username) != NULL)
		{
			cont_gasit[0] = '1';
			break;
		}
	}

	if(cont_gasit[0] == '1')
	{
		printf("[server] Client logat cu succces! \n");
		if(send(client, cont_gasit, sizeof(cont_gasit), 0) < 0){
			perror("[server] Eroare trimitere raspuns.\n");
			return 0;
		}
		return 1;
	}
	else
	{
		printf("[server] Username-ul sau parola au fost introduse gresit, sau contul nu se afla in fisier.\n");
		if(send(client, cont_gasit, sizeof(cont_gasit), 0) < 0){
			perror("[server] Eroare trimitere raspuns.\n");
			return 0;
		}
		return 0;
	}
}

void displayMenu(int client) {
    char menu[] = "\nMeniu:\n1. Afiseaza anunturile\n2. Adauga anunt\n3. Iesire\n";
    send(client, menu, sizeof(menu), 0);
}

int trimite_anunturi(int clientSocket) {

	char c = '\0';
    FILE *file = fopen("anunturi.txt", "a+");
    if (file == NULL) {
        perror("Eroare la deschiderea fisierului");
        return 0;
    }

	char buffer[MAX_BUFFER_SIZE];
    	size_t bytesRead;
	
	memset(buffer, '\0', sizeof(buffer));
    	
    	c = fgetc(file);
    	while(c != EOF){
    		strncat(buffer, &c, 1);
    		c = fgetc(file);
    	}
    	
    	if (send(clientSocket, buffer, sizeof(buffer), 0) == -1) {
            		perror("[server] Eroare la trimiterea anunturilor.\n");
            		return 0;
        	}
    		

    if (fclose(file) == EOF) {
        perror("Eroare la inchiderea fisierului");
        return 0;
    }
    
    return 1;
}

int adauga_anunt(int client) {

    FILE *file;
    char title[20];
    char details[100];
    
    if ((file = fopen("anunturi.txt", "a+")) == NULL) {
        perror("Eroare la deschiderea fisierului de anunturi");
        return 0;
    }

	memset(title, '\0', sizeof(title));
	memset(details, '\0', sizeof(details));

	/* citim titlul si detaliile anuntului primite de la client */
	if(read(client, title, sizeof(title)) < 0)
	{
		perror("[server] Eroare la citirea titlului.\n");
		return 0;
	}
	title[strlen(title)-1] = '\0';

	if(read(client, details, sizeof(details)) < 0)
	{
		perror("[server] Eroare la citirea detaliilor.\n");
		return 0;
	}
	details[strlen(details)] = '\0';
    	
    	fprintf(file, "\nANUNT: %s\nDETALII: %s", title, details);
    

   if (fclose(file) == EOF) {
        perror("Eroare la inchiderea fisierului");
        return 0;
    }
    
    return 1;
}



int main ()
{

  struct sockaddr_in server;	/* structurile pentru server si clienti */
  struct sockaddr_in from;
  fd_set readfds;		/* multimea descriptorilor de citire */
  fd_set actfds;		/* multimea descriptorilor activi */
  struct timeval tv;		/* structura de timp pentru select() */
  int sd, client;		/* descriptori de socket */
  int optval=1; 		/* optiune folosita pentru setsockopt()*/ 
  int fd;			/* descriptor folosit pentru 
				   parcurgerea listelor de descriptori */
  int nfds;			/* numarul maxim de descriptori */
  int len;			/* lungimea structurii sockaddr_in */

  /* creare socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server] Eroare la socket().\n");
      return errno;
    }

  /*setam pentru socket optiunea SO_REUSEADDR */ 
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval));

  /* pregatim structurile de date */
  bzero (&server, sizeof (server));

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (PORT);

  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server] Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1)
    {
      perror ("[server] Eroare la listen().\n");
      return errno;
    }
  
  /* completam multimea de descriptori de citire */
  FD_ZERO (&actfds);		/* initial, multimea este vida */
  FD_SET (sd, &actfds);		/* includem in multime socketul creat */

  tv.tv_sec = 1;		/* se va astepta un timp de 1 sec. */
  tv.tv_usec = 0;
  
  /* valoarea maxima a descriptorilor folositi */
  nfds = sd;

  printf ("[server] Asteptam la portul %d...\n", PORT);
  fflush (stdout);
        
  /* servim in mod concurent clientii... */
  while (1)
    {
      /* ajustam multimea descriptorilor activi (efectiv utilizati) */
      bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

      /* apelul select() */
      if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
	{
	  perror ("[server] Eroare la select().\n");
	  return errno;
	}
      /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
      if (FD_ISSET (sd, &readfds))
	{
	  /* pregatirea structurii client */
	  len = sizeof (from);
	  bzero (&from, sizeof (from));

	  /* a venit un client, acceptam conexiunea */
	  client = accept (sd, (struct sockaddr *) &from, &len);

	  /* eroare la acceptarea conexiunii de la un client */
	  if (client < 0)
	    {
	      perror ("[server] Eroare la accept().\n");
	      continue;
	    }

          if (nfds < client) /* ajusteaza valoarea maximului */
            nfds = client;
            
	  /* includem in lista de descriptori activi si acest socket */
	  FD_SET (client, &actfds);

	  printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
	  //fflush (stdout);
	  
	  if(conectare(client) == 0)
		{
			close(client);
			FD_CLR(client, &actfds);
			//return 0;
		}
	}
      /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
  
      for (fd = 0; fd <= nfds; fd++)	/* parcurgem multimea de descriptori */
	{
	  /* este un socket de citire pregatit? */
	  if (fd != sd && FD_ISSET (fd, &readfds))
	    {		
		/*if(gestioneaza_meniu(fd) == 0){
			printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
			close(fd);
			FD_CLR(fd, &actfds);
		}*/
		char option[2];
		memset(option, '\0', 2);
		
		if(recv(fd, option, sizeof(option), 0) < 0){
        		perror("[server] Eroare la primirea optiunii de la client.");
        	}

        	if(option[0] == '1'){
               	if(trimite_anunturi(fd) == 0){
               		perror("[server] Eroare la trimite_anunturi.\n");
               	}
            	}
           	 else if(option[0] == '2') {
                	if(adauga_anunt(fd) == 0){
                		perror("[server] Eroare la adauga_anunt.\n");
              		 }
            	}
            	else if (option[0] == '3') {
                	printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
			close(fd);
			FD_CLR(fd, &actfds);
                }
            else{
                send(fd, "Comanda invalida. Incercati din nou.", sizeof("Comanda invalida. Incercati din nou."), 0);
        	}
		
		
	    }
	}			/* for */
    }				/* while */
}				/* main */




